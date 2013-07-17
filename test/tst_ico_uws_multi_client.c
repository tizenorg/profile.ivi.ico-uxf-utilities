/*
 * Copyright (c) 2013, TOYOTA MOTOR CORPORATION.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 */
/**
 * @brief   test client to connect to multi servers
 *          (socket library for communicate)
 *
 * @date    June-27-2013
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#include <glib.h>
#include "ico_uws.h"

#include "tst_ico_uws.h"

/* ----------------------------------------------- */
/* Variable                                        */
/* ----------------------------------------------- */
#define SLEEP_TIME  3

struct tst_client_t{
    struct tst_client_t *next;
    struct ico_uws_context  *context;
    char            uri[128];
    unsigned char   data[256];
    size_t          len;
    void            *clt_id;        /* use to send data */
    int             receive_flag;
    int             open_flag;
    int             set_cb_flag;
    int             num_call_cb;
    int             id;
};

struct tst_client_t *first_clt = NULL;
struct tst_client_t *second_clt = NULL;

/* pthread mutex initialize */
static pthread_mutex_t multi_mutex = PTHREAD_MUTEX_INITIALIZER;

/* ----------------------------------------------- */
/* Define of static function                       */
/* ----------------------------------------------- */
static void tst_uws_callback(const struct ico_uws_context *context,
                             const ico_uws_evt_e event,
                             const void *id,
                             const ico_uws_detail *detail,
                             void *user_data);
static void tst_create_context(struct tst_client_t *clt_t);
static void tst_get_ready_state(struct tst_client_t *clt_t,
                                int state, char *str_state);
static void tst_get_uri(struct tst_client_t *clt_t);
static void tst_send(struct tst_client_t *clt_t);
static void tst_service_open(struct tst_client_t *clt_t);
static void tst_service_receive(struct tst_client_t *clt_t);
static void tst_close(struct tst_client_t *clt_t);
static void tst_set_evt_callback(struct tst_client_t *clt_t);
static void tst_unset_evt_callback(struct tst_client_t *clt_t);
static struct tst_client_t *ico_uws_client_test_init(int id);
static void *tst_client_thread(void *args);
static void *tst_client_thread_sec(void *args);
static int ico_uws_client_test_multi(void);

/* ----------------------------------------------- */
/* Public API Test                                 */
/* ----------------------------------------------- */
/* event callback */
static void
tst_uws_callback(const struct ico_uws_context *context,
                 const ico_uws_evt_e event,
                 const void *id,
                 const ico_uws_detail *detail,
                 void *user_data)
{
    char str[256];
    char *ret_str;
    struct tst_client_t *clt_t;

    if (context == NULL) return;

    if (first_clt != NULL && context == first_clt->context) {
        clt_t = first_clt;
    }
    else if (second_clt != NULL && context == second_clt->context) {
        clt_t = second_clt;
    }
    else {
        return;
    }

    clt_t->num_call_cb++;
    if (clt_t->set_cb_flag == SET_FLAG) {
        ret_str = TEST_OK;
    }
    else {
        ret_str = TEST_NG;
    }

    switch (event) {
    case ICO_UWS_EVT_OPEN:
        sprintf(str, "open");
        clt_t->clt_id = (void *)id;
        clt_t->open_flag = SET_FLAG;
        break;
    case ICO_UWS_EVT_ERROR:
        sprintf(str, "error");
        if (detail->_ico_uws_error.code == ICO_UWS_ERR_SEND) {
            dbg_print("ico_uws_service (client %d) : %s\n",
                      clt_t->id, TEST_NG);
            dbg_print("ico_uws_send (client %d) : %s\n",
                      clt_t->id, TEST_NG);
            clt_t->receive_flag = SET_FLAG;
        }
        break;
    case ICO_UWS_EVT_CLOSE:
        sprintf(str, "close");
        tst_get_ready_state(clt_t, ICO_UWS_STATE_CLOSED, "close");
        break;
    case ICO_UWS_EVT_RECEIVE:
        sprintf(str, "receive");
        char *data = (char *)detail->_ico_uws_message.recv_data;
        char *send_data = (char *)clt_t->data;
        if (strcmp(send_data, data) != 0) {
            dbg_print("ico_uws_send (client %d) : %s\n",
                      clt_t->id, TEST_NG);
        } else {
            dbg_print("ico_uws_send (client %d) : %s\n",
                      clt_t->id, TEST_OK);
        }
        clt_t->receive_flag = SET_FLAG;
        sprintf(str, "%s '%s'", str, data);
        break;
    case ICO_UWS_EVT_ADD_FD:
        sprintf(str, "add fd(%d)", detail->_ico_uws_fd.fd);
        break;
    case ICO_UWS_EVT_DEL_FD:
        sprintf(str, "delete fd(%d)", detail->_ico_uws_fd.fd);
        break;
    default:
        /* other event is not test */
        break;
    }

    printf("@@@ ico_uws_evt_cb [%d (%s)] (client %d) : %s\n",
           event, str, clt_t->id, ret_str);

    return;
}

