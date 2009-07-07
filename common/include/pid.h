/**
 *  \file pid.h
 *  \brief Proportional - Integral - Dervivative controller functions.
 */

#ifndef _PID_H_
#define _PID_H_

#include "messages.h"
#include "parser.h"
#include "labjack.h"


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

#ifndef PID_BOUNDS
#define PID_BOUNDS
#define PID_ROLL_TORQUE 80
#define PID_PITCH_TORQUE 80
#define PID_YAW_TORQUE 80
#define PID_VERTICAL_THRUST 80
#define PID_TOTAL_VERTICAL_THRUST 80
#define PID_ROLL_INTEGRAL 30
#define PID_PITCH_INTEGRAL 50
#define PID_YAW_INTEGRAL 40
#define PID_DEPTH_INTEGRAL 75
#endif /* PID_BOUNDS */

#ifndef PID_MODE
#define PID_MODE
#define PID_PITCH	1
#define PID_ROLL	2
#define PID_YAW		3
#define PID_DEPTH	4
#endif /* PID_MODE */


/******************************
**
** Data types
**
******************************/

#ifndef _PID_DATA_
#define _PID_DATA_

typedef struct _PID_DATA {
	float ref;	//!< Reference, desired, or target value.
	float kp;	//!< Proportional gain.
	float ki;	//!< Intgral gain.
	float kd;	//!< Derivative gain.
	float cval;	//!< Current value of measurement.
	float perr;	//!< Proportional error.
	float ierr;	//!< Integral error.
	float derr;	//!< Derivative error.
	int period;	//!< Desired period to run PID loop at.
} PID_DATA;

typedef struct _PID {
	PID_DATA pitch;			//!< PID values for pitch.
	PID_DATA roll;			//!< PID values for roll.
	PID_DATA yaw;			//!< PID values for yaw.
	PID_DATA depth;			//!< PID values for depth.
	float voith_angle;		//!< The angle for the thrust vector.
	int voith_speed;		//!< The rotational speed for the Voith motors.
	int voith_thrust;		//!< The thrust for the Voith motors.
	int vertical_thrust;	//!< The vertical thrust for the wing motors.
	int pitch_torque;		//!< The calculated pitch torque.
	int roll_torque;		//!< The calculated roll torque.
	int yaw_torque;			//!< The calculated yaw torque.
} PID;

#endif /* _PID_DATA_ */


/******************************
**
** Function prototypes
**
******************************/

//! Initializes PID values.
//! \param pid: Pointer to PID data.
//! \param cf Pointer to configuration variables.
//! \return 0 on success, -1 on error.
int pid_init( PID *pid, CONF_VARS *cf );

//! Runs through one iteration of a PID controller.
//! \param pololu_fd Pololu file descriptor.
//! \param pid PID struct.
//! \param cf Pointer to configuration variables.
//! \param msg Pointer to message data.
//! \param dt Time difference from last loop run.
//! \param mode Which PID loop to run.
//! \param motor_init Boolean for whether motor controller is initialized.
void pid_loop( int pololu_fd,
               PID *pid,
               CONF_VARS *cf,
               MSG_DATA *msg,
               int dt,
               int mode,
			   int motor_init
             );

//! Calculates the difference between two angles.
//! \param ang1 Angle 1.
//! \param ang2 Angle 2.
//! \return The difference between the two angles.
float pid_subtract_angles( float ang1, float ang2 );

//! Checks the bound for an integral.
//! \param value Value to be checked.
//! \param gain Gain for controller.
//! \param bound Bound to check against.
//! \return Value or the bound limit.
float pid_bound_integral( float value, float gain, float bound );


#endif /* _PID_H_ */
