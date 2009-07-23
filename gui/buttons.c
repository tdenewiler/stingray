/******************************************************************************
 *
 *  Title:        buttons.c
 *
 *  Description:  Functions to create buttons for the GUI.
 *
 *****************************************************************************/


#include <sys/timeb.h>
#include <gtk/gtk.h>

#include "buttons.h"
#include "events.h"
#include "messages.h"
#include "pololu.h"
#include "parser.h"


/* Handle for status output widget. */
GtkWidget *label_status;

/* Emergency stop button. */
GtkWidget *button_estop;

/* Operation mode buttons. */
GtkWidget *button_hold_yaw;
GtkWidget *button_hold_roll;
GtkWidget *button_hold_pitch;
GtkWidget *button_hold_accel;
GtkWidget *button_hold_ang_rate;
GtkWidget *button_manual;
GtkWidget *button_autonomous;

/* Vision setting buttons. */
GtkWidget *button_hpipe_lo;
GtkWidget *button_hpipe_hi;
GtkWidget *button_spipe_lo;
GtkWidget *button_spipe_hi;
GtkWidget *button_vpipe_lo;
GtkWidget *button_vpipe_hi;
GtkWidget *button_hbuoy_lo;
GtkWidget *button_hbuoy_hi;
GtkWidget *button_sbuoy_lo;
GtkWidget *button_sbuoy_hi;
GtkWidget *button_vbuoy_lo;
GtkWidget *button_vbuoy_hi;
GtkWidget *button_hfence_lo;
GtkWidget *button_hfence_hi;
GtkWidget *button_sfence_lo;
GtkWidget *button_sfence_hi;
GtkWidget *button_vfence_lo;
GtkWidget *button_vfence_hi;
GtkWidget *button_save_bframe;
GtkWidget *button_save_fframe;
GtkWidget *button_save_bvideo;
GtkWidget *button_save_fvideo;

/* Tasks buttons. */
GtkWidget *button_task_gate;
GtkWidget *button_task_buoy;
GtkWidget *button_task_pipe;
GtkWidget *button_task_square;
GtkWidget *button_task_none;
GtkWidget *button_task_fence;
GtkWidget *button_task_boxes;
GtkWidget *button_task_suitcase;
GtkWidget *button_task_course;
GtkWidget *button_task_nod;
GtkWidget *button_task_spin;
GtkWidget *button_task_square_time1;
GtkWidget *button_task_square_time2;
GtkWidget *button_task_square_time3;
GtkWidget *button_task_square_time4;

/* Gain buttons. */
GtkWidget *button_kp_yaw;
GtkWidget *button_ki_yaw;
GtkWidget *button_kd_yaw;
GtkWidget *button_kp_pitch;
GtkWidget *button_ki_pitch;
GtkWidget *button_kd_pitch;
GtkWidget *button_kp_roll;
GtkWidget *button_ki_roll;
GtkWidget *button_kd_roll;
GtkWidget *button_kp_depth;
GtkWidget *button_ki_depth;
GtkWidget *button_kd_depth;
GtkWidget *button_kp_fx;
GtkWidget *button_ki_fx;
GtkWidget *button_kd_fx;
GtkWidget *button_kp_fy;
GtkWidget *button_ki_fy;
GtkWidget *button_kd_fy;
GtkWidget *button_kp_roll_lateral;
GtkWidget *button_kp_depth_forward;
GtkWidget *button_kp_place_holder;
GtkWidget *button_cf_gains;
GtkWidget *button_zero_gains;

/* Target buttons. */
GtkWidget *button_target_yaw;
GtkWidget *button_target_roll;
GtkWidget *button_target_pitch;
GtkWidget *button_target_depth;
GtkWidget *button_target_fx;
GtkWidget *button_target_fy;
GtkWidget *button_target_speed;
GtkWidget *button_target_current;
GtkWidget *button_zero_pid;

/* Image option buttons. */
GtkWidget *button_image_fcolor;
GtkWidget *button_image_fbinary;
GtkWidget *button_image_bcolor;
GtkWidget *button_image_bbinary;
GtkWidget *button_image_none;

/* Network API messages. */
extern MSG_DATA msg;

/* Configuration file variables. */
extern CONF_VARS cf;


/******************************************************************************
 *
 * Title:       void buttons_update_values( )
 *
 * Description: Update the values of buttons based on state data sent by the
 *              UUV.
 *
 * Input:       None.
 *
 * Output:      None.
 *
 * Globals:     None.
 *
 *****************************************************************************/

void buttons_update_values( )
{
    /* Activate correct operational mode button based on state data. */
    switch( msg.target.data.mode ) {
	case MANUAL:
		gtk_toggle_button_set_active( ( GtkToggleButton * )button_manual, TRUE );
		break;
	case AUTONOMOUS:
		gtk_toggle_button_set_active( ( GtkToggleButton * )button_autonomous, TRUE );
		break;
    }
} /* end buttons_update_values() */


/******************************************************************************
 *
 * Title:       GtkWidget *buttons_make_frame( const gchar *name )
 *
 * Description: Creates a frame for a widget or group of widgets.
 *
 * Input:       name: Name of the frame that gets embedded into widget.
 *
 * Output:      frame: A GtkWidget * that can be used later on.
 *
 * Globals:     None.
 *
 *****************************************************************************/

GtkWidget *buttons_make_frame( const gchar *name )
{
    GtkWidget *frame;

    frame = gtk_frame_new( name );

    return frame;
} /* end buttons_make_frame() */


/******************************************************************************
 *
 * Title:       GtkWidget *buttons_make_radio( const char *name,
 *                                              GtkSignalFunc callback
 *                                          )
 *
 * Description: Creates a radio style GTK button.
 *
 * Input:       name: The name to display by the radio button.
 *              callback: The function to call when the specified action occurs
 *                        with the widget.
 *
 * Output:      radio_button: A GtkWidget * to be used later.
 *
 * Globals:     None.
 *
 *****************************************************************************/

GtkWidget *buttons_make_radio( const char *name, GtkSignalFunc callback, GtkWidget *group )
{
    GtkWidget *radio_button;

    radio_button = gtk_radio_button_new_with_label_from_widget(
		GTK_RADIO_BUTTON( group ), name );
    gtk_signal_connect( GTK_OBJECT( radio_button ), "released", callback, NULL );

    return radio_button;
} /* end buttons_make_radio() */


/******************************************************************************
 *
 * Title:       GtkWidget *buttons_make_radio_group( )
 *
 * Description: Creates a radio group for radio buttons.
 *
 * Input:       None.
 *
 * Output:      group: A GtkWidget * to be used later.
 *
 * Globals:     None.
 *
 *****************************************************************************/

GtkWidget *buttons_make_radio_group( )
{
    GtkWidget *group;

    group = gtk_radio_button_new( NULL );

    return group;
} /* end buttons_make_radio_group() */


/******************************************************************************
 *
 * Title:       GtkWidget *buttons_make_check(const char *name,
 *                                                  GtkSignalFunc callback
 *                                              )
 *
 * Description: Creates a check style GTK button.
 *
 * Input:       name: The name to display next to the check button.
 *              callback: The function to call when the specified action occurs
 *                        with the widget.
 *
 * Output:      check_button: A GtkWidget * to be used later.
 *
 * Globals:     None.
 *
 *****************************************************************************/

GtkWidget *buttons_make_check( const char *name,
                               GtkSignalFunc callback
                             )
{
    GtkWidget *check_button;

    check_button = gtk_check_button_new_with_label( name );
    gtk_signal_connect( GTK_OBJECT( check_button ), "clicked", callback, NULL );

    return check_button;
} /* end buttons_make_check() */


