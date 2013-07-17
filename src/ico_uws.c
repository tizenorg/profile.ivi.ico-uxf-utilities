/*
 * Copyright (c) 2013, TOYOTA MOTOR CORPORATION.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 */
/**
 * @brief   library for communicate
 *
 * @date    June-26-2013
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#include <libwebsockets.h>
#include "ico_uws.h"
#include "ico_uws_private.h"

/*===========================================================================*/
/* definition                                                                */
/*===========================================================================*/
#define URI_WS          "ws://"
#define URI_WS_SECURE   "wss://"

/* buffer of reserved data that failed to send */
#define DATA_LEN    LWS_SEND_BUFFER_PRE_PADDING + 1024 + \
                    LWS_SEND_BUFFER_POST_PADDING

/*===========================================================================*/
/* define static function prototype                                          */
/*===========================================================================*/
static struct ico_uws_connect *add_ws_instance(struct ico_uws_context *context,
                                               struct libwebsocket *wsi);
static void del_ws_instance(struct ico_uws_context *context,
                            const struct libwebsocket *wsi);
static void del_connect_list(struct ico_uws_context *context);
static int add_context(struct ico_uws_context *context);
static struct ico_uws_context *get_context_by_wsctx(
                               struct libwebsocket_context *ws_context);
static struct ico_uws_context *get_context_by_wsi(struct libwebsocket *wsi);
static void del_context(struct ico_uws_context *context);
static void del_context_list(void);
static int exec_callback(const struct ico_uws_context *context,
                         const ico_uws_evt_e event,
                         const void *id,
                         const ico_uws_detail *detail,
                         const void *user_data);
static int server_http_callback(struct libwebsocket_context *ws_context,
                                struct libwebsocket *wsi,
                                enum libwebsocket_callback_reasons reason,
                                void *user, void *in, size_t len);
static int client_http_callback(struct libwebsocket_context *ws_context,
                                struct libwebsocket *wsi,
                                enum libwebsocket_callback_reasons reason,
                                void *user, void *in, size_t len);
static int server_uws_callback(struct libwebsocket_context *ws_context,
                               struct libwebsocket *wsi,
                               enum libwebsocket_callback_reasons reason,
                               void *user, void *in, size_t len);
static int client_uws_callback(struct libwebsocket_context *ws_context,
                               struct libwebsocket *wsi,
                               enum libwebsocket_callback_reasons reason,
                               void *user, void *in, size_t len);
static struct ico_uws_context *create_server(const char *uri,
                                             const char *protocol);
static struct ico_uws_context *create_client(const char *uri,
                                             const char *protocol);

/*===========================================================================*/
/* variable & table                                                          */
/*===========================================================================*/
/* ico_uws_context */
struct context_info_t {
    struct context_info_t   *next;
    struct ico_uws_context  *context;
};
static struct context_info_t *ctx_list_first    = NULL;
static struct context_info_t *ctx_list_last     = NULL;
static struct context_info_t *ctx_list_free     = NULL;

/* pthread mutex initialize */
static pthread_mutex_t in_out_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t creating_mutex = PTHREAD_MUTEX_INITIALIZER;

/* current creating context */
static struct ico_uws_context   *current_creating_context = NULL;

/*===========================================================================*/
/* static function                                                           */
/*===========================================================================*/
/*--------------------------------------------------------------------------*/
/**
 * @brief   add_ws_instance
 *          Add wsi into the ico_uws_connect list.
 *
 * @param[in]   context             ico_uws context
 * @param[in]   wsi                 libwebsocket instance
 * @return      ico_uws_connect pointer
 * @retval      pointer of list     success
 * @retval      NULL                error
 */
/*--------------------------------------------------------------------------*/
static struct ico_uws_connect *
add_ws_instance(struct ico_uws_context *context, struct libwebsocket *wsi)
{
    int id;
    struct ico_uws_connect *con_table;
    ico_uws_detail detail;

    _DBG("add websocket instance called");
    if (context == NULL || wsi == NULL) {
        _WARN("context or wsi is NULL");
        return NULL;
    }

    /* mutex lock */
    pthread_mutex_lock(&in_out_mutex);
    con_table = context->con_list_first;
    /* search free table */
    for (id = 0; id < ICO_UWS_MAX_NUM; id++) {
        if (con_table == NULL) break;
        con_table = con_table->next;
    }

    /* list is full */
    if (id == ICO_UWS_MAX_NUM) {
        /* mutex unlock */
        pthread_mutex_unlock(&in_out_mutex);
        detail._ico_uws_error.code = ICO_UWS_ERR_UNKNOWN;
        _ERR("list is full (%d)", detail._ico_uws_error.code);
        exec_callback(NULL, ICO_UWS_EVT_ERROR, NULL, &detail, NULL);
        return NULL;
    }

    /* create and set wsi */
    if (context->con_list_free == NULL) {
        /* free table does not exist */
        con_table = calloc(1, sizeof(struct ico_uws_connect));
        if (con_table == NULL) {
            /* mutex unlock */
            pthread_mutex_unlock(&in_out_mutex);
            detail._ico_uws_error.code = ICO_UWS_ERR_OUT_OF_MEMORY;
            _ERR("no memory for ico_uws_connect (%d)",
                 detail._ico_uws_error.code);
            exec_callback(NULL, ICO_UWS_EVT_ERROR, NULL, &detail, NULL);
            return NULL;
        }
    }
    else {
        /* use free area */
        con_table = context->con_list_free;
        context->con_list_free = con_table->next;
    }
    /* add wsi */
    con_table->wsi = wsi;
    _DBG("add wsi to the connect info table success");

    /* add table to list */
    con_table->next = NULL;
    if (context->con_list_first == NULL) {
        context->con_list_first = con_table;
    }
    else {
        context->con_list_last->next = con_table;
    }
    context->con_list_last = con_table;
    /* mutex unlock */
    pthread_mutex_unlock(&in_out_mutex);

    return con_table;
}

