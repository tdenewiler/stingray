/******************************************************************************
 *
 *  Title:        events.c
 *
 *  Description:  Callback functions for button events.
 *
 *****************************************************************************/


#include <sys/timeb.h>
#include <gtk/gtk.h>

#include "events.h"
#include "buttons.h"
#include "messages.h"
#include "pololu.h"
#include "parser.h"
#include "network.h"
#include "task.h"
#include "visiond.h"


/* Operation mode buttons. */
extern GtkWidget *button_hold_yaw;
extern GtkWidget *button_hold_roll;
extern GtkWidget *button_hold_pitch;
extern GtkWidget *button_hold_accel;
extern GtkWidget *button_hold_ang_rate;
extern GtkWidget *button_manual;
extern GtkWidget *button_autonomous;

/* Vision setting buttons. */
extern GtkWidget *button_hpipe_lo;
extern GtkWidget *button_hpipe_hi;
extern GtkWidget *button_spipe_lo;
extern GtkWidget *button_spipe_hi;
extern GtkWidget *button_vpipe_lo;
extern GtkWidget *button_vpipe_hi;
extern GtkWidget *button_hbuoy_lo;
extern GtkWidget *button_hbuoy_hi;
extern GtkWidget *button_sbuoy_lo;
extern GtkWidget *button_sbuoy_hi;
extern GtkWidget *button_vbuoy_lo;
extern GtkWidget *button_vbuoy_hi;
extern GtkWidget *button_hfence_lo;
extern GtkWidget *button_hfence_hi;
extern GtkWidget *button_sfence_lo;
extern GtkWidget *button_sfence_hi;
extern GtkWidget *button_vfence_lo;
extern GtkWidget *button_vfence_hi;
extern GtkWidget *button_save_bframe;
extern GtkWidget *button_save_fframe;
extern GtkWidget *button_save_bvideo;
extern GtkWidget *button_save_fvideo;

/* Tasks buttons. */
extern GtkWidget *button_task_gate;
extern GtkWidget *button_task_buoy;
extern GtkWidget *button_task_pipe;
extern GtkWidget *button_task_square;
extern GtkWidget *button_task_none;
extern GtkWidget *button_task_fence;
extern GtkWidget *button_task_boxes;
extern GtkWidget *button_task_suitcase;
extern GtkWidget *button_task_course;
extern GtkWidget *button_task_nod;
extern GtkWidget *button_task_spin;
extern GtkWidget *button_task_square_time1;
extern GtkWidget *button_task_square_time2;
extern GtkWidget *button_task_square_time3;
extern GtkWidget *button_task_square_time4;

/* Gain buttons. */
extern GtkWidget *button_kp_yaw;
extern GtkWidget *button_ki_yaw;
extern GtkWidget *button_kd_yaw;
extern GtkWidget *button_kp_pitch;
extern GtkWidget *button_ki_pitch;
extern GtkWidget *button_kd_pitch;
extern GtkWidget *button_kp_roll;
extern GtkWidget *button_ki_roll;
extern GtkWidget *button_kd_roll;
extern GtkWidget *button_kp_depth;
extern GtkWidget *button_ki_depth;
extern GtkWidget *button_kd_depth;
extern GtkWidget *button_kp_fx;
extern GtkWidget *button_ki_fx;
extern GtkWidget *button_kd_fx;
extern GtkWidget *button_kp_fy;
extern GtkWidget *button_ki_fy;
extern GtkWidget *button_kd_fy;

/* Target buttons. */
extern GtkWidget *button_target_yaw;
extern GtkWidget *button_target_roll;
extern GtkWidget *button_target_pitch;
extern GtkWidget *button_target_depth;
extern GtkWidget *button_target_fx;
extern GtkWidget *button_target_fy;
extern GtkWidget *button_target_speed;
extern GtkWidget *button_target_current;

/* Image option buttons. */
extern GtkWidget *button_image_fcolor;
extern GtkWidget *button_image_fbinary;
extern GtkWidget *button_image_bcolor;
extern GtkWidget *button_image_bbinary;
extern GtkWidget *button_image_none;

/* Network API messages. */
extern MSG_DATA msg;
extern int planner_fd;
extern int vision_fd;
extern int nav_fd;

/* Configuration file variables. */
extern CONF_VARS cf;

/* A buffer to store network data. */
extern char planner_buf[MAX_MSG_SIZE];


/******************************************************************************
 *
 * Title:       void events_estop( )
 *
 * Description: Called when Emergency Stop button is clicked.
 *
 * Input:       None.
 *
 * Output:      None.
 *
 * Globals:     None.
 *
 *****************************************************************************/

void events_estop( GtkWidget *widget )
{
    /* Check the state of the button. */
    if( gtk_toggle_button_get_active( ( GtkToggleButton * )widget ) ) {
        msg.stop.data.state = TRUE;
    }
    else {
        msg.stop.data.state = FALSE;
    }

    /* Send the stop message. */
    if( planner_fd  > 0 ) {
        messages_send( planner_fd, STOP_MSGID, &msg );
    }
    if( nav_fd  > 0 ) {
        messages_send( nav_fd, STOP_MSGID, &msg );
    }
} /* end events_estop() */


/******************************************************************************
 *
 * Title:       void events_tasks( )
 *
 * Description: Called when one of the Tasks buttons is clicked.
 *
 * Input:       None.
 *
 * Output:      None.
 *
 * Globals:     None.
 *
 *****************************************************************************/