/******************************************************************************
 *
 * Title:       GtkWidget *buttons_make_spin( const char *name,
 *                                           GtkSignalFunc callback,
 *                                           GtkWidget *box,
 *                                           GtkAdjustment *adj
 *                                          )
 *
 * Description: Creates a spin style GTK button.
 *
 * Input:       name: The name to display next to the spin button.
 *              callback: The function to call when the specified action occurs
 *                        with the widget.
 *              box: The box to pack the whole spin button into.
 *              adj: Default values for how the spin button will operate.
 *
 * Output:      spin_button: A GtkWidget * to be used later.
 *
 * Globals:     None.
 *
 *****************************************************************************/

GtkWidget *buttons_make_spin( const char *name,
                              GtkSignalFunc callback,
                              GtkWidget *box,
                              GtkAdjustment *adj
                            )
{
    GtkWidget *spin_button;
    GtkWidget *frame;
    GtkWidget *hbox;
    GtkWidget *vbox;

    /* Create box and pack it. */
    hbox = gtk_hbox_new( TRUE, 0 );
    gtk_box_pack_start( GTK_BOX( box ), hbox, FALSE, TRUE, 0 );

    /* Create frame and pack it. */
    frame = gtk_frame_new( name );
    gtk_box_pack_start( GTK_BOX( hbox ), frame, FALSE, TRUE, 0 );

    /* Create box and pack it into frame. */
    vbox = gtk_vbox_new( TRUE, 0 );
    gtk_container_add( GTK_CONTAINER( frame ), vbox );

    /* Create spin button and set properties. */
    spin_button = gtk_spin_button_new( adj, 0.5, 3 );
    gtk_spin_button_set_numeric( GTK_SPIN_BUTTON( spin_button ), TRUE );
    gtk_spin_button_set_wrap( GTK_SPIN_BUTTON( spin_button ), FALSE );
    gtk_spin_button_set_snap_to_ticks( GTK_SPIN_BUTTON( spin_button ), TRUE );
    gtk_spin_button_set_update_policy( GTK_SPIN_BUTTON( spin_button ),
		GTK_UPDATE_IF_VALID );

    /* Pack spin button into box inside frame. */
    gtk_box_pack_start( GTK_BOX( vbox ), spin_button, FALSE, TRUE, 0 );

    /* Connect a callback to spin button. */
    g_signal_connect( GTK_OBJECT( spin_button ), "value_changed", callback, NULL );

    return spin_button;
} /* end buttons_make_spin() */


/******************************************************************************
 *
 * Title:       int buttons_make_estop( GtkWidget *box )
 *
 * Description: Sets up an hbox for the Emergency Stop button to be packed
 *              into.
 *
 * Input:       box: The box to pack the Emergency Stop buttons into.
 *
 * Output:      TRUE: No error checking implemented.
 *
 * Globals:     opmode_msg, radio_group, radio_frame, button_hold_heading,
 *              button_hold_velocity, button_hold_accel, button_hold_ang_rate,
 *              button_manual, button_waypoint, button_autonomous
 *
 *****************************************************************************/

int buttons_make_estop( GtkWidget *box )
{
    GtkWidget *hbox;
    GtkWidget *vbox;
    GtkWidget *estop_frame;
    GdkColor red = {0, 0xffff, 0x0000, 0x0000};

    /* Initialize values. */
    msg.stop.data.state = FALSE;

    /* Create new boxes. */
    hbox = gtk_hbox_new( TRUE, 10 );
    vbox = gtk_hbox_new( TRUE, 10 );

    /* Create a frame for the emergency stop button. */
    estop_frame = buttons_make_frame( "EMERGENCY STOP" );

    /* Add the frame to a box. */
    gtk_container_add( GTK_CONTAINER( vbox ), estop_frame );

    /* Add a box to the frame. */
    gtk_container_add( GTK_CONTAINER( estop_frame ), hbox );

    /* Create the check button and make it red. */
    button_estop = buttons_make_check( "E-Stop",
		GTK_SIGNAL_FUNC( events_estop ) );
    gtk_widget_modify_bg( button_estop, GTK_STATE_NORMAL, &red );
    gtk_widget_modify_bg( button_estop, GTK_STATE_ACTIVE, &red );
    gtk_widget_modify_bg( button_estop, GTK_STATE_PRELIGHT, &red );
    gtk_widget_modify_bg( button_estop, GTK_STATE_SELECTED, &red );
    gtk_widget_modify_bg( button_estop, GTK_STATE_INSENSITIVE, &red );
    gtk_widget_modify_fg( button_estop, GTK_STATE_NORMAL, &red );
    gtk_widget_modify_fg( button_estop, GTK_STATE_ACTIVE, &red );
    gtk_widget_modify_fg( button_estop, GTK_STATE_PRELIGHT, &red );
    gtk_widget_modify_fg( button_estop, GTK_STATE_SELECTED, &red );
    gtk_widget_modify_fg( button_estop, GTK_STATE_INSENSITIVE, &red );

    /* Pack the boxes. */
    gtk_box_pack_start( GTK_BOX( hbox ), button_estop, TRUE, TRUE, 10 );

    /* Add box with buttons to the frame. */
    gtk_box_pack_start( GTK_BOX( box ), vbox, TRUE, TRUE, 10 );

    return TRUE;
} /* end buttons_make_estop() */


/******************************************************************************
 *
 * Title:       int buttons_opmode( GtkWidget *box )
 *
 * Description: Sets up an hbox for the Operating Mode buttons to be packed
 *              into.
 *
 * Input:       box: The box to pack the Operating Mode buttons into.
 *
 * Output:      TRUE: No error checking implemented.
 *
 * Globals:     opmode_msg, radio_group, radio_frame, button_hold_heading,
 *              button_hold_velocity, button_hold_accel, button_hold_ang_rate,
 *              button_manual, button_waypoint, button_autonomous
 *
 *****************************************************************************/

int buttons_opmode( GtkWidget *box )
{
    GtkWidget *hbox;
    GtkWidget *vbox;
    GtkWidget *opmode_frame;
    GtkWidget *opmode_group;

    /* Initialize values. */
    msg.target.data.mode = HOLD_YAW;

    /* Create new boxes. */
    hbox = gtk_hbox_new( TRUE, 0 );
    vbox = gtk_vbox_new( TRUE, 0 );

    /* Create opmode radio group. */
    opmode_group = buttons_make_radio_group( );

    /* Create a frame for the radio buttons. */
    opmode_frame = buttons_make_frame( "Operating Mode" );

    /* Add the frame to a box. */
    gtk_container_add( GTK_CONTAINER( vbox ), opmode_frame );

    /* Add a box to the frame. */
    gtk_container_add( GTK_CONTAINER( opmode_frame ), hbox );

    /* Create the radio buttons. */
    button_hold_yaw = buttons_make_radio( "Yaw",
		GTK_SIGNAL_FUNC( events_opmode ), opmode_group );
    button_hold_roll = buttons_make_radio( "Roll",
		GTK_SIGNAL_FUNC( events_opmode ), opmode_group );
    button_hold_pitch = buttons_make_radio( "Pitch",
        GTK_SIGNAL_FUNC( events_opmode ), opmode_group );
    button_hold_accel = buttons_make_radio( "Acceleration",
        GTK_SIGNAL_FUNC( events_opmode ), opmode_group );
    button_hold_ang_rate = buttons_make_radio( "Angular Rate",
        GTK_SIGNAL_FUNC( events_opmode ), opmode_group );
    button_manual = buttons_make_radio( "Manual",
        GTK_SIGNAL_FUNC( events_opmode ), opmode_group );
    button_autonomous = buttons_make_radio( "Autonomous",
        GTK_SIGNAL_FUNC( events_opmode ), opmode_group );

    /* Pack the boxes. */
    gtk_box_pack_start( GTK_BOX( hbox ), button_autonomous, FALSE, TRUE, 0 );
    gtk_box_pack_start( GTK_BOX( hbox ), button_manual, FALSE, TRUE, 0 );

    /* Add box with buttons to the frame. */
    gtk_box_pack_start( GTK_BOX( box ), vbox, FALSE, TRUE, 0 );

    /* Activate specific buttons by default. */
    gtk_toggle_button_set_active( ( GtkToggleButton * )button_autonomous, TRUE );

    return TRUE;
} /* end buttons_opmode() */


