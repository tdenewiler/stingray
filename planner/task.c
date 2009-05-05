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
 * Title:       void task_run( MSG_DATA *msg, int dt )
 *
 * Description: Switch to the current task.
 *
 * Input:       msg: Current message data.
 *              dt: The task time.
 *
 * Output:      None.
 *
 * Globals:     None.
 *
 *****************************************************************************/

void task_run( MSG_DATA *msg, int dt )
{
	float heading = 0;

	switch ( msg->task.data.num ) {

		case TASK_BUOY:
			task_buoy( msg, dt );
			break;

		case TASK_GATE:
			task_gate( msg, heading, dt );
			break;

		case TASK_PIPE:
			task_pipe( msg, dt );
			break;

		case TASK_SQUARE:
			task_square( msg, heading, dt );
			break;
	}
} /* end task_run() */


/******************************************************************************
 *
 * Title:       void task_buoy( MSG_DATA *msg, int dt )
 *
 * Description: Find and follow the buoy.
 *
 * Input:       msg: Current message data.
 *              dt: The task time.
 *
 * Output:      None.
 *
 * Globals:     None.
 *
 *****************************************************************************/

void task_buoy( MSG_DATA *msg, int dt )
{
	/* Set target values based on current orientation and pixel error. */
	msg->target.data.yaw = msg->mstrain.data.yaw + (float)msg->vision.data.front_x / 10.;
	
} /* end task_buoy() */


/******************************************************************************
 *
 * Title:       void task_gate( MSG_DATA *msg, float heading, int dt )
 *
 * Description: Use dead reckoning to hold a heading and go straight.
 *
 * Input:       msg: Current message data.
 *              heading: The desired heading to hold.
 *              dt: The task time.
 *
 * Output:      None.
 *
 * Globals:     None.
 *
 *****************************************************************************/

void task_gate( MSG_DATA *msg, float heading, int dt )
{
	msg->target.data.yaw = heading;
} /* end task_gate() */


/******************************************************************************
 *
 * Title:       void task_pipe( MSG_DATA *msg, int dt )
 *
 * Description: Find and follow the pipe.
 *
 * Input:       msg: Current message data.
 *              dt: The task time.
 *
 * Output:      None.
 *
 * Globals:     None.
 *
 *****************************************************************************/

void task_pipe( MSG_DATA *msg, int dt )
{
	msg->target.data.yaw    = msg->vision.data.bottom_y;
	msg->target.data.fy     = msg->vision.data.bottom_x;
} /* end task_pipe() */


/******************************************************************************
 *
 * Title:       void task_square( MSG_DATA *msg, float heading, int dt )
 *
 * Description: Move in a square. Use the times in msg for the duration of
 *              motion in each direction.
 *
 * Input:       msg: Current message data.
 *              heading: The desired heading to hold.
 *              dt: The task time.
 *
 * Output:      None.
 *
 * Globals:     None.
 *
 *****************************************************************************/

void task_square( MSG_DATA *msg, float heading, int dt )
{
	msg->target.data.pitch = 0;
	msg->target.data.roll = 0;
	msg->target.data.depth = 0.518;
	msg->target.data.yaw = heading;
} /* end task_square() */
