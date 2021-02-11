//********************************************************************
//*                    Visual                                        *
//*==================================================================*
//* WRITTEN BY: Liam McEvoy    	                 		             *
//* DATE CREATED:   14/01/21                                         *
//* MODIFIED:                                                        *
//*==================================================================*
//* PROGRAMMED IN: Visual Studio                                     *
//*==================================================================*
//* DESCRIPTION: Program to visualise captured and processed         *
//*             sampled data for Software Defined Radio program      *
//********************************************************************
// INCLUDE FILES
//====================================================================

#include <../include/Test_Data.h>
#include <../include/tinywav.h>
#include <fftw3.h>
#include <math.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <../include/SDR.h>
#include <gtkglg.h>

//====================================================================
// STRUCTURES
//====================================================================

typedef struct Widgets {
	GtkWidget	*window;
	GtkWidget	*fixed;
	GtkWidget	*menubar;
	GtkWidget	*New;
	GtkWidget	*Open;
	GtkWidget	*Save;
	GtkWidget	*Save_as;
	GtkWidget	*Cut;
	GtkWidget	*Copy;
	GtkWidget	*Paste;
	GtkWidget	*Delete;
	GtkWidget	*FFT;
	GtkWidget	*Spectrogram;
	GtkWidget	*Original_Data;
	GtkWidget	*Triangle;
	GtkWidget	*Welch;
	GtkWidget	*Sine;
	GtkWidget	*Hann;
	GtkWidget	*Hamming;
	GtkWidget	*Blackman;
	GtkWidget	*Gaussian;
	GtkWidget	*No_window;
	GtkWidget	*About;
	GtkWidget	*Play;
	GtkWidget	*Pause;
	GtkWidget	*toolbar;
	GtkWidget	*volume;
	GtkWidget	*sampling;
	GtkWidget	*sampling_1;
	GtkWidget	*sampling_2;
	GtkWidget	*sampling_3;
	GtkBuilder  *builder;
	GlgObject   *dataplot;
	GtkWidget	*viewport;
	GtkWidget	*viewport1;
	GtkWidget	*viewport2;
	GtkWidget	*Skip;
	GtkWidget	*Zoom_out;
	GtkWidget	*Zoom_in;
	GlgObject   *Glg_viewport;
	GtkWidget   * glg;
	GlgObject   *Glg_viewport1;
	GtkWidget   * glg1;

} Widgets;

//====================================================================
// SYMBOLIC CONSTANTS
//====================================================================

#define _USE_MATH_DEFINES
#define UPDATE_INTERVAL    50 /* millisec */ 
#define TRACE_GLG_MESSAGES 0

//====================================================================
// GLOBAL VARIABLES
//====================================================================

Widgets glade;

//====================================================================
// FUNCTION DECLARATIONS
//====================================================================

static gint AnimateControlPanel( gpointer data );
void on_window_main_destroy();
void test_wav();