/******************************************************************************
 *
 * Title:       int buttons_tasks( GtkWidget *box )
 *
 * Description: Sets up a vbox for the UUV Tasks to be packed into.
 *
 * Input:       box: The box to pack the UUV Tasks buttons into.
 *
 * Output:      TRUE: No error checking implemented.
 *
 * Globals:     client_msg
 *
 *****************************************************************************/

int buttons_tasks( GtkWidget *box )
{
    GtkWidget *vbox1;
    GtkWidget *vbox2;
    GtkWidget *vbox3;
    GtkWidget *vbox4;
    GtkWidget *vbox5;
    GtkWidget *hbox1;
    GtkWidget *hbox2;
    GtkWidget *hbox3;
    GtkWidget *hbox4;
    GtkWidget *hbox5;
    GtkWidget *tasks_frame;
    GtkWidget *tasks_group;

    /* Create new boxes. */
    vbox1 = gtk_vbox_new( FALSE, 0 );
    vbox2 = gtk_vbox_new( TRUE, 0 );
    vbox3 = gtk_vbox_new( TRUE, 0 );
    vbox4 = gtk_vbox_new( TRUE, 0 );
    vbox5 = gtk_vbox_new( TRUE, 0 );
    hbox1 = gtk_hbox_new( FALSE, 0 );
    hbox2 = gtk_hbox_new( TRUE, 0 );
    hbox3 = gtk_hbox_new( TRUE, 0 );
    hbox4 = gtk_hbox_new( TRUE, 0 );
    hbox5 = gtk_hbox_new( TRUE, 0 );

    /* Create a frame for the task buttons. */
    tasks_frame = buttons_make_frame( "Tasks" );

    /* Create a radio group for the task buttons. */
    tasks_group = buttons_make_radio_group( );

    /* Add the frame to a box. */
    gtk_container_add( GTK_CONTAINER( vbox1 ), tasks_frame );

    /* Add an hbox to the frame. */
    gtk_container_add( GTK_CONTAINER( tasks_frame ), hbox1 );

    /* Add boxes for buttons. */
    gtk_container_add( GTK_CONTAINER( hbox1 ), vbox2 );
    gtk_container_add( GTK_CONTAINER( hbox1 ), vbox3 );
    gtk_container_add( GTK_CONTAINER( vbox2 ), hbox2 );
    gtk_container_add( GTK_CONTAINER( vbox2 ), hbox3 );
    gtk_container_add( GTK_CONTAINER( vbox3 ), hbox4 );
    gtk_container_add( GTK_CONTAINER( vbox3 ), hbox5 );
    gtk_container_add( GTK_CONTAINER( hbox2 ), vbox4 );
    gtk_container_add( GTK_CONTAINER( hbox2 ), vbox5 );

    /* Create radio buttons. */
    button_task_buoy = buttons_make_radio( "Buoy",
        GTK_SIGNAL_FUNC( events_tasks ), tasks_group );
    button_task_gate = buttons_make_radio( "Gate",
        GTK_SIGNAL_FUNC( events_tasks ), tasks_group );
    button_task_pipe = buttons_make_radio( "Pipe",
        GTK_SIGNAL_FUNC( events_tasks ), tasks_group );
    button_task_square = buttons_make_radio( "Square",
		GTK_SIGNAL_FUNC( events_tasks ), tasks_group );
    button_task_none = buttons_make_radio( "None",
		GTK_SIGNAL_FUNC( events_tasks ), tasks_group );
    button_task_fence = buttons_make_radio( "Fence",
		GTK_SIGNAL_FUNC( events_tasks ), tasks_group );
    button_task_boxes = buttons_make_radio( "Boxes",
		GTK_SIGNAL_FUNC( events_tasks ), tasks_group );
    button_task_suitcase = buttons_make_radio( "Suitcase",
		GTK_SIGNAL_FUNC( events_tasks ), tasks_group );
    button_task_course = buttons_make_radio( "Course",
		GTK_SIGNAL_FUNC( events_tasks ), tasks_group );
    button_task_nod = buttons_make_radio( "Nod",
		GTK_SIGNAL_FUNC( events_tasks ), tasks_group );
    button_task_spin = buttons_make_radio( "Spin",
		GTK_SIGNAL_FUNC( events_tasks ), tasks_group );

    /* Create the spin buttons. */
    //adj = ( GtkAdjustment * )gtk_adjustment_new( 0, 0, 100, 0.1, 10, 0 );
    //button_task_square_time1 = buttons_make_spin( "Forward Time [s]",
		//GTK_SIGNAL_FUNC( events_tasks ), vbox5, adj );

    //adj = ( GtkAdjustment * )gtk_adjustment_new( 0, 0, 100, 0.1, 10, 0 );
    //button_task_square_time2 = buttons_make_spin( "Left Time [s]",
		//GTK_SIGNAL_FUNC( events_tasks ), vbox5, adj );

    //adj = ( GtkAdjustment * )gtk_adjustment_new( 0, 0, 100, 0.1, 10, 0 );
    //button_task_square_time3 = buttons_make_spin( "Reverse Time [s]",
		//GTK_SIGNAL_FUNC( events_tasks ), vbox5, adj );

    //adj = ( GtkAdjustment * )gtk_adjustment_new( 0, 0, 100, 0.1, 10, 0 );
    //button_task_square_time4 = buttons_make_spin( "Right Time [s]",
		//GTK_SIGNAL_FUNC( events_tasks ), vbox5, adj );

    /* Pack the radio buttons into boxes. */
    gtk_box_pack_start( GTK_BOX( hbox3 ), button_task_none, FALSE, TRUE, 0 );
    gtk_box_pack_start( GTK_BOX( hbox3 ), button_task_buoy, FALSE, TRUE, 0 );
    gtk_box_pack_start( GTK_BOX( hbox3 ), button_task_pipe, FALSE, TRUE, 0 );
    gtk_box_pack_start( GTK_BOX( hbox3 ), button_task_gate, FALSE, TRUE, 0 );
    gtk_box_pack_start( GTK_BOX( hbox3 ), button_task_fence, FALSE, TRUE, 0 );
    gtk_box_pack_start( GTK_BOX( hbox3 ), button_task_boxes, FALSE, TRUE, 0 );
    gtk_box_pack_start( GTK_BOX( hbox3 ), button_task_suitcase, FALSE, TRUE, 0 );
    gtk_box_pack_start( GTK_BOX( hbox3 ), button_task_course, FALSE, TRUE, 0 );
    gtk_box_pack_start( GTK_BOX( hbox3 ), button_task_nod, FALSE, TRUE, 0 );
    gtk_box_pack_start( GTK_BOX( hbox3 ), button_task_spin, FALSE, TRUE, 0 );

    /* Pack the boxes. */
    gtk_box_pack_start( GTK_BOX( box ), vbox1, TRUE, FALSE, 0 );

    /* Activate specific buttons by default. */
    gtk_toggle_button_set_active( ( GtkToggleButton * )button_task_none, TRUE );

    return TRUE;
} /* end buttons_tasks() */