/* create context */
static void
tst_create_context(struct tst_client_t *clt_t)
{
    char *ret_str = TEST_OK;

    /* mutex lock */
    pthread_mutex_lock(&multi_mutex);
    clt_t->context = ico_uws_create_context(clt_t->uri, PROTOCOL_NAME);
    /* mutex unlock */
    pthread_mutex_unlock(&multi_mutex);
    if (clt_t->context == NULL) {
        ret_str = TEST_NG;
    }
    dbg_print("ico_uws_create_context (client %d) : %s\n",
              clt_t->id, ret_str);

    return;
}

/* get ready state */
static void
tst_get_ready_state(struct tst_client_t *clt_t,
                    int state, char *str_state)
{
    char *ret_str = TEST_OK;

    /* mutex lock */
    pthread_mutex_lock(&multi_mutex);
    ico_uws_state_e cur_state = ico_uws_get_ready_state(clt_t->context);
    /* mutex unlock */
    pthread_mutex_unlock(&multi_mutex);
    if (cur_state != state) {
        ret_str = TEST_NG;
    }
    dbg_print("ico_uws_get_ready_state [%s] (client %d) : %s\n",
              str_state, clt_t->id, ret_str);

    return;
}

/* get uri */
static void
tst_get_uri(struct tst_client_t *clt_t)
{
    char *ret_str = TEST_OK;

    /* mutex lock */
    pthread_mutex_lock(&multi_mutex);
    char *uri = ico_uws_get_uri(clt_t->context);
    /* mutex unlock */
    pthread_mutex_unlock(&multi_mutex);
    if (strcmp(uri, clt_t->uri) != 0) {
        ret_str = TEST_NG;
    }
    dbg_print("ico_uws_get_uri [%s] (client %d) : %s\n",
              uri, clt_t->id, ret_str);

    return;
}

/* send data */
static void
tst_send(struct tst_client_t *clt_t)
{
    int i;

    for (i = 0; i < 10; i++) {
        /* mutex lock */
        pthread_mutex_lock(&multi_mutex);
        ico_uws_service(clt_t->context);
        /* mutex unlock */
        pthread_mutex_unlock(&multi_mutex);
        usleep(100);
    }

    /* mutex lock */
    pthread_mutex_lock(&multi_mutex);
    ico_uws_send(clt_t->context, clt_t->clt_id, clt_t->data, clt_t->len);
    /* mutex unlock */
    pthread_mutex_unlock(&multi_mutex);

    return;
}

