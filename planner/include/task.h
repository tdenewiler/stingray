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
void task_run( MSG_DATA *msg, int dt );

//! Find and follow the buoy.
//! \param msg The current message data.
//! \param dt The task time.
void task_buoy( MSG_DATA *msg, int dt );

//! Use dead reckoning to go straight through the gate.
//! \param msg The current message data.
//! \param heading The desired heading to hold.
//! \param dt The task time.
void task_gate( MSG_DATA *msg, float heading, int dt );

//! Find and follow the pipe.
//! \param msg The current message data.
//! \param dt The task time.
void task_pipe( MSG_DATA *msg, int dt );

//! Move in a square. Use the times in msg for the duration of motion in each
//! direction.
//! \param msg The current message data.
//! \param heading The desired heading to hold.
//! \param dt The task time.
void task_square( MSG_DATA *msg, float heading, int dt );

//! Hold the current position.
//! \param msg The current message data.
//! \param dt The task time.
void task_none( MSG_DATA *msg, int dt );

//! Find and go to the boxes. Drop marbles over the correct boxes.
//! \param msg The current message data.
//! \param dt The task time.
void task_boxes( MSG_DATA *msg, int dt );

//! Find and go under the two fence pieces. Stay below the horizontal fence
//! members but above a minimum depth.
//! \param msg The current message data.
//! \param dt The task time.
void task_fence( MSG_DATA *msg, int dt );

//! Find and retrieve the suitcase. First center vehicle above the suitcase and
//! then lower depth until the suitcase is picked up using the caribiners on
//! the bottoms of the Voith motors.
//! \param msg The current message data.
//! \param dt The task time.
void task_suitcase( MSG_DATA *msg, int dt );


#endif /* _TASK_H_ */