/******************************************************************************
 *
 * Title:       int buttons_tasks( GtkWidget *box )
 *
 * Description: Sets up a vbox for the UUV Vision buttons to be packed into.
 *
 * Input:       box: The box to pack the UUV Vision buttons into.
 *
 * Output:      TRUE: No error checking implemented.
 *
 * Globals:     client_msg
 *
 *****************************************************************************/

int buttons_vision( GtkWidget *box )
{
    GtkAdjustment *adj;
    GtkWidget *vbox1;
    GtkWidget *vbox2;
    GtkWidget *vbox3;
    GtkWidget *vbox4;
    GtkWidget *vbox5;
    GtkWidget *vbox6;
    GtkWidget *vbox7;
    GtkWidget *vbox8;
    GtkWidget *vbox9;
    GtkWidget *hbox1;
    GtkWidget *hbox2;
    GtkWidget *hbox3;
    GtkWidget *hbox4;
    GtkWidget *hbox5;
    GtkWidget *hbox6;
    GtkWidget *hbox7;
    GtkWidget *hbox8;
    GtkWidget *hbox9;
    GtkWidget *hbox10;
    GtkWidget *hbox11;
    GtkWidget *hbox12;
    GtkWidget *hbox13;
    GtkWidget *pipe_frame;
    GtkWidget *buoy_frame;
    GtkWidget *fence_frame;
    GtkWidget *save_frame;

    /* Create new boxes. */
    vbox1 = gtk_vbox_new( TRUE, 0 );
    vbox2 = gtk_vbox_new( TRUE, 0 );
    vbox3 = gtk_vbox_new( TRUE, 0 );
    vbox4 = gtk_vbox_new( TRUE, 0 );
    vbox5 = gtk_vbox_new( TRUE, 0 );
    vbox6 = gtk_vbox_new( TRUE, 0 );
    vbox7 = gtk_vbox_new( TRUE, 0 );
    vbox8 = gtk_vbox_new( TRUE, 0 );
    vbox9 = gtk_vbox_new( TRUE, 0 );
    hbox1 = gtk_hbox_new( TRUE, 0 );
    hbox2 = gtk_hbox_new( TRUE, 0 );
    hbox3 = gtk_hbox_new( TRUE, 0 );
    hbox4 = gtk_hbox_new( TRUE, 0 );
    hbox5 = gtk_hbox_new( TRUE, 0 );
    hbox6 = gtk_hbox_new( TRUE, 0 );
    hbox7 = gtk_hbox_new( TRUE, 0 );
    hbox8 = gtk_hbox_new( TRUE, 0 );
    hbox9 = gtk_hbox_new( TRUE, 0 );
    hbox10 = gtk_hbox_new( TRUE, 0 );
    hbox11 = gtk_hbox_new( TRUE, 0 );
    hbox12 = gtk_hbox_new( TRUE, 0 );
    hbox13 = gtk_hbox_new( TRUE, 0 );

    /* Create a frame for the task buttons. */
    pipe_frame = buttons_make_frame( "Pipe" );
    buoy_frame = buttons_make_frame( "Buoy" );
    fence_frame = buttons_make_frame( "Fence" );
    save_frame = buttons_make_frame( "Save" );

    /* Add boxes for buttons. */
    gtk_container_add( GTK_CONTAINER( hbox1 ), vbox1 );
    gtk_container_add( GTK_CONTAINER( hbox1 ), vbox2 );
    gtk_container_add( GTK_CONTAINER( vbox1 ), hbox2 );
    gtk_container_add( GTK_CONTAINER( vbox1 ), hbox3 );
    gtk_container_add( GTK_CONTAINER( vbox2 ), hbox4 );
    gtk_container_add( GTK_CONTAINER( vbox2 ), hbox5 );
    gtk_container_add( GTK_CONTAINER( hbox2 ), pipe_frame );
    gtk_container_add( GTK_CONTAINER( hbox3 ), fence_frame );
    gtk_container_add( GTK_CONTAINER( hbox4 ), buoy_frame );
    gtk_container_add( GTK_CONTAINER( hbox5 ), save_frame );
    gtk_container_add( GTK_CONTAINER( pipe_frame ), hbox6 );
    gtk_container_add( GTK_CONTAINER( fence_frame ), hbox7 );
    gtk_container_add( GTK_CONTAINER( buoy_frame ), hbox8 );
    gtk_container_add( GTK_CONTAINER( save_frame ), hbox9 );
    gtk_container_add( GTK_CONTAINER( hbox6 ), vbox3 );
    gtk_container_add( GTK_CONTAINER( hbox6 ), vbox4 );
    gtk_container_add( GTK_CONTAINER( hbox7 ), vbox5 );
    gtk_container_add( GTK_CONTAINER( hbox7 ), vbox6 );
    gtk_container_add( GTK_CONTAINER( hbox8 ), vbox7 );
    gtk_container_add( GTK_CONTAINER( hbox8 ), vbox8 );
    gtk_container_add( GTK_CONTAINER( hbox9 ), vbox9 );
    gtk_container_add( GTK_CONTAINER( vbox9 ), hbox10 );
    gtk_container_add( GTK_CONTAINER( vbox9 ), hbox11 );
    gtk_container_add( GTK_CONTAINER( vbox9 ), hbox12 );
    gtk_container_add( GTK_CONTAINER( vbox9 ), hbox13 );

    /* Create the Save buttons. */
    button_save_bframe = gtk_button_new_with_label( "Bottom Frame" );
    button_save_fframe = gtk_button_new_with_label( "Front Frame" );
    button_save_bvideo = buttons_make_check( "Bottom Video",
		GTK_SIGNAL_FUNC(events_vision) );
    button_save_fvideo = buttons_make_check( "Front Video",
		GTK_SIGNAL_FUNC(events_vision) );

    /* Connect the normal buttons to callbacks. */
    g_signal_connect( button_save_bframe,
		"clicked", G_CALLBACK( events_vision ), NULL );
    g_signal_connect( button_save_fframe,
		"clicked", G_CALLBACK( events_vision ), NULL );

    /* Put normal and check buttons into box. */
    gtk_container_add( GTK_CONTAINER( hbox10 ), button_save_bframe );
    gtk_container_add( GTK_CONTAINER( hbox11 ), button_save_fframe );
    gtk_container_add( GTK_CONTAINER( hbox12 ), button_save_bvideo );
    gtk_container_add( GTK_CONTAINER( hbox13 ), button_save_fvideo );

    /* Create the spin buttons. */
    adj = ( GtkAdjustment * )gtk_adjustment_new( cf.pipe_hL, 0, 255, 1, 10, 0 );
    button_hpipe_lo = buttons_make_spin( "H Low",
		GTK_SIGNAL_FUNC( events_vision ), vbox3, adj );

    adj = ( GtkAdjustment * )gtk_adjustment_new( cf.pipe_sL, 0, 255, 1, 10, 0 );
    button_spipe_lo = buttons_make_spin( "S Low",
		GTK_SIGNAL_FUNC( events_vision ), vbox3, adj );

    adj = ( GtkAdjustment * )gtk_adjustment_new( cf.pipe_vL, 0, 255, 1, 10, 0 );
    button_vpipe_lo = buttons_make_spin( "V Low",
		GTK_SIGNAL_FUNC( events_vision ), vbox3, adj );

    adj = ( GtkAdjustment * )gtk_adjustment_new( cf.pipe_hH, 0, 255, 1, 10, 0 );
    button_hpipe_hi = buttons_make_spin( "H High",
		GTK_SIGNAL_FUNC( events_vision ), vbox4, adj );

    adj = ( GtkAdjustment * )gtk_adjustment_new( cf.pipe_sH, 0, 255, 1, 10, 0 );
    button_spipe_hi = buttons_make_spin( "S High",
		GTK_SIGNAL_FUNC( events_vision ), vbox4, adj );

    adj = ( GtkAdjustment * )gtk_adjustment_new( cf.pipe_vH, 0, 255, 1, 10, 0 );
    button_vpipe_hi = buttons_make_spin( "V High",
		GTK_SIGNAL_FUNC( events_vision ), vbox4, adj );

    adj = ( GtkAdjustment * )gtk_adjustment_new( cf.buoy_hL, 0, 255, 1, 10, 0 );
    button_hbuoy_lo = buttons_make_spin( "H Low",
		GTK_SIGNAL_FUNC( events_vision ), vbox7, adj );

    adj = ( GtkAdjustment * )gtk_adjustment_new( cf.buoy_sL, 0, 255, 1, 10, 0 );
    button_sbuoy_lo = buttons_make_spin( "S Low",
		GTK_SIGNAL_FUNC( events_vision ), vbox7, adj );

    adj = ( GtkAdjustment * )gtk_adjustment_new( cf.buoy_vL, 0, 255, 1, 10, 0 );
    button_vbuoy_lo = buttons_make_spin( "V Low",
		GTK_SIGNAL_FUNC( events_vision ), vbox7, adj );

    adj = ( GtkAdjustment * )gtk_adjustment_new( cf.buoy_hH, 0, 255, 1, 10, 0 );
    button_hbuoy_hi = buttons_make_spin( "H High",
		GTK_SIGNAL_FUNC( events_vision ), vbox8, adj );

    adj = ( GtkAdjustment * )gtk_adjustment_new( cf.buoy_sH, 0, 255, 1, 10, 0 );
    button_sbuoy_hi = buttons_make_spin( "S High",
		GTK_SIGNAL_FUNC( events_vision ), vbox8, adj );

    adj = ( GtkAdjustment * )gtk_adjustment_new( cf.buoy_vH, 0, 255, 1, 10, 0 );
    button_vbuoy_hi = buttons_make_spin( "V High",
		GTK_SIGNAL_FUNC( events_vision ), vbox8, adj );

    adj = ( GtkAdjustment * )gtk_adjustment_new( cf.fence_hL, 0, 255, 1, 10, 0 );
    button_hfence_lo = buttons_make_spin( "H Low",
		GTK_SIGNAL_FUNC( events_vision ), vbox5, adj );

    adj = ( GtkAdjustment * )gtk_adjustment_new( cf.fence_sL, 0, 255, 1, 10, 0 );
    button_sfence_lo = buttons_make_spin( "S Low",
		GTK_SIGNAL_FUNC( events_vision ), vbox5, adj );

    adj = ( GtkAdjustment * )gtk_adjustment_new( cf.fence_vL, 0, 255, 1, 10, 0 );
    button_vfence_lo = buttons_make_spin( "V Low",
		GTK_SIGNAL_FUNC( events_vision ), vbox5, adj );

    adj = ( GtkAdjustment * )gtk_adjustment_new( cf.fence_hH, 0, 255, 1, 10, 0 );
    button_hfence_hi = buttons_make_spin( "H High",
		GTK_SIGNAL_FUNC( events_vision ), vbox6, adj );

    adj = ( GtkAdjustment * )gtk_adjustment_new( cf.fence_sH, 0, 255, 1, 10, 0 );
    button_sfence_hi = buttons_make_spin( "S High",
		GTK_SIGNAL_FUNC( events_vision ), vbox6, adj );

    adj = ( GtkAdjustment * )gtk_adjustment_new( cf.fence_vH, 0, 255, 1, 10, 0 );
    button_vfence_hi = buttons_make_spin( "V High",
		GTK_SIGNAL_FUNC( events_vision ), vbox6, adj );

    /* Pack the boxes. */
    gtk_box_pack_start( GTK_BOX( box ), hbox1, TRUE, FALSE, 0 );

    return TRUE;
} /* end buttons_vision() */