/*--------------------------------------------------------------------------*/
/**
 * @brief   del_ws_instance
 *          Delete wsi from the ico_uws_connect list.
 *
 * @param[in]   context             ico_uws context
 * @param[in]   wsi                 libwebsocket instance
 * @return      none
 */
/*--------------------------------------------------------------------------*/
static void
del_ws_instance(struct ico_uws_context *context,
                const struct libwebsocket *wsi)
{
    int id;
    struct ico_uws_connect *con_table, *prev_con_table;

    if (context == NULL || wsi == NULL) {
        _WARN("context or wsi is NULL");
        return;
    }

    /* mutex lock */
    pthread_mutex_lock(&in_out_mutex);
    con_table = context->con_list_first;
    prev_con_table = NULL;
    /* search wsi */
    for (id = 0; id < ICO_UWS_MAX_NUM; id++) {
        if (con_table == NULL) break;
        if (con_table->wsi == wsi) {
            /* update list */
            if (prev_con_table == NULL) {
                context->con_list_first = con_table->next;
            }
            else {
                prev_con_table->next = con_table->next;
            }
            /* update list's last */
            if (context->con_list_last == con_table) {
                context->con_list_last = prev_con_table;
            }
            /* clear data */
            con_table->wsi = NULL;
            _DBG("delete wsi from the connect info table success");
            /* add to free list */
            con_table->next = context->con_list_free;
            context->con_list_free = con_table;
            break;
        }
        prev_con_table = con_table;
        con_table = con_table->next;
    }

    /* mutex unlock */
    pthread_mutex_unlock(&in_out_mutex);

    return;
}

/*--------------------------------------------------------------------------*/
/**
 * @brief   del_connect_list
 *          Delete connect information list.
 *
 * @param[in]   context             ico_uws context
 * @return      none
 */
/*--------------------------------------------------------------------------*/
static void
del_connect_list(struct ico_uws_context *context)
{
    int id;
    struct ico_uws_connect *con_table;

    _DBG("delete connect information list");
    /* mutex lock */
    pthread_mutex_lock(&in_out_mutex);
    for (id = 0; id < ICO_UWS_MAX_NUM; id++) {
        con_table = context->con_list_first;
        if (con_table == NULL) break;
        if (con_table->wsi != NULL) con_table->wsi = NULL;
        context->con_list_first = con_table->next;
        free(con_table);
    }

    for (id = 0; id < ICO_UWS_MAX_NUM; id++) {
        con_table = context->con_list_free;
        if (con_table == NULL) break;
        if (con_table->wsi != NULL) con_table->wsi = NULL;
        context->con_list_free = con_table->next;
        free(con_table);
    }
    /* mutex unlock */
    pthread_mutex_unlock(&in_out_mutex);

    return;
}

/*--------------------------------------------------------------------------*/
/**
 * @brief   add_context
 *          Add ico_uws context to the local list.
 *
 * @param[in]   context             ico_uws context
 * @return      result
 * @retval      ICO_UWS_ERR_NONE    success
 * @retval      < 0                 error
 */
/*--------------------------------------------------------------------------*/
static int
add_context(struct ico_uws_context *context)
{
    int id;
    struct context_info_t *ctx_table;
    ico_uws_detail detail;

    if (context == NULL) {
        detail._ico_uws_error.code = ICO_UWS_ERR_INVALID_PARAM;
        _ERR("invalid param (%d)", detail._ico_uws_error.code);
        exec_callback(NULL, ICO_UWS_EVT_ERROR, NULL, &detail, NULL);
        return detail._ico_uws_error.code;
    }

    /* mutex lock */
    pthread_mutex_lock(&in_out_mutex);
    ctx_table = ctx_list_first;
    /* search free table */
    for (id = 0; id < ICO_UWS_MAX_NUM; id++) {
        if (ctx_table == NULL) break;
        ctx_table = ctx_table->next;
    }

    /* local list is full */
    if (id == ICO_UWS_MAX_NUM) {
        /* mutex unlock */
        pthread_mutex_unlock(&in_out_mutex);
        detail._ico_uws_error.code = ICO_UWS_ERR_UNKNOWN;
        _ERR("list is full (%d)", detail._ico_uws_error.code);
        exec_callback(NULL, ICO_UWS_EVT_ERROR, NULL, &detail, NULL);
        return detail._ico_uws_error.code;
    }

    /* create and set context */
    if (ctx_list_free == NULL) {
        /* free table does not exist */
        ctx_table = calloc(1, sizeof(struct context_info_t));
        if (ctx_table == NULL) {
            /* mutex unlock */
            pthread_mutex_unlock(&in_out_mutex);
            detail._ico_uws_error.code = ICO_UWS_ERR_OUT_OF_MEMORY;
            _ERR("no memory for ico_uws_connect (%d)",
                 detail._ico_uws_error.code);
            exec_callback(NULL, ICO_UWS_EVT_ERROR, NULL, &detail, NULL);
            return detail._ico_uws_error.code;
        }
    }
    else {
        /* use free area */
        ctx_table = ctx_list_free;
        ctx_list_free = ctx_table->next;
    }
    /* add data */
    ctx_table->context = context;
    _DBG("add context to the local list success");

    /* add table to list */
    ctx_table->next = NULL;
    if (ctx_list_first == NULL) {
        ctx_list_first = ctx_table;
    }
    else {
        ctx_list_last->next = ctx_table;
    }
    ctx_list_last = ctx_table;
    /* mutex unlock */
    pthread_mutex_unlock(&in_out_mutex);

    return ICO_UWS_ERR_NONE;
}

/*--------------------------------------------------------------------------*/
/**
 * @brief   get_context_by_wsctx
 *          Get ico_uws_context from the local list by websocket context.
 *
 * @param[in]   ws_context          libwebsocket context
 * @return      ico_uws context
 * @retval      context             ico_uws context's address
 * @retval      NULL                error
 */
