/*************************************************************************
*
*	gui_utility.c
*
*  Copyright 2005,2006,2007,2008,2009 Steve Stancliff
*
*  Part of the TAOSF SSA system.
*
*  This module provides Gnome/GTK GUI building and control functions
*
*
**************************************************************************
*
*  version 1.0.5, 1/31/09, SBS
*    - rename functions tto guiutil_
*    - cleanup
*
*  version 1.0.4, 1/27/09, SBS
*    - rename to gui_utility.c from utility.c
*    - eliminate typedefs.h
*
*  version 1.0.3, 1/26/09, SBS
*    - move dialogConfirm here
*
*  version 1.0.2, 1/2/09, SBS
*    - documentation, cleanup
*
*  version 1.0.1, 12/06/08, SBS
*    - incorporated into SSA platform_gui
*
*  version 1.0	12/23/2005
*    - part of SMRCgui
*
**************************************************************************
*
*  This program is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation; either version 2 of the License, or
*  (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful, but
*  WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
*  General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program; if not, write to the Free Software Foundation,
*  Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
*
**************************************************************************/
/**
*  @file
*  @ingroup common
*  @brief
*  This module provides various routines for making GTK GUI interfaces
*  <hr>
**/

#include <gtk/gtk.h>

#include "gui_utility.h"


/****************************************************************
*
*  guiutil_makebutton()
*/
/**
*
* @brief
*  Creates a pushbutton with tooltip
*
*  Parameters:
*   - tkWidget *vbox1  - packing box to add the button to
*   - char *text       - text used for button label
*   - char *tooltext   - text used for button tooltip
*   - GtkSignalFunc fn - function to call when button pressed
*   - GtkTooltips *tt  - handle to window tooltips
*
* Returns:
*   - the button
*
****************************************************************/
GtkWidget * guiutil_makebutton(
    GtkWidget *vbox1
    , char *text
    , char *tooltext
    , GtkSignalFunc fn
    , GtkTooltips *tt
    , GtkWidget *data )
{
	GtkWidget *button, *align;

	button = gtk_button_new_with_label( text );
	align = gtk_alignment_new( 0.5, 0.5, 0, 0 );

	gtk_box_pack_start( GTK_BOX( vbox1 ), align, FALSE, FALSE, 0 );
	gtk_container_add( GTK_CONTAINER( align ), button );

	gtk_widget_show( button );
	gtk_widget_show( align );

	gtk_signal_connect( GTK_OBJECT( button ), "clicked", fn, data );
	gtk_tooltips_set_tip( tt, button, tooltext, NULL );

	return( button );
}


/****************************************************************
*
*  guiutil_makeentry()
*/
/**
*
* @brief
*  Creates a text entry with tooltip
*
*  Parameters:
*   - tkWidget *vbox1  - packing box to add the button to
*   - char *text       - text used for button label
*   - char *tooltext   - text used for button tooltip
*   - GtkTooltips *tt  - handle to window tooltips
*
* Returns:
*   - the button
*
****************************************************************/
ENTRY guiutil_makeentry(
    GtkWidget *vbox
    , char *labeltext
    , char *entrytext
    , char *tooltext
    , GtkTooltips *tt )
{
//	GtkWidget *ent,*align;
	GtkWidget *hbox;
//	GtkWidget *align;
	ENTRY rval;

	hbox = gtk_hbox_new( FALSE, 20 );
	gtk_box_pack_start( GTK_BOX( vbox ), hbox, FALSE, FALSE, 0 );
	gtk_widget_show( hbox );

	rval.label = gtk_label_new( labeltext );
	gtk_box_pack_start( GTK_BOX( hbox ), rval.label, FALSE, FALSE, 0 );
	gtk_widget_show( rval.label );


	rval.entry = gtk_entry_new();
	gtk_entry_set_text( GTK_ENTRY( rval.entry ), entrytext );
//	align = gtk_alignment_new(0.5,0.5,0,0);
	gtk_entry_set_width_chars( GTK_ENTRY( rval.entry ), 10 );

	gtk_box_pack_start( GTK_BOX( hbox ), rval.entry, FALSE, FALSE, 0 );

//	gtk_box_pack_start(GTK_BOX(hbox),align,FALSE,FALSE,0);
//	gtk_container_add (GTK_CONTAINER (align), rval.entry);

	gtk_widget_show( rval.entry );
//	gtk_widget_show (align);

//	gtk_signal_connect (GTK_OBJECT (button), "clicked", fn, data);
	gtk_tooltips_set_tip( tt, rval.entry, tooltext, NULL );

	return( rval );
}