/******************************************************************************
 *
 * Title:       int buttons_targets( GtkWidget * box)
 *
 * Description: Sets up an vbox for the Targets to be packed into. Also
 *              sets minimum, maximum and default values for the variables.
 *
 * Input:       box: The box to pack the Target buttons into.
 *
 * Output:      TRUE: No error checking implemented.
 *
 * Globals:     client_msg, spin_frame
 *
 *****************************************************************************/

int buttons_targets( GtkWidget *box )
{
    GtkWidget *targets_frame;
    GtkAdjustment *adj;
    GtkWidget *hbox;
    GtkWidget *vbox;
	GtkWidget *hbox1;

    /* Create new boxes. */
    hbox = gtk_hbox_new( TRUE, 0 );
    vbox = gtk_vbox_new( TRUE, 0 );
    hbox1 = gtk_hbox_new( TRUE, 0 );

    /* Create frame. */
    targets_frame = buttons_make_frame( "Targets" );

    /* Add the frames to boxes. */
    gtk_container_add( GTK_CONTAINER( vbox ), targets_frame );

    /* Add boxes to the frames. */
    gtk_container_add( GTK_CONTAINER( targets_frame ), hbox );
	gtk_container_add( GTK_CONTAINER( vbox ), hbox1 );

    /* Create the spin buttons. */
    adj = ( GtkAdjustment * )gtk_adjustment_new( cf.target_pitch, 0, 360, 0.1, 10, 0 );
    button_target_pitch = buttons_make_spin( "Pitch",
		GTK_SIGNAL_FUNC( events_target_pitch ), hbox, adj );
    gtk_spin_button_set_wrap( (GtkSpinButton *)button_target_pitch, TRUE );

    adj = ( GtkAdjustment * )gtk_adjustment_new( cf.target_roll, 0, 360, 0.1, 10, 0 );
    button_target_roll = buttons_make_spin( "Roll",
		GTK_SIGNAL_FUNC( events_target_roll ), hbox, adj );
    gtk_spin_button_set_wrap( ( GtkSpinButton * )button_target_roll, TRUE );

    adj = ( GtkAdjustment * )gtk_adjustment_new( cf.target_yaw, 0, 360, 0.1, 10, 0 );
    button_target_yaw = buttons_make_spin( "Yaw",
		GTK_SIGNAL_FUNC( events_target_yaw ), hbox, adj );
    gtk_spin_button_set_wrap( ( GtkSpinButton * )button_target_yaw, TRUE );

    adj = ( GtkAdjustment * )gtk_adjustment_new( cf.target_depth, -100, 100, 0.001, 10, 0 );
    button_target_depth = buttons_make_spin( "Depth",
		GTK_SIGNAL_FUNC( events_target_depth ), hbox, adj );
    gtk_spin_button_set_wrap( ( GtkSpinButton * )button_target_depth, TRUE );

    adj = ( GtkAdjustment * )gtk_adjustment_new( msg.target.data.fx, -100, 100, 1, 10, 0 );
    button_target_fx = buttons_make_spin( "Fx",
		GTK_SIGNAL_FUNC( events_target_fx ), hbox, adj );

    adj = ( GtkAdjustment * )gtk_adjustment_new( msg.target.data.fy, -100, 100, 1, 10, 0 );
    button_target_fy = buttons_make_spin( "Fy",
		GTK_SIGNAL_FUNC( events_target_fy ), hbox, adj );

    adj = ( GtkAdjustment * )gtk_adjustment_new( msg.target.data.speed, 0, 100, 1, 10, 0 );
    button_target_speed = buttons_make_spin( "Speed",
		GTK_SIGNAL_FUNC( events_target_speed ), hbox, adj );
	
	/* Create normal buttons. */
    button_target_current = gtk_button_new_with_label( "Set Targets to Current Status" );
    button_zero_pid = gtk_button_new_with_label( "Zero PID Errors" );

    /* Connect the normal buttons to callbacks. */
    g_signal_connect( button_target_current, "clicked",
            G_CALLBACK( events_target_current ), NULL );
    g_signal_connect( button_zero_pid, "clicked",
            G_CALLBACK( events_zero_pid ), NULL );

    /* Pack the boxes. */
    gtk_box_pack_start( GTK_BOX( box ), vbox, TRUE, FALSE, 0 );
    gtk_container_add( GTK_CONTAINER( hbox1 ), button_target_current );
    gtk_container_add( GTK_CONTAINER( hbox1 ), button_zero_pid );

    return TRUE;
} /* end buttons_targets() */