/*--------------------------------------------------------------------------*/
static struct ico_uws_context *
get_context_by_wsctx(struct libwebsocket_context *ws_context)
{
    struct context_info_t *ctx_table;
    struct ico_uws_context *context;
    int id;

    context = NULL;
    /* mutex lock */
    pthread_mutex_lock(&in_out_mutex);
    ctx_table = ctx_list_first;
    for (id = 0; id < ICO_UWS_MAX_NUM; id++) {
        if (ctx_table == NULL) {
            /* mutex unlock */
            pthread_mutex_unlock(&in_out_mutex);
            _DBG("ws_context(0x%08x) does not exist in the local list", (int)ws_context);
            return current_creating_context;
        }
        if (ctx_table->context->ws_context == ws_context) {
            context = ctx_table->context;
            _DBG("get context success (ctx: 0x%08x)", (int)context);
            break;
        }
        ctx_table = ctx_table->next;
    }
    /* mutex unlock */
    pthread_mutex_unlock(&in_out_mutex);

    return context;
}

/*--------------------------------------------------------------------------*/
/**
 * @brief   get_context_by_wsi
 *          Get ico_uws_context from the local list by websocket instance.
 *
 * @param[in]   wsi                 libwebsocket instance
 * @return      ico_uws context
 * @retval      context             ico_uws context's address
 * @retval      NULL                error
 */
/*--------------------------------------------------------------------------*/
static struct ico_uws_context *
get_context_by_wsi(struct libwebsocket *wsi)
{
    struct context_info_t *ctx_table;
    struct ico_uws_connect *con_table;
    struct ico_uws_context *context;
    int id, con_id;

    context = NULL;
    /* mutex lock */
    pthread_mutex_lock(&in_out_mutex);
    ctx_table = ctx_list_first;
    for (id = 0; id < ICO_UWS_MAX_NUM; id++) {
        if (ctx_table == NULL) {
            /* mutex unlock */
            pthread_mutex_unlock(&in_out_mutex);
            _DBG("context does not exist in the local list");
            return current_creating_context;
        }
        con_table = ctx_table->context->con_list_first;
        for (con_id = 0; con_id < ICO_UWS_MAX_NUM; con_id++) {
            if (con_table == NULL) break;
            if (con_table->wsi == wsi) {
                context = ctx_table->context;
                _DBG("get context success (ctx: 0x%08x)", (int)context);
                break;
            }
            con_table = con_table->next;
        }
        if (context != NULL) break;
        ctx_table = ctx_table->next;
    }
    /* mutex unlock */
    pthread_mutex_unlock(&in_out_mutex);

    return context;
}

/*--------------------------------------------------------------------------*/
/**
 * @brief   del_context
 *          Delete the uws context's table from the local list.
 *
 * @param[in]   context             ico_uws context
 * @return      none
 */
/*--------------------------------------------------------------------------*/
static void
del_context(struct ico_uws_context *context)
{
    int id;
    struct context_info_t *ctx_table, *prev_ctx_table;

    if (context == NULL) {
        _WARN("context is NULL");
        return;
    }

    /* mutex lock */
    pthread_mutex_lock(&in_out_mutex);
    ctx_table = ctx_list_first;
    prev_ctx_table = NULL;
    /* search wsi */
    for (id = 0; id < ICO_UWS_MAX_NUM; id++) {
        if (ctx_table == NULL) break;
        if (ctx_table->context == context) {
            /* update list */
            if (prev_ctx_table == NULL) {
                ctx_list_first = ctx_table->next;
            }
            else {
                prev_ctx_table->next = ctx_table->next;
            }
            /* update list's last */
            if (ctx_list_last == ctx_table) {
                ctx_list_last = prev_ctx_table;
            }
            /* clear data */
            free(ctx_table->context);
            ctx_table->context = NULL;
            _DBG("delete context success");
            /* add to free list */
            ctx_table->next = ctx_list_free;
            ctx_list_free = ctx_table;
            break;
        }
        prev_ctx_table = ctx_table;
        ctx_table = ctx_table->next;
    }

    /* mutex unlock */
    pthread_mutex_unlock(&in_out_mutex);

    return;
}

/*--------------------------------------------------------------------------*/
/**
 * @brief   del_context_list
 *          Delete the local uws context's list.
 *
 * @param       none
 * @return      none
 */
/*--------------------------------------------------------------------------*/
static void
del_context_list(void)
{
    int id;
    struct context_info_t *ctx_table;

    _DBG("delete context info list");
    /* mutex lock */
    pthread_mutex_lock(&in_out_mutex);
    for (id = 0; id < ICO_UWS_MAX_NUM; id++) {
        ctx_table = ctx_list_first;
        if (ctx_table == NULL) break;

        ctx_list_first = ctx_table->next;
        free(ctx_table);
        _DBG("delete context info table success");
    }

    for (id = 0; id < ICO_UWS_MAX_NUM; id++) {
        ctx_table = ctx_list_free;
        if (ctx_table == NULL) break;

        ctx_list_free = ctx_table->next;
        free(ctx_table);
        _DBG("delete context info free table success");
    }
    /* mutex unlock */
    pthread_mutex_unlock(&in_out_mutex);

    return;
}

/*--------------------------------------------------------------------------*/
/**
 * @brief   exec_callback
 *          Execute callback function.
 *
 * @param[in]   context             ico_uws context
 * @param[in]   event               event code
 * @param[in]   id                  unique id
 * @param[in]   detail              detail information
 * @param[in]   user_data           user data
 * @return      result
 * @retval      ICO_UWS_ERR_NONE            success (called callback function)
 * @retval      ICO_UWS_ERR_INVALID_PARAM   error (no context or no callback function)
 */
/*--------------------------------------------------------------------------*/
static int
exec_callback(const struct ico_uws_context *context,
              const ico_uws_evt_e event,
              const void *id, const ico_uws_detail *detail,
              const void *user_data)
{
    if (context == NULL || context->callback == NULL) {
        _DBG("callback function does not exist");
        /* always success */
        return ICO_UWS_ERR_CONNECT;
    }

    _DBG("exec_callback (event: %d)", event);
    context->callback(context, event, id, detail, context->user_data);

    /* always success */
    return ICO_UWS_ERR_NONE;
}

