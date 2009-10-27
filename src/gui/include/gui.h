/**
 *  \file gui.h
 *  \brief Sets up GTK+-2.0 GUI interface for the UUV client.
 */

#ifndef _GUI_H_
#define _GUI_H_

#include <sys/timeb.h>
#include <string.h>
#include <gtk/gtk.h>
#include <unistd.h>

#include "buttons.h"
#include "network.h"
#include "messages.h"
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

#ifndef GUI_ICON
#define GUI_ICON "icons/hull.png"
#endif /* GUI_ICON */


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

//! Initializes the GUI functions and data.
//!
void gui_init( );

//! Sets up the actions for the `delete` Gtk event.
//! Returns FALSE.
//! \param widget A pointer to a widget.
//! \param event A pointer to an event.
//! \param data A pointer to data that can be manipulated.
//! \return FALSE. No error checking implemented.
gint gui_delete( GtkWidget *widget, GdkEvent *event, gpointer data );

//! Sets up the actions for the `destroy` Gtk event.
//! Returns FALSE.
//! \param widget A pointer to a widget.
//! \param event A pointer to an event.
//! \param data A pointer to data that can be manipulated.
//! \return FALSE. No error checking implemented.
gint gui_destroy( GtkWidget *widget, GdkEvent *event, gpointer data );

//! Sets up the timers used in the client program.
//!
void gui_set_timers( );

//! Updates the text displayed in the Status window.
//!
void gui_update_status_text( );

//! Sets up a 1 second Gtk timer.
//! Returns TRUE.
//! \param data A pointer to data that can be manipulated.
//! \return TRUE. No error checking implemented.
gint gui_timer_1s( gpointer data );

//! Sets up a 500ms Gtk timer.
//! Returns TRUE.
//! \param data A pointer to data that can be manipulated.
//! \return TRUE. No error checking implemented.
gint gui_timer_500ms( gpointer data );

//! Sets up a 200ms Gtk timer.
//! Returns TRUE.
//! \param data A pointer to data that can be manipulated.
//! \return TRUE. No error checking implemented.
gint gui_timer_200ms( gpointer data );

//! Sets up a 100ms Gtk timer.
//! Returns TRUE.
//! \param data A pointer to data that can be manipulated.
//! \return TRUE. No error checking implemented.
gint gui_timer_100ms( gpointer data );

//! Sets up a 50ms Gtk timer.
//! Returns TRUE.
//! \param data A pointer to data that can be manipulated.
//! \return TRUE. No error checking implemented.
gint gui_timer_50ms( gpointer data );

//! Kills (stops) the Gtk timers.
//!
void gui_kill_timers( );

//! Starts the process of packing buttons into boxes.
//! Returns TRUE.
//! \param top_level_window The main GUI window.
//! \return TRUE. No error checking implemented.
int gui_pack_boxes( GtkWidget *top_level_window );

//! Creates the main Gtk window.
//! Returns a GtkWidget.
//! \return The top level window for the GUI.
GtkWidget *gui_create_main_window( );


#endif /* _GUI_H_ */
