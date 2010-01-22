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

/*------------------------------------------------------------------------------
 * void pid_init()
 * Initializes PID values.
 *----------------------------------------------------------------------------*/

int pid_init(PID *pid, CONF_VARS *cf)
{
	pid->pitch.ref		= cf->target_pitch;
	pid->pitch.kp		= cf->kp_pitch;
	pid->pitch.ki		= cf->ki_pitch;
	pid->pitch.kd		= cf->kd_pitch;
	pid->pitch.period	= cf->period_pitch;

	pid->roll.ref		= cf->target_roll;
	pid->roll.kp		= cf->kp_roll;
	pid->roll.ki		= cf->ki_roll;
	pid->roll.kd		= cf->kd_roll;
	pid->roll.period	= cf->period_roll;

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

	pid->fx.ref			= cf->target_fx;
	pid->fx.kp			= cf->kp_fx;
	pid->fx.ki			= cf->ki_fx;
	pid->fx.kd			= cf->kd_fx;

	pid->fy.ref			= cf->target_fy;
	pid->fy.kp			= cf->kp_fy;
	pid->fy.ki			= cf->ki_fy;
	pid->fy.kd			= cf->kd_fy;

	pid->kp_roll_lateral  = cf->kp_roll_lateral;
	pid->kp_depth_forward = cf->kp_depth_forward;
	pid->kp_place_holder  = cf->kp_place_holder;

	pid->voith_angle	= 0;
	pid->voith_speed	= 0;
	pid->voith_thrust	= 0;
	pid->pitch_torque	= 0;
	pid->roll_torque	= 0;
	pid->yaw_torque		= 0;

	return 0;
} /* end pid_init() */


/*------------------------------------------------------------------------------
 * void pid_loop()
 * Runs through one iteration of a PID controller for the axis specified in mode.
 *----------------------------------------------------------------------------*/