void events_tasks( GtkWidget *widget,
                   GdkEvent *event,
                   gpointer data
                 )
{
    
    /* Set the course to off. If the course button is selected,
     * this will be turned back on. */
    msg.task.data.course = TASK_COURSE_OFF;
    
    /* Check the state of the buttons. */
    if( widget == button_task_square ||
		widget == button_task_square_time1 ||
		widget == button_task_square_time2 ||
		widget == button_task_square_time3 ||
		widget == button_task_square_time4 ) {
        if( gtk_toggle_button_get_active( ( GtkToggleButton * )button_task_square ) ) {
            msg.task.data.task = TASK_SQUARE;
            msg.task.data.time_forward  = gtk_spin_button_get_value_as_float(
				GTK_SPIN_BUTTON( button_task_square_time1 ) );
            msg.task.data.time_left = gtk_spin_button_get_value_as_float(
				GTK_SPIN_BUTTON( button_task_square_time2 ) );
            msg.task.data.time_reverse  = gtk_spin_button_get_value_as_float(
				GTK_SPIN_BUTTON( button_task_square_time3 ) );
            msg.task.data.time_right    = gtk_spin_button_get_value_as_float(
				GTK_SPIN_BUTTON( button_task_square_time4 ) );
        }
    }
    else if( widget == button_task_buoy ) {
        msg.task.data.task = TASK_BUOY;
    }
    else if( widget == button_task_pipe ) {
        msg.task.data.task = TASK_PIPE;
    }
    else if( widget == button_task_gate ) {
        msg.task.data.task = TASK_GATE;
    }
    else if( widget == button_task_none ) {
        msg.task.data.task     = TASK_NONE;
		msg.target.data.pitch = msg.status.data.pitch;
		msg.target.data.roll  = msg.status.data.roll;
		msg.target.data.yaw   = msg.status.data.yaw;
    }
    else if( widget == button_task_fence ) {
        msg.task.data.task = TASK_FENCE;
    }
    else if( widget == button_task_boxes ) {
        msg.task.data.task = TASK_BOXES;
    }
    else if( widget == button_task_suitcase ) {
        msg.task.data.task = TASK_SUITCASE;
    }
    else if( widget == button_task_course ) {
        msg.task.data.task = TASK_GATE;
        msg.task.data.course = TASK_COURSE_ON;
    }
    else if( widget == button_task_nod ) {
        msg.task.data.task = TASK_NOD;
    }
    else if( widget == button_task_spin ) {
        msg.task.data.task = TASK_SPIN;
    }
	
    if( planner_fd > 0 ) {
        messages_send( planner_fd, TASK_MSGID, &msg );
    }
    if( nav_fd > 0 ) {
        messages_send( nav_fd, TASK_MSGID, &msg );
    }
} /* end events_tasks() */


/******************************************************************************
 *
 * Title:       void events_vision( )
 *
 * Description: Called when one of the Vision buttons is clicked.
 *
 * Input:       None.
 *
 * Output:      None.
 *
 * Globals:     None.
 *
 *****************************************************************************/

void events_vision( GtkWidget *widget,
                   GdkEvent *event,
                   gpointer data
                 )
{
    if( widget == button_hpipe_lo ) {
        msg.vsetting.data.pipe_hsv.hL = gtk_spin_button_get_value_as_float(
			GTK_SPIN_BUTTON( widget ) );
    }
    else if( widget == button_spipe_lo ) {
        msg.vsetting.data.pipe_hsv.sL = gtk_spin_button_get_value_as_float(
			GTK_SPIN_BUTTON( widget ) );
    }
    else if( widget == button_vpipe_lo ) {
        msg.vsetting.data.pipe_hsv.vL = gtk_spin_button_get_value_as_float(
			GTK_SPIN_BUTTON( widget ) );
    }
    else if( widget == button_hbuoy_lo ) {
        msg.vsetting.data.buoy_hsv.hL = gtk_spin_button_get_value_as_float(
			GTK_SPIN_BUTTON( widget ) );
    }
    else if( widget == button_sbuoy_lo ) {
        msg.vsetting.data.buoy_hsv.sL = gtk_spin_button_get_value_as_float(
			GTK_SPIN_BUTTON( widget ) );
    }
    else if( widget == button_vbuoy_lo ) {
        msg.vsetting.data.buoy_hsv.vL = gtk_spin_button_get_value_as_float(
			GTK_SPIN_BUTTON( widget ) );
    }
    else if( widget == button_hfence_lo ) {
        msg.vsetting.data.fence_hsv.hL = gtk_spin_button_get_value_as_float(
			GTK_SPIN_BUTTON( widget ) );
    }
    else if( widget == button_sfence_lo ) {
        msg.vsetting.data.fence_hsv.sL = gtk_spin_button_get_value_as_float(
			GTK_SPIN_BUTTON( widget ) );
    }
    else if( widget == button_vfence_lo ) {
        msg.vsetting.data.fence_hsv.vL = gtk_spin_button_get_value_as_float(
			GTK_SPIN_BUTTON( widget ) );
    }
    else if( widget == button_hpipe_hi ) {
        msg.vsetting.data.pipe_hsv.hH = gtk_spin_button_get_value_as_float(
			GTK_SPIN_BUTTON( widget ) );
    }
    else if( widget == button_spipe_hi ) {
        msg.vsetting.data.pipe_hsv.sH = gtk_spin_button_get_value_as_float(
			GTK_SPIN_BUTTON( widget ) );
    }
    else if( widget == button_vpipe_hi ) {
        msg.vsetting.data.pipe_hsv.vH = gtk_spin_button_get_value_as_float(
			GTK_SPIN_BUTTON( widget ) );
    }
    else if( widget == button_hbuoy_hi ) {
        msg.vsetting.data.buoy_hsv.hH = gtk_spin_button_get_value_as_float(
			GTK_SPIN_BUTTON( widget ) );
    }
    else if( widget == button_sbuoy_hi ) {
        msg.vsetting.data.buoy_hsv.sH = gtk_spin_button_get_value_as_float(
			GTK_SPIN_BUTTON( widget ) );
    }
    else if( widget == button_vbuoy_hi ) {
        msg.vsetting.data.buoy_hsv.vH = gtk_spin_button_get_value_as_float(
			GTK_SPIN_BUTTON( widget ) );
    }
    else if( widget == button_hfence_hi ) {
        msg.vsetting.data.fence_hsv.hH = gtk_spin_button_get_value_as_float(
			GTK_SPIN_BUTTON( widget ) );
    }
    else if( widget == button_sfence_hi ) {
        msg.vsetting.data.fence_hsv.sH = gtk_spin_button_get_value_as_float(
			GTK_SPIN_BUTTON( widget ) );
    }
    else if( widget == button_vfence_hi ) {
        msg.vsetting.data.fence_hsv.vH = gtk_spin_button_get_value_as_float(
			GTK_SPIN_BUTTON( widget ) );
    }
    else if( widget == button_save_bframe ) {
        msg.vsetting.data.save_bframe = TRUE;
    }
    else if( widget == button_save_fframe ) {
        msg.vsetting.data.save_fframe = TRUE;
    }
    else if( widget == button_save_bvideo ) {
        if( gtk_toggle_button_get_active((GtkToggleButton *)widget) ) {
            msg.vsetting.data.save_bvideo = TRUE;
        }
        else {
            msg.vsetting.data.save_bvideo = FALSE;
        }
    }
    else if( widget == button_save_fvideo ) {
        if( gtk_toggle_button_get_active((GtkToggleButton *)widget) ) {
            msg.vsetting.data.save_fvideo = TRUE;
        }
        else {
            msg.vsetting.data.save_fvideo = FALSE;
        }
    }

    /* Send the state of the buttons to the server. */
    messages_send( vision_fd, VSETTING_MSGID, &msg );

    /* Reset values for save frame buttons. */
    msg.vsetting.data.save_bframe = FALSE;
    msg.vsetting.data.save_fframe = FALSE;
} /* end events_vision() */


