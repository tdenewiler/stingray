/******************************************************************************
 *
 *  Title:        pid.c
 *
 *  Description:  Proportional - Integral - Dervivative controller functions.
 *
 *****************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

#include "pid.h"
#include "microstrain.h"
#include "parser.h"
#include "labjack.h"
#include "pololu.h"
#include "util.h"


/******************************************************************************
 *
 * Title:       void pid_init( PID *pid, CONF_VARS *cf )
 *
 * Description: Initializes PID values.
 *
 * Input:       pid: Pointer to PID data.
 * 				cf: Pointer to configuration variables.
 *
 * Output:      0 on success, -1 on error.
 *
 * Globals:     None.
 *
 *****************************************************************************/

int pid_init( PID *pid, CONF_VARS *cf )
{
	pid->roll.ref		= cf->target_roll;
	pid->roll.kp		= cf->kp_roll;
	pid->roll.ki		= cf->ki_roll;
	pid->roll.kd		= cf->kd_roll;
	pid->roll.period	= cf->period_roll;

	pid->pitch.ref		= cf->target_pitch;
	pid->pitch.kp		= cf->kp_pitch;
	pid->pitch.ki		= cf->ki_pitch;
	pid->pitch.kd		= cf->kd_pitch;
	pid->pitch.period	= cf->period_pitch;

	pid->yaw.ref		= cf->target_yaw;
	pid->yaw.kp			= cf->kp_yaw;
	pid->yaw.ki			= cf->ki_yaw;
	pid->yaw.kd			= cf->kd_yaw;
	pid->yaw.period		= cf->period_yaw;

	pid->depth.ref		= cf->target_depth;
	pid->depth.kp		= cf->kp_depth;
	pid->depth.ki		= cf->ki_depth;
	pid->depth.kd		= cf->kd_depth;
	pid->depth.period	= cf->period_depth;

	pid->voith_angle	= 0;
	pid->voith_speed	= 0;
	pid->voith_thrust	= 0;
	pid->pitch_torque	= 0;
	pid->roll_torque	= 0;
	pid->yaw_torque		= 0;

	return 0;
} /* end pid_init() */


/******************************************************************************
 *
 * Title:       void pid_loop( int pololu_fd,
 * 								PID *pid,
 * 								CONF_VARS *cf,
 * 								MSG_DATA *msg,
 * 								LABJACK_DATA *lj,
 * 								int mode
 * 								)
 *
 * Description: Runs through one iteration of a PID controller.
 *
 * Input:		pololu_fd: Pololu file descriptor.
 * 				pid: PID struct.
 * 				cf: Pointer to configuration variables.
 * 				msg: Pointer to message data.
 * 				lj: Pointer to labjack data.
 * 				dt: Time difference from last loop run.
 * 				mode: Which PID loop to run.
 *
 * Output:      None.
 *
 * Globals:     None.
 *
 *****************************************************************************/

