/******************************************************************************
 *
 *  Title:        messages.c
 *
 *  Description:  Messages that can be sent to/from the API server and client.
 *
 *****************************************************************************/


#include <stdio.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

#include "network.h"
#include "pololu.h"
#include "messages.h"
#include "labjack.h"
#include "pid.h"


/******************************************************************************
 *
 * Title:       void messages_send( int fd, int msg_id, MSG_DATA *msg )
 *
 * Description: Sends message to the UUV based on the message ID. Only integer
 *              types need conversion from hex to network byte ordering. Use
 *              ntohs() for short int and ntohl() for int.
 *
 * Input:       fd: Socket to send message to.
 *              msg_id: Message ID.
 *              msg: Pointer to message data.
 *
 * Output:      None.
 *
 *****************************************************************************/

void messages_send( int fd,
                    int msg_id,
                    MSG_DATA *msg
                  )
{
    switch ( msg_id ) {
        case OPEN_MSGID:
            msg->open.hdr.msgid = OPEN_MSGID;

            /* Actually send message here. */
            net_send( fd, &msg->open, sizeof( OPEN_MSG ) );
            break;

        case MSTRAIN_MSGID:
            msg->mstrain.hdr.msgid = MSTRAIN_MSGID;

            /* Use network byte order. */
            msg->mstrain.data.serial_number   = htonl( msg->mstrain.data.serial_number );
            msg->mstrain.data.eeprom_address  = htons( msg->mstrain.data.eeprom_address );
            msg->mstrain.data.eeprom_value    = htons( msg->mstrain.data.eeprom_value );

            /* Actually send message here. */
            net_send( fd, &msg->mstrain, sizeof(MSTRAIN_MSG) );

            /* Convert the values back to host byte order. */
            msg->mstrain.data.serial_number   = ntohl( msg->mstrain.data.serial_number );
            msg->mstrain.data.eeprom_address  = ntohs( msg->mstrain.data.eeprom_address );
            msg->mstrain.data.eeprom_value    = ntohs( msg->mstrain.data.eeprom_value );
            break;

        case STOP_MSGID:
            msg->stop.hdr.msgid = STOP_MSGID;

            /* Actually send message here. */
            net_send( fd, &msg->stop, sizeof(STOP_MSG) );
            break;

        case SERVO_MSGID:
            msg->servo.hdr.msgid = SERVO_MSGID;
            msg->servo.data.sync = SSC_SYNC;

            /* Actually send message here. */
            net_send( fd, &msg->servo, sizeof(SERVO_MSG) );
            break;

        case CLIENT_MSGID:
            msg->client.hdr.msgid = CLIENT_MSGID;

            /* Use network byte order. */
            msg->client.data.enable_servos  = htonl( msg->client.data.enable_servos );
            msg->client.data.enable_log     = htonl( msg->client.data.enable_log );
            msg->client.data.enable_imu     = htonl( msg->client.data.enable_imu );
            msg->client.data.imu_stab       = htonl( msg->client.data.imu_stab );
            msg->client.data.debug_level    = htonl( msg->client.data.debug_level );
            msg->client.data.dropper        = htonl( msg->client.data.dropper );

            /* Actually send message here. */
            net_send( fd, &msg->client, sizeof(CLIENT_MSG) );

            /* Convert the values back to host byte order. */
            msg->client.data.enable_servos  = ntohl( msg->client.data.enable_servos );
            msg->client.data.enable_log     = ntohl( msg->client.data.enable_log );
            msg->client.data.enable_imu     = ntohl( msg->client.data.enable_imu );
            msg->client.data.imu_stab       = ntohl( msg->client.data.imu_stab );
            msg->client.data.debug_level    = ntohl( msg->client.data.debug_level );
            msg->client.data.dropper        = ntohl( msg->client.data.dropper );
            break;

        case TARGET_MSGID:
            msg->target.hdr.msgid = TARGET_MSGID;

            /* Use network byte order. */
            msg->target.data.mode  = htonl( msg->target.data.mode );
            msg->target.data.task  = htonl( msg->target.data.task );
            msg->target.data.vision_status  = htonl( msg->target.data.vision_status );

            /* Actually send message here. */
            net_send( fd, &msg->target, sizeof(TARGET_MSG) );

            /* Convert the values back to host byte order. */
            msg->target.data.mode  = ntohl( msg->target.data.mode );
            msg->target.data.task  = ntohl( msg->target.data.task );
            msg->target.data.vision_status = ntohl( msg->target.data.vision_status );
            break;

        case GAIN_MSGID:
            msg->gain.hdr.msgid = GAIN_MSGID;

            /* Actually send message here. */
            net_send( fd, &msg->gain, sizeof(GAIN_MSG) );
            break;

        case STATUS_MSGID:
            msg->status.hdr.msgid = STATUS_MSGID;

            /* Use network byte order. */
            msg->status.data.pitch_period   = htonl( msg->status.data.pitch_period );
            msg->status.data.roll_period    = htonl( msg->status.data.roll_period );
            msg->status.data.yaw_period     = htonl( msg->status.data.yaw_period );
            msg->status.data.depth_period   = htonl( msg->status.data.depth_period );
            msg->status.data.fx_period   	= htonl( msg->status.data.fx_period );
            msg->status.data.fy_period   	= htonl( msg->status.data.fy_period );

            /* Actually send message here. */
            net_send( fd, &msg->status, sizeof(STATUS_MSG) );

            /* Convert the values back to host byte order. */
            msg->status.data.pitch_period   = ntohl( msg->status.data.pitch_period );
            msg->status.data.roll_period    = ntohl( msg->status.data.roll_period );
            msg->status.data.yaw_period     = ntohl( msg->status.data.yaw_period );
            msg->status.data.depth_period   = ntohl( msg->status.data.depth_period );
            msg->status.data.fx_period   	= ntohl( msg->status.data.fx_period );
            msg->status.data.fy_period   	= ntohl( msg->status.data.fy_period );
            break;

        case VISION_MSGID:
            msg->vision.hdr.msgid = VISION_MSGID;

            /* Use network byte order. */
            msg->vision.data.front_x    = htonl( msg->vision.data.front_x );
            msg->vision.data.front_y    = htonl( msg->vision.data.front_y );
            msg->vision.data.bottom_x   = htonl( msg->vision.data.bottom_x );
            msg->vision.data.bottom_y   = htonl( msg->vision.data.bottom_y );
			msg->vision.data.box1_x		= htonl( msg->vision.data.box1_x );
			msg->vision.data.box1_y		= htonl( msg->vision.data.box1_y );
			msg->vision.data.box2_x		= htonl( msg->vision.data.box2_x );
			msg->vision.data.box2_y		= htonl( msg->vision.data.box2_y );
			msg->vision.data.suitcase_x	= htonl( msg->vision.data.suitcase_x );
			msg->vision.data.suitcase_y	= htonl( msg->vision.data.suitcase_y );
			msg->vision.data.status		= htonl( msg->vision.data.status );
			msg->vision.data.confidence	= htonl( msg->vision.data.confidence );
			msg->vision.data.mode		= htonl( msg->vision.data.mode );

            /* Actually send message here. */
            net_send( fd, &msg->vision, sizeof(VISION_MSG) );

            /* Convert the values back to host byte order. */
            msg->vision.data.front_x    = ntohl( msg->vision.data.front_x );
            msg->vision.data.front_y    = ntohl( msg->vision.data.front_y );
            msg->vision.data.bottom_x   = ntohl( msg->vision.data.bottom_x );
            msg->vision.data.bottom_y   = ntohl( msg->vision.data.bottom_y );
			msg->vision.data.box1_x		= ntohl( msg->vision.data.box1_x );
			msg->vision.data.box1_y		= ntohl( msg->vision.data.box1_y );
			msg->vision.data.box2_x		= ntohl( msg->vision.data.box2_x );
			msg->vision.data.box2_y		= ntohl( msg->vision.data.box2_y );
			msg->vision.data.suitcase_x	= ntohl( msg->vision.data.suitcase_x );
			msg->vision.data.suitcase_y	= ntohl( msg->vision.data.suitcase_y );
			msg->vision.data.status		= ntohl( msg->vision.data.status );
			msg->vision.data.confidence	= ntohl( msg->vision.data.confidence );
			msg->vision.data.mode		= ntohl( msg->vision.data.mode );
            break;

        case TASK_MSGID:
            msg->task.hdr.msgid = TASK_MSGID;

			/* Use network byte order. */
			/*msg->task.data.num = htonl( msg->task.data.num );*/
			msg->task.data.task = htonl( msg->task.data.task );
			msg->task.data.subtask = htonl( msg->task.data.subtask );
			msg->task.data.course = htonl( msg->task.data.course );

            /* Actually send message here. */
            net_send( fd, &msg->task, sizeof(TASK_MSG) );

			/* Convert the values back to host byte order. */
			/*msg->task.data.num = ntohl( msg->task.data.num );*/
			msg->task.data.task = ntohl( msg->task.data.task );
			msg->task.data.subtask = ntohl( msg->task.data.subtask );
			msg->task.data.course = ntohl( msg->task.data.course );
            break;

        case LJ_MSGID:
            msg->lj.hdr.msgid = LJ_MSGID;

            /* Actually send message here. */
            net_send( fd, &msg->lj, sizeof(LJ_MSG) );
            break;

        case VSETTING_MSGID:
            msg->vsetting.hdr.msgid = VSETTING_MSGID;

            /* Use network byte order. */
            msg->vsetting.data.save_bframe = htonl( msg->vsetting.data.save_bframe );
            msg->vsetting.data.save_fframe = htonl( msg->vsetting.data.save_fframe );
            msg->vsetting.data.save_bvideo = htonl( msg->vsetting.data.save_bvideo );
            msg->vsetting.data.save_fvideo = htonl( msg->vsetting.data.save_fvideo );

            /* Actually send message here. */
            net_send( fd, &msg->vsetting, sizeof(VSETTING_MSG) );

            /* Convert the values back to host byte order. */
            msg->vsetting.data.save_bframe = ntohl( msg->vsetting.data.save_bframe );
            msg->vsetting.data.save_fframe = ntohl( msg->vsetting.data.save_fframe );
            msg->vsetting.data.save_bvideo = ntohl( msg->vsetting.data.save_bvideo );
            msg->vsetting.data.save_fvideo = ntohl( msg->vsetting.data.save_fvideo );

        case TELEOP_MSGID:
            msg->teleop.hdr.msgid = TELEOP_MSGID;

            /* Actually send message here. */
            net_send( fd, &msg->teleop, sizeof(TELEOP_MSG) );
            break;
    }
} /* end messages_send() */


