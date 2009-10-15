/**
 *  \file messages.h
 *  \brief Messages that can be sent to/from the API server and client.
 */

#ifndef _MESSAGES_H_
#define _MESSAGES_H_

#include "microstrain.h"
#include "labjack.h"
#include "msgtypes.h"


/******************************
**
** #defines
**
******************************/

#ifndef TRUE
#define TRUE 1
#endif /* TRUE */

#ifndef FALSE
#define FALSE 0
#endif /* FALSE */

#ifndef STRING_SIZE
#define STRING_SIZE 64
#endif /* STRING_SIZE */

#ifndef MAX_MSG_SIZE
#define MAX_MSG_SIZE 65536
#endif /* MAX_MSG_SIZE */

#ifndef SSC_SYNC
#define SSC_SYNC 255
#endif /* SSC_SYNC */

#ifndef GETSETGAIN
#define GAIN_SET 1
#define GAIN_GET 2
#endif /* GETSETGAIN */

#ifndef API_MSGID
/** @name Message IDs for API commands. */
//@{
#define API_MSGID
#define OPEN_MSGID          1
#define MSTRAIN_MSGID       2
#define SERVO_MSGID         3
#define CLIENT_MSGID        4
#define TARGET_MSGID        5
#define GAIN_MSGID          6
#define STATUS_MSGID        7
#define VISION_MSGID        8
#define STOP_MSGID          9
#define TASK_MSGID          10
#define VSETTING_MSGID      11
#define LJ_MSGID            12
#define TELEOP_MSGID		13
//@}
#endif /* API_MSGID */

/* Operating modes. */
#ifndef OPERATING_MODES
/** @name Valid operational modes for the uuv. */
//@{
#define OPERATING_MODES
#define HOLD_YAW        1
#define HOLD_ROLL       2
#define HOLD_PITCH      3
#define HOLD_ACCEL      4
#define HOLD_ANG_RATE   5
#define MANUAL          6
#define AUTONOMOUS      7
#define ZERO_PID_ERRORS 8
//@}
#endif /* OPERATING_MODES */

/* Header and footer. */
#ifndef MSG_START
#define MSG_START 'A'
#endif /* MSG_START */

#ifndef MSG_END
#define MSG_END 'Z'
#endif /* MSG_END */


/******************************
**
** Function prototypes
**
******************************/

//! Send the API message to the server.
//! \param fd Socket to send message to.
//! \param msg_id Message ID.
//! \param msg Pointer to message data.
void messages_send( int fd, int msg_id, MSG_DATA *msg );

//! Decode received API messages.
//! \param fd Network file descriptor.
//! \param buf A buffer to store network data.
//! \param msg Pointer to message data.
//! \param bytes Number of bytes in buffer.
//! \return Number of bytes remaining in buffer.
int messages_decode( int fd, char *buf, MSG_DATA *msg, int bytes );

//! Updates status data with current data.
//! \param msg Pointer to message data.
void messages_update( MSG_DATA *msg );

//! Initialize the header and footer variables.
//! \param msg Pointer to message data.
void messages_init( MSG_DATA *msg );


#endif /* _MESSAGES_H_ */
