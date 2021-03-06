/**
 *  \file task.h
 *  \brief Tasks available for planner.
 */

#ifndef _TASK_H_
#define _TASK_H_

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
#include "pololu.h"

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

/* Tasks. */
#ifndef TASKS
/** @name Tasks that are available for the planner. */
//@{
#define TASKS							1
#define TASK_NONE						0
#define TASK_DOCK						134
#define TASK_GATE   					1
#define TASK_PIPE1   					2
#define TASK_BUOY   					3
#define TASK_PIPE2   					4
#define TASK_FENCE						5
#define TASK_PIPE3   					6
#define TASK_BOXES						7
#define TASK_PIPE4   					8
#define TASK_SURFACE					9
#define TASK_SUITCASE					11
#define TASK_COURSE						100
#define TASK_COURSE_ON					TRUE
#define TASK_COURSE_OFF					FALSE
#define TASK_SQUARE 					20
#define TASK_PIPE						21
#define TASK_NOD						30
#define TASK_SPIN						31
#define TASK_YAW_PREVIOUS_NOT_SET		(-999)
#define TASK_YAW_DETECTED_NOT_SET		(-999)
#define TASK_SWEEP_YAW_ANGLE			2.5
#define TASK_SWEEP_YAW_TIMEOUT			180
#define TASK_SWEEP_YAW_PERIOD			4
#define TASK_BUOY_YAW_GAIN				0.1
#define TASK_BUOY_DEPTH_GAIN			-0.25
#define TASK_BUOY_DEPTH					6.5
#define TASK_BUOY_MAX_DEPTH_DELTA		0.75
#define TASK_BUOY_WAIT_TIME				3
#define TASK_BUOY_HEADING				0.0
#define TASK_BUOY_MAX_SEARCH_TIME		60
#define TASK_PIPE2_YAW					0
#define TASK_FENCE_YAW_GAIN				0.25
#define TASK_FENCE_DEPTH_GAIN			0.01
#define TASK_FENCE_MAX_TIME				100
#define TASK_MAX_TIME					120
#define TASK_PIPE_YAW_GAIN				0.1
#define TASK_PIPE_FX_GAIN				0.15
#define TASK_PIPE_FY_GAIN				0.15
#define TASK_PIPE_FX_MAX				75
#define TASK_PIPE_FY_MAX				75
#define TASK_PIPE_X_THRESH				50
#define TASK_PIPE_Y_THRESH				50
#define TASK_PIPE_YAW_THRESH			10
#define TASK_PIPE_MAX_TIME				5
#define TASK_BOXES_FX_GAIN				0.25
#define TASK_BOXES_FY_GAIN				0.25
#define TASK_BOXES_FX_MAX				75
#define TASK_BOXES_FY_MAX				75
#define TASK_GATE_MAX_TIME				25
#define TASK_GATE_MAX_YAW				30
#define TASK_SUCCESS					1
#define TASK_FAILURE					-1
#define TASK_CONTINUING					0
//@}
#endif /* TASKS */

#ifndef SUBTASKS
/** @name Subtasks that are available for the planner. */
//@{
#define SUBTASKS						1
#define SUBTASK_SEARCH_DEPTH			1
#define SUBTASK_SEARCH					2
#define SUBTASK_CORRECT					3
#define SUBTASK_GATE_MOVE				2
#define SUBTASK_PIPE_CORRECT			1
#define SUBTASK_PIPE_END				2
#define SUBTASK_PIPE_ANGLE_MARGIN		2
#define SUBTASK_PIPE_WINDOW_X			15
#define SUBTASK_PIPE_WINDOW_Y			15
#define SUBTASK_PIPE_DRIVE_TIMEOUT		3
#define SUBTASK_BUOY_TOUCH				4
#define SUBTASK_TIME_REF_NOT_SET		(-1)
#define SUBTASK_TIME_WAIT_CENTER		2
#define SUBTASK_TIME_WAIT_BEARING		4
#define SUBTASK_BOXES_DROP_DEPTH		4
#define SUBTASK_BOXES_DROP				5
#define SUBTASK_BOXES_WINDOW_X			20
#define SUBTASK_BOXES_WINDOW_Y			20
#define SUBTASK_SUITCASE_DROP			4
#define SUBTASK_SURFACE_RISE			4
#define SUBTASK_DEPTH_MARGIN			1.5
#define SUBTASK_YAW_MARGIN				3.0
#define SUBTASK_MAX_SEARCH_TIME			3 //200
#define SUBTASK_GATE_MOVE_TIME			2
#define SUBTASK_SUCCESS					2
#define SUBTASK_FAILURE					-2
#define SUBTASK_CONTINUING				0
//@}
#endif /* SUBTASKS */

#ifndef TASK_STATUS
#define TASK_STATUS
/** @Vision status messages. */
//@{
#define TASK_NOT_DETECTED 		0
#define TASK_BUOY_DETECTED		1
#define TASK_PIPE_DETECTED		2
#define TASK_FENCE_DETECTED		3
#define	TASK_BOXES_DETECTED		4
#define	TASK_SUITCASE_DETECTED	5
#define TASK_GATE_DETECTED	    6
#define TASK_BUOY_TOUCHED       101
#define TASK_PIPE_CENTERED		102
#define TASK_FENCE_CLEARED		103
#define TASK_BOXES_DROPPED		104
#define TASK_SUITCASE_ACQUIRED  105
#define TASK_GATE_CLEARED       106
//@}
#endif /* TASK_STATUS */

