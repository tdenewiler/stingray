/**
 *  \file pololu.h
 *  \brief	This library implements the Pololu protocol for the Pololu
 *			16-Channel USB servo controller.  The user's manual is available at:
 *			www.pololu.com.  In addition to the protocol higher level functions
 *			for using the Stingray's propulsion systems are included.
 */

#ifndef _POLOLU_H_
#define _POLOLU_H_

#include <stdio.h>
#include <math.h>
#include <unistd.h>


/******************************
 *
 * #defines
 *
 *****************************/

/** @name Standard values for the Pololu protocol. */
//@{
#define POLOLU_SUCCESS          1
#define POLOLU_FAILURE          -1
#define POLOLU_START_BYTE       0x80
#define POLOLU_DEVICE_ID        0x01
#define POLOLU_ON               1
#define POLOLU_OFF              0
#define POLOLU_FORWARD          1
#define POLOLU_REVERSE          0
#define POLOLU_DEFAULT_RANGE    15
#define POLOLU_DEFAULT_NEUTRAL  3000
#define POLOLU_SLEEP            10
//@}

/** @name Neutral positions for the Voith servos and motors. */
//@{
#define POLOLU_CH1_NEUTRAL      	2824
#define POLOLU_CH2_NEUTRAL      	3192
#define POLOLU_CH4_NEUTRAL      	3083
#define POLOLU_CH5_NEUTRAL      	3127
#define POLOLU_DROPPER_NEUTRAL		3000
#define POLOLU_SERVO_NEUTRAL		63.5
#define POLOLU_DEADZONE				0.1
#define POLOLU_DZ_NEUTRAL			4
#define POLOLU_NEUTRAL_GAIN			0.595
#define POLOLU_SERVO_GAIN			0.2
#define POLOLU_LEFT_ANGLE_OFFSET	230
#define POLOLU_RIGHT_ANGLE_OFFSET	40
#define POLOLU_VOITH_GAIN			0.64
#define POLOLU_VOITH_NEUTRAL		63
#define POLOLU_SERVO_BOUND			100
#define POLOLU_MAX_YAW_TORQUE		80
#define POLOLU_YAW_CORRECTION		45
#define POLOLU_MOVE_FORWARD			50
#define POLOLU_MOVE_REVERSE			-50
#define POLOLU_MOVE_LEFT			-50
#define POLOLU_MOVE_RIGHT			50
#define POLOLU_MOVE_STOP			0
//@}

/** @name The time (ms) it takes for the Pololu to initialize. */
//@{
#define POLOLU_INIT_TIME	7000000
//@}

/** @name The number of bytes expected to be sent for vertical and Voith commands. */
//@{
#define POLOLU_BYTES_VERTICAL	15
#define POLOLU_BYTES_VOITH		30
//@}

/** @name The channel assignments for the servos and motors. */
//@{
#define POLOLU_LEFT_SERVO1			1
#define POLOLU_LEFT_SERVO2			2
#define POLOLU_RIGHT_SERVO1			4
#define POLOLU_RIGHT_SERVO2			5
#define POLOLU_RIGHT_VOITH_MOTOR	0
#define POLOLU_LEFT_VOITH_MOTOR		3
#define POLOLU_LEFT_WING_MOTOR		7
#define POLOLU_RIGHT_WING_MOTOR		8
#define POLOLU_TAIL_MOTOR			10
#define POLOLU_DROPPER_SERVO		15
//@}


/******************************
 *
 * Data types
 *
 *****************************/



/******************************
 *
 * Function prototypes
 *
 *****************************/

//! Establish communications with the Pololu.
//! \param *portname The name of the port that the Pololu is plugged
//!                  into.
//! \param baud The baud rate to use for the Pololu serial port.
//! \return A file descriptor for the Pololu.
int pololuSetup( char *portname,
                 int baud
               );

//! Set the parameters for a channel.  By default, at poweron, all channels are off.
//! \param fd A file descriptor for the Pololu port.
//! \param channel The servo channel (0-15).
//! \param channelOn 0 for OFF (default), any other value for ON.
//! \param direction 0 for REVERSE, any other value for FORWARD
//!                  (default).
//! \param range The servo range multiplier (0-15, 15 default).
//! \return The number of bytes sent to the Pololu.
int pololuSetParameters( int fd,
                         int channel,
                         int channelOn,
                         int direction,
                         int range
                       );

//! Sets the speed that the channel changes from one value to another.
//! \param fd A file descriptor for the Pololu port.
//! \param channel The servo channel (0-15).
//! \param speed 0-127 Sets the speed.  (0 is default for instant)
//! \return The number of bytes sent to the Pololu.
int pololuSetSpeed( int fd,
                    int channel,
                    int speed
                  );


//! Sets the relative position or pulse width of a channel using a
//! 7-bit number.
//! \param fd A file descriptor for the Pololu port.
//! \param channel The servo channel (0-15).
//! \param position A 7-bit relative position value. (0-127)
//! \return The number of bytes sent to the Pololu.
int pololuSetPosition7Bit( int fd,
                           int channel,
                           int position
                         );

//! Sets the relative position or pulse width of a channel using an
//! 8-bit number.
//! \param fd A file descriptor for the Pololu port.
//! \param channel The servo channel (0-15).
//! \param position An 8-bit relative position value. (0-255)
//! \return The number of bytes sent to the Pololu.
int pololuSetPosition8Bit( int fd,
                           int channel,
                           int position
                         );

//! Sets the absolute position or pulse width.
//! \param fd A file descriptor for the Pololu port.
//! \param channel The servo channel (0-15).
//! \param position An absolute postion value. (500-5500)
//! \return The number of bytes sent to the Pololu.
int pololuSetPositionAbsolute( int fd,
                               int channel,
                               int position
                             );

//! Sets the absolute neutral position or pulse width.
//! \param fd A file descriptor for the Pololu port.
//! \param channel The servo channel (0-15).
//! \param position An absolute postion value. (500-5500)
//! \return The number of bytes sent to the Pololu.
int pololuSetNeutral( int fd,
                      int channel,
                      int position
                    );


//! This function initializes or resets all of the channels with the Stinray's custom profile.
//! \param fd A file descriptor for the Pololu port.
//! \return A value that indicates success or failure.
int pololuInitializeChannels( int fd );


//! This is a higher level function that controls both VSPs together.
//! \param voithThrust between 0 and 100
//! \param thrustAngle between 0 and 360
//! \param thrust between -100 and 100
//! \param yawTorque between -100 and 100
//! \return A value that indicates success or failure.
int pololuControlVoiths( int fd,
                   int voithThrust,
                   float thrustAngle,
                   int thrust,
                   int yawTorque
                 );

//! The is a higher level function that controls the vertical
//! thrusters together.
//! \param vertForce between 0 and 100
//! \param rollTorque between 0 and 100
//! \param pitchTorque between 0 and 100
//! \return A value that indicates success or failure.
int pololuControlVertical( int fd,
                     int vertForce,
                     int rollTorque,
                     int pitchTorque
                   );


#endif /* _POLOLU_H_ */
