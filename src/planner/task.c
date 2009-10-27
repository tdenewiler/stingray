/******************************************************************************
 *
 *  Title:        task.c
 *
 *  Description:  Tasks available for planner.
 *
 *****************************************************************************/


#include "task.h"


/******************************************************************************
 *
 * Title:       int task_run( MSG_DATA *msg, CONF_VARS *cf, int task, int dt, int subtask, int subtask_dt )
 *
 * Description: Switch to the current task.
 *
 * Input:       msg: Current message data.
 * 				cf: Configuration variables.
 * 				task: The current task to be attempted.
 *              dt: The task time.
 *				subtask: Used to set which part of the task is to be run. Modify
 * 				upon success or failure.
 * 				subtask_dt: The subtask time.
 *
 * Output:      Task status: Success, failure, continuing.
 *
 *****************************************************************************/

int task_run( MSG_DATA *msg, CONF_VARS *cf, int dt, int subtask_dt )
{
	int status = TASK_CONTINUING;
	switch ( msg->task.data.task ) {
	case TASK_BUOY:
		status = task_buoy( msg, cf, dt, subtask_dt );
		break;

	case TASK_GATE:
		status = task_gate( msg, cf, dt, subtask_dt );
		break;

	case TASK_PIPE:
		status = task_pipe( msg, cf, dt, subtask_dt );
		break;

	case TASK_PIPE1:
		status = task_pipe( msg, cf, dt, subtask_dt );
		break;

	case TASK_PIPE2:
		status = task_pipe( msg, cf, dt, subtask_dt );
		break;

	case TASK_PIPE3:
		status = task_pipe( msg, cf, dt, subtask_dt );
		break;

	case TASK_PIPE4:
		status = task_pipe( msg, cf, dt, subtask_dt );
		break;

	case TASK_SQUARE:
		status = task_square( msg, cf, dt, subtask_dt );
		break;

	case TASK_NONE:
		status = task_none( msg, cf, dt, subtask_dt );
		break;

	case TASK_BOXES:
		status = task_boxes( msg, cf, dt, subtask_dt );
		break;

	case TASK_FENCE:
		status = task_fence( msg, cf, dt, subtask_dt );
		break;

	case TASK_SURFACE:
		status = task_surface( msg, cf, dt, subtask_dt );
		break;

	case TASK_SUITCASE:
		status = task_suitcase( msg, cf, dt, subtask_dt );
		break;

	case TASK_NOD:
		status = task_nod( msg, cf, dt, subtask_dt );
		break;

	case TASK_SPIN:
		status = task_spin( msg, cf, dt, subtask_dt );
		break;

	case TASK_COURSE:
		status = task_course( msg, cf, dt, subtask_dt );
		break;

	case TASK_DOCK:
		status = task_dock( msg, cf, dt, subtask_dt );
		break;

	case TASK_SKIP:
		status = task_skip( );
		break;
	}

	return status;
} /* end task_run() */


/******************************************************************************
 *
 * Title:       int task_buoy( MSG_DATA *msg, CONF_VARS *cf, int dt, int subtask_dt )
 *
 * Description: Find and follow the buoy.
 *
 * Input:       msg: Current message data.
 * 				cf: Configuration variables.
 *              dt: The task time.
 * 				subtask_dt: The subtask time.
 *
 * Output:      Task status: Success, failure, continuing.
 *
 *****************************************************************************/