/* service loop (wait to open) */
static void
tst_service_open(struct tst_client_t *clt_t)
{
    char *ret_str = TEST_OK;

    while (clt_t->open_flag == UNSET_FLAG) {
        /* mutex lock */
        pthread_mutex_lock(&multi_mutex);
        ico_uws_service(clt_t->context);
        /* mutex unlock */
        pthread_mutex_unlock(&multi_mutex);
        usleep(50);
    }
    clt_t->open_flag = UNSET_FLAG;
    dbg_print("ico_uws_service (client %d open) : %s\n", clt_t->id, ret_str);

    return;
}

/* service loop (wait to receive data) */
static void
tst_service_receive(struct tst_client_t *clt_t)
{
    char *ret_str = TEST_OK;

    while (clt_t->receive_flag == UNSET_FLAG) {
        /* mutex lock */
        pthread_mutex_lock(&multi_mutex);
        ico_uws_service(clt_t->context);
        /* mutex unlock */
        pthread_mutex_unlock(&multi_mutex);
        usleep(50);
    }
    clt_t->receive_flag = UNSET_FLAG;
    dbg_print("ico_uws_service (client %d receive) : %s\n", clt_t->id, ret_str);

    return;
}

/* close */
static void
tst_close(struct tst_client_t *clt_t)
{
    char *ret_str = TEST_OK;

    /* mutex lock */
    pthread_mutex_lock(&multi_mutex);
    ico_uws_close(clt_t->context);
    /* mutex unlock */
    pthread_mutex_unlock(&multi_mutex);

    dbg_print("ico_uws_close (client %d) : %s\n", clt_t->id, ret_str);

    return;
}

/* set callback */
static void
tst_set_evt_callback(struct tst_client_t *clt_t)
{
    int ret;
    char *ret_str = TEST_OK;

    clt_t->set_cb_flag = SET_FLAG;
    /* mutex lock */
    pthread_mutex_lock(&multi_mutex);
    /* set callback */
    ret = ico_uws_set_event_cb(clt_t->context, tst_uws_callback, NULL);
    /* mutex unlock */
    pthread_mutex_unlock(&multi_mutex);
    if (ret != ICO_UWS_ERR_NONE) {
        ret_str = TEST_NG;
        dbg_print("ico_uws_set_event_cb (client %d) : %s (%d)\n",
                  clt_t->id, ret_str, ret);
        return;
    }

    dbg_print("ico_uws_set_event_cb (client %d) : %s\n",
              clt_t->id, ret_str);

    return;
}

/* unset callback */
static void
tst_unset_evt_callback(struct tst_client_t *clt_t)
{
    char *ret_str = TEST_OK;
    
    /* mutex lock */
    pthread_mutex_lock(&multi_mutex);
    /* unset callback */
    ico_uws_unset_event_cb(clt_t->context);
    /* mutex unlock */
    pthread_mutex_unlock(&multi_mutex);

    clt_t->set_cb_flag = UNSET_FLAG;
    clt_t->num_call_cb = 0;

    /* mutex lock */
    pthread_mutex_lock(&multi_mutex);
    /* occurs the error event */
    (void)ico_uws_get_uri(NULL);
    /* mutex unlock */
    pthread_mutex_unlock(&multi_mutex);
    sleep(SLEEP_TIME);
    if (clt_t->num_call_cb > 0) {
        ret_str = TEST_NG;
    }

    dbg_print("ico_uws_unset_event_cb (client %d) : %s\n",
              clt_t->id, ret_str);

    return;
}

/* prepare for test */
static struct tst_client_t *
ico_uws_client_test_init(int id)
{
    struct tst_client_t *clt_t;

    clt_t = calloc(1, sizeof(struct tst_client_t));
    if (clt_t == NULL) {
        printf("calloc failed\n");
        return NULL;
    }

    /* set uri to connect to */
    sprintf(clt_t->uri, "%s:%s", SRV_URI, srv_ports[id]);
    /* set data to send */
    sprintf((char *)clt_t->data, "%s", clt_datas[id]);
    clt_t->len = strlen(clt_datas[id]) + 1;
    clt_t->data[clt_t->len] = '\0';

    /* initialize */
    clt_t->context = NULL;
    clt_t->clt_id = NULL;
    clt_t->receive_flag = UNSET_FLAG;
    clt_t->open_flag = UNSET_FLAG;
    clt_t->set_cb_flag = UNSET_FLAG;
    clt_t->num_call_cb = 0;
    clt_t->id = id;

    return clt_t;
}