//Main for running the GUI part of the program
int main(int argc, char *argv[]) {
	char * full_path;

    gtk_init(&argc, &argv);
	gtk_glg_toolkit_init( argc, argv );

    glade.builder       = gtk_builder_new();
    gtk_builder_add_from_file (glade.builder, "../GUI/sdr_window.glade", NULL);

    glade.window        = GTK_WIDGET(gtk_builder_get_object(glade.builder, "window_main"));
    gtk_builder_connect_signals(glade.builder, NULL);
	
	glade.glg           = gtk_glg_new();
	glade.glg1           = gtk_glg_new();
	glade.fixed         = GTK_WIDGET(gtk_builder_get_object(glade.builder, "fixed"));
	glade.menubar       = GTK_WIDGET(gtk_builder_get_object(glade.builder, "menubar"));
	glade.New           = GTK_WIDGET(gtk_builder_get_object(glade.builder, "New"));
	glade.Open          = GTK_WIDGET(gtk_builder_get_object(glade.builder, "Open"));
	glade.Save          = GTK_WIDGET(gtk_builder_get_object(glade.builder, "Save"));
	glade.Save_as       = GTK_WIDGET(gtk_builder_get_object(glade.builder, "Save as"));
	glade.Cut           = GTK_WIDGET(gtk_builder_get_object(glade.builder, "Cut"));
	glade.Copy          = GTK_WIDGET(gtk_builder_get_object(glade.builder, "Copy"));
	glade.Paste         = GTK_WIDGET(gtk_builder_get_object(glade.builder, "Paste"));
	glade.Delete        = GTK_WIDGET(gtk_builder_get_object(glade.builder, "Delete"));
	glade.FFT           = GTK_WIDGET(gtk_builder_get_object(glade.builder, "FFT"));
	glade.Spectrogram   = GTK_WIDGET(gtk_builder_get_object(glade.builder, "Spectrogram"));
	glade.Original_Data = GTK_WIDGET(gtk_builder_get_object(glade.builder, "Original_Data"));
	glade.Triangle      = GTK_WIDGET(gtk_builder_get_object(glade.builder, "Triangle"));
	glade.Welch         = GTK_WIDGET(gtk_builder_get_object(glade.builder, "Welch"));
	glade.Sine          = GTK_WIDGET(gtk_builder_get_object(glade.builder, "Sine"));
	glade.Hann          = GTK_WIDGET(gtk_builder_get_object(glade.builder, "Hann"));
	glade.Hamming       = GTK_WIDGET(gtk_builder_get_object(glade.builder, "Hamming"));
	glade.Blackman      = GTK_WIDGET(gtk_builder_get_object(glade.builder, "Blackman"));
	glade.Gaussian      = GTK_WIDGET(gtk_builder_get_object(glade.builder, "Gaussian"));
	glade.No_window     = GTK_WIDGET(gtk_builder_get_object(glade.builder, "No_window"));
	glade.About         = GTK_WIDGET(gtk_builder_get_object(glade.builder, "About"));
	glade.Play          = GTK_WIDGET(gtk_builder_get_object(glade.builder, "Play"));
	glade.Pause         = GTK_WIDGET(gtk_builder_get_object(glade.builder, "Pause"));
	glade.toolbar       = GTK_WIDGET(gtk_builder_get_object(glade.builder, "toolbar"));
	glade.volume        = GTK_WIDGET(gtk_builder_get_object(glade.builder, "volume"));
	glade.sampling      = GTK_WIDGET(gtk_builder_get_object(glade.builder, "sampling"));
	glade.sampling_1    = GTK_WIDGET(gtk_builder_get_object(glade.builder, "sampling_1"));
	glade.sampling_2    = GTK_WIDGET(gtk_builder_get_object(glade.builder, "sampling_2"));
	glade.sampling_3    = GTK_WIDGET(gtk_builder_get_object(glade.builder, "sampling_3"));
	glade.viewport      = GTK_WIDGET(gtk_builder_get_object(glade.builder, "viewport"));
	glade.viewport1     = GTK_WIDGET(gtk_builder_get_object(glade.builder, "viewport1"));
	glade.viewport2     = GTK_WIDGET(gtk_builder_get_object(glade.builder, "viewport2"));
	glade.Skip          = GTK_WIDGET(gtk_builder_get_object(glade.builder, "Skip"));
	glade.Zoom_out      = GTK_WIDGET(gtk_builder_get_object(glade.builder, "Zoom_out"));
	glade.Zoom_in       = GTK_WIDGET(gtk_builder_get_object(glade.builder, "Zoom_in"));

  	full_path = GlgCreateRelativePath( argv[0], "../Drawings", False, False );
  	GlgSetSResource( NULL, "$config/GlgSearchPath", full_path );
  	GlgFree( full_path );

	glade.Glg_viewport = GlgLoadWidgetFromFile( "Data_graph.g"  );
	glade.Glg_viewport1 = GlgLoadWidgetFromFile( "Data_graph.g" );
	

  	gtk_glg_set_viewport(glade.glg, glade.Glg_viewport   );
	gtk_glg_set_viewport(glade.glg1, glade.Glg_viewport1 );
  	GlgDropObject( glade.Glg_viewport );
  	GlgDropObject( glade.Glg_viewport1);
	
    g_object_unref(glade.builder);
	gtk_container_add( GTK_CONTAINER( glade.viewport  ), glade.glg  );
	gtk_container_add( GTK_CONTAINER( glade.viewport1 ), glade.glg1 );

  	gtk_widget_show( glade.glg    );
	gtk_widget_show( glade.glg1   );
  	gtk_widget_show( glade.window );
  
  	/* Add timer to update control panel with data. */
  	g_timeout_add( (guint32) UPDATE_INTERVAL, AnimateControlPanel, (gpointer) glade.Glg_viewport );   

	//----- ENTER THE GTK MAIN LOOP -----
	gtk_main();		//Enter the GTK+ main loop until the application closes.

	return 0;
}

