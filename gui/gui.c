/******************************************************************************
 *
 *  Title:        gui.c
 *
 *  Description:  Sets up GTK+-2.0 GUI interface for the UUV client.
 *
 *****************************************************************************/


#include <sys/timeb.h>
#include <string.h>
#include <gtk/gtk.h>
#include <unistd.h>

#include "gui.h"
#include "buttons.h"
#include "network.h"
#include "messages.h"
#include "parser.h"


/* Handlers for timers. */
//guint timer50ms;
//guint timer100ms;
guint timer200ms;
//guint timer500ms;
//guint timer1s;

/* Status label. */
extern GtkWidget *label_status;

/* A buffer to store network data. */
char planner_buf[MAX_MSG_SIZE];
char nav_buf[MAX_MSG_SIZE];

/* Network API messages. */
extern int planner_fd;
extern int nav_fd;
extern MSG_DATA msg;

/* Configuration file variables. */
extern CONF_VARS cf;


/******************************************************************************
 *
 * Title:       void gui_update_status_text( )
 *
 * Description: Updates the UUV status shown to the user on the GUI.
 *
 * Input:       None.
 *
 * Output:      None.
 *
 * Globals:     msg, label_status
 *
 *****************************************************************************/

void gui_update_status_text( )
{
    char sbuff[2048];

    sprintf( sbuff,
             "Pitch, Roll, Yaw, Depth:\t[ %.3f\t%.3f\t%.3f\t%.3f ]\n"
             "Accel X, Y, Z:\t\t\t[ %.3f\t%.3f\t%.3f ]\n"
             "Ang Rate X, Y, Z:\t\t[ %.3f\t%.3f\t%.3f ]\n"
             "Labjack (B1 B2 D W):\t[ %.3fV\t%.3fV\t%.3f\t%.3f ]\n"
             "PID Errors:\n"
             "Pitch:\t\t\t\t[ %.3f\t%.3f\t%.3f ]\n"
             "Roll:\t\t\t\t\t[ %.3f\t%.3f\t%.3f ]\n"
             "Yaw:\t\t\t\t\t[ %.3f\t%.3f\t%.3f ]\n"
             "Depth:\t\t\t\t[ %.3f\t%.3f\t%.3f ]\n"
             , msg.status.data.pitch
             , msg.status.data.roll
             , msg.status.data.yaw
             , msg.status.data.depth
             , msg.status.data.accel[0]
             , msg.status.data.accel[1]
             , msg.status.data.accel[2]
             , msg.status.data.ang_rate[0]
             , msg.status.data.ang_rate[1]
             , msg.status.data.ang_rate[2]
             , msg.lj.data.battery1
             , msg.lj.data.battery2
             , msg.lj.data.pressure
             , msg.lj.data.water
             , msg.status.data.pitch_perr
             , msg.status.data.pitch_ierr
             , msg.status.data.pitch_derr
             , msg.status.data.roll_perr
             , msg.status.data.roll_ierr
             , msg.status.data.roll_derr
             , msg.status.data.yaw_perr
             , msg.status.data.yaw_ierr
             , msg.status.data.yaw_derr
             , msg.status.data.depth_perr
             , msg.status.data.depth_ierr
             , msg.status.data.depth_derr
           );
    //printf( "GUI_UPDATE_STATUS_TEXT:\n%s\n", sbuff );

    gtk_label_set_text( GTK_LABEL( label_status ), sbuff );
} /* end gui_update_status_text() */


/******************************************************************************
 *
 * Title:       gint gui_timer_1s( gpointer data )
 *
 * Description: Creates a callback which is called at a 1 s period.
 *
 * Input:       data: A pointer to data that can be manipulated.
 *
 * Output:      TRUE: No error checking implemented.
 *
 * Globals:     None.
 *
 *****************************************************************************/

gint gui_timer_1s( gpointer data )
{

    return TRUE; /* continue timer */
} /* end gui_timer_1s() */