/*--------------------------------------------------------------------------*/
/**
 * @brief   server_http_callback
 *          HTTP server callback function.
 *
 * @param[in]   ws_context          libwebsocket context
 * @param[in]   wsi                 libwebsocket instance
 * @param[in]   reason              callback reason
 * @param[in]   user                user data
 * @param[in]   in                  data (used for some callback reasons)
 * @param[in]   len                 size (used for some callback reasons)
 * @return      result
 * @retval      ICO_UWS_ERR_NONE    success (this function is always success)
 */
/*--------------------------------------------------------------------------*/
static int
server_http_callback(struct libwebsocket_context *ws_context,
                     struct libwebsocket *wsi,
                     enum libwebsocket_callback_reasons reason,
                     void *user, void *in, size_t len)
{
    struct ico_uws_context *context = get_context_by_wsctx(ws_context);
    ico_uws_detail detail;
    int i;

    switch (reason) {
    case LWS_CALLBACK_FILTER_NETWORK_CONNECTION:
        _DBG("server http cb: filter network connection");
        break;
    case LWS_CALLBACK_ADD_POLL_FD:
        detail._ico_uws_fd.fd = libwebsocket_get_socket_fd(wsi);
        _DBG("server http cb: add connection socket (%d)", detail._ico_uws_fd.fd);
        if ((exec_callback(context, ICO_UWS_EVT_ADD_FD, (void *)wsi, &detail, NULL)
             != ICO_UWS_ERR_NONE) && (context != NULL)) {
            for (i = 0; i < ICO_UWS_MAX_FDS; i++)   {
                if (context->callback_fd_list[i].fd == 0)   break;
            }
            if (i < ICO_UWS_MAX_FDS) {
                context->callback_fd_list[i].fd = detail._ico_uws_fd.fd;
                context->callback_fd_list[i].wsi = (void *)wsi;
            }
        }
        break;
    case LWS_CALLBACK_DEL_POLL_FD:
        detail._ico_uws_fd.fd = libwebsocket_get_socket_fd(wsi);
        _DBG("server http cb: delete connection socket (%d)", detail._ico_uws_fd.fd);
        if (context) {
            for (i = 0; i < ICO_UWS_MAX_FDS; i++)   {
                if (context->callback_fd_list[i].fd == detail._ico_uws_fd.fd)   {
                    context->callback_fd_list[i].fd = 0;
                }
            }
        }
        exec_callback(context, ICO_UWS_EVT_DEL_FD, (void *)wsi,
                      &detail, NULL);
        break;
    default:
        _DBG("server http cb: unhandled callback: %d", reason);
        break;
    }

    /* always success */
    return ICO_UWS_ERR_NONE;
}

/*--------------------------------------------------------------------------*/
/**
 * @brief   client_http_callback
 *          HTTP client callback function.
 *
 * @param[in]   ws_context          libwebsocket context
 * @param[in]   wsi                 libwebsocket instance
 * @param[in]   reason              callback reason
 * @param[in]   user                user data
 * @param[in]   in                  data (used for some callback reasons)
 * @param[in]   len                 size (used for some callback reasons)
 * @return      result
 * @retval      ICO_UWS_ERR_NONE    success (this function is always success)
 */
/*--------------------------------------------------------------------------*/
static int
client_http_callback(struct libwebsocket_context *ws_context,
                     struct libwebsocket *wsi,
                     enum libwebsocket_callback_reasons reason,
                     void *user, void *in, size_t len)
{
    struct ico_uws_context *context = get_context_by_wsi(wsi);
    ico_uws_detail detail;
    int fd;
    int i;

    switch (reason) {
    case LWS_CALLBACK_FILTER_NETWORK_CONNECTION:
        _DBG("client http cb: filter network connection");
        break;
    case LWS_CALLBACK_ADD_POLL_FD:
        fd = libwebsocket_get_socket_fd(wsi);
        detail._ico_uws_fd.fd = fd;
        _DBG("client http cb: add connection socket (%d)", detail._ico_uws_fd.fd);
        if ((exec_callback(context, ICO_UWS_EVT_ADD_FD, (void *)wsi, &detail, NULL)
             != ICO_UWS_ERR_NONE) && (context != NULL)) {
            for (i = 0; i < ICO_UWS_MAX_FDS; i++)   {
                if (context->callback_fd_list[i].fd == 0)   break;
            }
            if (i < ICO_UWS_MAX_FDS) {
                context->callback_fd_list[i].fd = detail._ico_uws_fd.fd;
                context->callback_fd_list[i].wsi = (void *)wsi;
            }
        }
        break;
    case LWS_CALLBACK_DEL_POLL_FD:
        fd = libwebsocket_get_socket_fd(wsi);
        detail._ico_uws_fd.fd = fd;
        _DBG("client http cb: delete connection socket (%d)", detail._ico_uws_fd.fd);
        if (context) {
            for (i = 0; i < ICO_UWS_MAX_FDS; i++) {
                if (context->callback_fd_list[i].fd == detail._ico_uws_fd.fd) {
                    context->callback_fd_list[i].fd = 0;
                }
            }
        }
        exec_callback(context, ICO_UWS_EVT_DEL_FD, (void *)wsi,
                      &detail, NULL);
        break;
    default:
        _DBG("client http cb: unhandled callback: %d", reason);
        break;
    }

    /* always success */
    return ICO_UWS_ERR_NONE;
}

/*--------------------------------------------------------------------------*/
/**
 * @brief   server_uws_callback
 *          Server callback function for ico_uws.
 *
 * @param[in]   ws_context          libwebsocket context
 * @param[in]   wsi                 libwebsocket instance
 * @param[in]   reason              callback reason
 * @param[in]   user                user data
 * @param[in]   in                  data (used for some callback reasons)
 * @param[in]   len                 size (used for some callback reasons)
 * @return      result
 * @retval      ICO_UWS_ERR_NONE    success
 * @retval      ICO_UWS_ERR_CLOSED  connection closed
 */
