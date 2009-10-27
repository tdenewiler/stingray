// GUI helper functions

typedef struct {
	GtkWidget *button;
	GtkWidget *label;
	GtkAdjustment *adj;
}SPIN;

typedef struct {
	GtkWidget *entry;
	GtkWidget *label;
}ENTRY;

GtkWidget * guiutil_makebutton( GtkWidget *vbox1, char *text, char *tooltext
                                , GtkSignalFunc fn, GtkTooltips *tt, GtkWidget *data );

ENTRY guiutil_makeentry( GtkWidget *vbox1, char *labeltext, char *entrytext
                         , char *tooltext, GtkTooltips *tt );

SPIN guiutil_make_spinbutton( GtkWidget *vbox, char *text, double a
                              , double b, double c, double d, double e, double f
                              , GtkTooltips *tt, char *tooltext );

SPIN guiutil_make_spinbutton2( GtkWidget *table, int aa, int bb, int cc
                               , int dd, char *text, double a
                               , double b, double c, double d, double e, double f
                               , GtkTooltips *tt, char *tooltext );
//void util_showhide_default(SPINNERS spinners);


GtkWidget *guiutil_new_frame_with_vbox( GtkWidget *vbox2, char *text );
GtkWidget *guiutil_new_frame_with_hbox( GtkWidget *vbox, char *text );
GtkWidget *guiutil_new_frame_with_table( GtkWidget *vbox, char *text, int x, int y );
GtkWidget *guiutil_new_tab_with_vbox( GtkWidget *nb, char *text );
GtkWidget *guiutil_new_hbox( GtkWidget *vbox );
GtkWidget *guiutil_new_vbox( GtkWidget *vbox1 );

GtkResponseType guiutil_DialogConfirm( char *query, char *title );
GtkResponseType guiutil_DialogInfo( char *query, char *title );

void guiutil_nbtab_SetFont( GtkWidget *nb, GtkWidget *child, char *font );
void guiutil_SetDefaultFont( char *font );

