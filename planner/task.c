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


/******************************************************************************
 *
 * Title:       int task_run( MSG_DATA *msg, int dt, int subtask, int subtask_dt )
 *
 * Description: Switch to the current task.
 *
 * Input:       msg: Current message data.
 *              dt: The task time.
 *				subtask: Used to set which part of the task is to be run. Modify
 * 				upon success or failure.
 * 				subtask_dt: The subtask time.
 * 
 * Output:      Task status: Success, failure, continuing.
 *
 *****************************************************************************/

int task_run( MSG_DATA *msg, int dt, int subtask, int subtask_dt )
{
	int status = TASK_CONTINUING;

	switch ( msg->task.data.num ) {
	case TASK_BUOY:
		status = task_buoy( msg, dt, subtask, subtask_dt );
		break;

	case TASK_GATE:
		status = task_gate( msg, dt, subtask, subtask_dt );
		break;

	case TASK_PIPE:
		status = task_pipe( msg, dt, subtask, subtask_dt );
		break;

	case TASK_PIPE1:
		status = task_pipe( msg, dt, subtask, subtask_dt );
		break;

	case TASK_PIPE2:
		status = task_pipe( msg, dt, subtask, subtask_dt );
		break;

	case TASK_PIPE3:
		status = task_pipe( msg, dt, subtask, subtask_dt );
		break;

	case TASK_PIPE4:
		status = task_pipe( msg, dt, subtask, subtask_dt );
		break;

	case TASK_SQUARE:
		status = task_square( msg, dt, subtask, subtask_dt );
		break;

	case TASK_NONE:
		status = task_none( msg, dt, subtask, subtask_dt );
		break;

	case TASK_BOXES:
		status = task_boxes( msg, dt, subtask, subtask_dt );
		break;

	case TASK_FENCE:
		status = task_fence( msg, dt, subtask, subtask_dt );
		break;

	case TASK_SURFACE:
		status = task_surface( msg, dt, subtask, subtask_dt );
		break;

	case TASK_SUITCASE:
		status = task_suitcase( msg, dt, subtask, subtask_dt );
		break;

	case TASK_NOD:
		status = task_nod( msg, dt, subtask, subtask_dt );
		break;

	case TASK_SPIN:
		status = task_spin( msg, dt, subtask, subtask_dt );
		break;

	case TASK_COURSE:
		status = task_course( msg, dt, subtask, subtask_dt );
		break;
	}

	return status;
} /* end task_run() */


/******************************************************************************
 *
 * Title:       int task_buoy( MSG_DATA *msg, int dt, int subtask, int subtask_dt )
 *
 * Description: Find and follow the buoy.
 *
 * Input:       msg: Current message data.
 *              dt: The task time.
 *				subtask: Used to set which part of the task is to be run. Modify
 * 				upon success or failure.
 * 				subtask_dt: The subtask time.
 *
 * Output:      Task status: Success, failure, continuing.
 *
 *****************************************************************************/

int task_buoy( MSG_DATA *msg, int dt, int subtask, int subtask_dt )
{
	if( msg->task.data.num == TASK_COURSE ) {
		switch( subtask ) {
		case SUBTASK_SEARCH_DEPTH:
			msg->target.data.depth = 0;
			/* Check to see if we have reached the target depth. */
			if( fabsf(msg->status.data.depth - msg->target.data.depth) < SUBTASK_BUOY_DEPTH_MARGIN ) {
				return SUBTASK_SUCCESS;
			}
			/* Check to see if too much time has elapsed. */
			else if( subtask_dt > TASK_BUOY_MAX_DETECT_TIME ) {
				return SUBTASK_FAILURE;
			}
			/* Otherwise we are continuing. */
			else {
				return SUBTASK_CONTINUING;
			}
		case SUBTASK_SEARCH:
			/* Now set the vision server to look for the buoy. */
			msg->task.data.subtask = TASK_BUOY;
			/* Start moving forward. */
			msg->target.data.fy = POLOLU_MOVE_FORWARD;
			if( msg->vision.data.front_x < -1000 ) {
				return SUBTASK_SUCCESS;
			}
			else if( subtask_dt > TASK_BUOY_MAX_DETECT_TIME ) {
				return SUBTASK_FAILURE;
			}
			else {
				return SUBTASK_CONTINUING;
			}
		case SUBTASK_CORRECT:
			/* Set target values based on current orientation and pixel error. */
			msg->target.data.yaw = msg->status.data.yaw + (float)msg->vision.data.front_x * TASK_BUOY_GAIN;
			msg->target.data.depth = msg->status.data.depth + (float)msg->vision.data.front_y * TASK_BUOY_GAIN / 100.;
			return SUBTASK_CONTINUING;
		}
	}
	else {
		/* Set target values based on current orientation and pixel error. */
		msg->target.data.yaw = msg->status.data.yaw + (float)msg->vision.data.front_x * TASK_BUOY_GAIN;
		
		/* Need to divide the depth target by a largish number because the gain works
		 * for yaw in degrees but not for depth in volts. The pixel error needs to
		 * be converted so that it is meaningful for volts as well. */
		msg->target.data.depth = msg->status.data.depth + (float)msg->vision.data.front_y * TASK_BUOY_GAIN / 100.;

		return TASK_CONTINUING;
	}
	
	return TASK_CONTINUING;
} /* end task_buoy() */