/******************************************************************************
 *
 * Title:       int buttons_options( GtkWidget *box )
 *
 * Description: Sets up an hbox for the UUV Options to be packed into.
 *
 * Input:       box: The box to pack the UUV Options into.
 *
 * Output:      TRUE: No error checking implemented.
 *
 * Globals:     client_msg, check_frame
 *
 *****************************************************************************/

int buttons_options( GtkWidget *box )
{
    GtkWidget *button_enable_servos;
    GtkWidget *button_enable_imu;
    GtkWidget *button_enable_log;
    GtkWidget *button_imu_stab;
    GtkWidget *button_debug_level;
    GtkWidget *button_dropper;
	GtkWidget *hbox1;
    GtkWidget *vbox1;
    GtkWidget *vbox2;
	GtkWidget *vbox3;
	GtkWidget *vbox4;
    GtkWidget *options_frame;
    GtkAdjustment *adj;

    /* Initialize values. */
    msg.client.data.enable_servos = FALSE;
    msg.client.data.enable_imu = FALSE;
    msg.client.data.enable_log = FALSE;
    msg.client.data.imu_stab = TRUE;
    msg.client.data.debug_level = 5;

    /* Create new boxes. */
    hbox1 = gtk_hbox_new( TRUE, 0 );
    vbox1 = gtk_vbox_new( TRUE, 0 );
    vbox2 = gtk_vbox_new( TRUE, 0 );
    vbox3 = gtk_vbox_new( TRUE, 0 );
    vbox4 = gtk_vbox_new( TRUE, 0 );

    /* Create a frame for the check buttons. */
    options_frame = buttons_make_frame( "UUV Options" );
	
	/* Add a box to the main box. */
    gtk_container_add( GTK_CONTAINER( box ), hbox1 );
    gtk_container_add( GTK_CONTAINER( hbox1 ), vbox1 );

    /* Add the frames to a box. */
    gtk_container_add( GTK_CONTAINER( vbox1 ), options_frame );

    /* Add boxes to the frames. */
    gtk_container_add( GTK_CONTAINER( options_frame ), vbox3 );

    /* Create the enable check buttons. */
    button_enable_servos = buttons_make_check( "Enable servos",
		GTK_SIGNAL_FUNC( events_enable_servos ) );
    button_enable_imu = buttons_make_check( "Enable IMU",
		GTK_SIGNAL_FUNC( events_enable_imu ) );
    button_enable_log = buttons_make_check( "Enable log",
		GTK_SIGNAL_FUNC( events_enable_log ) );
    button_imu_stab = buttons_make_check( "IMU Stabilization",
		GTK_SIGNAL_FUNC( events_imu_stab ) );
	
    /* Pack the buttons into frames. */
    gtk_box_pack_start( GTK_BOX( vbox3 ), button_enable_servos, TRUE, TRUE, 0 );
    gtk_box_pack_start( GTK_BOX( vbox3 ), button_enable_imu, TRUE, TRUE, 0 );
    gtk_box_pack_start( GTK_BOX( vbox3 ), button_enable_log, TRUE, TRUE, 0 );
    gtk_box_pack_start( GTK_BOX( vbox3 ), button_imu_stab, TRUE, TRUE, 0 );

    /* Create the spin buttons. */
    adj = ( GtkAdjustment * )gtk_adjustment_new( msg.client.data.debug_level,
		0, 20, 1, 0, 0 );
    button_debug_level = buttons_make_spin( "Debug Level",
		GTK_SIGNAL_FUNC( events_debug_level ), vbox3, adj );

    adj = ( GtkAdjustment * )gtk_adjustment_new( msg.client.data.dropper,
		0, 253, 1, 0, 0 );
    button_dropper = buttons_make_spin( "Dropper",
		GTK_SIGNAL_FUNC ( events_dropper ), vbox3, adj );

    /* Activate specific buttons by default. */
    gtk_toggle_button_set_active( (GtkToggleButton *)button_imu_stab, TRUE );

    return TRUE;
} /* end buttons_options() */


/******************************************************************************
 *
 * Title:       int buttons_images( GtkWidget *box )
 *
 * Description: Sets up a box for the Images buttons to be packed into.
 *
 * Input:       box: The box to pack the Images buttons into.
 *
 * Output:      TRUE: No error checking implemented.
 *
 *****************************************************************************/

int buttons_images( GtkWidget *box )
{
	GtkWidget *hbox1;
	GtkWidget *hbox2;
	GtkWidget *image_frame;
	GtkWidget *image_group;

    /* Initialize values. */
    msg.vision.data.mode = 0;

    /* Create new boxes. */
    hbox1 = gtk_hbox_new( TRUE, 0 );
    hbox2 = gtk_hbox_new( TRUE, 0 );

    /* Create image radio group. */
    image_group = buttons_make_radio_group( );

    /* Create a frame for the check buttons. */
    image_frame = buttons_make_frame( "Image Display Options" );
	
	/* Add a box to the main box. */
    gtk_container_add( GTK_CONTAINER( box ), hbox1 );

    /* Add the frames to a box. */
    gtk_container_add( GTK_CONTAINER( hbox1 ), image_frame );

    /* Add boxes to the frames. */
    gtk_container_add( GTK_CONTAINER( image_frame ), hbox2 );

	/* Create the image radio buttons. */
    button_image_fcolor = buttons_make_radio( "Front Color",
		GTK_SIGNAL_FUNC( events_images ), image_group );
    button_image_fbinary = buttons_make_radio( "Front Binary",
		GTK_SIGNAL_FUNC( events_images ), image_group );
    button_image_bcolor = buttons_make_radio( "Bottom Color",
		GTK_SIGNAL_FUNC( events_images ), image_group );
    button_image_bbinary = buttons_make_radio( "Bottom Binary",
		GTK_SIGNAL_FUNC( events_images ), image_group );
    button_image_none = buttons_make_radio( "None",
		GTK_SIGNAL_FUNC( events_images ), image_group );

    /* Pack the buttons into frames. */
    gtk_box_pack_start( GTK_BOX( hbox2 ), button_image_fcolor, TRUE, TRUE, 0 );
    gtk_box_pack_start( GTK_BOX( hbox2 ), button_image_fbinary, TRUE, TRUE, 0 );
    gtk_box_pack_start( GTK_BOX( hbox2 ), button_image_bcolor, TRUE, TRUE, 0 );
    gtk_box_pack_start( GTK_BOX( hbox2 ), button_image_bbinary, TRUE, TRUE, 0 );
    gtk_box_pack_start( GTK_BOX( hbox2 ), button_image_none, TRUE, TRUE, 0 );

    /* Activate specific buttons by default. */
    gtk_toggle_button_set_active( (GtkToggleButton *)button_image_none, TRUE );

    return TRUE;
} /* end buttons_images() */