static gint AnimateControlPanel( gpointer data )
{
   	GlgObject viewport = (GlgObject) data;

	GlgUpdate( viewport );
    GlgSync( viewport );
   	/* Reinstall the timer */
   	g_timeout_add( (guint32) UPDATE_INTERVAL, AnimateControlPanel, (gpointer) viewport );   

	return False;
}

// called when window is closed
void on_window_main_destroy(){
    gtk_main_quit();
}

// called when Quit is clicked
void on_Quit_activate(GtkMenuItem *menuitem){
    gtk_main_quit();
}

// called when New is clicked
void on_New_activate(GtkMenuItem *menuitem){
	
}

// called when Open is clicked
void on_Open_activate(GtkMenuItem *menuitem){
	
}

// called when Save is clicked
void on_Save_activate(GtkMenuItem *menuitem){
	
}

// called when Save as is clicked
void on_Save_as_activate(GtkMenuItem *menuitem){
	
}

// called when Cut is clicked
void on_Cut_activate(GtkMenuItem *menuitem){
	
}

// called when Copy is clicked
void on_Copy_activate(GtkMenuItem *menuitem){
	
}

// called when Paste is clicked
void on_Paste_activate(GtkMenuItem *menuitem){
	
}

// called when Delete is clicked
void on_Delete_activate(GtkMenuItem *menuitem){
	
}

// called when FFT graph is toggled
void on_FFT_toggled(GtkCheckMenuItem *checkmenuitem){
	gboolean T = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(checkmenuitem));
	if(T){
		
	}
	else{
		
	}	
}

// called when Spectrogram graph is toggled
void on_Spectrogram_toggled(GtkCheckMenuItem *checkmenuitem){
	gboolean T = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(checkmenuitem));
	if(T){
		
	}
	else{
		
	}	
}

// called when Data graph is toggled
void on_Original_Data_toggled(GtkCheckMenuItem *checkmenuitem){
	gboolean T = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(checkmenuitem));
	if(T){
		
	}
	else{
		
	}

}

// called when Triangle window is toggled and needs to be implemented
void on_Triangle_toggled(GtkRadioMenuItem *radio) {
	gboolean T = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(radio));
	if(T){
		
	}
	else{
		
	}
}

// called when Welch window is toggled and needs to be implemented
void on_Welch_toggled(GtkRadioMenuItem *radio) {
	gboolean T = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(radio));
	if(T){

	}
	else{
		
	}
}

// called when Sine window is toggled and needs to be implemented
void on_Sine_toggled(GtkRadioMenuItem *radio) {
	gboolean T = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(radio));
	if(T){

	}
	else{
		
	}
}

// called when Hann window is toggled and needs to be implemented
void on_Hann_toggled(GtkRadioMenuItem *radio) {
	gboolean T = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(radio));
	if(T){

	}
	else{
		
	}
}

// called when Hamming window is toggled and needs to be implemented
void on_Hamming_toggled(GtkRadioMenuItem *radio) {
	gboolean T = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(radio));
	if(T){

	}
	else{
		
	}
}