void pid_loop(int pololu_fd,
               PID *pid,
               CONF_VARS *cf,
               MSG_DATA *msg,
               float dt,
               int mode,
			   int motor_init
           )
{
	/// Check to see if the errors should be reset to zero.
	if (msg->target.data.mode == ZERO_PID_ERRORS) {
		pid->pitch.perr = 0;
		pid->pitch.ierr = 0;
		pid->pitch.derr = 0;
		pid->roll.perr = 0;
		pid->roll.ierr = 0;
		pid->roll.derr = 0;
		pid->yaw.perr = 0;
		pid->yaw.ierr = 0;
		pid->yaw.derr = 0;
		pid->depth.perr = 0;
		pid->depth.ierr = 0;
		pid->depth.derr = 0;
		pid->fx.perr = 0;
		pid->fx.ierr = 0;
		pid->fx.derr = 0;
		pid->fy.perr = 0;
		pid->fy.ierr = 0;
		pid->fy.derr = 0;
		msg->target.data.mode = AUTONOMOUS;
	}

	/// These next three need to be set from vison, hydrophone, gui, etc. They depend on
	/// the target values reported from those sensor systems.
	/// Reference: float atan2f(float y, float x); theta = atan(y / x)
	float depth_perr_old = pid->depth.perr;

	if (msg->task.data.task == TASK_BOXES) {
		float fx_perr_old = pid->fx.perr;
		float fy_perr_old = pid->fy.perr;

		/// Update the gains.
		pid->fx.kp		= msg->gain.data.kp_fx;
		pid->fx.ki		= msg->gain.data.ki_fx;
		pid->fx.kd		= msg->gain.data.kd_fx;

		pid->fy.kp		= msg->gain.data.kp_fy;
		pid->fy.ki		= msg->gain.data.ki_fy;
		pid->fy.kd		= msg->gain.data.kd_fy;

		pid->kp_roll_lateral   = msg->gain.data.kp_roll_lateral;
		pid->kp_depth_forward  = msg->gain.data.kp_depth_forward;
		pid->kp_place_holder   = msg->gain.data.kp_place_holder;

		/// Calculate the errors for fx and fy.
		pid->fx.ref		= msg->target.data.fx;
		pid->fx.cval	= 0;
		pid->fx.perr	= pid->fx.cval - pid->fx.ref;
		pid->fx.ierr	+= pid->fx.perr * dt;
		pid->fx.ierr	= pid_bound_integral(pid->fx.ierr, pid->fx.ki, PID_FX_INTEGRAL);
		pid->fx.derr	= pid->fx.perr - fx_perr_old;

		pid->fy.ref		= msg->target.data.fy;
		pid->fy.cval	= 0;
		pid->fy.perr	= pid->fy.cval - pid->fy.ref;
		pid->fy.ierr	+= pid->fy.perr * dt;
		pid->fy.ierr	= pid_bound_integral(pid->fy.ierr, pid->fy.ki, PID_FY_INTEGRAL);
		pid->fy.derr	= pid->fy.perr - fy_perr_old;

		/// Update status message.
		msg->status.data.fx_perr	= pid->fx.perr;
		msg->status.data.fx_ierr	= pid->fx.ierr;
		msg->status.data.fx_derr	= pid->fx.derr;

		msg->status.data.fy_perr	= pid->fy.perr;
		msg->status.data.fy_ierr	= pid->fy.ierr;
		msg->status.data.fy_derr	= pid->fy.derr;


		/// PID equations.
		pid->lateral_thrust =	pid->fx.kp * pid->fx.perr +
								pid->fx.ki * pid->fx.ierr +
								pid->fx.kd * pid->fx.derr;

		pid->forward_thrust =	pid->fy.kp * pid->fy.perr +
								pid->fy.ki * pid->fy.ierr +
								pid->fy.kd * pid->fy.derr;
	}
	else {
		pid->lateral_thrust = msg->target.data.fx;
		pid->forward_thrust = msg->target.data.fy;
	}

	/// Check bounds.
	if (fabsf(pid->forward_thrust) > PID_TOTAL_FY_THRUST) {
		pid->forward_thrust = util_sign_value(pid->forward_thrust) * PID_TOTAL_FY_THRUST;
	}
	if (fabsf(pid->lateral_thrust) > PID_TOTAL_FX_THRUST) {
		pid->lateral_thrust = util_sign_value(pid->lateral_thrust) * PID_TOTAL_FX_THRUST;
	}

	/// Update status
	msg->status.data.fx = pid->lateral_thrust;
	msg->status.data.fy = pid->forward_thrust;

	/// Compute voith actuator values
	pid->voith_angle = pid_compute_sub_angle(pid->lateral_thrust, pid->forward_thrust);
	pid->voith_speed = msg->target.data.speed;
	pid->voith_thrust = sqrt(pid->lateral_thrust * pid->lateral_thrust +
	                         pid->forward_thrust * pid->forward_thrust);

	/// Initialize return values.
	int r1 = -1;
	int r2 = -1;

	/// Switch based on which loop we are going to control.
	switch (mode) {
	case PID_PITCH:
		/// Update the gains.
		pid->pitch.kp	= msg->gain.data.kp_pitch;
		pid->pitch.ki	= msg->gain.data.ki_pitch;
		pid->pitch.kd	= msg->gain.data.kd_pitch;

		/// Calculate the errors.
		pid->pitch.ref	= msg->target.data.pitch;
		pid->pitch.cval	= msg->mstrain.data.pitch;
		pid->pitch.perr = pid_subtract_angles(pid->pitch.cval, pid->pitch.ref);
		pid->pitch.ierr += pid->pitch.perr * dt;
		pid->pitch.ierr = pid_bound_integral(pid->pitch.ierr, pid->pitch.ki, PID_PITCH_INTEGRAL);
		pid->pitch.derr = msg->mstrain.data.ang_rate[0];

		/// Update status message.
		msg->status.data.pitch_perr	= pid->pitch.perr;
		msg->status.data.pitch_ierr	= pid->pitch.ierr;
		msg->status.data.pitch_derr	= pid->pitch.derr;

		/// PID equations.
		pid->pitch_torque =	pid->pitch.kp * pid->pitch.perr +
							pid->pitch.ki * pid->pitch.ierr +
							pid->pitch.kd * pid->pitch.derr;

		/// Check bounds.
		if (fabsf(pid->pitch_torque) > PID_PITCH_TORQUE) {
			pid->pitch_torque = util_sign_value(pid->pitch_torque) * PID_PITCH_TORQUE;
		}

		break;

	case PID_ROLL:
		/// Update the gains.
		pid->roll.kp	= msg->gain.data.kp_roll;
		pid->roll.ki	= msg->gain.data.ki_roll;
		pid->roll.kd	= msg->gain.data.kd_roll;

		/// Calculate the errors.
		pid->roll.ref	= msg->target.data.roll;
		pid->roll.cval	= msg->mstrain.data.roll;
		pid->roll.perr	= pid_subtract_angles(pid->roll.cval, pid->roll.ref);
		pid->roll.ierr	+= pid->roll.perr * dt;
		pid->roll.ierr	= pid_bound_integral(pid->roll.ierr, pid->roll.ki, PID_ROLL_INTEGRAL);
		pid->roll.derr	= msg->mstrain.data.ang_rate[1];

		/// Update status message.
		msg->status.data.roll_perr	= pid->roll.perr;
		msg->status.data.roll_ierr	= pid->roll.ierr;
		msg->status.data.roll_derr	= pid->roll.derr;

		/// PID equations.
		pid->roll_torque =	pid->roll.kp * pid->roll.perr +
						   	pid->roll.ki * pid->roll.ierr +
						   	pid->roll.kd * pid->roll.derr +
						    pid->kp_roll_lateral * pid->fx.perr; /// Coupling between lateral thrust and roll.

		/// Check bounds.
		if (fabsf(pid->roll_torque) > PID_ROLL_TORQUE) {
			pid->roll_torque = util_sign_value(pid->roll_torque) * PID_ROLL_TORQUE;
		}

		/// Control motors.
		if (motor_init) {
			//r1 = pololuControlVertical(pololu_fd, pid->vertical_thrust, pid->roll_torque, pid->pitch_torque);
			r1 = pololu_control_vertical(pololu_fd, pid->vertical_thrust, pid->roll_torque, pid->pitch_torque);
		}

		break;

	case PID_YAW:
		/// Update the gains.
		pid->yaw.kp		= msg->gain.data.kp_yaw;
		pid->yaw.ki		= msg->gain.data.ki_yaw;
		pid->yaw.kd		= msg->gain.data.kd_yaw;

		/// Use different gains for buoy task.
		if ((msg->target.data.task == TASK_BUOY ||
		     msg->target.data.task == TASK_GATE ||
		     msg->target.data.task == TASK_FENCE) &&
			msg->target.data.vision_status != TASK_NOT_DETECTED) {
			pid->yaw.kp = cf->kp_buoy;
			pid->yaw.ki = cf->ki_buoy;
			pid->yaw.kd = cf->kd_buoy;
		}

		/// Calculate the errors.
		pid->yaw.ref	= msg->target.data.yaw;
		pid->yaw.cval	= msg->mstrain.data.yaw;
		pid->yaw.perr	= pid_subtract_angles(pid->yaw.cval, pid->yaw.ref);
		pid->yaw.ierr	+= pid->yaw.perr * dt;
		pid->yaw.ierr	= pid_bound_integral(pid->yaw.ierr, pid->yaw.ki, PID_YAW_INTEGRAL);
		pid->yaw.derr	= msg->mstrain.data.ang_rate[2];

		/// Update status message.
		msg->status.data.yaw_perr	= pid->yaw.perr;
		msg->status.data.yaw_ierr	= pid->yaw.ierr;
		msg->status.data.yaw_derr	= pid->yaw.derr;

		/// PID equations.
		pid->yaw_torque =	pid->yaw.kp * pid->yaw.perr +
						  	pid->yaw.ki * pid->yaw.ierr +
						  	pid->yaw.kd * pid->yaw.derr;

		/// Check bounds.
		if (fabsf(pid->yaw_torque) > POLOLU_MAX_YAW_TORQUE) {
			pid->yaw_torque = util_sign_value(pid->yaw_torque) * POLOLU_MAX_YAW_TORQUE;
		}

		/// Control Voiths.
		if (motor_init) {
			//r2 = pololuControlVoiths(pololu_fd, pid->voith_speed, pid->voith_angle, pid->voith_thrust, pid->yaw_torque);
			r2 = pololu_control_voiths(pololu_fd, pid->voith_speed, pid->voith_angle, pid->voith_thrust, pid->yaw_torque);
		}

		break;

	case PID_DEPTH:
		/// Update the gains.
		pid->depth.kp	= msg->gain.data.kp_depth;
		pid->depth.ki	= msg->gain.data.ki_depth;
		pid->depth.kd	= msg->gain.data.kd_depth;

		/// Calculate the errors.
		pid->depth.ref	= msg->target.data.depth;
		pid->depth.cval = msg->lj.data.pressure;
		pid->depth.perr = pid->depth.cval - pid->depth.ref;
		pid->depth.ierr += pid->depth.perr * dt;
		pid->depth.ierr = pid_bound_integral(pid->depth.ierr, pid->depth.ki, PID_DEPTH_INTEGRAL);
		pid->depth.derr	= pid->depth.perr - depth_perr_old;

		/// Update status message.
		msg->status.data.depth_perr	= pid->depth.perr;
		msg->status.data.depth_ierr	= pid->depth.ierr;
		msg->status.data.depth_derr	= pid->depth.derr;

		/// PID equations.
		pid->vertical_thrust =	pid->depth.kp * pid->depth.perr +
							   pid->depth.ki * pid->depth.ierr +
							   pid->depth.kd * pid->depth.derr;

		/// Check bounds.
		if ((fabsf(pid->vertical_thrust) + fabsf(pid->roll_torque)) > PID_TOTAL_VERTICAL_THRUST) {
			pid->vertical_thrust = util_sign_value(pid->vertical_thrust) *
				(PID_TOTAL_VERTICAL_THRUST - fabsf(pid->roll_torque));
		}

		/// Control depth.
		if (motor_init) {
			//r1 = pololuControlVertical(pololu_fd, pid->vertical_thrust, pid->roll_torque, pid->pitch_torque);
			r1 = pololu_control_vertical(pololu_fd, pid->vertical_thrust, pid->roll_torque, pid->pitch_torque);
		}

		break;
	}
} /* end pid_loop() */