/*--------------------------------------------------------------------------*/
static int
server_uws_callback(struct libwebsocket_context *ws_context,
                    struct libwebsocket *wsi,
                    enum libwebsocket_callback_reasons reason,
                    void *user, void *in, size_t len)
{
    struct ico_uws_context *context = get_context_by_wsctx(ws_context);
    ico_uws_detail detail;
    int ret = ICO_UWS_ERR_NONE;

    switch (reason) {
    case LWS_CALLBACK_ESTABLISHED:
        /* server connection is established */
        _DBG("server cb: established");
        if (context != NULL) {
            add_ws_instance(context, wsi);
            context->state = ICO_UWS_STATE_OPEN;
        }
        exec_callback(context, ICO_UWS_EVT_OPEN, (void *)wsi,
                      NULL, NULL);
        (void)libwebsocket_callback_on_writable(ws_context, wsi);
        break;
    case LWS_CALLBACK_RECEIVE:
        /* receive data */
        _DBG("server cb: receive data");
        detail._ico_uws_message.recv_data = in;
        detail._ico_uws_message.recv_len = len;
        exec_callback(context, ICO_UWS_EVT_RECEIVE, (void *)wsi,
                      &detail, NULL);
        break;
    case LWS_CALLBACK_SERVER_WRITEABLE:
        _DBG("server cb: server writable");
        break;
    case LWS_CALLBACK_CLOSED:
        /* websocket session ends */
        _DBG("server cb: websocket session ends");
        if (context != NULL) {
            context->state = ICO_UWS_STATE_CLOSED;
            del_ws_instance(context, wsi);
        }
        exec_callback(context, ICO_UWS_EVT_CLOSE, (void *)wsi, NULL, NULL);
        ret = ICO_UWS_ERR_CLOSED;
        break;
    default:
        /* unhandled callback reason */
        _DBG("server cb: unhandled callback %d", reason);
        break;
    }

    return ret;
}

/*--------------------------------------------------------------------------*/
/**
 * @brief   client_uws_callback
 *          Client callback function for ico_uws.
 *
 * @param[in]   ws_context          libwebsocket context
 * @param[in]   wsi                 libwebsocket instance
 * @param[in]   reason              callback reason
 * @param[in]   user                user data
 * @param[in]   in                  data (used for some callback reasons)
 * @param[in]   len                 size (used for some callback reasons)
 * @return      result
 * @retval      ICO_UWS_ERR_NONE    success
 * @retval      ICO_UWS_ERR_CLOSED  connection closed
 */
/*--------------------------------------------------------------------------*/
static int
client_uws_callback(struct libwebsocket_context *ws_context,
                    struct libwebsocket *wsi,
                    enum libwebsocket_callback_reasons reason,
                    void *user, void *in, size_t len)
{
    struct ico_uws_context *context = get_context_by_wsi(wsi);
    ico_uws_detail detail;
    int ret = ICO_UWS_ERR_NONE;

    switch (reason) {
    case LWS_CALLBACK_CLIENT_ESTABLISHED:
        /* client connection is established */
        _DBG("client cb: client established");
        if (context != NULL) {
            add_ws_instance(context, wsi);
            context->state = ICO_UWS_STATE_OPEN;
        }
        exec_callback(context, ICO_UWS_EVT_OPEN, (void *)wsi,
                      NULL, NULL);
        (void)libwebsocket_callback_on_writable(ws_context, wsi);
        break;
    case LWS_CALLBACK_CLIENT_RECEIVE:
        /* receive data */
        _DBG("client cb: client receive");
        detail._ico_uws_message.recv_data = in;
        detail._ico_uws_message.recv_len = len;
        exec_callback(context, ICO_UWS_EVT_RECEIVE, (void *)wsi,
                      &detail, NULL);
        break;
    case LWS_CALLBACK_CLIENT_WRITEABLE:
        _DBG("client cb: client writable");
        break;
    case LWS_CALLBACK_CLOSED:
        /* websocket session ends */
        _DBG("client cb: websocket session ends");
        if (context != NULL) {
            context->state = ICO_UWS_STATE_CLOSED;
            del_ws_instance(context, wsi);
        }
        exec_callback(context, ICO_UWS_EVT_CLOSE, (void *)wsi, NULL, NULL);
        ret = ICO_UWS_ERR_CLOSED;
        break;
    default:
        _DBG("client cb: unhandled callback %d", reason);
        break;
    }

    return ret;
}

/*--------------------------------------------------------------------------*/
/**
 * @brief   create_server
 *          Create server's ico_uws context.
 *
 * @param[in]   uri                 the uri to which to connect
 * @param[in]   protocol            the protocol name
 * @return      context
 * @retval      ico_uws context     success
 * @retval      NULL                error
 */