void pid_loop( int pololu_fd,
               PID *pid,
               CONF_VARS *cf,
               MSG_DATA *msg,
               LABJACK_DATA *lj,
               int dt,
               int mode
             )
{
	/* These next three need to be set from vison, hydrophone, gui, etc. */
	pid->voith_angle = atan2f( msg->target.data.fy, msg->target.data.fx );
	pid->voith_speed = msg->target.data.speed;
	pid->voith_thrust = sqrt( msg->target.data.fx * msg->target.data.fx +
	                          msg->target.data.fy * msg->target.data.fy );

	int r1 = -1;
	int r2 = -1;

	switch ( mode ) {

		case PID_PITCH:
			/* Update the gains. */
			pid->pitch.kp	= msg->gain.data.kp_pitch;
			pid->pitch.ki	= msg->gain.data.ki_pitch;
			pid->pitch.kd	= msg->gain.data.kd_pitch;

			/* Calculate the errors. */
			pid->pitch.ref	= msg->target.data.pitch;
			pid->pitch.cval	= msg->mstrain.data.pitch;
			pid->pitch.perr = pid_subtract_angles( pid->pitch.cval, pid->pitch.ref );
			pid->pitch.ierr += pid->pitch.perr * dt / 1000000;
			pid->pitch.ierr = pid_bound_integral( pid->pitch.ierr, pid->pitch.ki, PID_PITCH_INTEGRAL );
			pid->pitch.derr = msg->mstrain.data.ang_rate[0];

			/* Update status message. */
			msg->status.data.pitch_perr	= pid->pitch.perr;
			msg->status.data.pitch_ierr	= pid->pitch.ierr;
			msg->status.data.pitch_derr	= pid->pitch.derr;

			/* PID equations. */
			pid->pitch_torque =	pid->pitch.kp * pid->pitch.perr +
			                    pid->pitch.ki * pid->pitch.ierr +
			                    pid->pitch.kd * pid->pitch.derr;

			/* Check bounds. */
			if ( fabsf( pid->pitch_torque ) > PID_PITCH_TORQUE ) {
				pid->pitch_torque = util_sign_value( pid->pitch_torque ) * PID_PITCH_TORQUE;
			}

			break;

		case PID_ROLL:
			/* Update the gains. */
			pid->roll.kp	= msg->gain.data.kp_roll;
			pid->roll.ki	= msg->gain.data.ki_roll;
			pid->roll.kd	= msg->gain.data.kd_roll;

			/* Calculate the errors. */
			pid->roll.ref	= msg->target.data.roll;
			pid->roll.cval	= msg->mstrain.data.roll;
			pid->roll.perr	= pid_subtract_angles( pid->roll.cval, pid->roll.ref );
			pid->roll.ierr	+= pid->roll.perr * dt / 1000000;
			pid->roll.ierr	= pid_bound_integral( pid->roll.ierr, pid->roll.ki, PID_ROLL_INTEGRAL );
			pid->roll.derr	= msg->mstrain.data.ang_rate[1];

			/* Update status message. */
			msg->status.data.roll_perr	= pid->roll.perr;
			msg->status.data.roll_ierr	= pid->roll.ierr;
			msg->status.data.roll_derr	= pid->roll.derr;

			/* PID equations. */
			pid->roll_torque =	pid->roll.kp * pid->roll.perr +
			                   pid->roll.ki * pid->roll.ierr +
			                   pid->roll.kd * pid->roll.derr;

			/* Check bounds. */
			if ( fabsf( pid->roll_torque ) > PID_ROLL_TORQUE ) {
				pid->roll_torque = util_sign_value( pid->roll_torque ) * PID_ROLL_TORQUE;
			}

			/* Control motors. */
			r1 = controlVertical( pololu_fd, pid->vertical_thrust, pid->roll_torque, pid->pitch_torque );

			break;

		case PID_YAW:
			/* Update the gains. */
			pid->yaw.kp		= msg->gain.data.kp_yaw;
			pid->yaw.ki		= msg->gain.data.ki_yaw;
			pid->yaw.kd		= msg->gain.data.kd_yaw;

			/* Calculate the errors. */
			pid->yaw.ref	= msg->target.data.yaw;
			pid->yaw.cval	= msg->mstrain.data.yaw;
			pid->yaw.perr	= pid_subtract_angles( pid->yaw.cval, pid->yaw.ref );
			pid->yaw.ierr	+= pid->yaw.perr * dt / 1000000;
			pid->yaw.ierr	= pid_bound_integral( pid->yaw.ierr, pid->yaw.ki, PID_YAW_INTEGRAL );
			pid->yaw.derr	= msg->mstrain.data.ang_rate[2];

			/* Update status message. */
			msg->status.data.yaw_perr	= pid->yaw.perr;
			msg->status.data.yaw_ierr	= pid->yaw.ierr;
			msg->status.data.yaw_derr	= pid->yaw.derr;

			/* PID equations. */
			pid->yaw_torque =	pid->yaw.kp * pid->yaw.perr +
			                  pid->yaw.ki * pid->yaw.ierr +
			                  pid->yaw.kd * pid->yaw.derr;

			/* Check bounds. */
			if ( fabsf( pid->yaw_torque ) > PID_YAW_TORQUE ) {
				pid->yaw_torque = util_sign_value( pid->yaw_torque ) * PID_YAW_TORQUE;
			}

			/* Control Voiths. */
			r2 = controlVoiths( pololu_fd, pid->voith_speed, pid->voith_angle, pid->voith_thrust, pid->yaw_torque );

			break;

		case PID_DEPTH:
			/* Update the gains. */
			pid->depth.kp	= msg->gain.data.kp_depth;
			pid->depth.ki	= msg->gain.data.ki_depth;
			pid->depth.kd	= msg->gain.data.kd_depth;

			/* Calculate the errors. */
			pid->depth.ref	= msg->target.data.depth;
			pid->depth.cval = lj->pressure;
			pid->depth.perr = pid->depth.cval - pid->depth.ref;
			pid->depth.ierr += pid->depth.perr * dt / 1000000;
			pid->depth.ierr = pid_bound_integral( pid->depth.ierr, pid->depth.ki, PID_DEPTH_INTEGRAL );
			pid->depth.derr = 0.0;

			/* Update status message. */
			msg->status.data.depth_perr	= pid->depth.perr;
			msg->status.data.depth_ierr	= pid->depth.ierr;
			msg->status.data.depth_derr	= pid->depth.derr;

			/* PID equations. */
			pid->vertical_thrust =	pid->depth.kp * pid->depth.perr +
			                       pid->depth.ki * pid->depth.ierr +
			                       pid->depth.kd * pid->depth.derr;

			/* Check bounds. */
			if ( fabsf( pid->vertical_thrust ) > PID_VERTICAL_THRUST ) {
				pid->vertical_thrust = util_sign_value( pid->vertical_thrust ) * PID_VERTICAL_THRUST;
			}

			if ( ( fabsf( pid->vertical_thrust ) + fabsf( pid->roll_torque ) ) > PID_TOTAL_VERTICAL_THRUST ) {
				pid->vertical_thrust = PID_TOTAL_VERTICAL_THRUST - fabsf( pid->roll_torque );
			}

			/* Control depth. */
			r1 = controlVertical( pololu_fd, pid->vertical_thrust, pid->roll_torque, pid->pitch_torque );

			break;
	}
} /* end pid_loop() */