/******************************************************************************
 *
 * Title:       void events_gain( GtkWidget *widget,
 *                                          GdkEvent *event,
 *                                          gpointer data )
 *
 * Description: Called when gain button clicked.
 *
 * Input:       widget: A pointer to the button widget.
 *              event: A pointer to the event that triggered the callback.
 *              data: A pointer to data that can be manipulated.
 *
 * Output:      None.
 *
 * Globals:     None.
 *
 *****************************************************************************/

void events_gain( GtkWidget *widget,
                  GdkEvent *event,
                  gpointer data )
{
	/* Get the values of the buttons. */
	msg.gain.data.mode 		= 0;
    msg.gain.data.kp_yaw    = gtk_spin_button_get_value( (GtkSpinButton *)
                              button_kp_yaw );
    msg.gain.data.ki_yaw    = gtk_spin_button_get_value( (GtkSpinButton *)
                              button_ki_yaw );
    msg.gain.data.kd_yaw    = gtk_spin_button_get_value( (GtkSpinButton *)
                              button_kd_yaw );
    msg.gain.data.kp_pitch  = gtk_spin_button_get_value( (GtkSpinButton *)
                              button_kp_pitch );
    msg.gain.data.ki_pitch  = gtk_spin_button_get_value( (GtkSpinButton *)
                              button_ki_pitch );
    msg.gain.data.kd_pitch  = gtk_spin_button_get_value( (GtkSpinButton *)
                              button_kd_pitch );
    msg.gain.data.kp_roll   = gtk_spin_button_get_value( (GtkSpinButton *)
                              button_kp_roll );
    msg.gain.data.ki_roll   = gtk_spin_button_get_value( (GtkSpinButton *)
                              button_ki_roll );
    msg.gain.data.kd_roll   = gtk_spin_button_get_value( (GtkSpinButton *)
                              button_kd_roll );
    msg.gain.data.kp_depth  = gtk_spin_button_get_value( (GtkSpinButton *)
                              button_kp_depth );
    msg.gain.data.ki_depth  = gtk_spin_button_get_value( (GtkSpinButton *)
                              button_ki_depth );
    msg.gain.data.kd_depth  = gtk_spin_button_get_value( (GtkSpinButton *)
                              button_kd_depth );
    msg.gain.data.kp_fx     = gtk_spin_button_get_value( (GtkSpinButton *)
                              button_kp_fx );
    msg.gain.data.ki_fx     = gtk_spin_button_get_value( (GtkSpinButton *)
                              button_ki_fx );
    msg.gain.data.kd_fx     = gtk_spin_button_get_value( (GtkSpinButton *)
                              button_kd_fx );
    msg.gain.data.kp_fy     = gtk_spin_button_get_value( (GtkSpinButton *)
                              button_kp_fy );
    msg.gain.data.ki_fy     = gtk_spin_button_get_value( (GtkSpinButton *)
                              button_ki_fy );
    msg.gain.data.kd_fy     = gtk_spin_button_get_value( (GtkSpinButton *)
                              button_kd_fy );

    /* Send the gain message. */
	if( planner_fd > 0 ) {
		messages_send( planner_fd, GAIN_MSGID, &msg );
	}
	if( nav_fd > 0 ) {
		messages_send( nav_fd, GAIN_MSGID, &msg );
	}
} /* end events_gain() */


/******************************************************************************
 *
 * Title:       void events_gain_cf( GtkWidget *widget,
 *                                   GdkEvent *event,
 *                                   gpointer data )
 *
 * Description: Called when use config file gains button clicked. Sets the
 * 				current gains to be the gains in the configuration file.
 *
 * Input:       widget: A pointer to the button widget.
 *              event: A pointer to the event that triggered the callback.
 *              data: A pointer to data that can be manipulated.
 *
 * Output:      None.
 *
 * Globals:     None.
 *
 *****************************************************************************/

void events_gain_cf( GtkWidget *widget,
                  GdkEvent *event,
                  gpointer data )
{
	/* Set the message values. */
    msg.gain.data.kp_pitch  = cf.kp_pitch;
    msg.gain.data.ki_pitch  = cf.ki_pitch;
    msg.gain.data.kd_pitch  = cf.kd_pitch;
    msg.gain.data.kp_roll   = cf.kp_roll;
    msg.gain.data.ki_roll   = cf.ki_roll;
    msg.gain.data.kd_roll   = cf.kd_roll;
    msg.gain.data.kp_yaw    = cf.kp_yaw;
    msg.gain.data.ki_yaw    = cf.ki_yaw;
    msg.gain.data.kd_yaw    = cf.kd_yaw;
    msg.gain.data.kp_depth  = cf.kp_depth;
    msg.gain.data.ki_depth  = cf.ki_depth;
    msg.gain.data.kd_depth  = cf.kd_depth;
    msg.gain.data.kp_fx  	= cf.kp_fx;
    msg.gain.data.ki_fx  	= cf.ki_fx;
    msg.gain.data.kd_fx  	= cf.kd_fx;
    msg.gain.data.kp_fy  	= cf.kp_fy;
    msg.gain.data.ki_fy  	= cf.ki_fy;
    msg.gain.data.kd_fy  	= cf.kd_fy;
	
	/* Set the button values. */
    gtk_spin_button_set_value( (GtkSpinButton *)button_kp_yaw, msg.gain.data.kp_yaw );
    gtk_spin_button_set_value( (GtkSpinButton *)button_ki_yaw, msg.gain.data.ki_yaw );
    gtk_spin_button_set_value( (GtkSpinButton *)button_kd_yaw, msg.gain.data.kd_yaw );
    gtk_spin_button_set_value( (GtkSpinButton *)button_kp_pitch, msg.gain.data.kp_pitch );
    gtk_spin_button_set_value( (GtkSpinButton *)button_ki_pitch, msg.gain.data.ki_pitch );
    gtk_spin_button_set_value( (GtkSpinButton *)button_kd_pitch, msg.gain.data.kd_pitch );
    gtk_spin_button_set_value( (GtkSpinButton *)button_kp_roll, msg.gain.data.kp_roll );
    gtk_spin_button_set_value( (GtkSpinButton *)button_ki_roll, msg.gain.data.ki_roll );
    gtk_spin_button_set_value( (GtkSpinButton *)button_kd_roll, msg.gain.data.kd_roll );
    gtk_spin_button_set_value( (GtkSpinButton *)button_kp_depth, msg.gain.data.kp_depth );
    gtk_spin_button_set_value( (GtkSpinButton *)button_ki_depth, msg.gain.data.ki_depth );
    gtk_spin_button_set_value( (GtkSpinButton *)button_kd_depth, msg.gain.data.kd_depth );
	
    /* Send the gain message. */
	if( planner_fd > 0 ) {
		messages_send( planner_fd, GAIN_MSGID, &msg );
	}
	if( nav_fd > 0 ) {
		messages_send( nav_fd, GAIN_MSGID, &msg );
	}
} /* end events_gain_cf() */


