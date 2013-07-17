/*
 * Copyright (c) 2013, TOYOTA MOTOR CORPORATION.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 */
/**
 * @brief   test server to listen to multi servers
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

struct tst_server_t{
    struct tst_server_t *next;
    struct ico_uws_context  *context;
    char            uri[128];
    unsigned char   data[256];
    size_t          len;
    void            *srv_id;        /* use to send data */
    int             receive_flag;
    int             close_flag;
    int             set_cb_flag;
    int             num_call_cb;
    int             id;
};

struct tst_server_t *first_srv = NULL;
struct tst_server_t *second_srv = NULL;

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
static void tst_create_context(struct tst_server_t *srv_t);
static void tst_get_ready_state(struct tst_server_t *srv_t,
                                int state, char *str_state);
static void tst_get_uri(struct tst_server_t *srv_t);
static void tst_send(struct tst_server_t *srv_t);
static void tst_service_receive(struct tst_server_t *srv_t);
static void tst_service_close(struct tst_server_t *srv_t);
static void tst_close(struct tst_server_t *srv_t);
static void tst_set_evt_callback(struct tst_server_t *srv_t);
static void tst_unset_evt_callback(struct tst_server_t *srv_t);
static struct tst_server_t *ico_uws_server_test_init(int id);
static void *tst_server_thread(void *args);
static void *tst_server_thread_sec(void *args);
static int ico_uws_server_test_multi(void);

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
    struct tst_server_t *srv_t;

    if (context == NULL) return;

    if (first_srv != NULL && context == first_srv->context) {
        srv_t = first_srv;
    }
    else if (second_srv != NULL && context == second_srv->context) {
        srv_t = second_srv;
    }
    else {
        return;
    }

    srv_t->num_call_cb++;
    if (srv_t->set_cb_flag == SET_FLAG) {
        ret_str = TEST_OK;
    }
    else {
        ret_str = TEST_NG;
    }

    switch (event) {
    case ICO_UWS_EVT_OPEN:
        sprintf(str, "open");
        break;
    case ICO_UWS_EVT_ERROR:
        sprintf(str, "error");
        break;
    case ICO_UWS_EVT_CLOSE:
        sprintf(str, "close");
        srv_t->close_flag = SET_FLAG;
        break;
    case ICO_UWS_EVT_RECEIVE:
        sprintf(str, "receive");
        /* set id */
        srv_t->srv_id = (void *)id;
        char *data = (char *)detail->_ico_uws_message.recv_data;
        size_t len = detail->_ico_uws_message.recv_len;
        /* set data to send */
        sprintf((char *)srv_t->data, "%s", data);
        srv_t->len = len;
        srv_t->receive_flag = SET_FLAG;
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

    printf("@@@ ico_uws_evt_cb [%d (%s)] (server %d) : %s\n",
           event, str, srv_t->id, ret_str);

    return;
}

/* create context */
static void
tst_create_context(struct tst_server_t *srv_t)
{
    char *ret_str = TEST_OK;

    /* mutex lock */
    pthread_mutex_lock(&multi_mutex);
    srv_t->context = ico_uws_create_context(srv_t->uri, PROTOCOL_NAME);
    /* mutex unlock */
    pthread_mutex_unlock(&multi_mutex);
    if (srv_t->context == NULL) {
        ret_str = TEST_NG;
    }
    dbg_print("ico_uws_create_context (server %d) : %s\n",
              srv_t->id, ret_str);

    return;
}

/* get ready state */
static void
tst_get_ready_state(struct tst_server_t *srv_t,
                    int state, char *str_state)
{
    char *ret_str = TEST_OK;

    /* mutex lock */
    pthread_mutex_lock(&multi_mutex);
    ico_uws_state_e cur_state = ico_uws_get_ready_state(srv_t->context);
    /* mutex unlock */
    pthread_mutex_unlock(&multi_mutex);
    if (cur_state != state) {
        ret_str = TEST_NG;
    }
    dbg_print("ico_uws_get_ready_state [%s] (server %d) : %s\n",
              str_state, srv_t->id, ret_str);

    return;
}

