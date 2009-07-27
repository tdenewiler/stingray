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

static int hasPrinted  = FALSE;
static int hasDetected = FALSE;
static int hasCentered = FALSE;
static int hasSuccess = FALSE;


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
			/* Set depth to configuration file value. */
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
			/* Go forward without visual feedback for an amount of time set in
			 * configuration file. */
			if( subtask_dt < cf->buoy_blind_time * 1000000 ) {
				msg->target.data.fy = POLOLU_MOVE_FORWARD;
				return SUBTASK_CONTINUING;
			}
			else {
				return SUBTASK_SUCCESS;
			}
			
		case SUBTASK_CORRECT:
			/* Set search yaw to configuration file value. */
			msg->target.data.yaw = cf->heading_buoy;
			
			/* Start moving forward. */
			msg->target.data.fy = POLOLU_MOVE_FORWARD;

			/* Check to see if we think we have detected the buoy. */
			if( msg->vision.data.status == TASK_BUOY_DETECTED ) {
				return SUBTASK_SUCCESS;
			}
			else if( subtask_dt > SUBTASK_MAX_SEARCH_TIME ) {
				return SUBTASK_FAILURE;
			}
			else {
				return SUBTASK_CONTINUING;
			}
		
		case SUBTASK_BUOY_TOUCH:
			/* Set target values based on current orientation and pixel error. */
			msg->target.data.yaw = msg->status.data.yaw +
				(float)msg->vision.data.front_x * TASK_BUOY_YAW_GAIN;
			msg->target.data.depth = msg->status.data.depth +
				(float)msg->vision.data.front_y * TASK_BUOY_DEPTH_GAIN;

			/* Check to see if we think we have touched the buoy. */
			if( msg->vision.data.status == TASK_BUOY_TOUCHED ) {
				return TASK_SUCCESS;
			}
			else if( subtask_dt > SUBTASK_MAX_SEARCH_TIME ) {
				return SUBTASK_FAILURE;
			}
			else {
				return SUBTASK_CONTINUING;
			}
		}
	}

	/* Non-course mode. */
	else {
		
		if( !hasPrinted ) {
			printf( "Planner: Looking for Bouy\n" );
			hasPrinted = TRUE;
		}
			
		
		/* Check to see if we have detected the buoy. Else don't change yaw or
		 * depth, just keep old values. */
		if( msg->vision.data.status == TASK_BUOY_DETECTED ) {
			
			/* Set target values based on current position and pixel error. */
			msg->target.data.yaw = msg->status.data.yaw +
				(float)msg->vision.data.front_x * TASK_BUOY_YAW_GAIN;
				
			msg->target.data.depth = msg->status.data.depth +
				(float)msg->vision.data.front_y * TASK_BUOY_DEPTH_GAIN;
				
			if( !hasDetected ) {
				printf( "Planner: Bouy Detected\n" );
				hasDetected = TRUE;
			}
		}
		
		if( msg->vision.data.status == TASK_BUOY_TOUCHED ) {
			if( !hasSuccess ) {
				printf( "Planner: Bouy Touched\n" );
				hasSuccess = TRUE;
			}
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
	if( msg->task.data.course == TASK_COURSE_ON ) {
		switch( msg->task.data.subtask ) {

		case SUBTASK_SEARCH_DEPTH:
			/* Set depth to configuration file value. */
			msg->target.data.depth = cf->depth_buoy;

			/* Check to see if we have reached the target depth. */
			if( fabsf(msg->status.data.depth - msg->target.data.depth) <
				SUBTASK_DEPTH_MARGIN ) {
				return SUBTASK_SUCCESS;
			}
			/* Check to see if too much time has elapsed. */
			else if( subtask_dt > SUBTASK_MAX_SEARCH_TIME ) {
				return TASK_FAILURE;
			}
			else {
				return SUBTASK_CONTINUING;
			}
		
		case SUBTASK_SEARCH:
			/* Set heading to configuration file value. */
			msg->target.data.yaw = cf->heading_gate;
			
			/* Check to see if we have reached the target heading. */
			if( fabsf(msg->status.data.yaw - msg->target.data.yaw) <
				SUBTASK_YAW_MARGIN ) {
				return SUBTASK_SUCCESS;
			}
			/* Check to see if too much time has elapsed. */
			else if( subtask_dt > SUBTASK_MAX_SEARCH_TIME ) {
				return TASK_FAILURE;
			}
			else {
				return SUBTASK_CONTINUING;
			}
		
		case SUBTASK_CORRECT:
			/* Move forward for a set amount of time. Multiply by 1,000,000 to
			 * convert from seconds to microseconds because dt is in
			 * microseconds. */
			if( subtask_dt < SUBTASK_GATE_MOVE_TIME * 1000000 ) {
				msg->target.data.fy = POLOLU_MOVE_FORWARD;
				return SUBTASK_CONTINUING;
			}
			else if( dt > SUBTASK_MAX_SEARCH_TIME ) {
				return TASK_FAILURE;
			}
			else {
				return TASK_SUCCESS;
			}
		}
	}
	
	/* Non-course mode. */
	else {
		/* Use the known direction from the start dock to the gate here. */
		msg->target.data.yaw = cf->heading_buoy;

		/* Use a nominal starting depth for getting through the gate. It will
		 * probably be equal to the depth of the buoy so that we can start
		 * looking for the buoy right away. */
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
	if( msg->task.data.course == TASK_COURSE_ON ) {
		switch( msg->task.data.subtask ) {

		case SUBTASK_SEARCH_DEPTH:
			/* Set depth to configuration file value. */
			msg->target.data.depth = cf->depth_pipe;
			
			/* Check to see if we have reached the target depth. */
			if( fabsf(msg->status.data.depth - msg->target.data.depth) <
				SUBTASK_DEPTH_MARGIN ) {
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
				/* Set forward speed to zero. If we are not at the centroid the
				 * next subtask will correct for it. */
				msg->target.data.fy = 0;
				return SUBTASK_SUCCESS;
			}
			/* Check to see if too much time has elapsed. */
			else if( subtask_dt > SUBTASK_MAX_SEARCH_TIME ) {
				return SUBTASK_FAILURE;
			}
			else {
				return SUBTASK_CONTINUING;
			}

		case SUBTASK_CORRECT:
			/* Set fx and fy based on centroid pixel error. */
			msg->target.data.fx = msg->vision.data.bottom_x * TASK_PIPE_FX_GAIN;
			msg->target.data.fy = msg->vision.data.bottom_y * TASK_PIPE_FY_GAIN;

			/* Check to see if we are over the centroid. */
			if( msg->vision.data.bottom_x < TASK_PIPE_X_THRESH &&
				msg->vision.data.bottom_y < TASK_PIPE_Y_THRESH ) {
				return SUBTASK_SUCCESS;
			}
			else if( subtask_dt > SUBTASK_MAX_SEARCH_TIME ) {
				return SUBTASK_FAILURE;
			}
			else {
				return SUBTASK_CONTINUING;
			}

		case SUBTASK_PIPE_END:
			/* Set heading base on pixel error. */
			msg->target.data.yaw = msg->status.data.yaw +
				msg->vision.data.bearing * TASK_PIPE_YAW_GAIN;
			
			/* Check to see if we are at pipe heading. */
			if( fabsf(msg->target.data.yaw - msg->status.data.yaw) <
				TASK_PIPE_YAW_THRESH ) {
				return TASK_SUCCESS;
			}
			/* Check to see if too much time has elapsed. */
			else if( subtask_dt > SUBTASK_MAX_SEARCH_TIME ) {
				return SUBTASK_FAILURE;
			}
			else {
				return SUBTASK_CONTINUING;
			}
		}
	}
	
	/* Non-course mode. */
	else {
		
		if( !hasPrinted ) {
			printf( "Planner: Looking for Pipe\n" );
			hasPrinted = TRUE;
		}
		
		/* Move forward until pipe is detected. */
		if( msg->vision.data.status != TASK_PIPE_CENTERED ||
			msg->vision.data.status != TASK_PIPE_DETECTED ) {
			msg->target.data.fy = POLOLU_MOVE_FORWARD;
		}
		
		if( msg->vision.data.status == TASK_PIPE_CENTERED ) {
			msg->target.data.fx = 0;
			msg->target.data.fy = 0;
			msg->target.data.yaw = msg->status.data.yaw +
				msg->vision.data.bearing * TASK_PIPE_YAW_GAIN;
				
			if( !hasCentered ) {
				printf( "Planner: Pipe Centered\n" );
				hasCentered = TRUE;
			}
		}
		else if( msg->vision.data.status == TASK_PIPE_DETECTED ) {
			
			msg->target.data.fx = msg->vision.data.bottom_x * TASK_PIPE_FX_GAIN;
			msg->target.data.fy = msg->vision.data.bottom_y * TASK_PIPE_FY_GAIN;
			
			if( !hasDetected ) {
				printf( "Planner: Pipe Detected\n" );
				hasDetected = TRUE;
			}
		}
		
		
		
		
		/* Move fx and fy until we are centered. *
		else if( msg->vision.data.status == TASK_PIPE_DETECTED ) {
			msg->target.data.fx = msg->vision.data.bottom_x * TASK_PIPE_FX_GAIN;
			msg->target.data.fy = msg->vision.data.bottom_y * TASK_PIPE_FY_GAIN;
			
			if( !hasDetected ) {
				printf( "Planner: Pipe Detected\n" );
				hasDetected = TRUE;
			}
		}
		/* Correct yaw. ( status ==  TASK_PIPE_CENTERED ) *
		else {
			msg->target.data.fx = 0;
			msg->target.data.fy = 0;
			msg->target.data.yaw = msg->status.data.yaw +
				msg->vision.data.bearing * TASK_PIPE_YAW_GAIN;
				
			if( !hasCentered ) {
				printf( "Planner: Pipe Centered\n" );
				hasCentered = TRUE;
			}
		}
		* 
		*/
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
	hasPrinted = FALSE;
	hasDetected = FALSE;
	hasSuccess = FALSE;
	hasCentered = FALSE;
	
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
			msg->target.data.yaw   = msg->status.data.yaw +
				(float)msg->vision.data.front_x * TASK_FENCE_YAW_GAIN;

			/* TODO: Need a way to check for SUCCESS or FAILURE in this case. */
			return SUBTASK_CONTINUING;
		}
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