// called when Blackman window is toggled and needs to be implemented
void on_Blackman_toggled(GtkRadioMenuItem *radio) {
	gboolean T = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(radio));
	if(T){

	}
	else{
		
	}
}

// called when Gaussian window is toggled and needs to be implemented
void on_Gaussian_toggled(GtkRadioMenuItem *radio) {
	gboolean T = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(radio));
	if(T){

	}
	else{
		
	}
}

// called when Uniform window is toggled and needs to be implemented
void on_No_window_toggled(GtkRadioMenuItem *radio) {
	gboolean T = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(radio));
	if(T){

	}
	else{
		
	}
}

// called when About button is clicked
void on_About_activate(GtkMenuItem *menuitem){
	
}

// called when Pause tool is clicked
void on_Pause_toggled(GtkToolItem *toolitem){
	gboolean T = gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(toolitem));
	if(T){
		gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(glade.Play),False);
	}
	else{
		
	}
}

// called when Play tool is clicked
void on_Play_toggled(GtkToolItem *toolitem){
	gboolean T = gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(toolitem));
	if(T){
		gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(glade.Pause),False);
	}
	else{
		
	}	
}

// called when Skip tool is clicked
void on_Skip_toggled(GtkToolItem *toolitem){
	gboolean T = gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(toolitem));
	if(T){

	}
	else{
		
	}	
}

// called when Zoom out tool is clicked
void on_Zoom_out_toggled(GtkToolItem *toolitem){
	gboolean T = gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(toolitem));
	if(T){
		gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(glade.Zoom_in),False);
	}
	else{
		
	}	
}

// called when Zoom in tool is clicked
void on_Zoom_in_toggled(GtkToolItem *toolitem){
	gboolean T =gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(toolitem));
	if(T){
		gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(glade.Zoom_out),False);
	}
	else{
		
	}	
}

// called when Volume is clicked
void on_volume_value_changed(GtkVolumeButton *volume){
	gdouble scale=gtk_scale_button_get_value(GTK_SCALE_BUTTON(volume));
	
}

// called when sampling is set to 44.1 KHz
void on_sampling_toggled(GtkRadioMenuItem *radio) {
	gboolean T = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(radio));
	if(T){

	}
	else{
		
	}
}

// called when sampling is set to 48 KHz
void on_sampling_1_toggled(GtkRadioMenuItem *radio) {
	gboolean T = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(radio));
	if(T){

	}
	else{
		
	}
}

// called when sampling is set to 88.2 KHz
void on_sampling_2_toggled(GtkRadioMenuItem *radio) {
	gboolean T = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(radio));
	if(T){

	}
	else{
		
	}
}

// called when sampling is set to 96 KHz
void on_sampling_3_toggled(GtkRadioMenuItem *radio) {
	gboolean T = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(radio));
	if(T){

	}
	else{
		
	}
}

//Simple test of reading from wav file containing music and rewriting to a new wav file and txt file of the data read
void test_wav(){
	int NUM_CHANNELS,SAMPLE_RATE,BLOCK_SIZE;
	NUM_CHANNELS=1;SAMPLE_RATE=48000;BLOCK_SIZE=48000;

	TinyWav tw,tw1;
	tinywav_open_read(&tw, "../data/Secrets.wav", TW_INLINE, TW_FLOAT32);
	char * outputPath;
    outputPath="../data/secret-test.wav"; 
    tinywav_open_write(&tw1, NUM_CHANNELS, SAMPLE_RATE, TW_FLOAT32, TW_INLINE, outputPath);
	int i;
	for ( i = 0; i < 5; i++) {
  		// samples are always presented in float32 format
  		float samples[BLOCK_SIZE];
  		tinywav_read_f(&tw, samples, BLOCK_SIZE);
		if(i==0){
			writetextf(samples,BLOCK_SIZE,"../data/secret-test", 1);	
		}
		tinywav_write_f(&tw1, samples, BLOCK_SIZE);
	}
	
	tinywav_close_read(&tw);
	tinywav_close_write(&tw1);
}