/******************************************************************************
 *
 * Title:       void events_gain_zero( GtkWidget *widget,
 *                                   GdkEvent *event,
 *                                   gpointer data )
 *
 * Description: Called when use zero gains button clicked. Sets the
 * 				current gains to be zero.
 *
 * Input:       widget: A pointer to the button widget.
 *              event: A pointer to the event that triggered the callback.
 *              data: A pointer to data that can be manipulated.
 *
 * Output:      None.
 *
 * Globals:     None.
 *
 *****************************************************************************/

void events_gain_zero( GtkWidget *widget,
                  GdkEvent *event,
                  gpointer data )
{
	/* Set the message values. */
    msg.gain.data.kp_pitch  = 0;
    msg.gain.data.ki_pitch  = 0;
    msg.gain.data.kd_pitch  = 0;
    msg.gain.data.kp_roll   = 0;
    msg.gain.data.ki_roll   = 0;
    msg.gain.data.kd_roll   = 0;
    msg.gain.data.kp_yaw    = 0;
    msg.gain.data.ki_yaw    = 0;
    msg.gain.data.kd_yaw    = 0;
    msg.gain.data.kp_depth  = 0;
    msg.gain.data.ki_depth  = 0;
    msg.gain.data.kd_depth  = 0;
    msg.gain.data.kp_fx     = 0;
    msg.gain.data.ki_fx     = 0;
    msg.gain.data.kd_fx     = 0;
    msg.gain.data.kp_fy     = 0;
    msg.gain.data.ki_fy     = 0;
    msg.gain.data.kd_fy     = 0;
	
	/* Set the button values. */
    gtk_spin_button_set_value( (GtkSpinButton *)button_kp_yaw, msg.gain.data.kp_yaw );
    gtk_spin_button_set_value( (GtkSpinButton *)button_ki_yaw, msg.gain.data.ki_yaw );
    gtk_spin_button_set_value( (GtkSpinButton *)button_kd_yaw, msg.gain.data.kd_yaw );
    gtk_spin_button_set_value( (GtkSpinButton *)button_kp_pitch, msg.gain.data.kp_pitch );
    gtk_spin_button_set_value( (GtkSpinButton *)button_ki_pitch, msg.gain.data.ki_pitch );
    gtk_spin_button_set_value( (GtkSpinButton *)button_kd_pitch, msg.gain.data.kd_pitch );
    gtk_spin_button_set_value( (GtkSpinButton *)button_kp_roll, msg.gain.data.kp_roll );
    gtk_spin_button_set_value( (GtkSpinButton *)button_ki_roll, msg.gain.data.ki_roll );
    gtk_spin_button_set_value( (GtkSpinButton *)button_kd_roll, msg.gain.data.kd_roll );
    gtk_spin_button_set_value( (GtkSpinButton *)button_kp_depth, msg.gain.data.kp_depth );
    gtk_spin_button_set_value( (GtkSpinButton *)button_ki_depth, msg.gain.data.ki_depth );
    gtk_spin_button_set_value( (GtkSpinButton *)button_kd_depth, msg.gain.data.kd_depth );
    gtk_spin_button_set_value( (GtkSpinButton *)button_kp_fx, msg.gain.data.kp_fx );
    gtk_spin_button_set_value( (GtkSpinButton *)button_ki_fx, msg.gain.data.ki_fx );
    gtk_spin_button_set_value( (GtkSpinButton *)button_kd_fx, msg.gain.data.kd_fx );
    gtk_spin_button_set_value( (GtkSpinButton *)button_kp_fy, msg.gain.data.kp_fy );
    gtk_spin_button_set_value( (GtkSpinButton *)button_ki_fy, msg.gain.data.ki_fy );
    gtk_spin_button_set_value( (GtkSpinButton *)button_kd_fy, msg.gain.data.kd_fy );
	
    /* Send the gain message. */
	if( planner_fd > 0 ) {
		messages_send( planner_fd, GAIN_MSGID, &msg );
	}
	if( nav_fd > 0 ) {
		messages_send( nav_fd, GAIN_MSGID, &msg );
	}
} /* end events_gain_zero() */


/******************************************************************************
 *
 * Title:       void events_gain_get( )
 *
 * Description: Called when Get Gain button is clicked.
 *
 * Input:       None.
 *
 * Output:      None.
 *
 * Globals:     None.
 *
 *****************************************************************************/

void events_gain_get( )
{
	/* Set up variables. */
    int recv_bytes = 0;
    int mode = MODE_STATUS;
    msg.gain.data.mode = GAIN_GET;

    /* Send the gain message. */
 	if( planner_fd > 0 ) {
		messages_send( planner_fd, GAIN_MSGID, &msg );
	}
	if( nav_fd > 0 ) {
		messages_send( nav_fd, GAIN_MSGID, &msg );
	}
   msg.gain.data.mode = 0;

    /* Get network data. */
    if( planner_fd > 0 ) {
        recv_bytes = net_client( planner_fd, planner_buf, &msg, mode );
        planner_buf[recv_bytes] = '\0';
    }
    if( recv_bytes > 0 ) {
        messages_decode( planner_fd, planner_buf, &msg, recv_bytes );
    }

	/* Modify buttons to reflect new values. */
    gtk_spin_button_set_value( (GtkSpinButton *)button_kp_yaw, msg.gain.data.kp_yaw );
    gtk_spin_button_set_value( (GtkSpinButton *)button_ki_yaw, msg.gain.data.ki_yaw );
    gtk_spin_button_set_value( (GtkSpinButton *)button_kd_yaw, msg.gain.data.kd_yaw );
    gtk_spin_button_set_value( (GtkSpinButton *)button_kp_pitch, msg.gain.data.kp_pitch );
    gtk_spin_button_set_value( (GtkSpinButton *)button_ki_pitch, msg.gain.data.ki_pitch );
    gtk_spin_button_set_value( (GtkSpinButton *)button_kd_pitch, msg.gain.data.kd_pitch );
    gtk_spin_button_set_value( (GtkSpinButton *)button_kp_roll, msg.gain.data.kp_roll );
    gtk_spin_button_set_value( (GtkSpinButton *)button_ki_roll, msg.gain.data.ki_roll );
    gtk_spin_button_set_value( (GtkSpinButton *)button_kd_roll, msg.gain.data.kd_roll );
    gtk_spin_button_set_value( (GtkSpinButton *)button_kp_depth, msg.gain.data.kp_depth );
    gtk_spin_button_set_value( (GtkSpinButton *)button_ki_depth, msg.gain.data.ki_depth );
    gtk_spin_button_set_value( (GtkSpinButton *)button_kd_depth, msg.gain.data.kd_depth );
} /* end events_gain_get() */