/******************************************************************************
 *
 * Title:       gint gui_timer_200ms( gpointer data )
 *
 * Description: Creates a callback which is called at a 200 ms period.
 *
 * Input:       data: A pointer to data that can be manipulated.
 *
 * Output:      TRUE: No error checking implemented.
 *
 * Globals:     planner_fd
 *
 *****************************************************************************/

gint gui_timer_200ms( gpointer data )
{
    buttons_update_values( );
    static int bytes_left;
	int recv_bytes;
	
    /* Get network data from planner. */
    if ( planner_fd > 0 ) {
        recv_bytes = net_client( planner_fd, planner_buf, &msg, MODE_OPEN );
		if ( recv_bytes > 0 ) {
        	bytes_left = messages_decode( planner_fd, planner_buf, &msg, recv_bytes );
    	}
    }

    /* Get network data from nav. */
    if ( nav_fd > 0 ) {
        recv_bytes = net_client( nav_fd, nav_buf, &msg, MODE_OPEN );
		if ( recv_bytes > 0 ) {
        	bytes_left = messages_decode( nav_fd, nav_buf, &msg, recv_bytes );
    	}
    }

    /* Update GUI status. */
    if ( planner_fd > 0 ) {
	    gui_update_status_text( );
	}
    if ( nav_fd > 0 ) {
	    gui_update_status_text( );
	}

    return TRUE; /* continue timer */
} /* end gui_timer_200ms() */


/******************************************************************************
 *
 * Title:       gint gui_timer_500ms( gpointer data )
 *
 * Description: Creates a callback which is called at a 500 ms period.
 *
 * Input:       data: A pointer to data that can be manipulated.
 *
 * Output:      TRUE: No error checking implemented.
 *
 * Globals:     nav_fd
 *
 *****************************************************************************/

gint gui_timer_500ms( gpointer data )
{

    return TRUE; /* continue timer */
} /* end gui_timer_500ms() */


/******************************************************************************
 *
 * Title:       gint gui_timer_100ms( gpointer data )
 *
 * Description: Creates a callback which is called at a 100 ms period.
 *
 * Input:       data: A pointer to data that can be manipulated.
 *
 * Output:      TRUE: No error checking implemented.
 *
 * Globals:     None.
 *
 *****************************************************************************/

gint gui_timer_100ms( gpointer data )
{

    return TRUE; /* continue timer */
} /* end gui_timer_100ms() */


/******************************************************************************
 *
 * Title:       gint gui_timer_50ms( gpointer data )
 *
 * Description: Creates a callback which is called at a 50 ms period.
 *
 * Input:       data: A pointer to data that can be manipulated.
 *
 * Output:      TRUE: No error checking implemented.
 *
 * Globals:     None.
 *
 *****************************************************************************/

gint gui_timer_50ms( gpointer data )
{

    return TRUE; /* continue timer */
} /* end gui_timer_50ms() */


/******************************************************************************
 *
 * Title:       void gui_set_timers( )
 *
 * Description: Creates all the gtk timers. Sets callback functions for the
 *              timers.
 *
 * Input:       None.
 *
 * Output:      None.
 *
 * Globals:     timer100ms, timer1s, timer5s
 *
 *****************************************************************************/

void gui_set_timers( )
{
    //timer50ms = gtk_timeout_add( 50, gui_timer_50ms, NULL );
    //timer100ms = gtk_timeout_add( 100, gui_timer_100ms, NULL );
    timer200ms = gtk_timeout_add( 200, gui_timer_200ms, NULL );
    //timer500ms = gtk_timeout_add( 500, gui_timer_500ms, NULL );
    //timer1s = gtk_timeout_add( 1000, gui_timer_1s, NULL );
} /* end gui_set_timers() */