/******************************************************************************
 *
 * Title:       int buttons_make_status( GtkWidget *box )
 *
 * Description: Sets up an hbox for the status to be packed into.
 *
 * Input:       box: The box to pack the status label into.
 *
 * Output:      TRUE: No error checking implemented.
 *
 * Globals:     label_frame
 *
 *****************************************************************************/

int buttons_make_status( GtkWidget *box )
{
    GtkWidget *vbox;
    GtkWidget *label_frame;

    /* Create new boxes. */
    vbox = gtk_vbox_new( TRUE, 0 );

    /* Create a frame for the status. */
    label_frame = buttons_make_frame( "Current Status" );

    /* Add the frame to a box. */
    gtk_container_add( GTK_CONTAINER( box ), label_frame );

    /* Add boxes to the frames. */
    gtk_container_add( GTK_CONTAINER( label_frame ), vbox );

    /* Create the label. */
    label_status = gtk_label_new_with_mnemonic( "Current Status:\n"
		"NOT CONNECTED.\n\n" );

    /* Pack label into box. */
    gtk_box_pack_start( GTK_BOX( vbox ), label_status, FALSE, TRUE, 0 );

    return TRUE;
} /* end buttons_make_status() */


/******************************************************************************
 *
 * Title:       int buttons_gains( GtkWidget *box )
 *
 * Description: Sets up an vbox for the Gains to be packed into. Also
 *              sets minimum, maximum and default values for the variables.
 *
 * Input:       box: The box to pack the Gain buttons into.
 *
 * Output:      TRUE: No error checking implemented.
 *
 * Globals:     None.
 *
 *****************************************************************************/

