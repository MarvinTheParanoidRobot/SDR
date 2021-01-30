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
// SYMBOLIC CONSTANTS
//====================================================================
#define _USE_MATH_DEFINES
//====================================================================
// GLOBAL VARIABLES
//====================================================================

//====================================================================
// FUNCTION DECLARATIONS
//====================================================================
void on_window_main_destroy();

int main(int argc, char *argv[]) {
	testsdr();
	GtkBuilder      *builder; 
    GtkWidget       *window;

    gtk_init(&argc, &argv);

    builder = gtk_builder_new();
    gtk_builder_add_from_file (builder, "../GUI/sdr_window.glade", NULL);

    window = GTK_WIDGET(gtk_builder_get_object(builder, "window_main"));
    gtk_builder_connect_signals(builder, NULL);

    g_object_unref(builder);

    gtk_widget_show(window);                


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
	//----- ENTER THE GTK MAIN LOOP -----
	gtk_main();		//Enter the GTK+ main loop until the application closes.

	return 0;
}

// called when window is closed
void on_window_main_destroy(){
    gtk_main_quit();
}

void on_quit_activate(GtkMenuItem *menuitem)
{
    gtk_main_quit();
}