/******************************************************************************
 *
 * Title:       void pid_subtract_angles( float ang1, float ang2 )
 *
 * Description: Calculates the difference between two angles.
 *
 * Input:		ang1: Angle 1.
 * 				ang2: Angle 2.
 *
 * Output:      diff: Difference between angles.
 *
 * Globals:     None.
 *
 *****************************************************************************/

float pid_subtract_angles( float ang1, float ang2 )
{
	float error = 0.0;

	if ( ang2 >= 0 ) {
		if ( ang2 - 180.0 < ang1 ) {
			error = ang1 - ang2;
		}
		else {
			error = ang1 - ang2 + 360.0;
		}
	}
	else {
		if ( ang1 < ang2 + 180.0 ) {
			error = ( ang2 - ang1 ) * -1.0;
		}
		else {
			error = ( 360.0 + ang2 - ang1 ) * 1.0;
		}
	}

	return error;
} /* end pid_subtract_angles() */


/******************************************************************************
 *
 * Title:       float pid_bound_integral( float value, float gain, float bound )
 *
 * Description: Checks the bound for an integral.
 *
 * Input:       value: Value to be checked.
 * 				gain: Gain for controller.
 * 				bound: Bound to check against.
 *
 * Output:      Value or the bound limit.
 *
 * Globals:     None.
 *
 *****************************************************************************/

float pid_bound_integral( float value, float gain, float bound )
{
	if ( fabsf( value * gain ) > bound ) {
		return util_sign_value( value ) * fabsf( bound / gain );
	}
	else {
		return value;
	}
} /* end pid_bound_integral() */