/* get uri */
static void
tst_get_uri(struct tst_server_t *srv_t)
{
    char *ret_str = TEST_OK;

    /* mutex lock */
    pthread_mutex_lock(&multi_mutex);
    char *uri = ico_uws_get_uri(srv_t->context);
    /* mutex unlock */
    pthread_mutex_unlock(&multi_mutex);
    if (strcmp(uri, srv_t->uri) != 0) {
        ret_str = TEST_NG;
    }
    dbg_print("ico_uws_get_uri [%s] (server %d) : %s\n",
              uri, srv_t->id, ret_str);

    return;
}

/* send data */
static void
tst_send(struct tst_server_t *srv_t)
{
    int i;

    for (i = 0; i < 10; i++) {
        /* mutex lock */
        pthread_mutex_lock(&multi_mutex);
        ico_uws_service(srv_t->context);
        /* mutex unlock */
        pthread_mutex_unlock(&multi_mutex);
        usleep(100);
    }

    /* mutex lock */
    pthread_mutex_lock(&multi_mutex);
    ico_uws_send(srv_t->context, srv_t->srv_id, srv_t->data, srv_t->len);
    /* mutex unlock */
    pthread_mutex_unlock(&multi_mutex);

    return;
}

/* service loop (wait to receive data) */
static void
tst_service_receive(struct tst_server_t *srv_t)
{
    char *ret_str = TEST_OK;

    while (srv_t->receive_flag == UNSET_FLAG) {
        /* mutex lock */
        pthread_mutex_lock(&multi_mutex);
        ico_uws_service(srv_t->context);
        /* mutex unlock */
        pthread_mutex_unlock(&multi_mutex);
        usleep(50);
    }
    srv_t->receive_flag = UNSET_FLAG;
    dbg_print("ico_uws_service (server %d received) : %s\n", srv_t->id, ret_str);

    return;
}

/* service loop (wait to close connection) */
static void
tst_service_close(struct tst_server_t *srv_t)
{
    char *ret_str = TEST_OK;

    while (srv_t->close_flag == UNSET_FLAG) {
        /* mutex lock */
        pthread_mutex_lock(&multi_mutex);
        ico_uws_service(srv_t->context);
        /* mutex unlock */
        pthread_mutex_unlock(&multi_mutex);
        usleep(50);
    }
    srv_t->close_flag = UNSET_FLAG;
    dbg_print("ico_uws_service (server %d close) : %s\n", srv_t->id, ret_str);

    return;
}

/* close */
static void
tst_close(struct tst_server_t *srv_t)
{
    char *ret_str = TEST_OK;

    /* mutex lock */
    pthread_mutex_lock(&multi_mutex);
    ico_uws_close(srv_t->context);
    /* mutex unlock */
    pthread_mutex_unlock(&multi_mutex);

    dbg_print("ico_uws_close (server %d) : %s\n", srv_t->id, ret_str);

    return;
}

/* set callback */
static void
tst_set_evt_callback(struct tst_server_t *srv_t)
{
    int ret;
    char *ret_str = TEST_OK;

    srv_t->set_cb_flag = SET_FLAG;
    /* mutex lock */
    pthread_mutex_lock(&multi_mutex);
    /* set callback */
    ret = ico_uws_set_event_cb(srv_t->context, tst_uws_callback, NULL);
    /* mutex unlock */
    pthread_mutex_unlock(&multi_mutex);
    if (ret != ICO_UWS_ERR_NONE) {
        ret_str = TEST_NG;
        dbg_print("ico_uws_set_event_cb (server %d) : %s (%d)\n",
                  srv_t->id, ret_str, ret);
        return;
    }

    dbg_print("ico_uws_set_event_cb (server %d) : %s\n",
              srv_t->id, ret_str);

    return;
}

/* unset callback */
static void
tst_unset_evt_callback(struct tst_server_t *srv_t)
{
    char *ret_str = TEST_OK;
    
    /* mutex lock */
    pthread_mutex_lock(&multi_mutex);
    /* unset callback */
    ico_uws_unset_event_cb(srv_t->context);
    /* mutex unlock */
    pthread_mutex_unlock(&multi_mutex);

    srv_t->set_cb_flag = UNSET_FLAG;
    srv_t->num_call_cb = 0;

    /* mutex lock */
    pthread_mutex_lock(&multi_mutex);
    /* occurs the error event */
    (void)ico_uws_get_uri(NULL);
    /* mutex unlock */
    pthread_mutex_unlock(&multi_mutex);
    sleep(SLEEP_TIME);
    if (srv_t->num_call_cb > 0) {
        ret_str = TEST_NG;
    }

    dbg_print("ico_uws_unset_event_cb (server %d) : %s\n",
              srv_t->id, ret_str);

    return;
}