int task_buoy( MSG_DATA *msg, CONF_VARS *cf, int dt, int subtask_dt )
{
	if( msg->task.data.course ) {
		/* Check if buoy touch threshold reached. */
		if( msg->vision.data.status == TASK_BUOY_TOUCHED ) {
			return TASK_SUCCESS;
		}

		/* Check for a timeout. */
		if( dt > TASK_BUOY_MAX_SEARCH_TIME ) {
			return TASK_FAILURE;
		}

		/* Check to see if we have detected the buoy. Else don't change yaw or
		 * depth, just keep old values. */
		if( msg->vision.data.status == TASK_BUOY_DETECTED ) {
			/* Set target values based on current position and pixel error. */
			msg->target.data.yaw = msg->status.data.yaw +
				(float)msg->vision.data.front_x * TASK_BUOY_YAW_GAIN;

			return TASK_CONTINUING;
		}
		/* CHRIS: Keep yaw if we don't see buoy. */
		else {
			/* Set target values based on current position and pixel error. */
			msg->target.data.yaw = cf->task_init_yaw;

			return TASK_CONTINUING;
		}
	}

	/* Non-course mode. */
	else {
		/* Check to see if we have detected the buoy. Else don't change yaw or
		 * depth, just keep old values. */
		if( msg->vision.data.status == TASK_BUOY_DETECTED ||
			msg->vision.data.status == TASK_BUOY_TOUCHED ) {

			/* Set target values based on current position and pixel error. */
			msg->target.data.yaw = msg->status.data.yaw +
				(float)msg->vision.data.front_x * TASK_BUOY_YAW_GAIN;

			msg->target.data.depth = msg->status.data.depth +
				(float)msg->vision.data.front_y * TASK_BUOY_DEPTH_GAIN;
		}

		/* CHRIS: Keep yaw if we don't see buoy. */
		else {
			/* Set target values based on current position and pixel error. */
			msg->target.data.yaw = cf->target_yaw;
		}
	}

	return TASK_CONTINUING;
} /* end task_buoy() */


/******************************************************************************
 *
 * Title:       int task_gate( MSG_DATA *msg, CONF_VARS *cf, int dt, int subtask, int subtask_dt )
 *
 * Description: Use dead reckoning to hold a heading and go straight.
 *
 * Input:       msg: Current message data.
 * 				cf: Configuration variables.
 *              dt: The task time.
 * 				subtask_dt: The subtask time.
 *
 * Output:      Task status: Success, failure, continuing.
 *
 *****************************************************************************/

int task_gate( MSG_DATA *msg, CONF_VARS *cf, int dt, int subtask_dt )
{
	if( msg->task.data.course ) {
		msg->target.data.depth = cf->depth_gate;
		msg->target.data.fy = -60;
		msg->target.data.speed = 65;

		/* Check to see if we have detected the buoy. Else don't change yaw or
		 * depth, just keep old values. */
		if( msg->vision.data.status == TASK_GATE_DETECTED ||
			msg->vision.data.status == TASK_GATE_CLEARED ) {

			/* Set target values based on current position and pixel error. */
			msg->target.data.yaw = msg->status.data.yaw +
				(float)msg->vision.data.front_x * TASK_BUOY_YAW_GAIN;

			if( msg->vision.data.status == TASK_GATE_CLEARED ) {
				cf->target_yaw = msg->status.data.yaw;
				printf("TASK_GATE: cleared, yaw = %lf\n", cf->target_yaw);
			}

			if( dt > TASK_GATE_MAX_TIME ) {
				if( msg->status.data.yaw > cf->task_init_yaw + TASK_GATE_MAX_YAW ) {
					cf->target_yaw = cf->task_init_yaw + TASK_GATE_MAX_YAW;
				}
				else if( msg->status.data.yaw < cf->task_init_yaw - TASK_GATE_MAX_YAW ) {
					cf->target_yaw = cf->task_init_yaw - TASK_GATE_MAX_YAW;
				}
				else {
					cf->target_yaw = msg->status.data.yaw;
				}
				cf->task_init_yaw = cf->target_yaw;

				msg->target.data.depth = cf->depth_buoy;
				return TASK_SUCCESS;
			}

			return TASK_CONTINUING;
		}

		/* Keep yaw if we don't see buoy. */
		else {
			if( dt > TASK_GATE_MAX_TIME ) {
				if( msg->status.data.yaw > cf->task_init_yaw + TASK_GATE_MAX_YAW ) {
					cf->target_yaw += TASK_GATE_MAX_YAW;
				}
				else if( msg->status.data.yaw < cf->task_init_yaw - TASK_GATE_MAX_YAW ) {
					cf->target_yaw -= TASK_GATE_MAX_YAW;
				}
				else {
					cf->target_yaw = msg->status.data.yaw;
				}
				cf->task_init_yaw = cf->target_yaw;
				msg->target.data.depth = cf->depth_buoy;

				return TASK_FAILURE;
			}
			else {
				/* Set target values based on current position and pixel error. */
				msg->target.data.yaw = cf->target_yaw;

				return TASK_CONTINUING;
			}
		}
	}

	/* Non-course mode. */
	else {
		/* Check to see if we have detected the buoy. Else don't change yaw or
		 * depth, just keep old values. */
		if( msg->vision.data.status == TASK_GATE_DETECTED ||
			msg->vision.data.status == TASK_GATE_CLEARED ) {

			/* Set target values based on current position and pixel error. */
			msg->target.data.yaw = msg->status.data.yaw +
				(float)msg->vision.data.front_x * TASK_BUOY_YAW_GAIN;
		}

		/* CHRIS: Keep yaw if we don't see buoy. */
		else {
			/* Set target values based on current position and pixel error. */
			msg->target.data.yaw = cf->target_yaw;
		}
	}

	return TASK_CONTINUING;
} /* end task_gate() */


