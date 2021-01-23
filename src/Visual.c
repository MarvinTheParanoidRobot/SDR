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
int main(int argc, char *argv[]) {
	//---------------------------------
	//----- CREATE THE GTK WINDOW -----
	//---------------------------------
	GtkWidget *MainWindow;
	
	gtk_init(&argc, &argv);
	
	MainWindow= gtk_window_new(GTK_WINDOW_TOPLEVEL); 		//GTK_WINDOW_TOPLEVEL = Has a titlebar and border, managed by the window manager. 
	gtk_window_set_title(GTK_WINDOW(MainWindow), "My Application");
	gtk_window_set_default_size(GTK_WINDOW(MainWindow), 400, 300);		//Size of the the client area (excluding the additional areas provided by the window manager)
	gtk_window_set_position(GTK_WINDOW(MainWindow), GTK_WIN_POS_CENTER);
	gtk_widget_show_all(MainWindow);

	//Close the application if the x button is pressed if ALT+F4 is used
	g_signal_connect(G_OBJECT(MainWindow), "destroy", G_CALLBACK(gtk_main_quit), NULL);



	int NUM_CHANNELS,SAMPLE_RATE,BLOCK_SIZE,NUM_ITERATIONS;
	NUM_CHANNELS=1;SAMPLE_RATE=48000;BLOCK_SIZE=48000;NUM_ITERATIONS=0;

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