/****************************************************************
*
*  guiutil_make_spinbutton()
*/
/**
*
* @brief
*  Creates a spinbutton with label and tooltip.
*
*  Parameters:
*   - GtkWidget *vbox - packing box to add button to
*   - char *text      - text to use for button label
*   - double a        - value
*   - double b        - lower limit
*   - double c        - upper limit
*   - double d        - increment
*   - double e        - page size
*   - double f        -
*   - GtkTooltips *tt - handle to window tooltips
*   - char *tooltext  - text to use for tooltip
*
* Returns:
*   - the spinbutton
*
****************************************************************/
SPIN guiutil_make_spinbutton(
    GtkWidget *vbox
    , char *text
    , double a
    , double b
    , double c
    , double d
    , double e
    , double f
    , GtkTooltips *tt
    , char *tooltext )
{
	GtkWidget *hbox;
	SPIN rval;

	hbox = gtk_hbox_new( FALSE, 20 );
	gtk_box_pack_start( GTK_BOX( vbox ), hbox, FALSE, FALSE, 0 );
	gtk_widget_show( hbox );

	rval.label = gtk_label_new( text );
	gtk_box_pack_start( GTK_BOX( hbox ), rval.label, FALSE, FALSE, 0 );
	gtk_widget_show( rval.label );

	rval.adj = ( GtkAdjustment * ) gtk_adjustment_new( a, b, c, d, e, 0.0 );
	rval.button = gtk_spin_button_new( rval.adj, 0, f );
	gtk_box_pack_start( GTK_BOX( hbox ), rval.button, FALSE, FALSE, 0 );

	gtk_widget_show( rval.button );

	gtk_tooltips_set_tip( tt, rval.button, tooltext, NULL );

	return( rval );
}


/****************************************************************
*
*  guiutil_make_spinbutton2()
*/
/**
*
* @brief
*
*  Parameters:
*
*
****************************************************************/
SPIN guiutil_make_spinbutton2(
    GtkWidget *table
    , int aa
    , int bb
    , int cc
    , int dd
    , char *text
    , double a
    , double b
    , double c
    , double d
    , double e
    , double f
    , GtkTooltips *tt
    , char *tooltext )
{
	GtkWidget *hbox;
	GtkAdjustment *adj;

	SPIN rval;

	hbox = gtk_hbox_new( FALSE, 20 );
	gtk_container_set_border_width( GTK_CONTAINER( hbox ), 5 );
	gtk_table_attach_defaults( GTK_TABLE( table ), hbox, aa, bb, cc, dd );
	gtk_widget_show( hbox );

	rval.label = gtk_label_new( text );
	gtk_box_pack_start( GTK_BOX( hbox ), rval.label, FALSE, FALSE, 0 );
	gtk_widget_show( rval.label );

	adj = ( GtkAdjustment * ) gtk_adjustment_new( a, b, c, d, e, 0.0 );
	rval.button = gtk_spin_button_new( adj, 0, f );
	gtk_box_pack_start( GTK_BOX( hbox ), rval.button, FALSE, FALSE, 0 );
	gtk_widget_show( rval.button );

	gtk_tooltips_set_tip( tt, rval.button, tooltext, NULL );

	return( rval );
}

/****************************************************************
*
*  guiutil_spinhide()
*/
/**
*
* @brief
*  Hides a spinbutton and it's label.
*
*  Parameters:
*   SPIN spin : the spinbutton
*
****************************************************************/
void guiutil_spinhide(
    SPIN spin )
{
	gtk_widget_hide( spin.button );
	gtk_widget_hide( spin.label );

	return;
}


/****************************************************************
*
*  guiutil_texthide()
*/
/**
*
* @brief
*  Hides the label for a spinbutton
*
*  Parameters:
*   SPIN spin : the spinbutton
*
****************************************************************/
void guiutil_texthide(
    SPIN spin )
{
	gtk_widget_hide( spin.label );

	return;
}


/****************************************************************
*
*  guiutil_spinshow()
*/
/**
*
* @brief
*  Shows a spinbutton and it's label
*
*  Parameters:
*   SPIN spin : the spinbutton
*
****************************************************************/
void guiutil_spinshow(
    SPIN spin )
{
	gtk_widget_show( spin.button );
	gtk_widget_show( spin.label );
	return;
}


/****************************************************************
*
*  guiutil_new_frame_with_vbox()
*/
/**
*
* @brief
*
*
*
****************************************************************/
GtkWidget *guiutil_new_frame_with_vbox(
    GtkWidget *vbox2
    , char *text )
{
	GtkWidget *frame, *vbox1;

	frame = gtk_frame_new( text );
	gtk_frame_set_label_align( GTK_FRAME( frame ), 0.5, 0 );
	gtk_container_set_border_width( GTK_CONTAINER( frame ), 5 );
	gtk_container_add( GTK_CONTAINER( vbox2 ), frame );
	gtk_widget_show( frame );

	vbox1 = gtk_vbox_new( FALSE, 10 );
	gtk_container_add( GTK_CONTAINER( frame ), vbox1 );
	gtk_container_set_border_width( GTK_CONTAINER( vbox1 ), 10 );
	gtk_widget_show( vbox1 );

	return( vbox1 );
}