/******************************************************************************
 *
 * Title:       int task_pipe( MSG_DATA *msg, CONF_VARS *cf, int dt, int subtask, int subtask_dt )
 *
 * Description: Find and follow the pipe.
 *
 * Input:       msg: Current message data.
 * 				cf: Configuration variables.
 *              dt: The task time.
 * 				subtask_dt: The subtask time.
 *
 * Output:      Task status: Success, failure, continuing.
 *
 *****************************************************************************/

int task_pipe( MSG_DATA *msg, CONF_VARS *cf, int dt, int subtask_dt )
{
	if( msg->task.data.course ) {
		/* Check for timeout. */
		if( dt > TASK_PIPE_MAX_TIME ) {
			return TASK_FAILURE;
		}

		/* Move forward until pipe is detected. */
		if( msg->vision.data.status == TASK_NOT_DETECTED ) {
			/* Do nothing new here. */

			return TASK_CONTINUING;
		}
		/* Correct yaw. ( status ==  TASK_PIPE_DETECTED ) */
		else {
			msg->target.data.yaw = msg->status.data.yaw +
				msg->vision.data.bearing * TASK_PIPE_YAW_GAIN;
			cf->task_init_yaw = msg->target.data.yaw;
			printf( "TASK_PIPE: Detected yaw=%lf\n", cf->task_init_yaw );

			return TASK_SUCCESS;
		}
	}

	/* Non-course mode. */
	else {
		/* Move forward until pipe is detected. */
		if( msg->vision.data.status != TASK_PIPE_CENTERED &&
			msg->vision.data.status != TASK_PIPE_DETECTED ) {
			msg->target.data.fy = POLOLU_MOVE_FORWARD;
		}
		/* Move fx and fy until we are centered. */
		else if( msg->vision.data.status == TASK_PIPE_DETECTED ) {
			msg->target.data.fx = msg->vision.data.bottom_x * TASK_PIPE_FX_GAIN;
			msg->target.data.fy = msg->vision.data.bottom_y * TASK_PIPE_FY_GAIN;
		}
		/* Correct yaw. ( status ==  TASK_PIPE_CENTERED ) */
		else {
			msg->target.data.fx = 0;
			msg->target.data.fy = 0;
			msg->target.data.yaw = msg->status.data.yaw +
				msg->vision.data.bearing * TASK_PIPE_YAW_GAIN;
		}
	}

	return TASK_CONTINUING;
} /* end task_pipe() */


/******************************************************************************
 *
 * Title:       int task_square( MSG_DATA *msg, CONF_VARS *cf, int dt, int subtask, int subtask_dt )
 *
 * Description: Move in a square. Use the times in msg for the duration of
 *              motion in each direction.
 *
 * Input:       msg: Current message data.
 * 				cf: Configuration variables.
 *              dt: The task time.
 * 				subtask_dt: The subtask time.
 *
 * Output:      Task status: Success, failure, continuing.
 *
 *****************************************************************************/