/******************************************************************************
 *
 * Title:       int messages_decode( int fd,
 *                                   char *buf,
 *                                   MSG_DATA *msg
 * 								     int bytes
 * 									)
 *
 * Description: Called if data is received on the network buffer. The network
 *              data is decoded here and the appropriate variables are
 *              set. If enabled, the data is saved to the log file.
 *
 * Input:       fd: Network file descriptor.
 *              buf: A buffer to store network data.
 *              msg: A pointer to message data.
 * 				bytes: Number of bytes in buffer.
 *
 * Output:      bytes: Number of bytes remaining in buf.
 *
 *****************************************************************************/

int messages_decode( int fd, char *buf, MSG_DATA *msg, int bytes )
{
	/* Check that a full message is available in the buffer. */
	int ii = 0;
	int jj = 0;
	int found_header = FALSE;
	int found_footer = FALSE;
	int num_footers = 0;

	/* Decode messages in buffer. */
	while( bytes > 0 ) {
		/* Look for header. */
		for( ii = jj; ii < bytes; ii++ ) {
			if( buf[ii] == MSG_START ) {
				found_header = TRUE;
				break;
			}
		}

		/* Look for footer. */
		for( jj = ii; jj < bytes; jj++ ) {
			if( buf[jj] == MSG_END ) {
				found_footer = TRUE;
				num_footers++;
				break;
			}
		}

		/* Return if a header OR a footer are NOT found. */
		if( !found_header || !found_footer ) {
			return bytes;
		}

		/* Determine what message type was received. */
		switch ( ((HEADER *)buf)->msgid ) {
		case OPEN_MSGID:
			msg->open.hdr.msgid = ((HEADER *)buf)->msgid;

			bytes -= sizeof(OPEN_MSG);
			memmove( msg, msg, sizeof(OPEN_MSG) );
			break;

		case MSTRAIN_MSGID:
			msg->mstrain.data = ((MSTRAIN_MSG *)buf)->data;

			/* Convert from network to host byte order. */
			msg->mstrain.data.serial_number  = ntohl( msg->mstrain.data.serial_number );
			msg->mstrain.data.eeprom_address = ntohs( msg->mstrain.data.eeprom_address );
			msg->mstrain.data.eeprom_value   = ntohs( msg->mstrain.data.eeprom_value );

			bytes -= sizeof(MSTRAIN_MSG);
			memmove( msg, msg, sizeof(MSTRAIN_MSG) );
			break;

		case STOP_MSGID:
			msg->stop.data = ((STOP_MSG *)buf)->data;

			bytes -= sizeof(STOP_MSG);
			memmove( msg, msg, sizeof(STOP_MSG) );
			break;

		case SERVO_MSGID:
			msg->servo.data = ((SERVO_MSG *)buf)->data;

			bytes -= sizeof(SERVO_MSG);
			memmove( msg, msg, sizeof(SERVO_MSG) );
			break;

		case CLIENT_MSGID:
			msg->client.data = ((CLIENT_MSG *)buf)->data;

			/* Convert from network to host byte order. */
			msg->client.data.enable_servos = ntohl( msg->client.data.enable_servos );
			msg->client.data.enable_log    = ntohl( msg->client.data.enable_log );
			msg->client.data.enable_imu    = ntohl( msg->client.data.enable_imu );
			msg->client.data.imu_stab      = ntohl( msg->client.data.imu_stab );
			msg->client.data.debug_level   = ntohl( msg->client.data.debug_level );
			msg->client.data.dropper       = ntohl( msg->client.data.dropper );

			bytes -= sizeof(CLIENT_MSG);
			memmove( msg, msg, sizeof(CLIENT_MSG) );
			break;

		case TARGET_MSGID:
			msg->target.data = ((TARGET_MSG *)buf)->data;

			/* Convert from network to host byte order. */
			msg->target.data.mode  = ntohl( msg->target.data.mode );
			msg->target.data.task  = ntohl( msg->target.data.task );
			msg->target.data.vision_status  = ntohl( msg->target.data.vision_status );

			bytes -= sizeof(TARGET_MSG);
			memmove( msg, msg, sizeof(TARGET_MSG) );
			break;

		case GAIN_MSGID:
			msg->gain.data = ((GAIN_MSG *)buf)->data;

			bytes -= sizeof(GAIN_MSG);
			memmove( msg, msg, sizeof(GAIN_MSG) );
			break;

		case STATUS_MSGID:
			msg->status.data = ((STATUS_MSG *)buf)->data;

			bytes -= sizeof(STATUS_MSG);
			memmove( msg, msg, sizeof(STATUS_MSG) );
			break;

		case VISION_MSGID:
			msg->vision.data = ((VISION_MSG *)buf)->data;

			/* Convert from network to host byte order. */
			msg->vision.data.front_x    = ntohl( msg->vision.data.front_x );
			msg->vision.data.front_y    = ntohl( msg->vision.data.front_y );
			msg->vision.data.bottom_x   = ntohl( msg->vision.data.bottom_x );
			msg->vision.data.bottom_y   = ntohl( msg->vision.data.bottom_y );
			msg->vision.data.box1_x		= ntohl( msg->vision.data.box1_x );
			msg->vision.data.box1_y		= ntohl( msg->vision.data.box1_y );
			msg->vision.data.box2_x		= ntohl( msg->vision.data.box2_x );
			msg->vision.data.box2_y		= ntohl( msg->vision.data.box2_y );
			msg->vision.data.suitcase_x	= ntohl( msg->vision.data.suitcase_x );
			msg->vision.data.suitcase_y	= ntohl( msg->vision.data.suitcase_y );
			msg->vision.data.status		= ntohl( msg->vision.data.status );
			msg->vision.data.confidence = ntohl( msg->vision.data.confidence );
			msg->vision.data.mode		= ntohl( msg->vision.data.mode );

			bytes -= sizeof(VISION_MSG);
			memmove( msg, msg, sizeof(VISION_MSG) );
			break;

		case TASK_MSGID:
			msg->task.data = ((TASK_MSG *)buf)->data;

			/* Convert from network to host byte order. */
			/*msg->task.data.num = ntohl( msg->task.data.num);*/
			msg->task.data.task = ntohl( msg->task.data.task);
			msg->task.data.subtask = ntohl( msg->task.data.subtask);
			msg->task.data.course = ntohl( msg->task.data.course);

			bytes -= sizeof(TASK_MSG);
			memmove( msg, msg, sizeof(TASK_MSG) );
			break;

		case LJ_MSGID:
			msg->lj.data = ((LJ_MSG *)buf)->data;

			bytes -= sizeof(LJ_MSG);
			memmove( msg, msg, sizeof(LJ_MSG) );
			break;

		case VSETTING_MSGID:
			msg->vsetting.data = ((VSETTING_MSG *)buf)->data;

			/* Convert from network to host byte order. */
			msg->vsetting.data.save_bframe = ntohl( msg->vsetting.data.save_bframe );
			msg->vsetting.data.save_fframe = ntohl( msg->vsetting.data.save_fframe );
			msg->vsetting.data.save_bvideo = ntohl( msg->vsetting.data.save_bvideo );
			msg->vsetting.data.save_fvideo = ntohl( msg->vsetting.data.save_fvideo );

			bytes -= sizeof(VSETTING_MSG);
			memmove( msg, msg, sizeof(VSETTING_MSG) );
			break;

		case TELEOP_MSGID:
			msg->teleop.data = ((TELEOP_MSG *)buf)->data;

			bytes -= sizeof(TELEOP_MSG);
			memmove( msg, msg, sizeof(TELEOP_MSG) );
			break;
		}
	}

	return bytes;
} /* end messages_decode() */


