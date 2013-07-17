/*
 * Copyright (c) 2013, TOYOTA MOTOR CORPORATION.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 */
/**
 * @brief   private header file of library for communicate
 *
 * @date    June-7-2013
 */

#ifndef __ICO_UWS_PRIVATE_H__
#define __ICO_UWS_PRIVATE_H__

#ifdef __cplusplus
extern "C" {
#endif

/*===========================================================================*/
/* variable & table                                                          */
/*===========================================================================*/
#define ICO_UWS_MAX_NUM     200     /**< max num of list's data         */
#define ICO_UWS_MAX_FDS     4       /**< max num of file descriptors    */

/* libwebsockets's protocol number */
enum ico_apf_protocols
{
    PROTOCOL_HTTP = 0,  /* always first */
    PROTOCOL_ICO_UWS,   /* for ico_uws communication */
    PROTOCOL_END        /* final */
};

/* structure for managing the ico_uws_context */
struct ico_uws_mng_context {
    struct ico_uws_mng_context   *next;
    struct ico_uws_context       *context;
};

/* structure for managing the ico_uws_context's list */
struct ico_uws_mng_context_list {
    struct ico_uws_mng_context  *first;
    struct ico_uws_mng_context  *last;
    struct ico_uws_mng_context  *free;
};

/* structure of connection info */
struct ico_uws_connect {
    struct ico_uws_connect *next;
    struct libwebsocket *wsi;
};

/* ico uws context */
struct ico_uws_context {
    struct libwebsocket_context *ws_context;

    /* connection information list */
    struct ico_uws_connect  *con_list_first;
    struct ico_uws_connect  *con_list_last;
    struct ico_uws_connect  *con_list_free;

    char uri[128];
    int port;
    ico_uws_state_e state;
    struct  {
        int     fd;
        void    *wsi;
    }   callback_fd_list[ICO_UWS_MAX_FDS];

    struct libwebsocket_protocols protocols[PROTOCOL_END + 1];

    ico_uws_evt_cb callback;
    void *user_data;
};

/*============================================================================*/
/* global API                                                                 */
/*============================================================================*/
#if defined(__GNUC__) && __GNUC__ >= 4
#define ICO_API __attribute__ ((visibility("default")))
#else
#define ICO_API
#endif

/*============================================================================*/
/* log macro                                                                  */
/*============================================================================*/
#ifndef  _NO_USE_DLOG

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "ICO_UWS"
#include <dlog.h>

static int  _ico_uws_debug = 0;

#define _ERR(fmt, arg...)                                       \
    do {                                                        \
        fprintf(stderr, "ico_uws E: %s:%d [ "fmt" ]\n",         \
                __FUNCTION__,                                   \
                __LINE__,                                       \
                ##arg);                                         \
        LOGE("%s:%d " fmt, __FUNCTION__, __LINE__, ##arg);      \
    } while (0)

#define _WARN(fmt, arg...)                                      \
    do {                                                        \
        LOGW("%s:%d " fmt, __FUNCTION__, __LINE__, ##arg);      \
    } while (0)

#define _INFO(fmt, arg...)                                      \
    do {                                                        \
        LOGI("%s:%d " fmt, __FUNCTION__, __LINE__, ##arg);      \
    } while (0)

#define _DBG(fmt, arg...)                                       \
    do {                                                        \
        if (_ico_uws_debug == 0)    {                           \
            if (getenv("ICO_UWS_DEBUG"))                        \
                _ico_uws_debug = 1;                             \
            else                                                \
                _ico_uws_debug = -1;                            \
        }                                                       \
        if (_ico_uws_debug > 0) {                               \
            LOGD("%s:%d " fmt, __FUNCTION__, __LINE__, ##arg);  \
        }                                                       \
    } while (0)

#else

#define _ERR(fmt, arg...)                                       \
    do {                                                        \
        fprintf(stderr,                                         \
                "ico_uws E: %s:%d [ "fmt" ]\n",                 \
                __FUNCTION__,                                   \
                __LINE__,                                       \
                ##arg);                                         \
    } while (0)

#define _WARN(fmt, arg...)                                      \
    do {                                                        \
        fprintf(stderr,                                         \
                "ico_uws W: %s:%d [ "fmt" ]\n",                 \
                __FUNCTION__,                                   \
                __LINE__,                                       \
                ##arg);                                         \
    } while (0)


#define _INFO(fmt, arg...)                                      \
    do {                                                        \
        fprintf(stderr,                                         \
                "ico_uws I: %s:%d [ "fmt" ]\n",                 \
                __FUNCTION__,                                   \
                __LINE__,                                       \
                ##arg);                                         \
    } while (0)

#define _DBG(fmt, arg...)                                       \
    do {                                                        \
        if (getenv("ICO_UWS_DEBUG")) {                          \
            fprintf(stderr,                                     \
                    "ico_uws D: %s:%d [ "fmt" ]\n",             \
                    __FUNCTION__,                               \
                    __LINE__,                                   \
                    ##arg);                                     \
        }                                                       \
    } while (0)

#endif

#ifdef __cplusplus
}
#endif

#endif /* __ICO_UWS_PRIVATE_H__ */