int task_square( MSG_DATA *msg, CONF_VARS *cf, int dt, int subtask_dt )
{
	msg->target.data.pitch = 0;
	msg->target.data.roll = 0;
	msg->target.data.depth = 0;

	return TASK_CONTINUING;
} /* end task_square() */


/******************************************************************************
 *
 * Title:       int task_none( MSG_DATA *msg, CONF_VARS *cf, int dt, int subtask_dt )
 *
 * Description: Hold the current position.
 *
 * Input:       msg: Current message data.
 * 				cf: Configuration variables.
 *              dt: The task time.
 * 				subtask_dt: The subtask time.
 *
 * Output:      Task status: Success, failure, continuing.
 *
 *****************************************************************************/

int task_none( MSG_DATA *msg, CONF_VARS *cf, int dt, int subtask_dt )
{
	/* Do nothing here. */

	return TASK_SUCCESS;
} /* end task_none() */


/******************************************************************************
 *
 * Title:       int task_boxes( MSG_DATA *msg, CONF_VARS *cf, int dt, int subtask, int subtask_dt )
 *
 * Description: Find and go to the boxes. Drop marbles over the correct boxes.
 *
 * Input:       msg: Current message data.
 * 				cf: Configuration variables.
 *              dt: The task time.
 * 				subtask_dt: The subtask time.
 *
 * Output:      Task status: Success, failure, continuing.
 *
 *****************************************************************************/

int task_boxes( MSG_DATA *msg, CONF_VARS *cf, int dt, int subtask_dt )
{
	/* TODO: Fill this function in like task_buoy() and task_pipe(). */
	if( msg->task.data.course ) {

		switch( msg->task.data.subtask ) {

		case SUBTASK_SEARCH_DEPTH:
			msg->target.data.depth = cf->depth_boxes;
			/* Check to see if we have reached the target depth. */
			if( fabsf(msg->status.data.depth - msg->target.data.depth) < SUBTASK_DEPTH_MARGIN ) {
				return TASK_SUCCESS;
			}
			/* Check to see if too much time has elapsed. */
			else if( subtask_dt > SUBTASK_MAX_SEARCH_TIME ) {
				return TASK_FAILURE;
			}
			else {
				return SUBTASK_CONTINUING;
			}
		case SUBTASK_SEARCH:

			/* TODO: Start moving forward and sideways to get centered over the box.
			 * Also check here to see if we should be working on box1 or box2. */
			//msg->target.data.fx = msg->vision.data.box1_x * TASK_BOXES_FX_GAIN;
			//msg->target.data.fy = msg->vision.data.box1_y * TASK_BOXES_FY_GAIN;
			/* TODO: Fix this magic number. */
			if( msg->vision.data.status == TASK_BOXES_DETECTED ) {
				return SUBTASK_SUCCESS;
			}
			else if( subtask_dt > SUBTASK_MAX_SEARCH_TIME ) {
				return SUBTASK_FAILURE;
			}
			else {
				return SUBTASK_CONTINUING;
			}
		case SUBTASK_CORRECT:
			/* Set target values based on current orientation and pixel error. */
			msg->target.data.fx = msg->vision.data.box1_x * TASK_BOXES_FX_GAIN;
			msg->target.data.fy = msg->vision.data.box1_y * TASK_BOXES_FY_GAIN;

			/* TODO: Need a way to check for SUCCESS or FAILURE in this case. */
			return SUBTASK_CONTINUING;
		case SUBTASK_BOXES_DROP_DEPTH:
			/* TODO: Add in details for this case. */
			break;
		case SUBTASK_BOXES_DROP:
			/* TODO: Add in details for this case. */
			break;
		}
	}

	/* Non-course mode. */
	else {
		/* Set fx and fy targets if we get a detection. */
		if( msg->vision.data.status == TASK_BOXES_DETECTED ) {

			/* Set the values based on current orientation and pixel error. */
			msg->target.data.fx = -1 * msg->vision.data.box1_x * TASK_BOXES_FX_GAIN;
			msg->target.data.fy = -1 * msg->vision.data.box1_y * TASK_BOXES_FY_GAIN;

			/* fx & fy bound check */
			if( fabsf(msg->target.data.fx) > TASK_BOXES_FX_MAX ) {
				msg->target.data.fx = util_sign_value( msg->target.data.fx ) * TASK_BOXES_FX_MAX;
			}
			if( fabsf(msg->target.data.fy) > TASK_BOXES_FY_MAX ) {
				msg->target.data.fy = util_sign_value( msg->target.data.fy ) * TASK_BOXES_FY_MAX;
			}

			/* Check that we are centered */
			if( abs(msg->vision.data.box1_x) < SUBTASK_BOXES_WINDOW_X &&
			    abs(msg->vision.data.box1_y) < SUBTASK_BOXES_WINDOW_Y ) {

				/* Print that we are done with this to the screen */
				printf( "Centered over boxes." );
			}
		}

		return TASK_CONTINUING;
	}

	return TASK_CONTINUING;
} /* end task_boxes() */


