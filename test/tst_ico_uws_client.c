/*
 * Copyright (c) 2013, TOYOTA MOTOR CORPORATION.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 */
/**
 * @brief   test client (socket library for communicate)
 *
 * @date    June-7-2013
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <glib.h>
#include "ico_uws.h"

#include "tst_ico_uws.h"

/* ----------------------------------------------- */
/* Variable                                        */
/* ----------------------------------------------- */
#define SLEEP_TIME  2
#define RETRY_NUM   10

/* context */
static struct ico_uws_context *clt_context;
static void *clt_id;

/* receive event check */
static int receive_flag = UNSET_FLAG;

/* callback function is setting or not setting */
static int set_cb_flag = UNSET_FLAG;
static int num_call_cb = 0;

/* ----------------------------------------------- */
/* Define of static function                       */
/* ----------------------------------------------- */
static void tst_uws_callback(const struct ico_uws_context *context,
                             const ico_uws_evt_e event,
                             const void *id,
                             const ico_uws_detail *detail,
                             void *user_data);
static void tst_create_context(char *uri);
static void tst_get_ready_state(int state, char *str_state);
static void tst_get_uri(char *set_uri);
static void tst_send(unsigned char *data);
static void tst_service(void);
static void tst_close(void);
static void tst_set_evt_callback(unsigned char *send_data);
static void tst_unset_evt_callback(void);
static int ico_uws_client_test(char *uri, unsigned char *data);

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

    num_call_cb++;
    if (set_cb_flag == SET_FLAG) {
        ret_str = TEST_OK;
    }
    else {
        ret_str = TEST_NG;
    }

    /* set id */
    clt_id = (void *)id;

    switch (event) {
    case ICO_UWS_EVT_OPEN:
        sprintf(str, "open");
        if (clt_context != NULL) {
            tst_get_ready_state(ICO_UWS_STATE_OPEN, "open");
        }
        break;
    case ICO_UWS_EVT_ERROR:
        sprintf(str, "error");
        if (detail->_ico_uws_error.code == ICO_UWS_ERR_SEND) {
            dbg_print("ico_uws_service (client) : %s\n", TEST_NG);
            dbg_print("ico_uws_send (client) : %s\n", TEST_NG);
            receive_flag = SET_FLAG;
        }
        break;
    case ICO_UWS_EVT_CLOSE:
        sprintf(str, "close");
        if (clt_context != NULL) {
            tst_get_ready_state(ICO_UWS_STATE_CLOSED, "close");
        }
        break;
    case ICO_UWS_EVT_RECEIVE:
        sprintf(str, "receive");
        char *data = (char *)detail->_ico_uws_message.recv_data;
        if (strcmp((char *)user_data, data) != 0) {
            dbg_print("ico_uws_send (client) : %s\n", TEST_NG);
        } else {
            dbg_print("ico_uws_send (client) : %s\n", TEST_OK);
        }
        sprintf(str, "%s '%s'", str, data);
        receive_flag = SET_FLAG;
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
    dbg_print("ico_uws_evt_cb [%d (%s)] (client) : %s\n",
              event, str, ret_str);

    return;
}

/* create context */
static void
tst_create_context(char *uri)
{
    char *ret_str = TEST_OK;
    int id;

    clt_context = ico_uws_create_context(uri, PROTOCOL_NAME);
    if (clt_context == NULL) {
        ret_str = TEST_NG;
        for (id = 0; id < RETRY_NUM; id++) {
            clt_context = ico_uws_create_context(uri, PROTOCOL_NAME);
            if (clt_context != NULL) {
                ret_str = TEST_OK;
                break;
            }
            sleep(0.01);
        }
    }
    dbg_print("ico_uws_create_context (client) : %s\n", ret_str);

    return;
}

/* get ready state */
static void
tst_get_ready_state(int state, char *str_state)
{
    char *ret_str = TEST_OK;

    ico_uws_state_e cur_state = ico_uws_get_ready_state(clt_context);
    if (cur_state != state) {
        ret_str = TEST_NG;
    }
    dbg_print("ico_uws_get_ready_state [%s] (client) : %s\n",
              str_state, ret_str);

    return;
}

/* get uri */
static void
tst_get_uri(char *set_uri)
{
    char *ret_str = TEST_OK;

    char *uri = ico_uws_get_uri(clt_context);
    if (strcmp(uri, set_uri) != 0) {
        ret_str = TEST_NG;
    }
    dbg_print("ico_uws_get_uri [%s] (client) : %s\n",
              uri, ret_str);

    return;
}