/******************************************************************************
 *
 * Title:       void events_gain_set( )
 *
 * Description: Called when Set Gain button is clicked.
 *
 * Input:       None.
 *
 * Output:      None.
 *
 * Globals:     None.
 *
 *****************************************************************************/

void events_gain_set( )
{
	/* Set message data to button values. */
    msg.gain.data.kp_yaw    = gtk_spin_button_get_value( (GtkSpinButton *)
        button_kp_yaw );
    msg.gain.data.ki_yaw    = gtk_spin_button_get_value( (GtkSpinButton *)
        button_ki_yaw );
    msg.gain.data.kd_yaw    = gtk_spin_button_get_value( (GtkSpinButton *)
        button_kd_yaw );
    msg.gain.data.kp_pitch  = gtk_spin_button_get_value( (GtkSpinButton *)
        button_kp_pitch );
    msg.gain.data.ki_pitch  = gtk_spin_button_get_value( (GtkSpinButton *)
        button_ki_pitch );
    msg.gain.data.kd_pitch  = gtk_spin_button_get_value( (GtkSpinButton *)
        button_kd_pitch );
    msg.gain.data.kp_roll   = gtk_spin_button_get_value( (GtkSpinButton *)
        button_kp_roll );
    msg.gain.data.ki_roll   = gtk_spin_button_get_value( (GtkSpinButton *)
        button_ki_roll );
    msg.gain.data.kd_roll   = gtk_spin_button_get_value( (GtkSpinButton *)
        button_kd_roll );
    msg.gain.data.kp_depth  = gtk_spin_button_get_value( (GtkSpinButton *)
        button_kp_depth );
    msg.gain.data.ki_depth  = gtk_spin_button_get_value( (GtkSpinButton *)
        button_ki_depth );
    msg.gain.data.kd_depth  = gtk_spin_button_get_value( (GtkSpinButton *)
        button_kd_depth );
    msg.gain.data.mode = GAIN_SET;

    /* Send the gain message. */
	if( planner_fd > 0 ) {
		messages_send( planner_fd, GAIN_MSGID, &msg );
	}
	if( nav_fd > 0 ) {
		messages_send( nav_fd, GAIN_MSGID, &msg );
	}
    msg.gain.data.mode = 0;
} /* end events_gain_set() */


/******************************************************************************
 *
 * Title:       void events_enable_servos( GtkWidget *widget,
 *                                                   GdkEvent *event,
 *                                                   gpointer data )
 *
 * Description: Called when enable servos button clicked.
 *
 * Input:       widget: A pointer to the button widget.
 *              event: A pointer to the event that triggered the callback.
 *              data: A pointer to data that can be manipulated.
 *
 * Output:      None.
 *
 * Globals:     None.
 *
 *****************************************************************************/

void events_enable_servos( GtkWidget *widget,
                           GdkEvent *event,
                           gpointer data
                         )
{
    /* Check the state of the button. */
    if( gtk_toggle_button_get_active( (GtkToggleButton *)widget ) ) {
        msg.client.data.enable_servos = TRUE;
    }
    else {
        msg.client.data.enable_servos = FALSE;
    }

    /* Send the state of the button to the server. */
    if( planner_fd > 0 ) {
        messages_send( planner_fd, CLIENT_MSGID, &msg );
    }
    if( nav_fd > 0 ) {
        messages_send( nav_fd, CLIENT_MSGID, &msg );
    }
} /* end events_enable_servos() */


/******************************************************************************
 *
 * Title:       void events_enable_imu( GtkWidget *widget,
 *                                                  GdkEvent * event,
 *                                                  gpointer data )
 *
 * Description: Called when enable IMU button clicked.
 *
 * Input:       widget: A pointer to the button widget.
 *              event: A pointer to the event that triggered the callback.
 *              data: A pointer to data that can be manipulated.
 *
 * Output:      None.
 *
 * Globals:     None.
 *
 *****************************************************************************/

void events_enable_imu( GtkWidget *widget,
                        GdkEvent *event,
                        gpointer data
                      )
{
    /* Check the state of the button. */
    if( gtk_toggle_button_get_active( ( GtkToggleButton * )widget ) ) {
        msg.client.data.enable_imu = TRUE;
    }
    else {
        msg.client.data.enable_imu = FALSE;
    }

    /* Send the state of the button to the server. */
    if( planner_fd > 0 ) {
        messages_send( planner_fd, CLIENT_MSGID, &msg );
    }
    if( nav_fd > 0 ) {
        messages_send( nav_fd, CLIENT_MSGID, &msg );
    }
} /* end events_enable_imu() */


/******************************************************************************
 *
 * Title:       void events_enable_log( GtkWidget *widget,
 *                                                  GdkEvent *event,
 *                                                  gpointer data )
 *
 * Description: Called when enable log button clicked.
 *
 * Input:       widget: A pointer to the button widget.
 *              event: A pointer to the event that triggered the callback.
 *              data: A pointer to data that can be manipulated.
 *
 * Output:      None.
 *
 * Globals:     None.
 *
 *****************************************************************************/

void events_enable_log( GtkWidget *widget,
                        GdkEvent *event,
                        gpointer data
                      )
{
    /* Check the state of the button. */
    if( gtk_toggle_button_get_active( ( GtkToggleButton * )widget ) ) {
        msg.client.data.enable_log = TRUE;
    }
    else {
        msg.client.data.enable_log = FALSE;
    }

    /* Send the state of the button to the server. */
    if( planner_fd > 0 ) {
        messages_send( planner_fd, CLIENT_MSGID, &msg );
    }
    if( nav_fd > 0 ) {
        messages_send( nav_fd, CLIENT_MSGID, &msg );
    }
} /* end events_enable_log() */


/******************************************************************************
 *
 * Title:       void events_imu_stab(   GtkWidget *widget,
 *                                              GdkEvent *event,
 *                                              gpointer data)
 *
 * Description: Called when IMU stabilized button clicked.
 *
 * Input:       widget: A pointer to the button widget.
 *              event: A pointer to the event that triggered the callback.
 *              data: A pointer to data that can be manipulated.
 *
 * Output:      None.
 *
 * Globals:     None.
 *
 *****************************************************************************/

