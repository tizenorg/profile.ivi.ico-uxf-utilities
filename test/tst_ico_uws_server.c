/*
 * Copyright (c) 2013, TOYOTA MOTOR CORPORATION.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 */
/**
 * @brief   test server (socket library for communicate)
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

/* context */
static struct ico_uws_context *context;

/* close event check */
static int close_flag = UNSET_FLAG;
static int num_connect = 0;

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
static void tst_service(void);
static void tst_close(void);
static void tst_set_evt_callback(void);
static void tst_unset_evt_callback(void);
static int ico_uws_server_test(char *uri);

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

    switch (event) {
    case ICO_UWS_EVT_OPEN:
        sprintf(str, "open");
        num_connect++;
        if (context != NULL) {
            tst_get_ready_state(ICO_UWS_STATE_OPEN, "open");
        }
        break;
    case ICO_UWS_EVT_ERROR:
        sprintf(str, "error");
        break;
    case ICO_UWS_EVT_CLOSE:
        sprintf(str, "close");
        num_connect--;
        if (num_connect == 0) {
            close_flag = SET_FLAG;
        }
        if (context != NULL) {
            tst_get_ready_state(ICO_UWS_STATE_CLOSED, "close");
        }
        break;
    case ICO_UWS_EVT_RECEIVE:
        sprintf(str, "receive");
        ico_uws_send((struct ico_uws_context *)context,
                     (void *)id,
                     (unsigned char *)detail->_ico_uws_message.recv_data,
                     detail->_ico_uws_message.recv_len);
        sprintf(str, "%s '%s'",
                str, (unsigned char *)detail->_ico_uws_message.recv_data);
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
    dbg_print("ico_uws_evt_cb [%d (%s)] (server) : %s\n",
              event, str, ret_str);

    return;
}

/* create context */
static void
tst_create_context(char *uri)
{
    char *ret_str = TEST_OK;

    context = ico_uws_create_context(uri, PROTOCOL_NAME);
    if (context == NULL) {
        ret_str = TEST_NG;
    }
    num_connect = 0;
    dbg_print("ico_uws_create_context (server) : %s\n", ret_str);

    return;
}

/* get ready state */
static void
tst_get_ready_state(int state, char *str_state)
{
    char *ret_str = TEST_OK;

    ico_uws_state_e cur_state = ico_uws_get_ready_state(context);
    if (cur_state != state) {
        ret_str = TEST_NG;
    }
    dbg_print("ico_uws_get_ready_state [%s] (server) : %s\n",
              str_state, ret_str);

    return;
}

/* get uri */
static void
tst_get_uri(char *set_uri)
{
    char *ret_str = TEST_OK;

    char *uri = ico_uws_get_uri(context);
    if (strcmp(uri, set_uri) != 0) {
        ret_str = TEST_NG;
    }
    dbg_print("ico_uws_get_uri [%s] (server) : %s\n",
              uri, ret_str);

    return;
}

/* service loop */
static void
tst_service()
{
    char *ret_str = TEST_OK;

    close_flag = UNSET_FLAG;
    /* wait to close the connection */
    while (close_flag == UNSET_FLAG) {
        ico_uws_service(context);
        usleep(50);
    }
    dbg_print("ico_uws_service (server) : %s\n", ret_str);

    return;
}

/* close */
static void
tst_close()
{
    char *ret_str = TEST_OK;

    ico_uws_close(context);
    dbg_print("ico_uws_close (server) : %s\n", ret_str);

    return;
}

/* set callback */
static void
tst_set_evt_callback()
{
    int ret;
    char *ret_str = TEST_OK;

    /* set callback */
    set_cb_flag = SET_FLAG;
    ret = ico_uws_set_event_cb(context, tst_uws_callback, NULL);
    if (ret != ICO_UWS_ERR_NONE) {
        ret_str = TEST_NG;
        dbg_print("ico_uws_set_event_cb (server) : %s (%d)\n",
                  ret_str, ret);
        return;
    }

    dbg_print("ico_uws_set_event_cb (server) : %s\n", ret_str);

    return;
}

/* unset callback */
static void
tst_unset_evt_callback()
{
    char *ret_str = TEST_OK;
    char *uri;

    /* unset callback */
    ico_uws_unset_event_cb(context);
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

    dbg_print("ico_uws_unset_event_cb (server) : %s\n", ret_str);

    return;
}

/* test main (server) */
static int
ico_uws_server_test(char *uri)
{
    /* create context */
    tst_create_context(uri);

    if (context) {
        /* set callback */
        tst_set_evt_callback();

        /* client does not connect */
        tst_get_ready_state(ICO_UWS_STATE_CONNECTING, "connecting");

        /* get uri */
        tst_get_uri(uri);

        /* service (loop) */
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
    char uri[256];
    int id;
    int set_uri_flag = UNSET_FLAG;

    for (id = 0; id < argc; id++) {
        if (strcmp(argv[id], "-p") == 0) {
            id++;
            sprintf(uri, ":%s", argv[id]);
            set_uri_flag = SET_FLAG;
        }
    }

    /* set default uri */
    if (set_uri_flag == UNSET_FLAG) {
        sprintf(uri, ":%s", SRV_PORT);
    }

    g_setenv("PKG_NAME", "org.tizen.ico.tst_ico_uws_server", 1);
    g_mainloop = g_main_loop_new(NULL, 0);

    printf("\n");
    printf("##### ico_uws API (server) Test Start #####\n");
    ico_uws_server_test(uri);
    printf("##### ico_uws API (server) Test End #####\n");
    printf("\n");

    g_timeout_add_seconds(2, exit_program, NULL);
    g_main_loop_run(g_mainloop);

    return 0;
}
