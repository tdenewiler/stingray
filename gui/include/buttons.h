/**
 *  \file buttons.h
 *  \brief Functions to create buttons for the GUI.
 */

#ifndef _BUTTONS_H_
#define _BUTTONS_H_

#include <sys/timeb.h>
#include <gtk/gtk.h>

#include "events.h"
#include "messages.h"
#include "pololu.h"
#include "parser.h"

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

#ifndef BUTTON_GAIN_INCR
#define BUTTON_GAIN_INCR 0.100
#endif /* BUTTON_SPIN_INCR */

#ifndef BUTTON_ACT_INCR
#define BUTTON_ACT_INCR 1.000
#endif /* BUTTON_ACT_INCR */

/** @name Values used with the servo controller. */
//@{
#ifndef ACTUATOR_DEFS
#define NUM_SERVOS 16
#define LEFT_THRUSTER 0
#define RIGHT_THRUSTER 1
#define LEFT_SERVO_ANG 7
#define LEFT_SERVO_RAD 8
#define RIGHT_SERVO_ANG 9
#define RIGHT_SERVO_RAD 10
#define LEFT_WING 3
#define RIGHT_WING 2
#define LEFT_TAIL 4
#define RIGHT_TAIL 5
#define SERVO_INIT 175
#define LEFT_THRUSTER_CENTER 0
#define LEFT_THRUSTER_MIN 0
#define LEFT_THRUSTER_MAX 254
#define RIGHT_THRUSTER_CENTER 0
#define RIGHT_THRUSTER_MIN 0
#define RIGHT_THRUSTER_MAX 254
#define RIGHT_SERVO_ANG_CENTER 107
#define RIGHT_SERVO_ANG_MIN 64
#define RIGHT_SERVO_ANG_MAX 150
#define RIGHT_SERVO_RAD_CENTER 165
#define RIGHT_SERVO_RAD_MIN 50
#define RIGHT_SERVO_RAD_MAX 198
#define LEFT_SERVO_ANG_CENTER 129
#define LEFT_SERVO_ANG_MIN 85
#define LEFT_SERVO_ANG_MAX 173
#define LEFT_SERVO_RAD_CENTER 59
#define LEFT_SERVO_RAD_MIN 4
#define LEFT_SERVO_RAD_MAX 101
#define LEFT_WING_CENTER 128
#define LEFT_WING_MIN 64
#define LEFT_WING_MAX 190
#define RIGHT_WING_CENTER 128
#define RIGHT_WING_MIN 64
#define RIGHT_WING_MAX 190
#define LEFT_TAIL_CENTER 128
#define LEFT_TAIL_MIN 64
#define LEFT_TAIL_MAX 190
#define RIGHT_TAIL_CENTER 128
#define RIGHT_TAIL_MIN 64
#define RIGHT_TAIL_MAX 190
#define TAILS_CENTER 128
#define TAILS_MIN 64
#define TAILS_MAX 190
#define WINGS_CENTER 128
#define WINGS_MIN 64
#define WINGS_MAX 190
#define LEFT_SERVO_OFFSET 45
#define RIGHT_SERVO_OFFSET 45
#endif /* ACTUATOR_DEFS */
//@}



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

//! Generic function for creating frames.
//! Returns a GtkWidget.
//! \param name Name of the frame that gets embedded into widget.
//!  \return A GtkWidget * that can be used later on.
GtkWidget *buttons_make_frame( const gchar *name );

//! Generic function creating radio buttons with a label.
//! Returns a GtkWidget.
//! \param name The name to display by the radio button.
//! \param callback The function to call when the specified action occurs with
//!                 the widget.
//! \return A GtkWidget * to be used later.
GtkWidget *buttons_make_radio( const char *name, GtkSignalFunc callback );

//! Generic function for creating a radio group for a set of radio buttons.
//! Returns a GtkWidget.
//! \return A GtkWidget * to be used later.
GtkWidget *buttons_make_radio_group( );

//! Generic function for creating a check button with a label.
//! Returns a GtkWidget.
//! \param name The name to display next to the check button.
//! \param callback The function to call when the specified action occurs with
//!                 the widget.
//! \return A GtkWidget * to be used later.
GtkWidget *buttons_make_check( const char *name, GtkSignalFunc callback );

//! Generic button for creating a spin button with a label and a frame.
//! Returns a GtkWidget.
//! \param name The name to display next to the spin button.
//! \param callback The function to call when the specified action occurs with
//!                 the widget.
//! \param box The box to pack the whole spin button into.
//! \param adj Default values for how the spin button will operate.
//! \return A GtkWidget * to be used later.
GtkWidget *buttons_make_spin( const char *name,
                              GtkSignalFunc callback,
                              GtkWidget *box,
                              GtkAdjustment *adj
                            );

//! Makes buttons related to Operational Mode.
//! Returns TRUE.
//! \param box The box to pack the Operating Mode buttons into.
//! \return TRUE. No error checking implemented.
int buttons_opmode( GtkWidget *box );

//! Makes buttons related to desired uuv targets.
//! Returns TRUE.
//! \param box The box to pack the Target buttons into.
//! \return TRUE. No error checking implemented.
int buttons_targets( GtkWidget *box );

//! Makes buttons related to images.
//! Returns TRUE.
//! \param box The box to pack the Images buttons into.
//! \return TRUE. No error checking implemented.
int buttons_images( GtkWidget *box );

//! Makes buttons to set the tasks for the UUV.
//! Returns TRUE.
//! \param box The box to pack the Tasks buttons into.
//! \return TRUE. No error checking implemented.
int buttons_tasks( GtkWidget *box );

//! Makes buttons to set the vision for the UUV.
//! Returns TRUE.
//! \param box The box to pack the Vision buttons into.
//! \return TRUE. No error checking implemented.
int buttons_vision( GtkWidget *box );

//! Makes buttons to control the thrusters, motors, and servos.
//! Returns TRUE.
//! \param box The box to pack the Actuators buttons into.
//! \return TRUE. No error checking implemented.
int buttons_actuators( GtkWidget *box );

//! Makes buttons related to uuv options.
//! Returns TRUE.
//! \param box The box to pack the UUV Options buttons into.
//! \return TRUE. No error checking implemented.
int buttons_options( GtkWidget *box );

//! Makes the status window.
//! Returns TRUE.
//! \param box The box to pack the status label into.
//! \return TRUE. No error checking implemented.
int buttons_make_status( GtkWidget *box );

//! Makes the emergency stop button.
//! Returns TRUE.
//! \param box The box to pack the button into.
//! \return TRUE. No error checking implemented.
int buttons_make_estop( GtkWidget *box );

//! Makes buttons related to PID loop gains.
//! Returns TRUE.
//! \param box The box to pack the Gain buttons into.
//! \return TRUE. No error checking implemented.
int buttons_gains( GtkWidget *box );

//! Update the values of buttons based on state data sent by the UUV.
//!
void buttons_update_values( );


#endif /* _BUTTONS_H_ */