/******************************************************************************
 *
 * Title:       int task_gate( MSG_DATA *msg, int dt, int subtask, int subtask_dt )
 *
 * Description: Use dead reckoning to hold a heading and go straight.
 *
 * Input:       msg: Current message data.
 *              dt: The task time.
 *				subtask: Used to set which part of the task is to be run. Modify
 * 				upon success or failure.
 * 				subtask_dt: The subtask time.
 *
 * Output:      Task status: Success, failure, continuing.
 *
 *****************************************************************************/

int task_gate( MSG_DATA *msg, int dt, int subtask, int subtask_dt )
{
	/* Use the known direction from the start dock to the validation gate here. */
	msg->target.data.yaw = TASK_BUOY_HEADING;

	/* Use a nominal starting depth for getting through the gate. It will
	 * probably be equal to the depth of the buoy so that we can start looking
	 * for the buoy right away. */
	msg->target.data.depth = TASK_BUOY_DEPTH;

	return TASK_CONTINUING;
} /* end task_gate() */


/******************************************************************************
 *
 * Title:       int task_pipe( MSG_DATA *msg, int dt, int subtask, int subtask_dt )
 *
 * Description: Find and follow the pipe.
 *
 * Input:       msg: Current message data.
 *              dt: The task time.
 *				subtask: Used to set which part of the task is to be run. Modify
 * 				upon success or failure.
 * 				subtask_dt: The subtask time.
 *
 * Output:      Task status: Success, failure, continuing.
 *
 *****************************************************************************/

int task_pipe( MSG_DATA *msg, int dt, int subtask, int subtask_dt )
{
	/* Set the values based on current orientation and pixel error. */
	msg->target.data.yaw    = msg->status.data.yaw + (float)msg->vision.data.bottom_y * TASK_PIPE_GAIN;
	//msg->target.data.fx     = msg->vision.data.bottom_x;

	return TASK_CONTINUING;
} /* end task_pipe() */


/******************************************************************************
 *
 * Title:       int task_square( MSG_DATA *msg, int dt, int subtask, int subtask_dt )
 *
 * Description: Move in a square. Use the times in msg for the duration of
 *              motion in each direction.
 *
 * Input:       msg: Current message data.
 *              dt: The task time.
 *				subtask: Used to set which part of the task is to be run. Modify
 * 				upon success or failure.
 * 				subtask_dt: The subtask time.
 *
 * Output:      Task status: Success, failure, continuing.
 *
 *****************************************************************************/

int task_square( MSG_DATA *msg, int dt, int subtask, int subtask_dt )
{
	msg->target.data.pitch = 0;
	msg->target.data.roll = 0;
	msg->target.data.depth = 0;

	return TASK_CONTINUING;
} /* end task_square() */


/******************************************************************************
 *
 * Title:       int task_none( MSG_DATA *msg, int dt, int subtask, int subtask_dt )
 *
 * Description: Hold the current position.
 *
 * Input:       msg: Current message data.
 *              dt: The task time.
 *				subtask: Used to set which part of the task is to be run. Modify
 * 				upon success or failure.
 * 				subtask_dt: The subtask time.
 *
 * Output:      Task status: Success, failure, continuing.
 *
 *****************************************************************************/

int task_none( MSG_DATA *msg, int dt, int subtask, int subtask_dt )
{

	return TASK_CONTINUING;
} /* end task_none() */


/******************************************************************************
 *
 * Title:       int task_boxes( MSG_DATA *msg, int dt, int subtask, int subtask_dt )
 *
 * Description: Find and go to the boxes. Drop marbles over the correct boxes.
 *
 * Input:       msg: Current message data.
 *              dt: The task time.
 *				subtask: Used to set which part of the task is to be run. Modify
 * 				upon success or failure.
 * 				subtask_dt: The subtask time.
 *
 * Output:      Task status: Success, failure, continuing.
 *
 *****************************************************************************/