/*------------------------------------------------------------------------------
 * void pid_subtract_angles()
 * Calculates the difference between two angles. A positive output is means the
 * ifference is a positive clockwise offset. This implies the sub must turn left.
 * A negative output means the sub must turn right.
 *----------------------------------------------------------------------------*/

float pid_subtract_angles(float ang1, float ang2)
{
	float e = 0.0;

	if (ang1 == ang2) {
		return e;
	}
	if (ang1 < ang2) {
		if ((ang2 - ang1) < 180) {
			e = ang1 - ang2;
		}
		else {
			e = 360 - ang2 + ang1;
		}
	}
	else {
		if ((ang1 - ang2) < 180) {
			e = ang1 - ang2;
		}
		else {
			e = -360 + ang1 -ang2;
		}
	}

	return e;
} /* end pid_subtract_angles() */


/*------------------------------------------------------------------------------
 * float pid_bound_integral()
 * Checks the bound for an integral to avoid saturation.
 *----------------------------------------------------------------------------*/

float pid_bound_integral(float value, float gain, float bound)
{
	if (fabsf(value * gain) > bound) {
		return util_sign_value(value) * fabsf(bound / gain);
	}
	else {
		return value;
	}
} /* end pid_bound_integral() */


/*------------------------------------------------------------------------------
 * float float pid_compute_sub_angle()
 * Computes the bounds via atan2f with some additional logic.
 *----------------------------------------------------------------------------*/

float pid_compute_sub_angle(float fx, float fy)
{
	/// There are a number of special cases in atan2f. Since we are repeatedly
	/// passing in zero, the angle we return should be carefully computed */
	if (fabsf(fx) < PID_SUB_ANGLE_EPSILON && fabsf(fy) < PID_SUB_ANGLE_EPSILON)
		return 0;

	return atan2f(fx, fy);
} /* end pid_bound_integral() */