/******************************************************************************
 *
 * Title:       void messages_update( MSG_DATA *msg )
 *
 * Description: Updates the status variables.
 *
 * Input:       msg: A pointer to message data.
 *
 * Output:      None.
 *
 *****************************************************************************/

void messages_update( MSG_DATA *msg )
{
    msg->status.data.mag[0]      = msg->mstrain.data.mag[0];
    msg->status.data.mag[1]      = msg->mstrain.data.mag[1];
    msg->status.data.mag[2]      = msg->mstrain.data.mag[2];
    msg->status.data.accel[0]    = msg->mstrain.data.accel[0];
    msg->status.data.accel[1]    = msg->mstrain.data.accel[1];
    msg->status.data.accel[2]    = msg->mstrain.data.accel[2];
    msg->status.data.ang_rate[0] = msg->mstrain.data.ang_rate[0];
    msg->status.data.ang_rate[1] = msg->mstrain.data.ang_rate[1];
    msg->status.data.ang_rate[2] = msg->mstrain.data.ang_rate[2];
    msg->status.data.quat[0]     = msg->mstrain.data.quat[0];
    msg->status.data.quat[1]     = msg->mstrain.data.quat[1];
    msg->status.data.quat[2]     = msg->mstrain.data.quat[2];
    msg->status.data.quat[3]     = msg->mstrain.data.quat[3];
    msg->status.data.pitch       = msg->mstrain.data.pitch;
    msg->status.data.roll        = msg->mstrain.data.roll;
    msg->status.data.yaw         = msg->mstrain.data.yaw;
} /* end messages_update() */