/* prepare for test */
static struct tst_server_t *
ico_uws_server_test_init(int id)
{
    struct tst_server_t *srv_t;

    srv_t = calloc(1, sizeof(struct tst_server_t));
    if (srv_t == NULL) {
        printf("calloc failed\n");
        return NULL;
    }

    /* set uri to connect to */
    sprintf(srv_t->uri, ":%s", srv_ports[id]);

    /* initialize */
    srv_t->context = NULL;
    srv_t->srv_id = NULL;
    srv_t->receive_flag = UNSET_FLAG;
    srv_t->close_flag = UNSET_FLAG;
    srv_t->set_cb_flag = UNSET_FLAG;
    srv_t->num_call_cb = 0;
    srv_t->id = id;

    return srv_t;
}

/* ----------------------------------------------- */
/* Test Main                                       */
/* ----------------------------------------------- */
static void *
tst_server_thread(void *args)
{
    /* prepare for test */
    first_srv = ico_uws_server_test_init(0);
    if (first_srv == NULL) {
        return NULL;
    }

    /* create context */
    tst_create_context(first_srv);

    if (first_srv->context != NULL) {
        /* set callback */
        tst_set_evt_callback(first_srv);

        /* check the state */
        tst_get_ready_state(first_srv, ICO_UWS_STATE_CONNECTING,
                            "connecting");

        /* check the uri */
        tst_get_uri(first_srv);

        /* wait to receive data */
        tst_service_receive(first_srv);

        /* send data */
        tst_send(first_srv);

        /* wait to close connection */
        tst_service_close(first_srv);

        /* check the state */
        tst_get_ready_state(first_srv, ICO_UWS_STATE_CLOSED, "close");

        /* interval */
        sleep(SLEEP_TIME);

        /* unset callback */
        tst_unset_evt_callback(first_srv);

        /* interval */
        sleep(SLEEP_TIME);

        /* session close */
        tst_close(first_srv);
    }
    /* free memory */
    free(first_srv);

    return NULL;
}

static void *
tst_server_thread_sec(void *args)
{
    /* prepare for test */
    second_srv = ico_uws_server_test_init(1);
    if (second_srv == NULL) {
        return NULL;
    }

    /* create context */
    tst_create_context(second_srv);

    if (second_srv->context != NULL) {
        /* set callback */
        tst_set_evt_callback(second_srv);

        /* check the state */
        tst_get_ready_state(second_srv, ICO_UWS_STATE_CONNECTING,
                            "connecting");

        /* check the uri */
        tst_get_uri(second_srv);

        /* wait to receive data */
        tst_service_receive(second_srv);

        /* send data */
        tst_send(second_srv);

        /* wait to close connection */
        tst_service_close(second_srv);

        /* check the state */
        tst_get_ready_state(second_srv, ICO_UWS_STATE_CLOSED, "close");

        /* interval */
        sleep(SLEEP_TIME);

        /* unset callback */
        tst_unset_evt_callback(second_srv);

        /* interval */
        sleep(SLEEP_TIME);

        /* session close */
        tst_close(second_srv);
    }

    /* free memory */
    free(second_srv);

    return NULL;
}

/* test main (to connect to multi servers) */
static int
ico_uws_server_test_multi()
{
    pthread_t thread, thread_sec;

    /* server to connect server (port: 8080) */
    pthread_create( &thread, NULL, tst_server_thread, (void *)NULL );
    /* server to connect server (port: 9090) */
    pthread_create( &thread_sec, NULL, tst_server_thread_sec, (void *)NULL );

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
    g_setenv("PKG_NAME", "org.tizen.ico.tst_ico_uws_mlt_server", 1);
    g_mainloop = g_main_loop_new(NULL, 0);

    printf("\n");
    printf("##### ico_uws API (server to listen to multi clients)");
    printf(" Test Start #####\n");
    ico_uws_server_test_multi();
    printf("##### ico_uws API (server to listen to multi clients)");
    printf(" Test End #####\n");
    printf("\n");

    g_timeout_add_seconds(2, exit_program, NULL);
    g_main_loop_run(g_mainloop);

    return 0;
}