/*--------------------------------------------------------------------------*/
static struct ico_uws_context *
create_server(const char *uri, const char *protocol)
{
    struct lws_context_creation_info ws_info;
    struct ico_uws_context *srv_context = NULL;
    ico_uws_detail detail;
    char *cpy_uri           = NULL;
    int port                = 0;
    int id;

    _DBG("create_server (uri: %s)", uri);
    /* create ico_uws_context */
    srv_context = (struct ico_uws_context *)
                              malloc(sizeof(struct ico_uws_context));
    if (srv_context == NULL) {
        detail._ico_uws_error.code = ICO_UWS_ERR_OUT_OF_MEMORY;
        _ERR("no memory for ico_uws_context (%d)",
             detail._ico_uws_error.code);
        exec_callback(NULL, ICO_UWS_EVT_ERROR, NULL, &detail, NULL);
        return NULL;
    }
    memset(srv_context, 0, sizeof(*srv_context));

    /* set port number */
    cpy_uri = strdup((char *)uri);
    port = atoi(&cpy_uri[1]);   /* delete colon(:) */
    free(cpy_uri);

    /* set protocol info */
    for (id = PROTOCOL_HTTP; id <= PROTOCOL_END; id++) {
        const char *name = NULL;
        void *callback = NULL;
        size_t size = 0;
        switch (id) {
        case PROTOCOL_HTTP:
            name = "http_only";
            callback = server_http_callback;
            break;
        case PROTOCOL_ICO_UWS:
            name = (const char *)strdup((char *)protocol);
            callback = server_uws_callback;
            break;
        case PROTOCOL_END:
            /* End of list's name and callback is NULL */
        default:
            /* never reach here */
            break;
        }
        srv_context->protocols[id].name = name;
        srv_context->protocols[id].callback = callback;
        srv_context->protocols[id].per_session_data_size = size;
    }

    /* clear libwebsocket info */
    memset(&ws_info, 0, sizeof(ws_info));
    /* set lws_context_creation_info */
    ws_info.port = port;
    ws_info.iface = NULL;  /* to bind the listen socket to all */
    ws_info.protocols = srv_context->protocols;
    ws_info.extensions = libwebsocket_get_internal_extensions();
    ws_info.ssl_cert_filepath = NULL;
    ws_info.ssl_private_key_filepath = NULL;
    ws_info.ssl_ca_filepath = NULL;
    ws_info.ssl_cipher_list = NULL;
    ws_info.gid = -1;
    ws_info.uid = -1;
    ws_info.options = 0;  /* no special options */
    ws_info.user = NULL;
    ws_info.ka_time = 0;
    ws_info.ka_probes = 0;
    ws_info.ka_interval = 0;

    /* create a server context */
    pthread_mutex_lock(&creating_mutex);
    current_creating_context = srv_context;
    srv_context->ws_context = libwebsocket_create_context(&ws_info);
    current_creating_context = NULL;
    pthread_mutex_unlock(&creating_mutex);

    if (srv_context->ws_context == NULL) {
        /* create context failed */
        detail._ico_uws_error.code = ICO_UWS_ERR_CREATE;
        _ERR("libwebsocket create context failed (%d)",
             detail._ico_uws_error.code);
        exec_callback(NULL, ICO_UWS_EVT_ERROR, NULL, &detail, NULL);
        free(srv_context);
        return NULL;
    }
    /* set data */
    strcpy((char *)(srv_context->uri), uri);
    srv_context->state = ICO_UWS_STATE_CONNECTING;
    /* add to the local list */
    add_context(srv_context);
    /* server created */
    _DBG("server created and listening on port %d", port);

    return srv_context;
}

/*--------------------------------------------------------------------------*/
/**
 * @brief   create_client
 *          Create client's ico_uws context.
 *
 * @param[in]   uri                 the uri to which to connect
 * @param[in]   protocol            the protocol name
 * @return      context
 * @retval      ico_uws context     success
 * @retval      NULL                error
 */
/*--------------------------------------------------------------------------*/
static struct ico_uws_context *
create_client(const char *uri, const char *protocol)
{
    struct lws_context_creation_info ws_info;
    struct ico_uws_context *clt_context = NULL;
    ico_uws_detail detail;
    char *split_mark        = ":/";
    char *cpy_uri           = NULL;
    char *address           = NULL;
    int port                = 0;
    int id;

    struct libwebsocket *wsi;
    int use_ssl             = 0;               /* not use ssl */
    int ietf_version        = -1;              /* -1: default value */
    char *host              = NULL;            /* host name */
    char *origin            = NULL;            /* socket name */

    _DBG("create_client (uri: %s)", uri);
    /* create ico_uws_context */
    clt_context = (struct ico_uws_context *)
                              malloc(sizeof(struct ico_uws_context));
    if (clt_context == NULL) {
        detail._ico_uws_error.code = ICO_UWS_ERR_OUT_OF_MEMORY;
        _ERR("no memory for ico_uws_context (%d)", detail._ico_uws_error.code);
        exec_callback(NULL, ICO_UWS_EVT_ERROR, NULL, &detail, NULL);
        return NULL;
    }
    memset(clt_context, 0, sizeof(*clt_context));

    /* set protocol info */
    for (id = PROTOCOL_HTTP; id <= PROTOCOL_END; id++) {
        const char *name = NULL;
        void *callback = NULL;
        size_t size = 0;
        switch (id) {
        case PROTOCOL_HTTP:
            name = "http_only";
            callback = client_http_callback;
            break;
        case PROTOCOL_ICO_UWS:
            name = (const char *)strdup((char *)protocol);
            callback = client_uws_callback;
            break;
        case PROTOCOL_END:
            /* End of list's name and callback is NULL */
        default:
            /* never reach here */
            break;
        }
        clt_context->protocols[id].name = name;
        clt_context->protocols[id].callback = callback;
        clt_context->protocols[id].per_session_data_size = size;
    }

    /* clear libwebsocket info */
    memset(&ws_info, 0, sizeof(ws_info));
    /* set lws_context_creation_info */
    ws_info.port = CONTEXT_PORT_NO_LISTEN;
    ws_info.iface = NULL;  /* to bind the listen socket to all */
    ws_info.protocols = clt_context->protocols;
    ws_info.extensions = libwebsocket_get_internal_extensions();
    ws_info.ssl_cert_filepath = NULL;
    ws_info.ssl_private_key_filepath = NULL;
    ws_info.ssl_ca_filepath = NULL;
    ws_info.ssl_cipher_list = NULL;
    ws_info.gid = -1;
    ws_info.uid = -1;
    ws_info.options = 0;  /* no special options */
    ws_info.user = NULL;
    ws_info.ka_time = 0;
    ws_info.ka_probes = 0;
    ws_info.ka_interval = 0;

    /* create a client context */
    pthread_mutex_lock(&creating_mutex);
    current_creating_context = clt_context;

    clt_context->ws_context = libwebsocket_create_context(&ws_info);
    if (clt_context->ws_context == NULL) {
        /* create context failed */
        current_creating_context = NULL;
        pthread_mutex_unlock(&creating_mutex);
        detail._ico_uws_error.code = ICO_UWS_ERR_CREATE;
        _ERR("libwebsocket create context failed (%d)",
             detail._ico_uws_error.code);
        exec_callback(NULL, ICO_UWS_EVT_ERROR, NULL, &detail, NULL);
        free(clt_context);
        return NULL;
    }

    /* alloc */
    cpy_uri = strdup((char *)uri);
    /* split uri and set address, port */
    strtok(cpy_uri, split_mark);    /* delet tag */
    address = strtok(NULL, split_mark);
    port = atoi(strtok(NULL, split_mark));

    /* create a client websocket instance */
    host = address;
    wsi = libwebsocket_client_connect(clt_context->ws_context,
                                      address, port,
                                      use_ssl, "/", host, origin,
                                      protocol, ietf_version);
    current_creating_context = NULL;
    pthread_mutex_unlock(&creating_mutex);

    clt_context->state = ICO_UWS_STATE_CONNECTING;
    /* free */
    free(cpy_uri);
    if (wsi == NULL) {
        /* client connect failed */
        detail._ico_uws_error.code = ICO_UWS_ERR_CONNECT;
        _ERR("libwebsocket client connect failed (%d)",
              detail._ico_uws_error.code);
        exec_callback(NULL, ICO_UWS_EVT_ERROR, NULL, &detail, NULL);
        libwebsocket_context_destroy(clt_context->ws_context);
        free(clt_context);
        return NULL;
    }
    /* set data */
    add_ws_instance(clt_context, wsi);
    strcpy((char *)(clt_context->uri), uri);
    clt_context->state = ICO_UWS_STATE_OPEN;
    /* add to the local list */
    add_context(clt_context);
    /* client connected */
    _DBG("client connected address: %s, port: %d", address, port);

    return clt_context;
}

