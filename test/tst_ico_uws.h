/*
 * Copyright (c) 2013, TOYOTA MOTOR CORPORATION.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 */

#ifndef __TST_ICO_UWS_H__
#define __TST_ICO_UWS_H__

#ifdef __cplusplus
extern "C" {
#endif

#define dbg_print(fmt, ...) \
 printf("[TestCase] " fmt, __VA_ARGS__);

#define TEST_OK "OK"
#define TEST_NG "NG"

#define SET_FLAG    1
#define UNSET_FLAG  0

#define SRV_URI         "ws://127.0.0.1"
#define SRV_PORT        "8080"
#define PROTOCOL_NAME   "test_protocol"

#define CLT_DATA        "test data from client"

#define MAX_DATA_NUM 2

char *srv_ports[MAX_DATA_NUM]   = {SRV_PORT, "9090"};
char *clt_datas[MAX_DATA_NUM]   = {CLT_DATA, "test to send data"};

#ifdef __cplusplus
}
#endif

#endif /* __TST_ICO_UWS_H__ */