/******************************************************************************
 *
 * Title:       int task_fence( MSG_DATA *msg, CONF_VARS *cf, int dt, int subtask, int subtask_dt )
 *
 * Description: Find and go under the two fence pieces. Stay below the
 * 				horizontal fence members but above a minimum depth.
 *
 * Input:       msg: Current message data.
 * 				cf: Configuration variables.
 *              dt: The task time.
 * 				subtask_dt: The subtask time.
 *
 * Output:      Task status: Success, failure, continuing.
 *
 *****************************************************************************/

int task_fence( MSG_DATA *msg, CONF_VARS *cf, int dt, int subtask_dt )
{
	if( msg->task.data.course ) {
		/* Check to see if we have detected the buoy. Else don't change yaw or
		 * depth, just keep old values. */
		if( msg->vision.data.status == TASK_FENCE_DETECTED ||
			msg->vision.data.status == TASK_FENCE_CLEARED ) {

			if( msg->vision.data.status == TASK_FENCE_CLEARED ) {
				cf->task_init_yaw = msg->status.data.yaw;
				printf("TASK_FENCE: cleared, yaw = %lf\n", cf->target_yaw);

				return TASK_SUCCESS;
			}

			if( dt > TASK_FENCE_MAX_TIME ) {
				return TASK_FAILURE;
			}
			else {
				/* Set target values based on current position and pixel error. */
				msg->target.data.yaw   = msg->status.data.yaw +
					msg->vision.data.front_x * TASK_FENCE_YAW_GAIN;
				msg->target.data.depth = cf->depth_fence;
			}
		}

		if( dt > TASK_FENCE_MAX_TIME ) {
			return TASK_FAILURE;
		}

		return TASK_CONTINUING;
	}

	else {
		/* Set the values based on current orientation and pixel error. */
		msg->target.data.yaw   = msg->status.data.yaw +
			msg->vision.data.front_x * TASK_FENCE_YAW_GAIN;
		msg->target.data.depth = cf->depth_fence;

		return TASK_CONTINUING;
	}

	return TASK_CONTINUING;
} /* end task_fence() */


/******************************************************************************
 *
 * Title:       int task_suitcase( MSG_DATA *msg, CONF_VARS *cf, int dt, int subtask, int subtask_dt )
 *
 * Description: Find and retrieve the suitcase. First center vehicle above the
 * 				suitcase and then lower depth until the suitcase is picked up
 * 				using the caribiners on the bottoms of the Voith motors.
 *
 * Input:       msg: Current message data.
 * 				cf: Configuration variables.
 *              dt: The task time.
 * 				subtask_dt: The subtask time.
 *
 * Output:      Task status: Success, failure, continuing.
 *
 *****************************************************************************/

