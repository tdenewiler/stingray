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
#define TASKS							1
#define TASK_NONE						0
#define TASK_GATE   					1
#define TASK_PIPE1   					2
#define TASK_BUOY   					3
#define TASK_PIPE2   					4
#define TASK_FENCE						5
#define TASK_PIPE3   					6
#define TASK_BOXES						7
#define TASK_PIPE4   					8
#define TASK_SUITCASE					9
#define TASK_SURFACE					10
#define TASK_COURSE						100
#define TASK_SQUARE 					20
#define TASK_PIPE						21
#define TASK_NOD						30
#define TASK_SPIN						31
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
#define TASK_SURFACE_MAX_DETECT_TIME	180
#define TASK_SURFACE_MAX_TIME			300
#define TASK_SUITCASE_MAX_DETECT_TIME	180
#define TASK_SUITCASE_MAX_TIME			300
#define TASK_PIPE_GAIN					0.5
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
//! \param subtask A pointer to which part of a task we are working on. This
//! value should be modified within the individual task functions to reflect
//! success or continuing of that part of a task.
//! \return Task status: Success, failure, continuing.
int task_run( MSG_DATA *msg, int dt, int *subtask );

//! Find and follow the buoy.
//! \param msg The current message data.
//! \param dt The task time.
//! \param subtask Used to set which part of the task is to be run. Modify upon
//! success or failure.
//! \return Task status: Success, failure, continuing.
int task_buoy( MSG_DATA *msg, int dt, int *subtask );

//! Use dead reckoning to go straight through the gate.
//! \param msg The current message data.
//! \param dt The task time.
//! \param subtask Used to set which part of the task is to be run. Modify upon
//! success or failure.
//! \return Task status: Success, failure, continuing.
int task_gate( MSG_DATA *msg, int dt, int *subtask );

//! Find and follow the pipe.
//! \param msg The current message data.
//! \param dt The task time.
//! \param subtask Used to set which part of the task is to be run. Modify upon
//! success or failure.
//! \return Task status: Success, failure, continuing.
int task_pipe( MSG_DATA *msg, int dt, int *subtask );

//! Move in a square. Use the times in msg for the duration of motion in each
//! direction.
//! \param msg The current message data.
//! \param dt The task time.
//! \param subtask Used to set which part of the task is to be run. Modify upon
//! success or failure.
//! \return Task status: Success, failure, continuing.
int task_square( MSG_DATA *msg, int dt, int *subtask );

//! Hold the current position.
//! \param msg The current message data.
//! \param dt The task time.
//! \param subtask Used to set which part of the task is to be run. Modify upon
//! success or failure.
//! \return Task status: Success, failure, continuing.
int task_none( MSG_DATA *msg, int dt, int *subtask );

//! Find and go to the boxes. Drop marbles over the correct boxes.
//! \param msg The current message data.
//! \param dt The task time.
//! \param subtask Used to set which part of the task is to be run. Modify upon
//! success or failure.
//! \return Task status: Success, failure, continuing.
int task_boxes( MSG_DATA *msg, int dt, int *subtask );

//! Find and go under the two fence pieces. Stay below the horizontal fence
//! members but above a minimum depth.
//! \param msg The current message data.
//! \param dt The task time.
//! \param subtask Used to set which part of the task is to be run. Modify upon
//! success or failure.
//! \return Task status: Success, failure, continuing.
int task_fence( MSG_DATA *msg, int dt, int *subtask );

//! Find and retrieve the suitcase. First center vehicle above the suitcase and
//! then lower depth until the suitcase is picked up using the caribiners on
//! the bottoms of the Voith motors.
//! \param msg The current message data.
//! \param dt The task time.
//! \param subtask Used to set which part of the task is to be run. Modify upon
//! success or failure.
//! \return Task status: Success, failure, continuing.
int task_suitcase( MSG_DATA *msg, int dt, int *subtask );

//! Surface within the octagon.
//! \param msg The current message data.
//! \param dt The task time.
//! \param subtask Used to set which part of the task is to be run. Modify upon
//! success or failure.
//! \return Task status: Success, failure, continuing.
int task_surface( MSG_DATA *msg, int dt, int *subtask );

//! Run the entire course.
//! \param msg The current message data.
//! \param dt The task time.
//! \param subtask Used to set which part of the task is to be run. Modify upon
//! success or failure.
//! \return Task status: Success, failure, continuing.
int task_course( MSG_DATA *msg, int dt, int *subtask );

//! Make the sub nod its head.
//! \param msg The current message data.
//! \param dt The task time.
//! \param subtask Used to set which part of the task is to be run. Modify upon
//! success or failure.
//! \return Task status: Success, failure, continuing.
int task_nod( MSG_DATA *msg, int dt, int *subtask );

//! Make the sub spin in place.
//! \param msg The current message data.
//! \param dt The task time.
//! \param subtask Used to set which part of the task is to be run. Modify upon
//! success or failure.
//! \return Task status: Success, failure, continuing.
int task_spin( MSG_DATA *msg, int dt, int *subtask );


#endif /* _TASK_H_ */