/******************************************************************************
 *
 * Title:       void gui_kill_timers( )
 *
 * Description: Stops all the gtk timers.
 *
 * Input:       None.
 *
 * Output:      None.
 *
 * Globals:     timer100ms, timer200ms, timer500ms, timer1s
 *
 *****************************************************************************/

void gui_kill_timers( )
{
    //gtk_timeout_remove( timer50ms );
    //gtk_timeout_remove( timer100ms );
    gtk_timeout_remove( timer200ms );
    //gtk_timeout_remove( timer500ms );
    //gtk_timeout_remove( timer1s );

    //timer50ms = 0;
    //timer100ms = 0;
    timer200ms = 0;
    //timer500ms = 0;
    //timer1s = 0;
} /* end gui_kill_timers() */


/******************************************************************************
 *
 * Title:       int gui_pack_boxes( GtkWidget *top_level_window )
 *
 * Description: Creates all the virtual boxes that buttons will be created in.
 *
 * Input:       top_level_window: The main GUI window.
 *
 * Output:      TRUE: No error checking implemented.
 *
 * Globals:     label_status
 *
 *****************************************************************************/

int gui_pack_boxes( GtkWidget *top_level_window )
{
    GtkWidget *vbox1;
    GtkWidget *vbox2;
    GtkWidget *vbox3;
    GtkWidget *vbox4;
    GtkWidget *vbox5;
    GtkWidget *vbox6;
    GtkWidget *hbox1;
    GtkWidget *hbox2;
    GtkWidget *hbox3;
    GtkWidget *hbox4;
    GtkWidget *hbox5;
    GtkWidget *notebook;

    /* Create new boxes. */
    vbox1 = gtk_vbox_new( FALSE, 0 );
    vbox2 = gtk_vbox_new( FALSE, 0 );
    vbox3 = gtk_vbox_new( FALSE, 0 );
    vbox4 = gtk_vbox_new( FALSE, 0 );
    vbox5 = gtk_vbox_new( TRUE, 0 );
    vbox6 = gtk_vbox_new( TRUE, 0 );
    hbox1 = gtk_hbox_new( TRUE, 0 );
    hbox2 = gtk_hbox_new( TRUE, 0 );
    hbox3 = gtk_hbox_new( TRUE, 0 );
    hbox4 = gtk_hbox_new( TRUE, 0 );
    hbox5 = gtk_hbox_new( TRUE, 0 );

    /* Add a box to the main window. */
    gtk_container_add( GTK_CONTAINER( top_level_window ), vbox1 );

    /* Add boxes to vbox1. */
    gtk_container_add( GTK_CONTAINER( vbox1 ), hbox1 );
    gtk_container_add( GTK_CONTAINER( vbox1 ), hbox5 );
    gtk_container_add( GTK_CONTAINER( vbox1 ), vbox2 );

    /* Create a notebook for tabbed pages. */
    notebook = gtk_notebook_new( );

    /* Add the notebook to the lower box. */
    gtk_container_add( GTK_CONTAINER( vbox2 ), notebook );
    gtk_container_add( GTK_CONTAINER( notebook ), vbox4 );
    gtk_notebook_set_tab_label( GTK_NOTEBOOK( notebook ), vbox4,
                                gtk_label_new( "Navigation" ) );

    /* Append another page to the notebook. */
    gtk_notebook_append_page( GTK_NOTEBOOK( notebook ), vbox3,
                              gtk_label_new( "Options" ) );

    /* Append another page to the notebook. */
    gtk_notebook_append_page( GTK_NOTEBOOK( notebook ), vbox6,
                              gtk_label_new( "Planner" ) );

    /* Append another page to the notebook. */
    gtk_notebook_append_page( GTK_NOTEBOOK( notebook ), vbox5,
                              gtk_label_new( "Vision" ) );                   

    /* Pack the rest of the boxes into the notebook pages. */
    gtk_box_pack_start( GTK_BOX( vbox3 ), hbox2, TRUE, FALSE, 0 );
    gtk_box_pack_start( GTK_BOX( vbox3 ), hbox3, TRUE, FALSE, 0 );
    gtk_box_pack_start( GTK_BOX( vbox3 ), hbox4, TRUE, FALSE, 0 );

    /* Make buttons. */
    buttons_make_status( hbox1 );
    buttons_make_estop( hbox5 );
    buttons_opmode( hbox2 );
    buttons_targets( vbox4 );
    buttons_options( hbox4 );
    buttons_gains( vbox4 );
    buttons_tasks( vbox6 );
    buttons_vision( vbox5 );

    return TRUE;
} /* end gui_pack_boxes() */