/* send data */
static void
tst_send(unsigned char *data)
{
    int i;
    size_t len = strlen((char *)data) + 1;

    for (i = 0; i < 10; i++) {
        ico_uws_service(clt_context);
        usleep(100);
    }
    ico_uws_send(clt_context, clt_id, data, len);

    return;
}

/* service loop (wait to receive data) */
static void
tst_service()
{
    char *ret_str = TEST_OK;

    /* wait to close the connection */
    while (receive_flag == UNSET_FLAG) {
        ico_uws_service(clt_context);
        usleep(50);
    }
    receive_flag = UNSET_FLAG;
    dbg_print("ico_uws_service (client) : %s\n", ret_str);

    return;
}

/* close */
static void
tst_close()
{
    char *ret_str = TEST_OK;

    ico_uws_close(clt_context);
    dbg_print("ico_uws_close (client) : %s\n", ret_str);

    return;
}

/* set callback */
static void
tst_set_evt_callback(unsigned char *send_data)
{
    int ret;
    char *ret_str = TEST_OK;

    /* set callback */
    set_cb_flag = SET_FLAG;
    ret = ico_uws_set_event_cb(clt_context, tst_uws_callback,
                               (void *)send_data);
    if (ret != ICO_UWS_ERR_NONE) {
        ret_str = TEST_NG;
        dbg_print("ico_uws_set_event_cb (client) : %s (%d)\n",
                  ret_str, ret);
        return;
    }

    dbg_print("ico_uws_set_event_cb (client) : %s\n", ret_str);

    return;
}

/* unset callback */
static void
tst_unset_evt_callback()
{
    char *ret_str = TEST_OK;
    char *uri;

    /* unset callback */
    ico_uws_unset_event_cb(clt_context);
    set_cb_flag = UNSET_FLAG;
    num_call_cb = 0;

    /* occurs the error event */
    printf("-- Occurs the error event to test unset_event_cb\n");
    uri = ico_uws_get_uri(NULL);
    if (uri == NULL) {
        printf("-- Error event happened. (ico_uws_get_uri return Errror)\n");
    }

    sleep(SLEEP_TIME);
    if (num_call_cb > 0) {
        ret_str = TEST_NG;
    }

    dbg_print("ico_uws_unset_event_cb (client) : %s\n", ret_str);

    return;
}

/* test main (to connect to single server) */
static int
ico_uws_client_test(char *uri, unsigned char *data)
{
    /* create context */
    tst_create_context(uri);

    /* set callback */
    tst_set_evt_callback(data);

    /* interval */
    sleep(SLEEP_TIME);

    if (clt_context) {
        /* get uri */
        tst_get_uri(uri);

        /* send data */
        tst_send(data);

        /* wait to receive data */
        tst_service();

        /* interval */
        sleep(SLEEP_TIME);

        /* unset callback */
        tst_unset_evt_callback();

        /* close */
        tst_close();
    }

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
    char uri[128];
    unsigned char data[256];
    int id;
    int set_uri_flag = UNSET_FLAG;
    int set_data_flag = UNSET_FLAG;

    for (id = 0; id < argc; id++) {
        if (strcmp(argv[id], "-p") == 0) {
            /* set uri to connect */
            id++;
            sprintf(uri, "%s:%s", SRV_URI, argv[id]);
            set_uri_flag = SET_FLAG;
        }
        else if (strcmp(argv[id], "-d") == 0) {
            /* set data to send */
            id++;
            sprintf((char *)data, "%s", argv[id]);
            data[strlen(argv[id]) + 1] = '\0';
            set_data_flag = SET_FLAG;
        }
    }

    /* set default uri to connect */
    if (set_uri_flag == UNSET_FLAG) {
        sprintf(uri, "%s:%s", SRV_URI, SRV_PORT);
    }

    /* set default data to send */
    if (set_data_flag == UNSET_FLAG) {
        sprintf((char *)data, "%s", CLT_DATA);
    }

    g_setenv("PKG_NAME", "org.tizen.ico.tst_ico_uws_client", 1);
    g_mainloop = g_main_loop_new(NULL, 0);

    printf("\n");
    printf("##### ico_uws API (client) Test Start #####\n");
    ico_uws_client_test(uri, data);
    printf("##### ico_uws API (client) Test End #####\n");
    printf("\n");

    g_timeout_add_seconds(2, exit_program, NULL);
    g_main_loop_run(g_mainloop);

    return 0;
}
