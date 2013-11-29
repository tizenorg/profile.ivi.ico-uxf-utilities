/*
 * Copyright (c) 2013, TOYOTA MOTOR CORPORATION.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 */

/*========================================================================*/    
/**
 *  @file   tst_ico_log.c
 *
 *  @brief  ico_log packege test program
 */
/*========================================================================*/    

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ico_log.h"

int
main(int argc, char **argv)
{
	int debug_level = 0;

	if (2 > argc) {
		printf("usage: tst_ico_log loglevel\n"
			   "\tloglevel=none\n"
			   "\tloglevel=[performance,][trace,][debug,][info],"
			   "[warning,][critical,][error]\n");
		exit(-1);
	}

	if (NULL != strstr(argv[1], "performance")) {
		debug_level |= ICO_LOG_LVL_PRF;
	}

	if (NULL != strstr(argv[1], "trace")) {
		debug_level |= ICO_LOG_LVL_TRA;
	}

	if (NULL != strstr(argv[1], "debug")) {
		debug_level |= ICO_LOG_LVL_DBG;
	}

	if (NULL != strstr(argv[1], "info")) {
		debug_level |= ICO_LOG_LVL_INF;
	}

	if (NULL != strstr(argv[1], "warning")) {
		debug_level |= ICO_LOG_LVL_WRN;
	}

	if (NULL != strstr(argv[1], "critical")) {
		debug_level |= ICO_LOG_LVL_CRI;
	}

	if (NULL != strstr(argv[1], "error")) {
		debug_level |= ICO_LOG_LVL_ERR;
	}

	ico_log_set_level(debug_level);
	char filename[128];
    sprintf(filename, "test_result_ico_log_%08X", debug_level);
	ico_log_open(filename);

	printf("debug_level=0x%08X\n", debug_level);

	ICO_PRF("test performance log");
	ICO_TRA("test trace       log");
	ICO_DBG("test debug       log");
	ICO_INF("test info        log");
	ICO_WRN("test warning     log");
	ICO_CRI("test critical    log");
	ICO_ERR("test error       log");

	ico_log_close();

	exit(0);
}
