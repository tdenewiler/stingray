/**
 *  \file events.h
 *  \brief Callback functions for button events.
 */

#ifndef _EVENTS_H_
#define _EVENTS_H_

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

//! Event associated with the Enable Servos button.
//! \param widget A pointer to the button widget.
//! \param event A pointer to the event that triggered the callback.
//! \param data A pointer to data that can be manipulated.
void events_enable_servos( GtkWidget *widget,
                           GdkEvent *event,
                           gpointer data
                         );

//! Event associated with the Enable IMU button.
//! \param widget A pointer to the button widget.
//! \param event A pointer to the event that triggered the callback.
//! \param data A pointer to data that can be manipulated.
void events_enable_imu( GtkWidget *widget,
                        GdkEvent *event,
                        gpointer data
                      );

//! Event associated with the Enable Log button.
//! \param widget A pointer to the button widget.
//! \param event A pointer to the event that triggered the callback.
//! \param data A pointer to data that can be manipulated.
void events_enable_log( GtkWidget *widget,
                        GdkEvent *event,
                        gpointer data
                      );

//! Event associated with the IMU Stabilized button.
//! \param widget A pointer to the button widget.
//! \param event A pointer to the event that triggered the callback.
//! \param data A pointer to data that can be manipulated.
void events_imu_stab( GtkWidget *widget,
                      GdkEvent *event,
                      gpointer data
                    );

//! Event associated with the Debug Level button.
//! \param widget A pointer to the button widget.
//! \param event A pointer to the event that triggered the callback.
//! \param data A pointer to data that can be manipulated.
void events_debug_level( GtkWidget *widget,
                         GdkEvent *event,
                         gpointer data
                       );

//! Event associated with the Dropper button.
//! \param widget A pointer to the button widget.
//! \param event A pointer to the event that triggered the callback.
//! \param data A pointer to data that can be manipulated.
void events_dropper( GtkWidget *widget,
                         GdkEvent *event,
                         gpointer data
                       );

//! Event associated with the Operational Mode button.
//! \param widget A pointer to the button widget.
//! \param event A pointer to the event that triggered the callback.
//! \param data A pointer to data that can be manipulated.
void events_opmode( GtkWidget *widget,
                    GdkEvent *event,
                    gpointer data
                  );

//! Event associated with the Target Yaw button.
//! \param widget A pointer to the button widget.
//! \param event A pointer to the event that triggered the callback.
//! \param data A pointer to data that can be manipulated.
void events_target_yaw( GtkWidget *widget,
                        GdkEvent *event,
                        gpointer data
                      );

//! Event associated with the Target Roll button.
//! \param widget A pointer to the button widget.
//! \param event A pointer to the event that triggered the callback.
//! \param data A pointer to data that can be manipulated.
void events_target_roll( GtkWidget *widget,
                         GdkEvent *event,
                         gpointer data
                       );

//! Event associated with the Target Pitch button.
//! \param widget A pointer to the button widget.
//! \param event A pointer to the event that triggered the callback.
//! \param data A pointer to data that can be manipulated.
void events_target_pitch( GtkWidget *widget,
                          GdkEvent *event,
                          gpointer data
                        );

//! Event associated with the Target Depth button.
//! \param widget A pointer to the button widget.
//! \param event A pointer to the event that triggered the callback.
//! \param data A pointer to data that can be manipulated.
void events_target_depth( GtkWidget *widget,
                          GdkEvent *event,
                          gpointer data
                        );

//! Event associated with the Target Fx button.
//! \param widget A pointer to the button widget.
//! \param event A pointer to the event that triggered the callback.
//! \param data A pointer to data that can be manipulated.
void events_target_fx( GtkWidget *widget,
                       GdkEvent *event,
                       gpointer data
                     );

//! Event associated with the Target Fy button.
//! \param widget A pointer to the button widget.
//! \param event A pointer to the event that triggered the callback.
//! \param data A pointer to data that can be manipulated.
void events_target_fy( GtkWidget *widget,
                       GdkEvent *event,
                       gpointer data
                     );

//! Event associated with the Target Speed button.
//! \param widget A pointer to the button widget.
//! \param event A pointer to the event that triggered the callback.
//! \param data A pointer to data that can be manipulated.
void events_target_speed( GtkWidget *widget,
                          GdkEvent *event,
                          gpointer data
                        );

//! Sets the target values to the current vehicle pose.
//! \param widget A pointer to the button widget.
//! \param event A pointer to the event that triggered the callback.
//! \param data A pointer to data that can be manipulated.
void events_target_current( GtkWidget *widget,
                          GdkEvent *event,
                          gpointer data
                        );

//! Sets the target mode to ZERO_PID_ERRORS.
//! \param widget A pointer to the button widget.
//! \param event A pointer to the event that triggered the callback.
//! \param data A pointer to data that can be manipulated.
void events_zero_pid( GtkWidget *widget,
                          GdkEvent *event,
                          gpointer data
                        );

//! Sets the mode for which image will be displayed.
//! \param widget A pointer to the button widget.
//! \param event A pointer to the event that triggered the callback.
//! \param data A pointer to data that can be manipulated.
void events_images( GtkWidget *widget,
                    GdkEvent *event,
                    gpointer data
                  );

//! Event associated with the Gains button.
//! \param widget A pointer to the button widget.
//! \param event A pointer to the event that triggered the callback.
//! \param data A pointer to data that can be manipulated.
void events_gain( GtkWidget *widget,
                  GdkEvent *event,
                  gpointer data
                );

//! Causes the configuration file gains to be set as current gains.
//! \param widget A pointer to the button widget.
//! \param event A pointer to the event that triggered the callback.
//! \param data A pointer to data that can be manipulated.
void events_gain_cf( GtkWidget *widget,
                  GdkEvent *event,
                  gpointer data
                );

//! Causes the gains to be set to zero.
//! \param widget A pointer to the button widget.
//! \param event A pointer to the event that triggered the callback.
//! \param data A pointer to data that can be manipulated.
void events_gain_zero( GtkWidget *widget,
                  GdkEvent *event,
                  gpointer data
                );

//! Called when Emergency Stop button is clicked.
//! \param widget A pointer to the button widget.
void events_estop( GtkWidget *widget );

//! Called when one of the Tasks buttons is clicked.
//! \param widget A pointer to the button widget.
//! \param event A pointer to the event that triggered the callback.
//! \param data A pointer to data that can be manipulated.
void events_tasks( GtkWidget *widget,
                   GdkEvent *event,
                   gpointer data
                 );

//! Called when one of the Vision buttons is clicked.
//! \param widget A pointer to the button widget.
//! \param event A pointer to the event that triggered the callback.
//! \param data A pointer to data that can be manipulated.
void events_vision( GtkWidget *widget,
                   GdkEvent *event,
                   gpointer data
                 );

//! Called when Get Gain button is clicked.
//!
void events_gain_get( );

//! Called when Set Gain button is clicked.
//!
void events_gain_set( );

//! Called when Set Gain Instantly button is clicked.
//!
void events_set_gain_inst( );


#endif /* _EVENTS_H_ */