#ifndef TASK_SKIP
#define TASK_SKIP 1000
#endif /* TASK_SKIP */


/******************************
**
** Data types
**
******************************/



/******************************
**
** Function prototypes
**
******************************/

//! Switch to the current task.
//! \param msg The current message data.
//! \param cf Configuration file variables.
//! \param dt The task time.
//! \param subtask_dt The subtask time.
//! \return Task status: Success, failure, continuing.
int task_run( MSG_DATA *msg, CONF_VARS *cf, int dt, int subtask_dt );

//! Find and follow the buoy.
//! \param msg The current message data.
//! \param cf Configuration file variables.
//! \param dt The task time.
//! \param subtask_dt The subtask time.
//! \return Task status: Success, failure, continuing.
int task_buoy( MSG_DATA *msg, CONF_VARS *cf, int dt, int subtask_dt );

//! Use dead reckoning to go straight through the gate.
//! \param msg The current message data.
//! \param cf Configuration file variables.
//! \param dt The task time. run.
//! \param subtask_dt The subtask time.
//! \return Task status: Success, failure, continuing.
int task_gate( MSG_DATA *msg, CONF_VARS *cf, int dt, int subtask_dt );

//! Find and follow the pipe.
//! \param msg The current message data.
//! \param dt The task time.
//! \param subtask_dt The subtask time.
//! \return Task status: Success, failure, continuing.
int task_pipe( MSG_DATA *msg, CONF_VARS *cf, int dt, int subtask_dt );

//! Move in a square. Use the times in msg for the duration of motion in each
//! direction.
//! \param msg The current message data.
//! \param cf Configuration file variables.
//! \param dt The task time.
//! \param subtask_dt The subtask time.
//! \return Task status: Success, failure, continuing.
int task_square( MSG_DATA *msg, CONF_VARS *cf, int dt, int subtask_dt );

//! Hold the current position.
//! \param msg The current message data.
//! \param cf Configuration file variables.
//! \param dt The task time.
//! \param subtask_dt The subtask time.
//! \return Task status: Success, failure, continuing.
int task_none( MSG_DATA *msg, CONF_VARS *cf, int dt, int subtask_dt );

//! Find and go to the boxes. Drop marbles over the correct boxes.
//! \param msg The current message data.
//! \param cf Configuration file variables.
//! \param dt The task time.
//! \param subtask_dt The subtask time.
//! \return Task status: Success, failure, continuing.
int task_boxes( MSG_DATA *msg, CONF_VARS *cf, int dt, int subtask_dt );

//! Find and go under the two fence pieces. Stay below the horizontal fence
//! members but above a minimum depth.
//! \param msg The current message data.
//! \param cf Configuration file variables.
//! \param dt The task time.
//! \param subtask_dt The subtask time.
//! \return Task status: Success, failure, continuing.
int task_fence( MSG_DATA *msg, CONF_VARS *cf, int dt, int subtask_dt );

//! Find and retrieve the suitcase. First center vehicle above the suitcase and
//! then lower depth until the suitcase is picked up using the caribiners on
//! the bottoms of the Voith motors.
//! \param msg The current message data.
//! \param cf Configuration file variables.
//! \param dt The task time.
//! \param subtask_dt The subtask time.
//! \return Task status: Success, failure, continuing.
int task_suitcase( MSG_DATA *msg, CONF_VARS *cf, int dt, int subtask_dt );

//! Surface within the octagon.
//! \param msg The current message data.
//! \param cf Configuration file variables.
//! \param dt The task time.
//! \param subtask_dt The subtask time.
//! \return Task status: Success, failure, continuing.
int task_surface( MSG_DATA *msg, CONF_VARS *cf, int dt, int subtask_dt );

//! Run the entire course.
//! \param msg The current message data.
//! \param cf Configuration file variables.
//! \param dt The task time.
//! \param subtask_dt The subtask time.
//! \return Task status: Success, failure, continuing.
int task_course( MSG_DATA *msg, CONF_VARS *cf, int dt, int subtask_dt );

//! Make the sub nod its head.
//! \param msg The current message data.
//! \param cf Configuration file variables.
//! \param dt The task time.
//! \param subtask_dt The subtask time.
//! \return Task status: Success, failure, continuing.
int task_nod( MSG_DATA *msg, CONF_VARS *cf, int dt, int subtask_dt );

//! Make the sub spin in place.
//! \param msg The current message data.
//! \param cf Configuration file variables.
//! \param dt The task time.
//! \param subtask_dt The subtask time.
//! \return Task status: Success, failure, continuing.
int task_spin( MSG_DATA *msg, CONF_VARS *cf, int dt, int subtask_dt );

//! Make the sub sweep side to side looking for obstacles
//! \param msg The current message data.
//! \param cf Configuration file variables.
//! \param dt The task time.
//! \param subtask_dt The subtask time.
//! \return Task status: Success, failure, continuing.
int task_sweep( MSG_DATA *msg, CONF_VARS *cf, int dt, int subtask_dt );

//! Operations to perform at the dock.
//! \param msg The current message data.
//! \param cf Configuration file variables.
//! \param dt The task time.
//! \param subtask_dt The subtask time.
//! \return Task status: Success, failure, continuing.
int task_dock( MSG_DATA *msg, CONF_VARS *cf, int dt, int subtask_dt );

//! A placeholder task to make editing order of course task easier.
//! \return TASK_SUCCESS.
int task_skip( );


#endif /* _TASK_H_ */