/******************************************************************************
 *
 * Title:       void messages_init( MSG_DATA *msg )
 *
 * Description: Initialize the header and footer variables.
 *
 * Input:       msg: A pointer to message data.
 *
 * Output:      None.
 *
 *****************************************************************************/

void messages_init( MSG_DATA *msg )
{
	/* Set the header and footer for each of the message types. */
	msg->open.hdr.msgstart		= MSG_START;
	msg->mstrain.hdr.msgstart	= MSG_START;
	msg->servo.hdr.msgstart		= MSG_START;
	msg->client.hdr.msgstart	= MSG_START;
	msg->target.hdr.msgstart	= MSG_START;
	msg->gain.hdr.msgstart		= MSG_START;
	msg->status.hdr.msgstart	= MSG_START;
	msg->vision.hdr.msgstart	= MSG_START;
	msg->task.hdr.msgstart		= MSG_START;
	msg->stop.hdr.msgstart		= MSG_START;
	msg->lj.hdr.msgstart		= MSG_START;
	msg->vsetting.hdr.msgstart	= MSG_START;
	msg->teleop.hdr.msgstart	= MSG_START;
	msg->open.ftr.msgend		= MSG_END;
	msg->mstrain.ftr.msgend		= MSG_END;
	msg->servo.ftr.msgend		= MSG_END;
	msg->client.ftr.msgend		= MSG_END;
	msg->target.ftr.msgend		= MSG_END;
	msg->gain.ftr.msgend		= MSG_END;
	msg->status.ftr.msgend		= MSG_END;
	msg->vision.ftr.msgend		= MSG_END;
	msg->task.ftr.msgend		= MSG_END;
	msg->stop.ftr.msgend		= MSG_END;
	msg->lj.ftr.msgend			= MSG_END;
	msg->vsetting.ftr.msgend	= MSG_END;
	msg->teleop.ftr.msgend		= MSG_END;
} /* end messages_init() */