/*===========================================================================*/
/* public interface function                                                 */
/*===========================================================================*/
/*--------------------------------------------------------------------------*/
/**
 * @brief   ico_uws_create_context
 *          Create ico_uws context.
 *          This API does not support secure access ("wss://") and
 *          the multi protocols.
 *          (If user sets "wss://", this function processes as "ws://".)
 *
 * @param[in]   uri                 the uri to which to connect
 *                                  server sets the string ":(port)"
 *                                  client sets the string "ws://(host):(port)"
 * @param[in]   protocol            the protocol name
 * @return      context
 * @retval      ico_uws context     success
 * @retval      NULL                error
 */
/*--------------------------------------------------------------------------*/
ICO_API struct ico_uws_context *
ico_uws_create_context(const char *uri, const char *protocol)
{
    struct ico_uws_context *context;
    ico_uws_detail detail;

    _DBG("ico_uws_create_context called");
    if (uri == NULL || protocol == NULL) {
        detail._ico_uws_error.code = ICO_UWS_ERR_INVALID_PARAM;
        _ERR("invalid param (%d)", detail._ico_uws_error.code);
        exec_callback(NULL, ICO_UWS_EVT_ERROR, NULL, &detail, NULL);
        return NULL;
    }

    if (strncmp(uri, ":", 1) == 0) {
        /* server */
        context = create_server(uri, protocol);
        _DBG("ico_uws_create_context created server context 0x%08x", (int)context);
    }
    else if (strstr(uri, URI_WS) != NULL || strstr(uri, URI_WS_SECURE) != NULL) {
        /* client */
        context = create_client(uri, protocol);
        _DBG("ico_uws_create_context created client context 0x%08x", (int)context);
    }
    else {
        detail._ico_uws_error.code = ICO_UWS_ERR_INVALID_PARAM;
        _ERR("invalid uri (%d)", detail._ico_uws_error.code);
        exec_callback(NULL, ICO_UWS_EVT_ERROR, NULL, &detail, NULL);
        return NULL;
    }

    return context;
}

/*--------------------------------------------------------------------------*/
/**
 * @brief   ico_uws_close
 *          Close the connection and destroy the ico_uws context.
 *
 * @param[in]   context             ico_uws context
 * @return      none
 */
/*--------------------------------------------------------------------------*/
ICO_API void
ico_uws_close(struct ico_uws_context *context)
{
    ico_uws_detail detail;

    _DBG("ico_uws_close called");
    if (context == NULL) {
        detail._ico_uws_error.code = ICO_UWS_ERR_INVALID_PARAM;
        _ERR("invalid param (%d)", detail._ico_uws_error.code);
        exec_callback(context, ICO_UWS_EVT_ERROR, NULL, &detail, NULL);
        return;
    }

    /* free list */
    del_connect_list(context);

    /* destroy websocket context */
    if (context->ws_context != NULL) {
        (void)libwebsocket_context_destroy(context->ws_context);
    }

    /* free "protocol name" area */
    if (context->protocols[PROTOCOL_ICO_UWS].name != NULL) {
        free((char *)context->protocols[PROTOCOL_ICO_UWS].name);
    }

    /* free ico_uws_context */
    (void)del_context(context);
    if (ctx_list_first == NULL) {
        /* no context exists in the local, but free list exists */
        (void)del_context_list();
    }

    return;
}

/*--------------------------------------------------------------------------*/
/**
 * @brief   ico_uws_send
 *          Send data to the connecting server or client.
 *          User needs to call the function "ico_uws_service"
 *          before calling this function.
 *
 * @param[in]   context             ico_uws context
 * @param[in]   id                  the id to connected to (callback notifies)
 * @param[in]   data                the data to send
 * @param[in]   len                 count of the data bytes
 * @return      none
 * @see         ico_uws_service
 */