int task_boxes( MSG_DATA *msg, int dt, int subtask, int subtask_dt )
{

	return TASK_CONTINUING;
} /* end task_boxes() */


/******************************************************************************
 *
 * Title:       int task_fence( MSG_DATA *msg, int dt, int subtask, int subtask_dt )
 *
 * Description: Find and go under the two fence pieces. Stay below the
 * 				horizontal fence members but above a minimum depth.
 *
 * Input:       msg: Current message data.
 *              dt: The task time.
 *				subtask: Used to set which part of the task is to be run. Modify
 * 				upon success or failure.
 * 				subtask_dt: The subtask time.
 *
 * Output:      Task status: Success, failure, continuing.
 *
 *****************************************************************************/

int task_fence( MSG_DATA *msg, int dt, int subtask, int subtask_dt )
{
	/* Put in depth control here. Need to go to the depth returned by the vision
	 * code without going lower than TASK_GATE_MIN_DEPTH as defined in task.h. */

	return TASK_CONTINUING;
} /* end task_fence() */


/******************************************************************************
 *
 * Title:       int task_suitcase( MSG_DATA *msg, int dt, int subtask, int subtask_dt )
 *
 * Description: Find and retrieve the suitcase. First center vehicle above the
 * 				suitcase and then lower depth until the suitcase is picked up
 * 				using the caribiners on the bottoms of the Voith motors.
 *
 * Input:       msg: Current message data.
 *              dt: The task time.
 *				subtask: Used to set which part of the task is to be run. Modify
 * 				upon success or failure.
 * 				subtask_dt: The subtask time.
 *
 * Output:      Task status: Success, failure, continuing.
 *
 *****************************************************************************/

int task_suitcase( MSG_DATA *msg, int dt, int subtask, int subtask_dt )
{

	return TASK_CONTINUING;
} /* end task_suitcase() */


/******************************************************************************
 *
 * Title:       int task_surface( MSG_DATA *msg, int dt, int subtask, int subtask_dt )
 *
 * Description: Surface within the octagon.
 *
 * Input:       msg: Current message data.
 *              dt: The task time.
 *				subtask: Used to set which part of the task is to be run. Modify
 * 				upon success or failure.
 * 				subtask_dt: The subtask time.
 *
 * Output:      Task status: Success, failure, continuing.
 *
 *****************************************************************************/

int task_surface( MSG_DATA *msg, int dt, int subtask, int subtask_dt )
{

	return TASK_CONTINUING;
} /* end task_surface() */


/******************************************************************************
 *
 * Title:       int task_course( MSG_DATA *msg, int dt, int subtask, int subtask_dt )
 *
 * Description: Run the entire course.
 *
 * Input:       msg: Current message data.
 *              dt: The task time.
 *				subtask: Used to set which part of the task is to be run. Modify
 * 				upon success or failure.
 * 				subtask_dt: The subtask time.
 *
 * Output:      Task status: Success, failure, continuing.
 *
 *****************************************************************************/

int task_course( MSG_DATA *msg, int dt, int subtask, int subtask_dt )
{

	return TASK_CONTINUING;
} /* end task_course() */


/******************************************************************************
 *
 * Title:       int task_nod( MSG_DATA *msg, int dt, int subtask, int subtask_dt )
 *
 * Description: Make the sub nod its head.
 *
 * Input:       msg: Current message data.
 *              dt: The task time.
 *				subtask: Used to set which part of the task is to be run. Modify
 * 				upon success or failure.
 * 				subtask_dt: The subtask time.
 *
 * Output:      Task status: Success, failure, continuing.
 *
 *****************************************************************************/

int task_nod( MSG_DATA *msg, int dt, int subtask, int subtask_dt )
{

	return TASK_CONTINUING;
} /* end task_nod() */


/******************************************************************************
 *
 * Title:       int task_spin( MSG_DATA *msg, int dt, int subtask, int subtask_dt )
 *
 * Description: Make the sub spin in place.
 *
 * Input:       msg: Current message data.
 *              dt: The task time.
 *				subtask: Used to set which part of the task is to be run. Modify
 * 				upon success or failure.
 * 				subtask_dt: The subtask time.
 *
 * Output:      Task status: Success, failure, continuing.
 *
 *****************************************************************************/

int task_spin( MSG_DATA *msg, int dt, int subtask, int subtask_dt )
{
	/* Continuously add 1 degree to yaw every time through this loop. It might
	 * be better to only add the 1 degree if enough time has elapsed by using
	 * the dt argument. */
	msg->target.data.yaw++;

	return TASK_CONTINUING;
} /* end task_spin() */