void events_imu_stab( GtkWidget *widget,
                      GdkEvent *event,
                      gpointer data
                    )
{
    /* Check the state of the button. */
    if( gtk_toggle_button_get_active( ( GtkToggleButton * )widget ) ) {
        msg.client.data.imu_stab = TRUE;
    }
    else {
        msg.client.data.imu_stab = FALSE;
    }

    /* Send the state of the button to the server. */
    if( planner_fd > 0 ) {
        messages_send( planner_fd, CLIENT_MSGID, &msg );
    }
    if( nav_fd > 0 ) {
        messages_send( nav_fd, CLIENT_MSGID, &msg );
    }
} /* end events_imu_stab() */


/******************************************************************************
 *
 * Title:       void events_cf.debug_level( GtkWidget *widget,
 *                                                      GdkEvent *event,
 *                                                      gpointer data )
 *
 * Description: Called when debug level value changes.
 *
 * Input:       widget: A pointer to the button widget.
 *              event: A pointer to the event that triggered the callback.
 *              data: A pointer to data that can be manipulated.
 *
 * Output:      None.
 *
 * Globals:     None.
 *
 *****************************************************************************/

void events_debug_level( GtkWidget *widget,
                         GdkEvent *event,
                         gpointer data
                       )
{
    /* Check the state of the button. */
    msg.client.data.debug_level = gtk_spin_button_get_value_as_int(
                                      (GtkSpinButton *)widget );

    /* Send the value of the button to the server. */
    if( planner_fd > 0 ) {
        messages_send( planner_fd, CLIENT_MSGID, &msg );
    }
    if( nav_fd > 0 ) {
        messages_send( nav_fd, CLIENT_MSGID, &msg );
    }
} /* end events_debug_level() */


/******************************************************************************
 *
 * Title:       void events_cf.dropper( GtkWidget *widget,
 *                                          GdkEvent *event,
 *                                          gpointer data )
 *
 * Description: Called when dropper value changes.
 *
 * Input:       widget: A pointer to the button widget.
 *              event: A pointer to the event that triggered the callback.
 *              data: A pointer to data that can be manipulated.
 *
 * Output:      None.
 *
 * Globals:     None.
 *
 *****************************************************************************/

void events_dropper( GtkWidget *widget,
                     GdkEvent *event,
                     gpointer data
                    )
{
    /* Check the state of the button. */
    msg.client.data.dropper = gtk_spin_button_get_value_as_int(
                                      (GtkSpinButton *)widget );

    /* Send the value of the button to the server. */
    if( planner_fd > 0 ) {
        messages_send( planner_fd, CLIENT_MSGID, &msg );
    }
    if( nav_fd > 0 ) {
        messages_send( nav_fd, CLIENT_MSGID, &msg );
    }
} /* end events_debug_level() */


/******************************************************************************
 *
 * Title:       void events_opmode( GtkWidget *widget,
 *                                              GdkEvent *event,
 *                                              gpointer data )
 *
 * Description: Called when operation mode value changes.
 *
 * Input:       widget: A pointer to the button widget.
 *              event: A pointer to the event that triggered the callback.
 *              data: A pointer to data that can be manipulated.
 *
 * Output:      None.
 *
 * Globals:     opmode_msg
 *
 *****************************************************************************/

void events_opmode( GtkWidget *widget,
                    GdkEvent *event,
                    gpointer data
                  )
{
    /* Check the state of the buttons and send message. */
    if( gtk_toggle_button_get_active( ( GtkToggleButton * )button_hold_yaw ) ) {
        msg.target.data.mode = (int)HOLD_YAW;
        if( planner_fd > 0 ) {
            messages_send( planner_fd, TARGET_MSGID, &msg );
        }
        if( nav_fd > 0 ) {
            messages_send( nav_fd, TARGET_MSGID, &msg );
        }
    }

    else if( gtk_toggle_button_get_active( ( GtkToggleButton * )button_hold_roll ) ) {
        msg.target.data.mode = (int)HOLD_ROLL;
        if( planner_fd > 0 ) {
            messages_send( planner_fd, TARGET_MSGID, &msg );
        }
        if( nav_fd > 0 ) {
            messages_send( nav_fd, TARGET_MSGID, &msg );
        }
    }

    else if( gtk_toggle_button_get_active( ( GtkToggleButton * )button_hold_pitch ) ) {
        msg.target.data.mode = (int)HOLD_PITCH;
        if( planner_fd > 0 ) {
            messages_send( planner_fd, TARGET_MSGID, &msg );
        }
        if( nav_fd > 0 ) {
            messages_send( nav_fd, TARGET_MSGID, &msg );
        }
    }

    else if( gtk_toggle_button_get_active( ( GtkToggleButton * )button_hold_accel ) ) {
        msg.target.data.mode = (int)HOLD_ACCEL;
        if( planner_fd > 0 ) {
            messages_send( planner_fd, TARGET_MSGID, &msg );
        }
        if( nav_fd > 0 ) {
            messages_send( nav_fd, TARGET_MSGID, &msg );
        }
    }

    else if( gtk_toggle_button_get_active( ( GtkToggleButton * )button_hold_ang_rate ) ) {
        msg.target.data.mode = (int)HOLD_ANG_RATE;
        if( planner_fd > 0 ) {
            messages_send( planner_fd, TARGET_MSGID, &msg );
        }
        if( nav_fd > 0 ) {
            messages_send( nav_fd, TARGET_MSGID, &msg );
        }
    }

    else if( gtk_toggle_button_get_active( ( GtkToggleButton * )button_manual ) ) {
        msg.target.data.mode = (int)MANUAL;
        if( planner_fd > 0 ) {
            messages_send( planner_fd, TARGET_MSGID, &msg );
        }
        if( nav_fd > 0 ) {
            messages_send( nav_fd, TARGET_MSGID, &msg );
        }
    }

    else if( gtk_toggle_button_get_active( ( GtkToggleButton * )button_autonomous ) ) {
        msg.target.data.mode = (int)AUTONOMOUS;
        if( planner_fd > 0 ) {
            messages_send( planner_fd, TARGET_MSGID, &msg );
        }
        if( nav_fd > 0 ) {
            messages_send( nav_fd, TARGET_MSGID, &msg );
        }
    }
} /* end events_opmode() */


/******************************************************************************
 *
 * Title:       void events_images( GtkWidget *widget,
 *                                  GdkEvent *event,
 *                                  gpointer data )
 *
 * Description: Called when image mode value changes.
 *
 * Input:       widget: A pointer to the button widget.
 *              event: A pointer to the event that triggered the callback.
 *              data: A pointer to data that can be manipulated.
 *
 * Output:      None.
 *
 *****************************************************************************/