/****************************************************************
*
*  guiutil_new_frame_with_hbox()
*/
/**
*
* @brief
*
*
*
****************************************************************/
GtkWidget *guiutil_new_frame_with_hbox(
    GtkWidget *vbox
    , char *text )
{
	GtkWidget *frame, *vbox2;

	frame = gtk_frame_new( text );
	gtk_frame_set_label_align( GTK_FRAME( frame ), 0.5, 0 );
	gtk_container_set_border_width( GTK_CONTAINER( frame ), 5 );
	gtk_container_add( GTK_CONTAINER( vbox ), frame );
	gtk_widget_show( frame );

	vbox2 = gtk_hbox_new( TRUE, 10 );
	gtk_container_add( GTK_CONTAINER( frame ), vbox2 );
	gtk_widget_show( vbox2 );

	return( vbox2 );
}


/****************************************************************
*
*  guiutil_new_frame_with_table()
*/
/**
*
* @brief
*
*
*
****************************************************************/
GtkWidget *guiutil_new_frame_with_table(
    GtkWidget *vbox
    , char *text
    , int x
    , int y )
{
	GtkWidget *table, *frame1;

	frame1 = gtk_frame_new( "Independent variable" );
	gtk_frame_set_label_align( GTK_FRAME( frame1 ), 0.5, 0 );
	gtk_container_add( GTK_CONTAINER( vbox ), frame1 );
	gtk_widget_show( frame1 );

	table = gtk_table_new( 4, 3, TRUE );
	gtk_container_add( GTK_CONTAINER( frame1 ), table );
	gtk_container_set_border_width( GTK_CONTAINER( table ), 10 );
	gtk_widget_show( table );

	return( table );
}


/****************************************************************
*
*  guiutil_new_tab_with_vbox()
*/
/**
*
* @brief
*
*
*
****************************************************************/
GtkWidget *guiutil_new_tab_with_vbox(
    GtkWidget *nb
    , char *text )
{
	GtkWidget *vbox1;
	GtkWidget *tab;

	vbox1 = gtk_vbox_new( FALSE, 10 );
	gtk_container_set_border_width( GTK_CONTAINER( vbox1 ), 20 );

	tab = gtk_label_new( text );
//	gtk_widget_set_size_request((GtkWidget *)(tab), 80, 45);
	gtk_notebook_append_page( GTK_NOTEBOOK( nb ), vbox1, tab );

//	gtk_widget_modify_font (GTK_WIDGET(tab)
//			,pango_font_description_from_string(TEXTFONT));

	gtk_widget_show( vbox1 );

	return( vbox1 );
}


/****************************************************************
*
*  guiutil_new_hbox()
*/
/**
*
* @brief
*
*
*
****************************************************************/
GtkWidget *guiutil_new_hbox( GtkWidget *vbox )
{
	GtkWidget *hbox;

	hbox = gtk_hbox_new( TRUE, 10 );
	gtk_container_set_border_width( GTK_CONTAINER( hbox ), 5 );
	gtk_container_add( GTK_CONTAINER( vbox ), hbox );
	gtk_widget_show( hbox );

	return( hbox );
}


/****************************************************************
*
*  guiutil_new_vbox()
*/
/**
*
* @brief
*
*
*
****************************************************************/
GtkWidget *guiutil_new_vbox(
    GtkWidget *vbox1 )
{
	GtkWidget *vbox2;

	vbox2 = gtk_vbox_new( FALSE, 10 );
	gtk_box_pack_start( GTK_BOX( vbox1 ), vbox2, FALSE, FALSE, 0 );
	gtk_widget_show( vbox2 );

	return( vbox2 );
}


/****************************************************************
*
*  guiutil_DialogConfirm()
*/
/**
*
* @brief
*
*
****************************************************************/
GtkResponseType guiutil_DialogConfirm(
    char *query
    , char *title )
{
	GtkWidget *dialog;
	GtkResponseType response;

	dialog = gtk_message_dialog_new( NULL,
	                                 GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
	                                 GTK_MESSAGE_QUESTION,
	                                 GTK_BUTTONS_YES_NO,
	                                 query );

	gtk_window_set_title( GTK_WINDOW( dialog ), title );

	/* run the dialog */
	response = gtk_dialog_run( GTK_DIALOG( dialog ) );
	gtk_widget_destroy( dialog );

	return response;
}


/****************************************************************
*
*  guiutil_DialogInfo()
*/
/**
*
* @brief
*
*
****************************************************************/
GtkResponseType guiutil_DialogInfo(
    char *query
    , char *title )
{
	GtkWidget *dialog;
	GtkResponseType response;

	dialog = gtk_message_dialog_new( NULL,
	                                 GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
	                                 GTK_MESSAGE_INFO,
	                                 GTK_BUTTONS_OK,
	                                 query );

	gtk_window_set_title( GTK_WINDOW( dialog ), title );

	/* run the dialog */
	response = gtk_dialog_run( GTK_DIALOG( dialog ) );
	gtk_widget_destroy( dialog );

	return response;
}


/****************************************************************
*
*  guiutil_SetDefaultFont()
*/
/**
*
* @brief
*
*
****************************************************************/
void guiutil_SetDefaultFont( char *font )
{
	GtkSettings *settings;

	settings = gtk_settings_get_default();

	g_object_set( G_OBJECT( settings ), "gtk-font-name"
	              , font, NULL );

	return;
}