/* ----------------------------------------------- */
/* Test Main                                       */
/* ----------------------------------------------- */
static void *
tst_client_thread(void *args)
{
    /* prepare for test */
    first_clt = ico_uws_client_test_init(0);
    if (first_clt == NULL) {
        return NULL;
    }

    /* create context */
    tst_create_context(first_clt);

    if (first_clt->context != NULL) {
        /* set callback */
        tst_set_evt_callback(first_clt);

        /* wait to open */
        tst_service_open(first_clt);

        /* check the state */
        tst_get_ready_state(first_clt, ICO_UWS_STATE_OPEN, "open");

        /* check the uri */
        tst_get_uri(first_clt);

        /* send data */
        tst_send(first_clt);

        /* wait to receive data */
        tst_service_receive(first_clt);

        /* interval */
        sleep(SLEEP_TIME);

        /* unset callback */
        tst_unset_evt_callback(first_clt);

        /* interval */
        sleep(SLEEP_TIME);

        /* session close */
        tst_close(first_clt);
    }
    /* free memory */
    free(first_clt);

    return NULL;
}

static void *
tst_client_thread_sec(void *args)
{
    /* prepare for test */
    second_clt = ico_uws_client_test_init(1);
    if (second_clt == NULL) {
        return NULL;
    }

    /* create context */
    tst_create_context(second_clt);

    if (second_clt->context != NULL) {
        /* set callback */
        tst_set_evt_callback(second_clt);

        /* wait to open */
        tst_service_open(second_clt);

        /* check the state */
        tst_get_ready_state(second_clt, ICO_UWS_STATE_OPEN, "open");

        /* check the uri */
        tst_get_uri(second_clt);

        /* send data */
        tst_send(second_clt);

        /* wait to receive data */
        tst_service_receive(second_clt);

        /* interval */
        sleep(SLEEP_TIME);

        /* unset callback */
        tst_unset_evt_callback(second_clt);

        /* interval */
        sleep(SLEEP_TIME);

        /* session close */
        tst_close(second_clt);
    }

    /* free memory */
    free(second_clt);

    return NULL;
}

/* test main (to connect to multi servers) */
static int
ico_uws_client_test_multi()
{
    pthread_t thread, thread_sec;

    /* client to connect server (port: 8080) */
    pthread_create( &thread, NULL, tst_client_thread, (void *)NULL );
    /* client to connect server (port: 9090) */
    pthread_create( &thread_sec, NULL, tst_client_thread_sec, (void *)NULL );

    pthread_join( thread, NULL );
    pthread_join( thread_sec, NULL );

    /* interval */
    sleep(SLEEP_TIME);

    return 1;
}

/* ----------------------------------------------- */
/* Main                                            */
/* ----------------------------------------------- */
static GMainLoop *g_mainloop = NULL;

static gboolean
exit_program(gpointer data)
{
    g_main_loop_quit(g_mainloop);

    return FALSE;
}

/* main */
int
main(int argc, char **argv)
{
    g_setenv("PKG_NAME", "org.tizen.ico.tst_ico_uws_mlt_client", 1);
    g_mainloop = g_main_loop_new(NULL, 0);

    printf("\n");
    printf("##### ico_uws API (client to connect to multi servers)");
    printf(" Test Start #####\n");
    ico_uws_client_test_multi();
    printf("##### ico_uws API (client to connect to multi servers)");
    printf(" Test End #####\n");
    printf("\n");

    g_timeout_add_seconds(2, exit_program, NULL);
    g_main_loop_run(g_mainloop);

    return 0;
}
