/*
 * Copyright (c) 2013, TOYOTA MOTOR CORPORATION.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 */
/**
 * @brief   header file of library for communicate
 *
 * @date    June-7-2013
 */

#ifndef __ICO_UWS_H__
#define __ICO_UWS_H__

#ifdef __cplusplus
extern "C" {
#endif

/*============================================================================*/
/* definition                                                                 */
/*============================================================================*/
struct ico_uws_context;

typedef enum {
    ICO_UWS_STATE_CONNECTING    = 0,
    ICO_UWS_STATE_OPEN          = 1,
    ICO_UWS_STATE_CLOSING       = 2,
    ICO_UWS_STATE_CLOSED        = 3,
    ICO_UWS_STATE_UNKNOWN       = -1
} ico_uws_state_e;

typedef enum {
    ICO_UWS_EVT_OPEN    = 100,
    ICO_UWS_EVT_ERROR   = 101,
    ICO_UWS_EVT_CLOSE   = 102,
    ICO_UWS_EVT_RECEIVE = 103,
    ICO_UWS_EVT_ADD_FD  = 104,
    ICO_UWS_EVT_DEL_FD  = 105
} ico_uws_evt_e;

typedef enum {
    ICO_UWS_ERR_NONE            = 0,    /**< success */
    ICO_UWS_ERR_CREATE          = -201,
    ICO_UWS_ERR_CONNECT         = -202,
    ICO_UWS_ERR_CLOSED          = -203,
    ICO_UWS_ERR_SEND            = -204,
    ICO_UWS_ERR_INVALID_PARAM   = -205,
    ICO_UWS_ERR_OUT_OF_MEMORY   = -206,
    ICO_UWS_ERR_UNKNOWN         = -300,
} ico_uws_error_e;

typedef union {
    struct {
        void            *recv_data;
        size_t          recv_len;
    } _ico_uws_message;
    struct {
        ico_uws_error_e code;
    } _ico_uws_error;
    struct {
        int             fd;
    } _ico_uws_fd;
} ico_uws_detail;

typedef void (*ico_uws_evt_cb)
             (const struct ico_uws_context *context,
              const ico_uws_evt_e event,
              const void *id,
              const ico_uws_detail *detail,
              void *user_data);

/*============================================================================*/
/* functions                                                                  */
/*============================================================================*/
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
struct ico_uws_context *ico_uws_create_context(const char *uri,
                                               const char *protocol);

/*--------------------------------------------------------------------------*/
/**
 * @brief   ico_uws_close
 *          Close the connection and destroy the ico_uws context.
 *
 * @param[in]   context             ico_uws context
 * @return      none
 */
/*--------------------------------------------------------------------------*/
void ico_uws_close(struct ico_uws_context *context);

/*--------------------------------------------------------------------------*/
/**
 * @brief   ico_uws_send
 *          Send data to which to connect.
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
void ico_uws_send(struct ico_uws_context *context, void *id,
                  unsigned char *data, size_t len);

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
void ico_uws_service(struct ico_uws_context *context);

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
char *ico_uws_get_uri(struct ico_uws_context *context);

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
ico_uws_state_e ico_uws_get_ready_state(struct ico_uws_context *context);

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
int ico_uws_set_event_cb(struct ico_uws_context *context, ico_uws_evt_cb callback,
                         void *user_data);

/*--------------------------------------------------------------------------*/
/**
 * @brief   ico_uws_unset_event_cb
 *          Unset the event callback function.
 *
 * @param[in]   context             ico_uws context
 * @return      none
 */
/*--------------------------------------------------------------------------*/
void ico_uws_unset_event_cb(struct ico_uws_context *context);


#ifdef __cplusplus
}
#endif

#endif /* __ICO_UWS_H__ */