/*--------------------------------------------------------------------------*/
ICO_API void
ico_uws_send(struct ico_uws_context *context, void *id,
             unsigned char *data, size_t len)
{
    int ret = 0;
    unsigned char buf[DATA_LEN] = {};
    struct libwebsocket *wsi;
    ico_uws_detail detail;

    _DBG("ico_uws_send called");
    if (context == NULL || id == NULL || data == NULL || len <= 0) {
        detail._ico_uws_error.code = ICO_UWS_ERR_INVALID_PARAM;
        _ERR("invalid param (%d)", detail._ico_uws_error.code);
        exec_callback(context, ICO_UWS_EVT_ERROR, id, &detail, NULL);
        return;
    }

    wsi = (struct libwebsocket *)id;
    /* set send data to buffer */
    memcpy(&buf[LWS_SEND_BUFFER_PRE_PADDING], data, len);
    /* send data using websocket instance */
    ret = libwebsocket_write(wsi,
                             &buf[LWS_SEND_BUFFER_PRE_PADDING],
                             len, LWS_WRITE_BINARY);
    if (ret < 0) {
        /* libwebsocket write failed */
        detail._ico_uws_error.code = ICO_UWS_ERR_SEND;
        _ERR("libwebsocket write failed ret=%d (%d)",
             ret, detail._ico_uws_error.code);
        exec_callback(context, ICO_UWS_EVT_ERROR, id, &detail, NULL);
    }

    return;
}

/*--------------------------------------------------------------------------*/
/**
 * @brief   ico_uws_service
 *          Service any pending websocket activity.
 *          This function deals with any pending websocket traffic,
 *          so you need to call this function periodically.
 *
 * @param[in]   context             ico_uws context
 * @return      none
 */
/*--------------------------------------------------------------------------*/
ICO_API void
ico_uws_service(struct ico_uws_context *context)
{
    int timedout_ms = 0;
    ico_uws_detail detail;

    if (context == NULL) {
        detail._ico_uws_error.code = ICO_UWS_ERR_INVALID_PARAM;
        _ERR("invalid param (%d)", detail._ico_uws_error.code);
        exec_callback(context, ICO_UWS_EVT_ERROR, NULL, &detail, NULL);
        return;
    }

    /* service any pending websocket activity */
    (void)libwebsocket_service(context->ws_context, timedout_ms);

    return;
}

/*--------------------------------------------------------------------------*/
/**
 * @brief   ico_uws_get_uri
 *          Get the uri that is connecting to now.
 *
 * @param[in]   context             ico_uws context
 * @return      uri
 * @retval      data of string      success
 * @retval      NULL                error
 */
/*--------------------------------------------------------------------------*/
ICO_API char *
ico_uws_get_uri(struct ico_uws_context *context)
{
    ico_uws_detail detail;

    _DBG("ico_uws_get_uri called");
    if (context == NULL) {
        detail._ico_uws_error.code = ICO_UWS_ERR_INVALID_PARAM;
        _ERR("invalid param (%d)", detail._ico_uws_error.code);
        exec_callback(context, ICO_UWS_EVT_ERROR, NULL, &detail, NULL);
        return NULL;
    }

    _DBG("return uri: %s", context->uri);
    return context->uri;
}

/*--------------------------------------------------------------------------*/
/**
 * @brief   ico_uws_get_ready_state
 *          Get the state of connection.
 *
 * @param[in]   context                 ico_uws context
 * @return      state
 * @retval      >= 0                    success
 * @retval      ICO_UWS_STATE_UNKNOWN   error
 */
/*--------------------------------------------------------------------------*/
ICO_API ico_uws_state_e
ico_uws_get_ready_state(struct ico_uws_context *context)
{
    ico_uws_detail detail;

    _DBG("ico_uws_get_ready_state called");
    if (context == NULL) {
        detail._ico_uws_error.code = ICO_UWS_ERR_INVALID_PARAM;
        _ERR("invalid param (%d)", detail._ico_uws_error.code);
        exec_callback(context, ICO_UWS_EVT_ERROR, NULL, &detail, NULL);
        return ICO_UWS_STATE_UNKNOWN;
    }

    _DBG("return state: %d", context->state);
    return context->state;
}

/*--------------------------------------------------------------------------*/
/**
 * @brief   ico_uws_set_event_cb
 *          Set the event callback function.
 *
 * @param[in]   context             ico_uws context
 * @param[in]   callback            callback function
 * @param[in]   user_data           user data
 * @return      result
 * @retval      ICO_UWS_ERR_NONE    success
 * @retval      others              error
 */
/*--------------------------------------------------------------------------*/
ICO_API int
ico_uws_set_event_cb(struct ico_uws_context *context,
                     ico_uws_evt_cb callback, void *user_data)
{
    ico_uws_detail detail;
    int i;

    _DBG("ico_uws_set_event_cb called");
    if (context == NULL || callback == NULL) {
        /* invalid parameter */
        _ERR("invalid param");
        return ICO_UWS_ERR_INVALID_PARAM;
    }

    /* set callback & user data */
    context->callback = callback;
    context->user_data = user_data;

    /* call callback            */
    for (i = 0; i < ICO_UWS_MAX_FDS; i++)   {
        if (context->callback_fd_list[i].fd != 0)   {
            detail._ico_uws_fd.fd = context->callback_fd_list[i].fd;
            context->callback_fd_list[i].fd = 0;
            exec_callback(context, ICO_UWS_EVT_ADD_FD,
                          context->callback_fd_list[i].wsi, &detail, NULL);
        }
    }

    return ICO_UWS_ERR_NONE;
}

/*--------------------------------------------------------------------------*/
/**
 * @brief   ico_uws_unset_event_cb
 *          Unset the event callback function.
 *
 * @param[in]   context             ico_uws context
 * @return      none
 */
/*--------------------------------------------------------------------------*/
ICO_API void
ico_uws_unset_event_cb(struct ico_uws_context *context)
{
    _DBG("ico_uws_unset_event_cb called");

    if (context == NULL) {
        /* invalid parameter */
        _ERR("invalid param");
        return;
    }

    /* unset callback & user data */
    context->callback = NULL;
    context->user_data = NULL;

    return;
}
