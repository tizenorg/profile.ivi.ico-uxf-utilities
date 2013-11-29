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
 *  @file   ico_log.h
 *
 *  @brief  debug log function
 */
/*========================================================================*/
#ifndef __ICO_LOG_H__
#define __ICO_LOG_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifndef ICO_LOG_STDOUT
#define ICO_LOG_STDOUT      0
                            /**!< Log output to stdout(=1) or file(=0) */
#endif /*ICO_APF_LOG_STDOUT*/

#define ICO_LOG_DIR         "/var/log/ico"
                           /**!< Directory name of Log destination */

/* Maximum lines/files */
#define ICO_LOG_MAXLINES    20000
                            /**!< Maximum output lines of log file  */
#define ICO_LOG_MAXFILES    6
                            /**!< Maximum number of the log file    */

/* Log output level */
#define ICO_LOG_LVL_PRF     0x0200  /**!< Performance */
#define ICO_LOG_LVL_TRA     0x0100  /**!< Trace       */
#define ICO_LOG_LVL_DBG     0x0080  /**!< Debug write */
#define ICO_LOG_LVL_INF     0x0040  /**!< Information */
#define ICO_LOG_LVL_WRN     0x0010  /**!< Warning     */
#define ICO_LOG_LVL_CRI     0x0008  /**!< Critical    */
#define ICO_LOG_LVL_ERR     0x0004  /**!< Error       */

/* Log output flush */
#define ICO_LOG_FLUSH       0x4000  /**!< Log outout with log flush     */
#define ICO_LOG_NOFLUSH     0x2000  /**!< Log output without log flush  */

#define ICO_PRF(fmt,...)                        \
{                                               \
    ico_log_print(ICO_LOG_LVL_PRF,              \
                  "%s> " fmt " (%s,%s:%d)\n",   \
                  ico_get_str_cur_time("PRF"),  \
                  ##__VA_ARGS__,                \
                  __func__,                     \
                  __FILE__,                     \
                  __LINE__);                    \
}

#define ICO_TRA(fmt,...)                        \
{                                               \
    ico_log_print(ICO_LOG_LVL_TRA,              \
                  "%s> " fmt " (%s,%s:%d)\n",   \
                  ico_get_str_cur_time("TRA"),  \
                  ##__VA_ARGS__,                \
                  __func__,                     \
                  __FILE__,                     \
                  __LINE__);                    \
}

#define ICO_DBG(fmt,...)                        \
{                                               \
    ico_log_print(ICO_LOG_LVL_DBG,              \
                  "%s> " fmt " (%s,%s:%d)\n",   \
                  ico_get_str_cur_time("DBG"),  \
                  ##__VA_ARGS__,                \
                  __func__,                     \
                  __FILE__,                     \
                  __LINE__);                    \
}

#define ICO_INF(fmt,...)                        \
{                                               \
    ico_log_print(ICO_LOG_LVL_INF,              \
                  "%s> " fmt " (%s,%s:%d)\n",   \
                  ico_get_str_cur_time("INF"),  \
                  ##__VA_ARGS__,                \
                  __func__,                     \
                  __FILE__,                     \
                  __LINE__);                    \
}

#define ICO_WRN(fmt,...)                        \
{                                               \
    ico_log_print(ICO_LOG_LVL_WRN,              \
                  "%s> " fmt " (%s,%s:%d)\n",   \
                  ico_get_str_cur_time("WRN"),  \
                  ##__VA_ARGS__,                \
                  __func__,                     \
                  __FILE__,                     \
                  __LINE__);                    \
}

#define ICO_CRI(fmt,...)                        \
{                                               \
    ico_log_print(ICO_LOG_LVL_CRI,              \
                  "%s> " fmt " (%s,%s:%d)\n",   \
                  ico_get_str_cur_time("CRI"),  \
                  ##__VA_ARGS__,                \
                  __func__,                     \
                  __FILE__,                     \
                  __LINE__);                    \
}

#define ICO_ERR(fmt,...)                        \
{                                               \
    ico_log_print(ICO_LOG_LVL_ERR,              \
                  "%s> " fmt " (%s,%s:%d)\n",   \
                  ico_get_str_cur_time("ERR"),  \
                  ##__VA_ARGS__,                \
                  __func__,                     \
                  __FILE__,                     \
                  __LINE__);                    \
}


void ico_log_print(int level, const char *fmt, ...);
void ico_log_open(const char *prog);
void ico_log_close(void);
void ico_log_flush(void);
char * ico_get_str_cur_time(const char *level);
void ico_log_set_level(int level);

#ifdef __cplusplus
}
#endif

#endif  // __ICO__LOG_H__
/* vim:set expandtab ts=4 sw=4: */
