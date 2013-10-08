/*
 * Copyright (c) 2013, TOYOTA MOTOR CORPORATION.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 */

/*------------------------------------------------------------------------*/
/**
 *  @file   ico_log.c
 *
 *  @brief
 */
/*------------------------------------------------------------------------*/

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include "ico_log.h"

/*============================================================================*/
/* global API                                                                 */
/*============================================================================*/
#if defined(__GNUC__) && __GNUC__ >= 4
#define ICO_API __attribute__ ((visibility("default")))
#else
#define ICO_API
#endif

/*========================================================================*/
/**
 *  static variable
 */
/*========================================================================*/

static int  time_zone    = 99*60*60;    /*!< local time difference(sec)       */
static int  log_level    = 0x7FFFFFFF;  /*!< output level debug log           */
static bool flush_mode   = true;        /*!< flush mode flag                  */
static bool initialized  = false;       /*!< initialized flag                 */
static FILE *log_fd      = NULL;        /*!< file descriptor of output debug log*/
static int  log_stdout   = 0;           /*!< flag for log output to stdout    */
static int  log_lines    = 0;           /*!< output lines                     */
static char log_prog[32] = {0,};        /*!< name of output source module     */


/*------------------------------------------------------------------------*/
/**
 *  @brief  printout log message
 *
 *  @param [in] level   log output level
 *  @param [in] fmt     message format(same as printf)
 *  @param [in] ...     arguments if need
 */
/*------------------------------------------------------------------------*/
ICO_API void
ico_log_print(int level, const char *fmt, ...)
{
    if (log_level < level) {
        return;
    }
    va_list     list;

    if (NULL == log_fd) {
        ico_log_open(NULL);
    }
#if ICO_APF_LOG_STDOUT == 0
    else if (log_lines >= (ICO_LOG_MAXLINES-2)) {
        if (log_lines >= ICO_LOG_MAXLINES)  {
            ico_log_close();
            ico_log_open(log_prog);
        }
        else    {
            fflush(log_fd);
        }
    }
#endif /*ICO_APF_LOG_STDOUT*/
    if (NULL != log_fd) {
        va_start(list, fmt);
        vfprintf(log_fd, fmt, list);
        va_end(list);
        if (flush_mode)  {
            fflush(log_fd);
        }
    }
    if (log_stdout == 0)    {
        log_lines ++;
    }
}

/*------------------------------------------------------------------------*/
/**
 *  @brief  open log file
 *
 *  @param [in] prog    program name
 */
/*------------------------------------------------------------------------*/
ICO_API void
ico_log_open(const char *prog)
{
#if ICO_LOG_STDOUT == 0
    int     idx;
    char    sPath[128];
    char    sPath2[128];
#endif /*ICO_LOG_STDOUT*/

    if (NULL != log_fd) {
        fflush(log_fd);
        if (log_stdout == 0)    {
            fclose(log_fd);
        }
    }

    log_lines = 0;

    if ((! prog) || (*prog == 0)) {
        log_stdout = 1;
        log_fd = stdout;
        log_prog[0] = 0;
        return;
    }
    else {
        strncpy(log_prog, prog, sizeof(log_prog)-1);
        log_prog[sizeof(log_prog)-1] = 0;
    }
#if ICO_LOG_STDOUT > 0
    log_stdout = 1;
    log_fd = stdout;
#else  /*ICO_LOG_STDOUT*/
    snprintf(sPath, sizeof(sPath)-1, "%s/%s.log%d",
             ICO_LOG_DIR, log_prog, ICO_LOG_MAXFILES-1);
    (void)remove(sPath);

    for (idx = (ICO_LOG_MAXFILES-1); idx > 0; idx--) {
        strcpy(sPath2, sPath);
        if (idx > 1)    {
            snprintf(sPath, sizeof(sPath)-1, "%s/%s.log%d",
                     ICO_LOG_DIR, log_prog, idx-1);
        }
        else    {
            snprintf(sPath, sizeof(sPath)-1, "%s/%s.log",
                     ICO_LOG_DIR, log_prog);
        }
        (void)rename(sPath, sPath2);
    }

    log_fd = fopen(sPath, "w");
    if (NULL == log_fd) {
        log_stdout = 1;
        log_fd = stdout;
    }
    else if ((initialized == false) &&
             (log_fd != stdout) && (log_fd != stderr)) {
        initialized = true;
        fflush(stdout);
        fflush(stderr);
        stdout = log_fd;
        stderr = log_fd;
    }
#endif /*ICO_LOG_STDOUT*/
}

/*------------------------------------------------------------------------*/
/**
 *  @brief  close log file
 */
/*------------------------------------------------------------------------*/
ICO_API void
ico_log_close(void)
{
#if ICO_LOG_STDOUT == 0
    if (NULL != log_fd) {
        fflush(log_fd);
        if (log_stdout == 0)    {
            fclose(log_fd);
        }
        log_fd = (FILE *)NULL;
    }
#endif /*ICO_LOG_STDOUT*/
}

/*------------------------------------------------------------------------*/
/**
 *  @brief  flush log file
 */
/*------------------------------------------------------------------------*/
ICO_API void
ico_log_flush(void)
{
    if ((NULL != log_fd) && (false == flush_mode)) {
        fflush(log_fd);
    }
}

/*------------------------------------------------------------------------*/
/**
 *  @brief   get current time string
 *
 *  @param [in] level   log level string(header of log message)
 *  @return current time string
 */
/*------------------------------------------------------------------------*/
ICO_API char *
ico_get_str_cur_time(const char *level)
{
    struct timeval  NowTime;
    extern long     timezone;
    static char     sBuf[28];

    gettimeofday(&NowTime, (struct timezone *)0);
    if (time_zone > (24*60*60)) {
        tzset();
        time_zone = timezone;
    }
    NowTime.tv_sec -= time_zone;

    sprintf(sBuf, "%02d:%02d:%02d.%03d[%s]@%d",
            (int)((NowTime.tv_sec/3600) % 24),
            (int)((NowTime.tv_sec/60) % 60),
            (int)(NowTime.tv_sec % 60),
            (int)NowTime.tv_usec/1000, level, getpid());

    return sBuf;
}

/*------------------------------------------------------------------------*/
/**
 *  @brief  set log output level
 *
 *  @param [in] level   log output level
 */
/*------------------------------------------------------------------------*/
ICO_API void
ico_log_set_level(int level)
{
    log_level = level & (~(ICO_LOG_FLUSH|ICO_LOG_NOFLUSH));

    if (log_level & (ICO_LOG_FLUSH|ICO_LOG_NOFLUSH)) {
        if (log_level & ICO_LOG_FLUSH) {
            flush_mode = true;
        }
        else    {
            flush_mode = false;
        }
    }
}
/* vim:set expandtab ts=4 sw=4: */