int task_suitcase( MSG_DATA *msg, CONF_VARS *cf, int dt, int subtask_dt )
{
	/* TODO: Fill this function in like task_buoy() and task_pipe(). */

	return TASK_CONTINUING;
} /* end task_suitcase() */


/******************************************************************************
 *
 * Title:       int task_surface( MSG_DATA *msg, CONF_VARS *cf, int dt, int subtask, int subtask_dt )
 *
 * Description: Surface within the octagon.
 *
 * Input:       msg: Current message data.
 * 				cf: Configuration variables.
 *              dt: The task time.
 * 				subtask_dt: The subtask time.
 *
 * Output:      Task status: Success, failure, continuing.
 *
 *****************************************************************************/

int task_surface( MSG_DATA *msg, CONF_VARS *cf, int dt, int subtask_dt )
{
	/* Make sure fx and fy are zero. */
	msg->target.data.fx = 0;
	msg->target.data.fy = 0;

	/* Set the target depth to the configuration file depth for surfacing. */
	msg->target.data.depth =  cf->depth_surface;

	return TASK_CONTINUING;
} /* end task_surface() */


/******************************************************************************
 *
 * Title:       int task_course( MSG_DATA *msg, CONF_VARS *cf, int dt, int subtask, int subtask_dt )
 *
 * Description: Run the entire course.
 *
 * Input:       msg: Current message data.
 * 				cf: Configuration variables.
 *              dt: The task time.
 * 				subtask_dt: The subtask time.
 *
 * Output:      Task status: Success, failure, continuing.
 *
 *****************************************************************************/

int task_course( MSG_DATA *msg, CONF_VARS *cf, int dt, int subtask_dt )
{
	/* Nothing to do here. Everything is taken care of in main planner function
	 * by incrementing the task and subtask values. The order that the course
	 * is performed in the order specified by the #define numbers in task.h. */

	return TASK_CONTINUING;
} /* end task_course() */


/******************************************************************************
 *
 * Title:       int task_nod( MSG_DATA *msg, CONF_VARS *cf, int dt, int subtask, int subtask_dt )
 *
 * Description: Make the sub nod its head.
 *
 * Input:       msg: Current message data.
 * 				cf: Configuration variables.
 *              dt: The task time.
 * 				subtask_dt: The subtask time.
 *
 * Output:      Task status: Success, failure, continuing.
 *
 *****************************************************************************/

int task_nod( MSG_DATA *msg, CONF_VARS *cf, int dt, int subtask_dt )
{

	return TASK_CONTINUING;
} /* end task_nod() */


/******************************************************************************
 *
 * Title:       int task_spin( MSG_DATA *msg, CONF_VARS *cf, int dt, int subtask, int subtask_dt )
 *
 * Description: Make the sub spin in place.
 *
 * Input:       msg: Current message data.
 * 				cf: Configuration variables.
 *              dt: The task time.
 * 				subtask_dt: The subtask time.
 *
 * Output:      Task status: Success, failure, continuing.
 *
 *****************************************************************************/

int task_spin( MSG_DATA *msg, CONF_VARS *cf, int dt, int subtask_dt )
{
	/* Continuously add 1 degree to yaw every time through this loop. It might
	 * be better to only add the 1 degree if enough time has elapsed by using
	 * the dt argument. */
	msg->target.data.yaw++;

	return TASK_CONTINUING;
} /* end task_spin() */


/******************************************************************************
 *
 * Title:       int task_spin( MSG_DATA *msg, CONF_VARS *cf, int dt, int subtask, int subtask_dt )
 *
 * Description: Make the sub spin in place.
 *
 * Input:       msg: Current message data.
 * 				cf: Configuration variables.
 *              dt: The task time.
 * 				subtask_dt: The subtask time.
 *
 * Output:      Task status: Success, failure, continuing.
 *
 *****************************************************************************/