void events_images( GtkWidget *widget,
                    GdkEvent *event,
                    gpointer data
                  )
{
    /* Check the state of the buttons and send message. */
    if( gtk_toggle_button_get_active( (GtkToggleButton *)button_image_fcolor ) ) {
        msg.vision.data.mode = VISIOND_FCOLOR;
        if( vision_fd > 0 ) {
            messages_send( vision_fd, VISION_MSGID, &msg );
        }
    }
    else if( gtk_toggle_button_get_active( ( GtkToggleButton * )button_image_fbinary ) ) {
        msg.vision.data.mode = VISIOND_FBINARY;
        if( vision_fd > 0 ) {
            messages_send( vision_fd, VISION_MSGID, &msg );
        }
    }
    else if( gtk_toggle_button_get_active( ( GtkToggleButton * )button_image_bcolor ) ) {
        msg.vision.data.mode = VISIOND_BCOLOR;
        if( vision_fd > 0 ) {
            messages_send( vision_fd, VISION_MSGID, &msg );
        }
    }
    else if( gtk_toggle_button_get_active( ( GtkToggleButton * )button_image_bbinary ) ) {
        msg.vision.data.mode = VISIOND_BBINARY;
        if( vision_fd > 0 ) {
            messages_send( vision_fd, VISION_MSGID, &msg );
        }
    }
    else if( gtk_toggle_button_get_active( ( GtkToggleButton * )button_image_none ) ) {
        msg.vision.data.mode = VISIOND_NONE;
        if( vision_fd > 0 ) {
            messages_send( vision_fd, VISION_MSGID, &msg );
        }
    }
} /* end events_images() */


/******************************************************************************
 *
 * Title:       void events_target_yaw( GtkWidget *widget,
                                                    GdkEvent *event,
 *                                                  gpointer data )
 *
 * Description: Called when target yaw value changes.
 *
 * Input:       widget: A pointer to the button widget.
 *              event: A pointer to the event that triggered the callback.
 *              data: A pointer to data that can be manipulated.
 *
 * Output:      None.
 *
 * Globals:     None.
 *
 *****************************************************************************/

void events_target_yaw( GtkWidget *widget,
                        GdkEvent *event,
                        gpointer data
                      )
{
    /* Set the target values to the spin buttons. */
    msg.target.data.yaw = gtk_spin_button_get_value_as_float(
                              (GtkSpinButton *)widget );

    /* Send the state of the button to the server if not in MANUAL mode. */
    if( msg.target.data.mode != MANUAL ) {
		if( planner_fd > 0 ) {
			messages_send( planner_fd, TARGET_MSGID, &msg );
		}
		if( nav_fd > 0 ) {
			messages_send( nav_fd, TARGET_MSGID, &msg );
		}
    }
} /* end events_target_yaw() */


/******************************************************************************
 *
 * Title:       void events_target_roll( GtkWidget *widget,
 *                                                  GdkEvent *event,
 *                                                  gpointer data )
 *
 * Description: Called when target roll value changes.
 *
 * Input:       widget: A pointer to the button widget.
 *              event: A pointer to the event that triggered the callback.
 *              data: A pointer to data that can be manipulated.
 *
 * Output:      None.
 *
 * Globals:     None.
 *
 *****************************************************************************/

void events_target_roll( GtkWidget *widget,
                         GdkEvent *event,
                         gpointer data
                       )
{
    /* Set the target values to the spin buttons. */
    msg.target.data.roll = gtk_spin_button_get_value_as_float(
                               (GtkSpinButton *)widget );

    /* Send the state of the button to the server if not in MANUAL mode. */
    if( msg.target.data.mode != MANUAL ) {
		if( planner_fd > 0 ) {
			messages_send( planner_fd, TARGET_MSGID, &msg );
		}
		if( nav_fd > 0 ) {
			messages_send( nav_fd, TARGET_MSGID, &msg );
		}
    }
} /* end events_target_roll() */


/******************************************************************************
 *
 * Title:       void events_target_pitch( GtkWidget *widget,
 *                                                  GdkEvent *event,
 *                                                  gpointer data )
 *
 * Description: Called when target pitch value changes.
 *
 * Input:       widget: A pointer to the button widget.
 *              event: A pointer to the event that triggered the callback.
 *              data: A pointer to data that can be manipulated.
 *
 * Output:      None.
 *
 * Globals:     None.
 *
 *****************************************************************************/

void events_target_pitch( GtkWidget *widget,
                          GdkEvent *event,
                          gpointer data
                        )
{
    /* Set the target values to the spin buttons. */
    msg.target.data.pitch = gtk_spin_button_get_value_as_float(
                                (GtkSpinButton *)widget );

    /* Send the state of the button to the server if not in MANUAL mode. */
    if( msg.target.data.mode != MANUAL ) {
		if( planner_fd > 0 ) {
			messages_send( planner_fd, TARGET_MSGID, &msg );
		}
		if( nav_fd > 0 ) {
			messages_send( nav_fd, TARGET_MSGID, &msg );
		}
    }
} /* end events_target_pitch() */


/******************************************************************************
 *
 * Title:       void events_target_depth( GtkWidget *widget,
 *                                                  GdkEvent *event,
 *                                                  gpointer data )
 *
 * Description: Called when target depth value changes.
 *
 * Input:       widget: A pointer to the button widget.
 *              event: A pointer to the event that triggered the callback.
 *              data: A pointer to data that can be manipulated.
 *
 * Output:      None.
 *
 * Globals:     None.
 *
 *****************************************************************************/

void events_target_depth( GtkWidget *widget,
                          GdkEvent *event,
                          gpointer data
                        )
{
    /* Set the target values to the spin buttons. */
    msg.target.data.depth = gtk_spin_button_get_value_as_float(
                                (GtkSpinButton *)widget );

    /* Send the state of the button to the server if not in MANUAL mode. */
    if( msg.target.data.mode != MANUAL ) {
		if( planner_fd > 0 ) {
			messages_send( planner_fd, TARGET_MSGID, &msg );
		}
		if( nav_fd > 0 ) {
			messages_send( nav_fd, TARGET_MSGID, &msg );
		}
    }
} /* end events_target_pitch() */


/******************************************************************************
 *
 * Title:       void events_target_fx( GtkWidget *widget,
 *                                      GdkEvent *event,
 *                                      gpointer data )
 *
 * Description: Called when target Fx value changes.
 *
 * Input:       widget: A pointer to the button widget.
 *              event: A pointer to the event that triggered the callback.
 *              data: A pointer to data that can be manipulated.
 *
 * Output:      None.
 *
 * Globals:     None.
 *
 *****************************************************************************/

