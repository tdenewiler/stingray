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


/******************************************************************************
 *
 * Title:       int task_run( MSG_DATA *msg, int dt )
 *
 * Description: Switch to the current task.
 *
 * Input:       msg: Current message data.
 *              dt: The task time.
 *
 * Output:      Task status.
 *
 *****************************************************************************/

int task_run( MSG_DATA *msg, int dt )
{
	float heading = 0;
	int status = TASK_CONTINUING;

	switch ( msg->task.data.num ) {
	case TASK_BUOY:
		status = task_buoy( msg, dt );
		break;

	case TASK_GATE:
		status = task_gate( msg, heading, dt );
		break;

	case TASK_PIPE:
		status = task_pipe( msg, dt );
		break;

	case TASK_SQUARE:
		status = task_square( msg, heading, dt );
		break;

	case TASK_NONE:
		status = task_none( msg, dt );
		break;

	case TASK_BOXES:
		status = task_boxes( msg, dt );
		break;

	case TASK_FENCE:
		status = task_fence( msg, dt );
		break;

	case TASK_SUITCASE:
		status = task_suitcase( msg, dt );
		break;
	}
	
	return status;
} /* end task_run() */


/******************************************************************************
 *
 * Title:       int task_buoy( MSG_DATA *msg, int dt )
 *
 * Description: Find and follow the buoy.
 *
 * Input:       msg: Current message data.
 *              dt: The task time.
 *
 * Output:      Task status.
 *
 *****************************************************************************/

int task_buoy( MSG_DATA *msg, int dt )
{
	/* Set target values based on current orientation and pixel error. */
	msg->target.data.yaw = msg->status.data.yaw + (float)msg->vision.data.front_x * TASK_BUOY_GAIN;
	
	return TASK_CONTINUING;
} /* end task_buoy() */


/******************************************************************************
 *
 * Title:       int task_gate( MSG_DATA *msg, float heading, int dt )
 *
 * Description: Use dead reckoning to hold a heading and go straight.
 *
 * Input:       msg: Current message data.
 *              heading: The desired heading to hold.
 *              dt: The task time.
 *
 * Output:      Task status.
 *
 *****************************************************************************/

int task_gate( MSG_DATA *msg, float heading, int dt )
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
 * Title:       int task_pipe( MSG_DATA *msg, int dt )
 *
 * Description: Find and follow the pipe.
 *
 * Input:       msg: Current message data.
 *              dt: The task time.
 *
 * Output:      Task status.
 *
 *****************************************************************************/

int task_pipe( MSG_DATA *msg, int dt )
{
	msg->target.data.yaw    = msg->vision.data.bottom_y;
	msg->target.data.fx     = msg->vision.data.bottom_x;
	
	return TASK_CONTINUING;
} /* end task_pipe() */


/******************************************************************************
 *
 * Title:       int task_square( MSG_DATA *msg, float heading, int dt )
 *
 * Description: Move in a square. Use the times in msg for the duration of
 *              motion in each direction.
 *
 * Input:       msg: Current message data.
 *              heading: The desired heading to hold.
 *              dt: The task time.
 *
 * Output:      Task status.
 *
 *****************************************************************************/

int task_square( MSG_DATA *msg, float heading, int dt )
{
	msg->target.data.pitch = 0;
	msg->target.data.roll = 0;
	msg->target.data.depth = 0;
	msg->target.data.yaw = heading;
	
	return TASK_CONTINUING;
} /* end task_square() */


/******************************************************************************
 *
 * Title:       int task_none( MSG_DATA *msg, int dt )
 *
 * Description: Hold the current position.
 *
 * Input:       msg: Current message data.
 *              dt: The task time.
 *
 * Output:      Task status.
 *
 *****************************************************************************/

int task_none( MSG_DATA *msg, int dt )
{

	return TASK_CONTINUING;
} /* end task_none() */


/******************************************************************************
 *
 * Title:       int task_boxes( MSG_DATA *msg, int dt )
 *
 * Description: Find and go to the boxes. Drop marbles over the correct boxes.
 *
 * Input:       msg: Current message data.
 *              dt: The task time.
 *
 * Output:      Task status.
 *
 *****************************************************************************/

int task_boxes( MSG_DATA *msg, int dt )
{

	return TASK_CONTINUING;
} /* end task_boxes() */


/******************************************************************************
 *
 * Title:       int task_fence( MSG_DATA *msg, int dt )
 *
 * Description: Find and go under the two fence pieces. Stay below the
 * 				horizontal fence members but above a minimum depth.
 *
 * Input:       msg: Current message data.
 *              dt: The task time.
 *
 * Output:      Task status.
 *
 *****************************************************************************/

int task_fence( MSG_DATA *msg, int dt )
{
	/* Put in depth control here. Need to go to the depth returned by the vision
	 * code without going lower than TASK_GATE_MIN_DEPTH as defined in task.h. */
	
	return TASK_CONTINUING;
} /* end task_fence() */


/******************************************************************************
 *
 * Title:       int task_suitcase( MSG_DATA *msg, int dt )
 *
 * Description: Find and retrieve the suitcase. First center vehicle above the
 * 				suitcase and then lower depth until the suitcase is picked up
 * 				using the caribiners on the bottoms of the Voith motors.
 *
 * Input:       msg: Current message data.
 *              dt: The task time.
 *
 * Output:      Task status.
 *
 *****************************************************************************/

int task_suitcase( MSG_DATA *msg, int dt )
{
	
	return TASK_CONTINUING;
} /* end task_suitcase() */


/******************************************************************************
 *
 * Title:       int task_course( MSG_DATA *msg, int dt )
 *
 * Description: Run the entire course.
 *
 * Input:       msg: Current message data.
 *              dt: The task time.
 *
 * Output:      Task status.
 *
 *****************************************************************************/

int task_course( MSG_DATA *msg, int dt )
{

	return TASK_CONTINUING;
} /* end task_course() */
