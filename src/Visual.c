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
	GtkBuilder  *builder;
} Widgets;
//====================================================================
// SYMBOLIC CONSTANTS
//====================================================================
#define _USE_MATH_DEFINES
//====================================================================
// GLOBAL VARIABLES
//====================================================================
Widgets glade;
//====================================================================
// FUNCTION DECLARATIONS
//====================================================================


void on_window_main_destroy();

void test_wav();

int main(int argc, char *argv[]) {
	

    gtk_init(&argc, &argv);

    glade.builder = gtk_builder_new();
    gtk_builder_add_from_file (glade.builder, "../GUI/sdr_window.glade", NULL);

    glade.window = GTK_WIDGET(gtk_builder_get_object(glade.builder, "window_main"));
    gtk_builder_connect_signals(glade.builder, NULL);
	

	glade.fixed = GTK_WIDGET(gtk_builder_get_object(glade.builder, "fixed"));
	glade.menubar = GTK_WIDGET(gtk_builder_get_object(glade.builder, "menubar"));
	glade.New = GTK_WIDGET(gtk_builder_get_object(glade.builder, "New"));
	glade.Open = GTK_WIDGET(gtk_builder_get_object(glade.builder, "Open"));
	glade.Save = GTK_WIDGET(gtk_builder_get_object(glade.builder, "Save"));
	glade.Save_as = GTK_WIDGET(gtk_builder_get_object(glade.builder, "Save as"));
	glade.Cut = GTK_WIDGET(gtk_builder_get_object(glade.builder, "Cut"));
	glade.Copy = GTK_WIDGET(gtk_builder_get_object(glade.builder, "Copy"));
	glade.Paste = GTK_WIDGET(gtk_builder_get_object(glade.builder, "Paste"));
	glade.Delete = GTK_WIDGET(gtk_builder_get_object(glade.builder, "Delete"));
	glade.FFT = GTK_WIDGET(gtk_builder_get_object(glade.builder, "FFT"));
	glade.Spectrogram = GTK_WIDGET(gtk_builder_get_object(glade.builder, "Spectrogram"));
	glade.Original_Data = GTK_WIDGET(gtk_builder_get_object(glade.builder, "Original_Data"));
	glade.Triangle = GTK_WIDGET(gtk_builder_get_object(glade.builder, "Triangle"));
	glade.Welch = GTK_WIDGET(gtk_builder_get_object(glade.builder, "Welch"));
	glade.Sine = GTK_WIDGET(gtk_builder_get_object(glade.builder, "Sine"));
	glade.Hann = GTK_WIDGET(gtk_builder_get_object(glade.builder, "Hann"));
	glade.Hamming = GTK_WIDGET(gtk_builder_get_object(glade.builder, "Hamming"));
	glade.Blackman = GTK_WIDGET(gtk_builder_get_object(glade.builder, "Blackman"));
	glade.Gaussian = GTK_WIDGET(gtk_builder_get_object(glade.builder, "Gaussian"));
	glade.No_window = GTK_WIDGET(gtk_builder_get_object(glade.builder, "No_window"));
	glade.About = GTK_WIDGET(gtk_builder_get_object(glade.builder, "About"));
	glade.Play = GTK_WIDGET(gtk_builder_get_object(glade.builder, "Play"));
	glade.Pause = GTK_WIDGET(gtk_builder_get_object(glade.builder, "Pause"));
	glade.toolbar = GTK_WIDGET(gtk_builder_get_object(glade.builder, "toolbar"));

    g_object_unref(glade.builder);

    gtk_widget_show(glade.window);                

	//----- ENTER THE GTK MAIN LOOP -----
	gtk_main();		//Enter the GTK+ main loop until the application closes.

	return 0;
}

// called when window is closed
void on_window_main_destroy(){
    gtk_main_quit();
}

void on_Quit_activate(GtkMenuItem *menuitem){
    gtk_main_quit();
}

void on_New_activate(GtkMenuItem *menuitem){
	
}

void on_Open_activate(GtkMenuItem *menuitem){
	
}

void on_Save_activate(GtkMenuItem *menuitem){
	
}

void on_Save_as_activate(GtkMenuItem *menuitem){
	
}

void on_Cut_activate(GtkMenuItem *menuitem){
	
}

void on_Copy_activate(GtkMenuItem *menuitem){
	
}

void on_Paste_activate(GtkMenuItem *menuitem){
	
}

void on_Delete_activate(GtkMenuItem *menuitem){
	
}

void on_FFT_toggled(GtkCheckMenuItem *checkmenuitem){
	
}

void on_Spectrogram_toggled(GtkCheckMenuItem *checkmenuitem){
	
}

void on_Original_Data_toggled(GtkCheckMenuItem *checkmenuitem){
	
}

void on_Triangle_toggled(GtkRadioMenuItem *radio) {
	gboolean T = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(radio));
	if(T){
		
	}
	else{
		
	}
}

void on_Welch_toggled(GtkRadioMenuItem *radio) {
	gboolean T = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(radio));
	if(T){

	}
	else{
		
	}
}

void on_Sine_toggled(GtkRadioMenuItem *radio) {
	gboolean T = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(radio));
	if(T){

	}
	else{
		
	}
}

void on_Hann_toggled(GtkRadioMenuItem *radio) {
	gboolean T = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(radio));
	if(T){

	}
	else{
		
	}
}

void on_Hamming_toggled(GtkRadioMenuItem *radio) {
	gboolean T = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(radio));
	if(T){

	}
	else{
		
	}
}

void on_Blackman_toggled(GtkRadioMenuItem *radio) {
	gboolean T = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(radio));
	if(T){

	}
	else{
		
	}
}

void on_Gaussian_toggled(GtkRadioMenuItem *radio) {
	gboolean T = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(radio));
	if(T){

	}
	else{
		
	}
}

void on_No_window_toggled(GtkRadioMenuItem *radio) {
	gboolean T = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(radio));
	if(T){

	}
	else{
		
	}
}

void on_About_activate(GtkMenuItem *menuitem){
	
}

void on_Pause_toggled(GtkToolItem *toolitem){
	
}

void on_Play_toggled(GtkToolItem *toolitem){
	
}

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