/**
 *  \file task.h
 *  \brief Tasks available for planner.
 */

#ifndef _TASK_H_
#define _TASK_H_

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
#define TASKS
#define TASK_NONE						0
#define TASK_BUOY   					1
#define TASK_PIPE   					2
#define TASK_GATE   					3
#define TASK_SQUARE 					4
#define TASK_FENCE						5
#define TASK_BOXES						6
#define TASK_SUITCASE					7
#define TASK_COURSE						8
#define TASK_BUOY_GAIN					0.25
#define TASK_BUOY_DEPTH					0.690
#define TASK_BUOY_HEADING				0.0
#define TASK_BUOY_MAX_DETECT_TIME		180
#define TASK_BUOY_MAX_TIME				300
#define TASK_FENCE_MIN_DEPTH			0.700
#define TASK_FENCE_MAX_DETECT_TIME		180
#define TASK_FENCE_MAX_TIME				300
#define TASK_PIPE_MAX_DETECT_TIME		180
#define TASK_PIPE_MAX_TIME				300
#define TASK_BOXES_MAX_DETECT_TIME		180
#define TASK_BOXES_MAX_TIME				300
#define TASK_SUITCASE_MAX_DETECT_TIME	180
#define TASK_SUITCASE_MAX_TIME			300
#define TASK_SUCCESS					1
#define TASK_FAILURE					-1
#define TASK_CONTINUING					0
//@}
#endif /* TASKS */


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
//! \param dt The task time.
//! \return Task status.
int task_run( MSG_DATA *msg, int dt );

//! Find and follow the buoy.
//! \param msg The current message data.
//! \param dt The task time.
//! \return Task status.
int task_buoy( MSG_DATA *msg, int dt );

//! Use dead reckoning to go straight through the gate.
//! \param msg The current message data.
//! \param heading The desired heading to hold.
//! \param dt The task time.
//! \return Task status.
int task_gate( MSG_DATA *msg, float heading, int dt );

//! Find and follow the pipe.
//! \param msg The current message data.
//! \param dt The task time.
//! \return Task status.
int task_pipe( MSG_DATA *msg, int dt );

//! Move in a square. Use the times in msg for the duration of motion in each
//! direction.
//! \param msg The current message data.
//! \param heading The desired heading to hold.
//! \param dt The task time.
//! \return Task status.
int task_square( MSG_DATA *msg, float heading, int dt );

//! Hold the current position.
//! \param msg The current message data.
//! \param dt The task time.
//! \return Task status.
int task_none( MSG_DATA *msg, int dt );

//! Find and go to the boxes. Drop marbles over the correct boxes.
//! \param msg The current message data.
//! \param dt The task time.
//! \return Task status.
int task_boxes( MSG_DATA *msg, int dt );

//! Find and go under the two fence pieces. Stay below the horizontal fence
//! members but above a minimum depth.
//! \param msg The current message data.
//! \param dt The task time.
//! \return Task status.
int task_fence( MSG_DATA *msg, int dt );

//! Find and retrieve the suitcase. First center vehicle above the suitcase and
//! then lower depth until the suitcase is picked up using the caribiners on
//! the bottoms of the Voith motors.
//! \param msg The current message data.
//! \param dt The task time.
//! \return Task status.
int task_suitcase( MSG_DATA *msg, int dt );

//! Run the entire course.
//! \param msg The current message data.
//! \param dt The task time.
//! \return Task status.
int task_course( MSG_DATA *msg, int dt );


#endif /* _TASK_H_ */