/******************************************************************************
 *
 * Title:       gint gui_destroy( GtkWidget *widget,
 *                                  GdkEvent *event,
 *                                  gpointer data )
 *
 * Description: Callback required for closing the GTK application.
 *
 * Input:       widget: A pointer to a widget.
 *              event: A pointer to an event.
 *              data: A pointer to data that can be manipulated.
 *
 * Output:      FALSE: No error checking implemented.
 *
 * Globals:     None.
 *
 *****************************************************************************/

gint gui_destroy( GtkWidget *widget, GdkEvent *event, gpointer data )
{
    gui_kill_timers( );
    gtk_main_quit( );

    return FALSE;
} /* end gui_destroy() */


/******************************************************************************
 *
 * Title:       gint giu_delete( GtkWidget *widget,
 *                                  GdkEvent *event,
 *                                  gpointer data )
 *
 * Description: Callback required for closing the GTK application.
 *
 * Input:       widget: A pointer to a widget.
 *              event: A pointer to an event.
 *              data: A pointer to data that can be manipulated.
 *
 * Output:      FALSE: No error checking implemented.
 *
 * Globals:     None.
 *
 *****************************************************************************/

gint gui_delete( GtkWidget *widget, GdkEvent *event, gpointer data )
{

    return FALSE;
} /* end gui_delete() */


/******************************************************************************
 *
 * Title:       GtkWidget *gui_create_main_window( )
 *
 * Description: Actually creates a window that all other GUI elements are
 *              packed into.
 *
 * Input:       None.
 *
 * Output:      window: The top level window for the GUI.
 *
 * Globals:     None.
 *
 *****************************************************************************/

GtkWidget *gui_create_main_window( )
{
    /* Create main window. */
    GtkWidget *window = gtk_window_new( GTK_WINDOW_TOPLEVEL );

    /* Set window properties. */
    gtk_window_set_title( GTK_WINDOW( window ), "STINGRAY" );

    /* Connect the events to the window. */
    gtk_signal_connect( GTK_OBJECT( window ), "delete_event",
                        GTK_SIGNAL_FUNC( gui_delete ), NULL );
    gtk_signal_connect( GTK_OBJECT( window ), "destroy",
                        GTK_SIGNAL_FUNC( gui_destroy ), NULL );

    return window;
} /* end gui_create_main_window() */


/******************************************************************************
 *
 * Title:       void gui_init( )
 *
 * Description: This is where the GTK GUI magic starts. It is the entrance
 *              point for outside functions to hand over control to the GUI.
 *
 * Input:       None.
 *
 * Output:      None.
 *
 * Globals:     None.
 *
 *****************************************************************************/

void gui_init( )
{
    GtkWidget *top_level_window;

    /* Create the main window for the GUI. */
    top_level_window = gui_create_main_window( );

    /* Create virtual boxes that buttons and text can be added into. */
    gui_pack_boxes( top_level_window );

    /* Set properties for the main window of the GUI. */
    gtk_widget_set_usize( top_level_window, cf.window_width, cf.window_height );

    /* Set an icon for the application. */
    gtk_window_set_icon_from_file( ( GtkWindow * )top_level_window,
		GUI_ICON, NULL );

    /* Show all the widgets that have been created in the main window. */
    gtk_widget_show_all( top_level_window );
} /* end gui_init() */
