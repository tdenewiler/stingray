/******************************************************************************
 *
 *  Title:        task.c
 *
 *  Description:  Tasks available for planner.
 *
 *****************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>

#include "planner.h"
#include "network.h"
#include "labjack.h"
#include "util.h"
#include "messages.h"
#include "pid.h"
#include "task.h"
#include "pololu.h"

#define PIPE_MODE_PIPE  		 0
#define PIPE_MODE_LOOK_AHEAD	 1


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

	if( msg->task.data.course == TASK_COURSE_ON ) {

		switch( msg->task.data.subtask ) {

		case SUBTASK_SEARCH_DEPTH:
			msg->target.data.depth = cf->depth_buoy;
			/* Check to see if we have reached the target depth. */
			if( fabsf(msg->status.data.depth - msg->target.data.depth) < SUBTASK_DEPTH_MARGIN ) {
				return SUBTASK_SUCCESS;
			}
			/* Check to see if too much time has elapsed. */
			else if( subtask_dt > SUBTASK_MAX_SEARCH_TIME ) {
				return SUBTASK_FAILURE;
			}
			/* Otherwise we are continuing. */
			else {
				return SUBTASK_CONTINUING;
			}
		case SUBTASK_SEARCH:
			/* Start moving forward. */
			msg->target.data.fy = POLOLU_MOVE_FORWARD;

			/* If the bouy has been detected, front_x will be different
			 * than BOUY_NOT_DETECTED */
			if( msg->vision.data.status == TASK_BOUY_DETECTED ) {
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
			msg->target.data.yaw = msg->status.data.yaw + (float)msg->vision.data.front_x * TASK_BUOY_YAW_GAIN;
			msg->target.data.depth = msg->status.data.depth + (float)msg->vision.data.front_y * TASK_BUOY_DEPTH_GAIN;

			/* TODO: Need a way to check for SUCCESS or FAILURE in this case. */
			return SUBTASK_CONTINUING;
		}
	}
	else { /* Non-Course Mode */
		
		/* Check to see if we have a previous value from the pipe routine. */
		if( util_fequals( msg->target.data.yaw_previous , TASK_YAW_PREVIOUS_NOT_SET ) ) {
			msg->target.data.yaw_previous = msg->status.data.yaw;
		}

		/* Check for timeout */
		if( dt > TASK_BOUY_MAX_SEARCH_TIME ) {
			/* Reset yaw to our initial yaw if we have a timeout. */
			msg->target.data.yaw = TASK_PIPE2_YAW;

			/* Set yaw detect to undetected value. */
			msg->target.data.yaw_detected = TASK_YAW_DETECTED_NOT_SET;

			/* We have failed ... */
			return TASK_FAILURE;
			
		} /* End if timeout */

		/*  If the bouy was detected ..*/
		if( msg->vision.data.status == TASK_BOUY_DETECTED ) {

			/* Set target values based on current orientation and pixel error. */
			msg->target.data.yaw = msg->status.data.yaw + (float)msg->vision.data.front_x * TASK_BUOY_YAW_GAIN;

			/* Save the current detection in case we lose it. */
			msg->target.data.yaw_detected = msg->target.data.yaw;


			/* Need to divide the depth target by a largish number because the gain works
			 * for yaw in degrees but not for depth in volts. The pixel error needs to
			 * be converted so that it is meaningful for volts as well. */
			msg->target.data.depth = msg->status.data.depth + (float)msg->vision.data.front_y * TASK_BUOY_DEPTH_GAIN;


			//printf("TASK_BUOY:     %f    %f\n", msg->target.data.depth,
				//(float)msg->vision.data.front_y * TASK_BUOY_DEPTH_GAIN);
		}
		else { /* If the bouy is not detected */

			/* Check to see if we had a previous detection.
			 * If we do not have a previous detection, start sweeping. */
			if( util_fequals( msg->target.data.yaw_detected , TASK_YAW_DETECTED_NOT_SET ) ) {

				/* If we don't have any detections, keep the previous bearing */
				msg->target.data.yaw = msg->target.data.yaw_previous;
			}
			else {
				/* If we had a detection, use it. */
				msg->target.data.yaw = msg->target.data.yaw_detected;
			}

		}/* End if bouy detected/not detected */

		/* Success criteria. */
		if( msg->vision.data.status == TASK_BOUY_TOUCHED ) {
			/* Set the target yaw to the anticipated pipe2 yaw */
			msg->target.data.yaw = TASK_PIPE2_YAW;

			/* Set yaw detect to undetected value */
			msg->target.data.yaw_detected = TASK_YAW_DETECTED_NOT_SET;
			return TASK_SUCCESS;
		}

		/* No events ( timeout or success ) indicate to switch tasks,
		 * lets continue. */
		return TASK_CONTINUING;

	}/* end Non-Course Mode */

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
	if( msg->task.data.course == TASK_COURSE_ON ) {

		switch( msg->task.data.subtask ) {

		case SUBTASK_SEARCH_DEPTH:
			msg->target.data.depth = cf->depth_pipe;
			msg->target.data.depth = cf->heading_gate;

			/* Check to see if we have reached the target depth. */
			if( fabsf(msg->status.data.depth - msg->target.data.depth) < SUBTASK_DEPTH_MARGIN ) {
				if( fabsf(msg->status.data.yaw - msg->target.data.yaw) < SUBTASK_YAW_MARGIN ) {
					return TASK_SUCCESS;
				}
			}
			/* Check to see if too much time has elapsed. */
			else if( subtask_dt > SUBTASK_MAX_SEARCH_TIME ) {
				return TASK_FAILURE;
			}
			else {
				return SUBTASK_CONTINUING;
			}
		case SUBTASK_GATE_MOVE:
			if( subtask_dt < SUBTASK_GATE_MOVE_TIME ) {
				msg->target.data.fy = POLOLU_MOVE_FORWARD;
			}
			else if( dt > SUBTASK_MAX_SEARCH_TIME ) {
				return TASK_FAILURE;
			}
			else {
				return TASK_SUCCESS;
			}
		}
	}
	else {
		/* Use the known direction from the start dock to the validation gate here. */
		msg->target.data.yaw = cf->heading_buoy;

		/* Use a nominal starting depth for getting through the gate. It will
		 * probably be equal to the depth of the buoy so that we can start looking
		 * for the buoy right away. */
		msg->target.data.depth = cf->depth_gate;

		return TASK_CONTINUING;
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
	/* Temporary for testing of non course "else" */
	const static int NON_COURSE_SUBTASK_CENTER 		= 0;
	const static int NON_COURSE_SUBTASK_YAW_CORRECT = 1;
	const static int NON_COURSE_SUBTASK_DRIVE 		= 2;

	/* Holds the place of msg->task.data.subtask */
	static int non_course_subtask = NON_COURSE_SUBTASK_CENTER;

	/* Timers for centering and bearing correction */
	static int time_ref_center  = SUBTASK_TIME_REF_NOT_SET;
	static int time_ref_bearing = SUBTASK_TIME_REF_NOT_SET;
	static int time_ref_drive   = SUBTASK_TIME_REF_NOT_SET;

	if( msg->task.data.course == TASK_COURSE_ON ) {

		switch( msg->task.data.subtask ) {

		case SUBTASK_SEARCH_DEPTH:
			msg->target.data.depth = cf->depth_pipe;
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
			/* Start moving forward. */
			msg->target.data.fy = POLOLU_MOVE_FORWARD;

			if( msg->vision.data.status == TASK_PIPE_DETECTED ) {
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
			msg->target.data.yaw    = msg->status.data.yaw + (float)msg->vision.data.bottom_y * TASK_PIPE_YAW_GAIN;
			msg->target.data.fx     = msg->vision.data.bottom_x * TASK_PIPE_FX_GAIN;
			if( msg->target.data.fx > TASK_PIPE_FX_MAX ) {
				msg->target.data.fx = TASK_PIPE_FX_MAX;
			}
			else if( msg->target.data.fx < -1 * TASK_PIPE_FX_MAX ) {
				msg->target.data.fx = -1 * TASK_PIPE_FX_MAX;
			}

			/* TODO: Need a way to check for SUCCESS or FAILURE in this case. */
			return SUBTASK_CONTINUING;

		case SUBTASK_PIPE_END:
			/* TODO: Add in details for this case. */
			/* To implement this correctly, the interface between having
			 * control in this method vs the next method needs to be worked out
			 * for each transition */
			/* A return value in this case to allow transition (ie success or failure)
			 * must be added here*/
			break;
		}
	}
	else {
		/* Non-course mode */
		/* If the pipe is detected, make course correction */
		if( msg->vision.data.status == TASK_PIPE_DETECTED ) {


			/* ======== CENTERING ============ */
			/* Get the x & y fixed correctly */
			/* Set the values based on current orientation and pixel error. */
			if( non_course_subtask == NON_COURSE_SUBTASK_CENTER ) {

				/* Set the new targets then check for bounds */
				msg->target.data.fx = msg->vision.data.bottom_x * TASK_PIPE_FX_GAIN;
				msg->target.data.fy = msg->vision.data.bottom_y * TASK_PIPE_FY_GAIN;

				/* fx & fy bound check */
				if( fabsf(msg->target.data.fx) > TASK_PIPE_FX_MAX ) {
					msg->target.data.fx = util_sign_value( msg->target.data.fx ) * TASK_PIPE_FX_MAX;
				}
				if( fabsf(msg->target.data.fy) > TASK_PIPE_FY_MAX ) {
					msg->target.data.fy = util_sign_value( msg->target.data.fy ) * TASK_PIPE_FY_MAX;
				}

				/* If we are in the window, we are done centering */
				if( abs(msg->vision.data.bottom_x) < SUBTASK_PIPE_WINDOW_X &&
				    abs(msg->vision.data.bottom_y) < SUBTASK_PIPE_WINDOW_Y ) {

				    /* Start the centering timer */
				    if( time_ref_center == SUBTASK_TIME_REF_NOT_SET ) {
				    	time_ref_center = dt;
					}

					/* Check if we have waited long enough for the centering timeout*/
					if( ( dt - time_ref_center ) > SUBTASK_TIME_WAIT_CENTER ) {

						/* Reset the centering timer to the "not set" value so it can
						 * be reused */
						time_ref_center = SUBTASK_TIME_REF_NOT_SET;

						/* Increment the subtask. This is where we should return subtask success. */
						non_course_subtask++;

						/* Print that we are done with this to the screen */
						printf( "Done Centering over Pipe" );
					}
				}
			}/* ======== END CENTERING ============ */



			/* ======== BEARING CORRECT ============ */
			/* Now that we are centered, correct the yaw and fx */
			if( non_course_subtask == NON_COURSE_SUBTASK_YAW_CORRECT ) {

				/* Set the values based on current orientation and pixel error. */
				msg->target.data.yaw = msg->status.data.yaw + msg->vision.data.bearing * TASK_PIPE_YAW_GAIN;
				msg->target.data.fx  = msg->vision.data.bottom_x * TASK_PIPE_FX_GAIN;

				/* Check the bounds of fx */
				if( fabsf(msg->target.data.fx) > TASK_PIPE_FX_MAX ) {
					msg->target.data.fx = util_sign_value( msg->target.data.fx ) * TASK_PIPE_FX_MAX;
				}

				/* If we are in the bearing range, we are done setting the bearing */
				if(  abs (msg->vision.data.bottom_x ) < SUBTASK_PIPE_WINDOW_X &&
				    fabsf(msg->status.data.yaw_perr ) < SUBTASK_PIPE_ANGLE_MARGIN ) {

				    /* Start the centering timer */
				    if( time_ref_bearing == SUBTASK_TIME_REF_NOT_SET ) {
				    	time_ref_bearing = dt;
					}

					/* Check if we have waited long enough for the centering timeout*/
					if( ( dt - time_ref_bearing ) > SUBTASK_TIME_WAIT_BEARING ) {

						/* Reset the centering timer to the "not set" value so it can
						 * be reused */
						time_ref_bearing = SUBTASK_TIME_REF_NOT_SET;

						/* Increment the subtask. This is where we should return subtask success. */
						non_course_subtask++;

						/* Print that we are done with this to the screen */
						printf( "Done Correcting the bearing over Pipe" );
					}
				}
			}/* ======== END BEARING CORRECT ============ */


			/* ======== DRIVE FORWARDS ============ */
			/* Drive forward once we have our bearing */
			if( non_course_subtask == NON_COURSE_SUBTASK_DRIVE ) {

				/* Set the timer if it hasn't been set */
				if( time_ref_drive == SUBTASK_TIME_REF_NOT_SET ) {
					time_ref_drive = dt;
				}

				/* If there is a timeout, we are done, reset everything */
				if( ( dt - time_ref_drive ) > SUBTASK_PIPE_DRIVE_TIMEOUT ) {

					/* Reset the reference time */
					time_ref_drive = SUBTASK_TIME_REF_NOT_SET;

					/* Reset the subtask number */
					non_course_subtask = NON_COURSE_SUBTASK_CENTER;

					/* This will get annoying */
					printf( "Done with pipe.\n" );

					/* In course mode, return success */
				}
			}/* ======== END DRIVE FORWARDS ============ */

			return SUBTASK_CONTINUING;
		}
		else { /* If we lose the pipe, reset wait times for centering & cearing correct */

			time_ref_center  = SUBTASK_TIME_REF_NOT_SET;
			time_ref_bearing = SUBTASK_TIME_REF_NOT_SET;
			time_ref_drive   = SUBTASK_TIME_REF_NOT_SET;
		}

		return TASK_CONTINUING;
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
	/* Reset the yaw_previous variable */
	msg->target.data.yaw_previous = TASK_YAW_PREVIOUS_NOT_SET;

	/* Reset the yaw_detected variable */
	msg->target.data.yaw_detected = TASK_YAW_PREVIOUS_NOT_SET;

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
	if( msg->task.data.course == TASK_COURSE_ON ) {

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
	else { /* Non-course mode. */

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
		else {
			//msg->target.data.fx = 0;
			//msg->target.data.fy = 0;
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
	/* TODO: Fill this function in like task_buoy() and task_pipe(). */
	if( msg->task.data.course == TASK_COURSE_ON ) {

		switch( msg->task.data.subtask ) {

		case SUBTASK_SEARCH_DEPTH:
			msg->target.data.depth = cf->depth_fence;
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

			/* Start moving forward. */
			msg->target.data.fy = POLOLU_MOVE_FORWARD;
			/* TODO: Fix this magic number. */
			if( msg->vision.data.front_x < -1000 ) {
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
			msg->target.data.yaw   = msg->status.data.yaw + (float)msg->vision.data.front_x * TASK_FENCE_YAW_GAIN;
			msg->target.data.depth = msg->status.data.depth + (float)msg->vision.data.front_y * TASK_FENCE_DEPTH_GAIN;

			/* TODO: Need a way to check for SUCCESS or FAILURE in this case. */
			return SUBTASK_CONTINUING;
		}
	}
	else {
		/* Set the values based on current orientation and pixel error. */
		//msg->target.data.yaw   = msg->status.data.yaw + (float)msg->vision.data.front_x * TASK_FENCE_YAW_GAIN;
		msg->target.data.depth = msg->status.data.depth + (float)msg->vision.data.front_y * TASK_FENCE_DEPTH_GAIN;

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