void events_target_fx( GtkWidget *widget,
                       GdkEvent *event,
                       gpointer data
                     )
{
    /* Set the target values to the spin buttons. */
    msg.target.data.fx = gtk_spin_button_get_value_as_float(
                             (GtkSpinButton *)widget );

    /* Send the state of the button to the server if not in MANUAL mode. */
    if( msg.target.data.mode != MANUAL ) {
		if( planner_fd > 0 ) {
			messages_send( planner_fd, TARGET_MSGID, &msg );
		}
		if( nav_fd > 0 ) {
			messages_send( nav_fd, TARGET_MSGID, &msg );
		}
    }
} /* end events_target_fx() */


/******************************************************************************
 *
 * Title:       void events_target_fy( GtkWidget *widget,
 *                                      GdkEvent *event,
 *                                      gpointer data )
 *
 * Description: Called when target Fy value changes.
 *
 * Input:       widget: A pointer to the button widget.
 *              event: A pointer to the event that triggered the callback.
 *              data: A pointer to data that can be manipulated.
 *
 * Output:      None.
 *
 * Globals:     None.
 *
 *****************************************************************************/

void events_target_fy( GtkWidget *widget,
                       GdkEvent *event,
                       gpointer data
                     )
{
    /* Set the target values to the spin buttons. */
    msg.target.data.fy = gtk_spin_button_get_value_as_float(
                             (GtkSpinButton *)widget ) * -1;

    /* Send the state of the button to the server if not in MANUAL mode. */
    if( msg.target.data.mode != MANUAL ) {
		if( planner_fd > 0 ) {
			messages_send( planner_fd, TARGET_MSGID, &msg );
		}
		if( nav_fd > 0 ) {
			messages_send( nav_fd, TARGET_MSGID, &msg );
		}
    }
} /* end events_target_fy() */


/******************************************************************************
 *
 * Title:       void events_target_speed( GtkWidget *widget,
 *                                      GdkEvent *event,
 *                                      gpointer data )
 *
 * Description: Called when target Speed value changes.
 *
 * Input:       widget: A pointer to the button widget.
 *              event: A pointer to the event that triggered the callback.
 *              data: A pointer to data that can be manipulated.
 *
 * Output:      None.
 *
 * Globals:     None.
 *
 *****************************************************************************/

void events_target_speed( GtkWidget *widget,
                          GdkEvent *event,
                          gpointer data
                        )
{
    /* Set the target values to the spin buttons. */
    msg.target.data.speed = gtk_spin_button_get_value_as_float(
                                (GtkSpinButton *)widget );

    /* Send the state of the button to the server if not in MANUAL mode. */
    if( msg.target.data.mode != MANUAL ) {
		if( planner_fd > 0 ) {
			messages_send( planner_fd, TARGET_MSGID, &msg );
		}
		if( nav_fd > 0 ) {
			messages_send( nav_fd, TARGET_MSGID, &msg );
		}
    }
} /* end events_target_speed() */


/******************************************************************************
 *
 * Title:       void events_target_current( GtkWidget *widget,
 *                                      GdkEvent *event,
 *                                      gpointer data )
 *
 * Description: Used to set target values to current vehicle pose.
 *
 * Input:       widget: A pointer to the button widget.
 *              event: A pointer to the event that triggered the callback.
 *              data: A pointer to data that can be manipulated.
 *
 * Output:      None.
 *
 * Globals:     None.
 *
 *****************************************************************************/

void events_target_current( GtkWidget *widget,
                          GdkEvent *event,
                          gpointer data
                        )
{
    /* Set the target values to the current vehicle pose. */
	msg.target.data.pitch = msg.status.data.pitch;
	msg.target.data.roll  = msg.status.data.roll;
	msg.target.data.yaw   = msg.status.data.yaw;
	msg.target.data.depth = msg.status.data.depth;
	msg.target.data.fx    = 0;
	msg.target.data.fy    = 0;
	
	/* Set the button values to the current target values. */
	gtk_spin_button_set_value( (GtkSpinButton *)button_target_pitch, msg.target.data.pitch);
	gtk_spin_button_set_value( (GtkSpinButton *)button_target_roll, msg.target.data.roll);
	gtk_spin_button_set_value( (GtkSpinButton *)button_target_yaw, msg.target.data.yaw);
	gtk_spin_button_set_value( (GtkSpinButton *)button_target_depth, msg.target.data.depth);
	gtk_spin_button_set_value( (GtkSpinButton *)button_target_fx, msg.target.data.fx);
	gtk_spin_button_set_value( (GtkSpinButton *)button_target_fy, msg.target.data.fy);
	gtk_spin_button_set_value( (GtkSpinButton *)button_target_speed, msg.target.data.speed);

    /* Send the state of the button to the server if not in MANUAL mode. */
    if( msg.target.data.mode != MANUAL ) {
		if( planner_fd > 0 ) {
			messages_send( planner_fd, TARGET_MSGID, &msg );
		}
		if( nav_fd > 0 ) {
			messages_send( nav_fd, TARGET_MSGID, &msg );
		}
    }
} /* end events_target_current() */


/******************************************************************************
 *
 * Title:       void events_zero_pid( GtkWidget *widget,
 *                                      GdkEvent *event,
 *                                      gpointer data )
 *
 * Description: Sets targets mode to ZERO_PID_ERRORS.
 *
 * Input:       widget: A pointer to the button widget.
 *              event: A pointer to the event that triggered the callback.
 *              data: A pointer to data that can be manipulated.
 *
 * Output:      None.
 *
 * Globals:     None.
 *
 *****************************************************************************/

void events_zero_pid( GtkWidget *widget,
                          GdkEvent *event,
                          gpointer data
                        )
{
    /* Set the target values to the current vehicle pose. */
	msg.target.data.mode = ZERO_PID_ERRORS;

    /* Send the value of the button to the server if not in MANUAL mode. */
    if( planner_fd > 0 ) {
        messages_send( planner_fd, TARGET_MSGID, &msg );
    }
	if( nav_fd > 0 ) {
        messages_send( nav_fd, TARGET_MSGID, &msg );
	}
	
	/* Reset the mode to AUTONOMOUS. */
	msg.target.data.mode = AUTONOMOUS;

    /* Send the value of the button to the server if not in MANUAL mode. */
	if( planner_fd > 0 ) {
		messages_send( planner_fd, TARGET_MSGID, &msg );
	}
	if( nav_fd > 0 ) {
		messages_send( nav_fd, TARGET_MSGID, &msg );
	}
} /* end events_zero_pid() */