int task_sweep( MSG_DATA *msg, CONF_VARS *cf, int dt, int subtask_dt )
{
	/* Check to see if we have a previous value for yaw
	 * If not, we set it. */
	if( msg->target.data.yaw_previous == TASK_YAW_PREVIOUS_NOT_SET ) {
		msg->target.data.yaw_previous = msg->status.data.yaw;
	}

	if( dt < TASK_SWEEP_YAW_TIMEOUT ) {
		/* Sweep the target angle back and forth */
		msg->target.data.yaw = msg->target.data.yaw_previous +
							   TASK_SWEEP_YAW_ANGLE * sin( dt * 2 * M_PI / TASK_SWEEP_YAW_PERIOD );
	}
	else {
		msg->target.data.yaw = msg->target.data.yaw_previous;
	}

	return TASK_CONTINUING;
} /* end task_spin() */


/******************************************************************************
 *
 * Title:       int task_dock( MSG_DATA *msg, CONF_VARS *cf, int dt, int subtask, int subtask_dt )
 *
 * Description: Operations to perform at dock.
 *
 * Input:       msg: Current message data.
 * 				cf: Configuration variables.
 *              dt: The task time.
 * 				subtask_dt: The subtask time.
 *
 * Output:      Task status: Success, failure, continuing.
 *
 *****************************************************************************/

int task_dock( MSG_DATA *msg, CONF_VARS *cf, int dt, int subtask_dt )
{
	/* This task should be the same whether we are running the competition or
	 * just testing the individual task. */

	/* Check to see if we have time limit. */
	if( subtask_dt > cf->dock_time ) {
		/* Set gains and speed to configuration file values. */
		msg->target.data.fx           = 0;
		msg->target.data.fy           = 0;
		msg->target.data.speed        = cf->target_speed;
		msg->gain.data.kp_pitch       = cf->kp_pitch;
		msg->gain.data.ki_pitch       = cf->ki_pitch;
		msg->gain.data.kd_pitch       = cf->kd_pitch;
		msg->gain.data.kp_roll        = cf->kp_roll;
		msg->gain.data.ki_roll        = cf->ki_roll;
		msg->gain.data.kd_roll        = cf->kd_roll;
		msg->gain.data.kp_yaw         = cf->kp_yaw;
		msg->gain.data.ki_yaw         = cf->ki_yaw;
		msg->gain.data.kd_yaw         = cf->kd_yaw;
		msg->gain.data.kp_depth       = cf->kp_depth;
		msg->gain.data.ki_depth       = cf->ki_depth;
		msg->gain.data.kd_depth       = cf->kd_depth;
		msg->target.data.yaw_previous = TASK_YAW_PREVIOUS_NOT_SET;
		msg->target.data.yaw_detected = TASK_YAW_DETECTED_NOT_SET;

		return TASK_SUCCESS;
	}
	else {
		/* Set gains and speed to zero. */
		msg->target.data.fx           = 0;
		msg->target.data.fy           = 0;
		msg->target.data.speed        = 0;
		msg->gain.data.kp_pitch       = 0;
		msg->gain.data.ki_pitch       = 0;
		msg->gain.data.kd_pitch       = 0;
		msg->gain.data.kp_roll        = 0;
		msg->gain.data.ki_roll        = 0;
		msg->gain.data.kd_roll        = 0;
		msg->gain.data.kp_yaw         = 0;
		msg->gain.data.ki_yaw         = 0;
		msg->gain.data.kd_yaw         = 0;
		msg->gain.data.kp_depth       = 0;
		msg->gain.data.ki_depth       = 0;
		msg->gain.data.kd_depth       = 0;
		msg->target.data.yaw_previous = TASK_YAW_PREVIOUS_NOT_SET;
		msg->target.data.yaw_detected = TASK_YAW_DETECTED_NOT_SET;

		return TASK_CONTINUING;
	}
} /* end task_dock() */


/******************************************************************************
 *
 * Title:       int task_skip( )
 *
 * Description: A placeholder task to make editing order of course task easier.
 *
 * Input:       None.
 *
 * Output:      TASK_SUCCESS.
 *
 *****************************************************************************/

int task_skip( )
{
	printf("TASK_SKIP: returning success.\n");

	return TASK_SUCCESS;
} /* end task_skip() */