int buttons_gains( GtkWidget *box )
{
    GtkWidget *yaw_frame;
    GtkWidget *pitch_frame;
    GtkWidget *roll_frame;
    GtkWidget *depth_frame;
    GtkWidget *fx_frame;
    GtkWidget *fy_frame;
    GtkWidget *coupling_frame;
    GtkWidget *getset_frame;
    GtkAdjustment *adj;
    GtkWidget *hbox;
    GtkWidget *vbox1;
    GtkWidget *vbox2;
    GtkWidget *vbox3;
    GtkWidget *vbox4;
    GtkWidget *vbox5;
    GtkWidget *vbox6;
    GtkWidget *vbox13;
    GtkWidget *vbox14;
    GtkWidget *vbox15;
    GtkWidget *vbox16;
    GtkWidget *vbox17;
    GtkWidget *vbox18;
    GtkWidget *vbox19;
    GtkWidget *vbox20;
    GtkWidget *vbox21;
    GtkWidget *vbox22;

    /* Create new boxes. */
    hbox   = gtk_hbox_new( FALSE, 0 );
    vbox1  = gtk_vbox_new( FALSE, 0 );
    vbox2  = gtk_vbox_new( FALSE, 0 );
    vbox3  = gtk_vbox_new( FALSE, 0 );
    vbox4  = gtk_vbox_new( TRUE, 0 );
    vbox5  = gtk_vbox_new( TRUE, 0 );
    vbox6  = gtk_vbox_new( TRUE, 0 );
    vbox13 = gtk_vbox_new( FALSE, 0 );
    vbox14 = gtk_vbox_new( TRUE, 0 );
    vbox15 = gtk_vbox_new( FALSE, 0 );
    vbox16 = gtk_vbox_new( TRUE, 0 );
    vbox17 = gtk_vbox_new( TRUE, 0 );
    vbox18 = gtk_vbox_new( TRUE, 0 );
    vbox19 = gtk_vbox_new( TRUE, 0 );
    vbox20 = gtk_vbox_new( TRUE, 0 );
    vbox21 = gtk_vbox_new( TRUE, 0 );
    vbox22 = gtk_vbox_new( TRUE, 0 );

    /* Create frames. */
    yaw_frame      = buttons_make_frame( "Yaw" );
    pitch_frame    = buttons_make_frame( "Pitch" );
    roll_frame     = buttons_make_frame( "Roll" );
    depth_frame    = buttons_make_frame( "Depth" );
    fx_frame       = buttons_make_frame( "Fx" );
    fy_frame       = buttons_make_frame( "Fy" );
    coupling_frame = buttons_make_frame( "Coupling" );
    getset_frame = buttons_make_frame( "Get/Set" );

    /* Add the frames to boxes. */
    gtk_container_add( GTK_CONTAINER( vbox1 ),  yaw_frame );
    gtk_container_add( GTK_CONTAINER( vbox2 ),  pitch_frame );
    gtk_container_add( GTK_CONTAINER( vbox3 ),  roll_frame );
    gtk_container_add( GTK_CONTAINER( vbox13 ), depth_frame );
    gtk_container_add( GTK_CONTAINER( vbox17 ), fx_frame );
    gtk_container_add( GTK_CONTAINER( vbox18 ), fy_frame );
    gtk_container_add( GTK_CONTAINER( vbox21 ), coupling_frame );
    gtk_container_add( GTK_CONTAINER( vbox15 ), getset_frame );

    /* Add boxes to the frames. */
    gtk_container_add( GTK_CONTAINER( yaw_frame ),      vbox4 );
    gtk_container_add( GTK_CONTAINER( pitch_frame ),    vbox5 );
    gtk_container_add( GTK_CONTAINER( roll_frame ),     vbox6 );
    gtk_container_add( GTK_CONTAINER( depth_frame ),    vbox14 );
    gtk_container_add( GTK_CONTAINER( fx_frame ),       vbox19 );
    gtk_container_add( GTK_CONTAINER( fy_frame ),       vbox20 );
    gtk_container_add( GTK_CONTAINER( coupling_frame ), vbox22 );
    gtk_container_add( GTK_CONTAINER( getset_frame ),   vbox16 );

    /* Create normal buttons. */
	button_cf_gains = gtk_button_new_with_label( "Use Config File Gains" );
	button_zero_gains = gtk_button_new_with_label( "Zero Gains" );

    /* Connect the normal buttons to callbacks. */
    g_signal_connect( button_cf_gains, "clicked",
            G_CALLBACK( events_gain_cf ), NULL );
    g_signal_connect( button_zero_gains, "clicked",
            G_CALLBACK( events_gain_zero ), NULL );

    /* Put normal buttons into box. */
    gtk_container_add( GTK_CONTAINER( vbox16 ), button_cf_gains );
    gtk_container_add( GTK_CONTAINER( vbox16 ), button_zero_gains );

    /* Create the spin buttons. */
    adj = ( GtkAdjustment * )gtk_adjustment_new( msg.gain.data.kp_yaw,
		-10000.0, 10000.0, BUTTON_GAIN_INCR, 1.0, 0 );
    button_kp_yaw = buttons_make_spin( "Kp",
		GTK_SIGNAL_FUNC( events_gain ), vbox4, adj );

    adj = ( GtkAdjustment * )gtk_adjustment_new( msg.gain.data.ki_yaw,
		-10000.0, 10000.0, BUTTON_GAIN_INCR, 1.0, 0 );
    button_ki_yaw = buttons_make_spin( "Ki",
		GTK_SIGNAL_FUNC( events_gain ), vbox4, adj );

    adj = ( GtkAdjustment * )gtk_adjustment_new( msg.gain.data.kd_yaw,
		-10000.0, 10000.0, BUTTON_GAIN_INCR, 1.0, 0 );
    button_kd_yaw = buttons_make_spin( "Kd",
		GTK_SIGNAL_FUNC( events_gain ), vbox4, adj );

    adj = ( GtkAdjustment * )gtk_adjustment_new( msg.gain.data.kp_pitch,
		-10000.0, 10000.0, BUTTON_GAIN_INCR, 1.0, 0 );
    button_kp_pitch = buttons_make_spin( "Kp",
		GTK_SIGNAL_FUNC( events_gain ), vbox5, adj );

    adj = ( GtkAdjustment * )gtk_adjustment_new( msg.gain.data.ki_pitch,
		-10000.0, 10000.0, BUTTON_GAIN_INCR, 1.0, 0 );
    button_ki_pitch = buttons_make_spin( "Ki",
		GTK_SIGNAL_FUNC( events_gain ), vbox5, adj );

    adj = ( GtkAdjustment * )gtk_adjustment_new( msg.gain.data.kd_pitch,
		-10000.0, 10000.0, BUTTON_GAIN_INCR, 1.0, 0 );
    button_kd_pitch = buttons_make_spin( "Kd",
		GTK_SIGNAL_FUNC( events_gain ), vbox5, adj );

    adj = ( GtkAdjustment * )gtk_adjustment_new( msg.gain.data.kp_roll,
		-10000.0, 10000.0, BUTTON_GAIN_INCR, 1.0, 0 );
    button_kp_roll = buttons_make_spin( "Kp",
		GTK_SIGNAL_FUNC( events_gain ), vbox6, adj );

    adj = ( GtkAdjustment * )gtk_adjustment_new( msg.gain.data.ki_roll,
		-10000.0, 10000.0, BUTTON_GAIN_INCR, 1.0, 0 );
    button_ki_roll = buttons_make_spin( "Ki",
		GTK_SIGNAL_FUNC( events_gain ), vbox6, adj );

    adj = ( GtkAdjustment * )gtk_adjustment_new( msg.gain.data.kd_roll,
		-10000.0, 10000.0, BUTTON_GAIN_INCR, 1.0, 0 );
    button_kd_roll = buttons_make_spin( "Kd",
		GTK_SIGNAL_FUNC( events_gain ), vbox6, adj );

    adj = ( GtkAdjustment * )gtk_adjustment_new( msg.gain.data.kp_depth,
		-10000.0, 10000.0, BUTTON_GAIN_INCR, 1.0, 0 );
    button_kp_depth = buttons_make_spin( "Kp",
		GTK_SIGNAL_FUNC( events_gain ), vbox14, adj );

    adj = ( GtkAdjustment * )gtk_adjustment_new( msg.gain.data.ki_depth,
		-10000.0, 10000.0, BUTTON_GAIN_INCR, 1.0, 0 );
    button_ki_depth = buttons_make_spin( "Ki",
       GTK_SIGNAL_FUNC( events_gain ), vbox14, adj );

    adj = ( GtkAdjustment * )gtk_adjustment_new( msg.gain.data.kd_depth,
	   -10000.0, 10000.0, BUTTON_GAIN_INCR, 1.0, 0 );
    button_kd_depth = buttons_make_spin( "Kd",
	   GTK_SIGNAL_FUNC( events_gain ), vbox14, adj );

    adj = ( GtkAdjustment * )gtk_adjustment_new( msg.gain.data.kp_fx,
		-10000.0, 10000.0, BUTTON_GAIN_INCR, 1.0, 0 );
    button_kp_fx = buttons_make_spin( "Kp",
		GTK_SIGNAL_FUNC( events_gain ), vbox19, adj );

    adj = ( GtkAdjustment * )gtk_adjustment_new( msg.gain.data.ki_fx,
		-10000.0, 10000.0, BUTTON_GAIN_INCR, 1.0, 0 );
    button_ki_fx = buttons_make_spin( "Ki",
       GTK_SIGNAL_FUNC( events_gain ), vbox19, adj );

    adj = ( GtkAdjustment * )gtk_adjustment_new( msg.gain.data.kd_fx,
	   -10000.0, 10000.0, BUTTON_GAIN_INCR, 1.0, 0 );
    button_kd_fx = buttons_make_spin( "Kd",
	   GTK_SIGNAL_FUNC( events_gain ), vbox19, adj );

    adj = ( GtkAdjustment * )gtk_adjustment_new( msg.gain.data.kp_fy,
		-10000.0, 10000.0, BUTTON_GAIN_INCR, 1.0, 0 );
    button_kp_fy = buttons_make_spin( "Kp",
		GTK_SIGNAL_FUNC( events_gain ), vbox20, adj );

    adj = ( GtkAdjustment * )gtk_adjustment_new( msg.gain.data.ki_fy,
		-10000.0, 10000.0, BUTTON_GAIN_INCR, 1.0, 0 );
    button_ki_fy = buttons_make_spin( "Ki",
       GTK_SIGNAL_FUNC( events_gain ), vbox20, adj );

    adj = ( GtkAdjustment * )gtk_adjustment_new( msg.gain.data.kd_fy,
	   -10000.0, 10000.0, BUTTON_GAIN_INCR, 1.0, 0 );
    button_kd_fy = buttons_make_spin( "Kd",
	   GTK_SIGNAL_FUNC( events_gain ), vbox20, adj );
	   
	adj = ( GtkAdjustment * )gtk_adjustment_new( msg.gain.data.kp_roll_lateral,
		-10000.0, 10000.0, BUTTON_GAIN_INCR, 1.0, 0 );
    button_kp_roll_lateral = buttons_make_spin( "Kp - Roll/Fx",
		GTK_SIGNAL_FUNC( events_gain ), vbox22, adj );

    adj = ( GtkAdjustment * )gtk_adjustment_new( msg.gain.data.kp_depth_forward,
		-10000.0, 10000.0, BUTTON_GAIN_INCR, 1.0, 0 );
    button_kp_depth_forward = buttons_make_spin( "Kp - Depth/Fy",
       GTK_SIGNAL_FUNC( events_gain ), vbox22, adj );

    adj = ( GtkAdjustment * )gtk_adjustment_new( msg.gain.data.kp_place_holder,
	   -10000.0, 10000.0, BUTTON_GAIN_INCR, 1.0, 0 );
    button_kp_place_holder = buttons_make_spin( "Unused",
	   GTK_SIGNAL_FUNC( events_gain ), vbox22, adj );

    /* Pack the boxes. */
    gtk_box_pack_start( GTK_BOX( box ), hbox, FALSE, TRUE, 0 );
    gtk_box_pack_start( GTK_BOX( hbox ), vbox2, FALSE, TRUE, 0 );
    gtk_box_pack_start( GTK_BOX( hbox ), vbox3, FALSE, TRUE, 0 );
    gtk_box_pack_start( GTK_BOX( hbox ), vbox1, FALSE, TRUE, 0 );
    gtk_box_pack_start( GTK_BOX( hbox ), vbox13, FALSE, TRUE, 0 );
    gtk_box_pack_start( GTK_BOX( hbox ), vbox17, FALSE, TRUE, 0 );
    gtk_box_pack_start( GTK_BOX( hbox ), vbox18, FALSE, TRUE, 0 );
    gtk_box_pack_start( GTK_BOX( hbox ), vbox21, FALSE, TRUE, 0 );
    gtk_box_pack_start( GTK_BOX( hbox ), vbox15, FALSE, TRUE, 0 );

    return TRUE;
} /* end buttons_gains() */
