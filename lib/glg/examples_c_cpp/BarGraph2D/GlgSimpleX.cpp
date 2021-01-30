#include <stdio.h>
#include <stdlib.h>

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>

#include "GlgClass.h"

#define WIDTH     800
#define HEIGHT    600

// Function prototypes
Boolean UpdateGraph( GlgWrapperC* );
double GetData( void );

/////////////////////////////////////////////////////////////////////////////
// This is a simple program which creates a GLG widget with a bar graph
// in it and fills it with random data. The program uses X Windows C++ API.
// In order to use this program template with a different drawing file,
// replace "bar1.g" with the name of your drawing file and adust resource
// names.
/////////////////////////////////////////////////////////////////////////////
int main( int argc, char *argv[] )
{
   XtAppContext AppContext;
   Display * display;
   Widget shell;
   Cardinal ac;
   Arg al[20];
   GlgWrapperC glg_wrapper;
   
   // Initialize X Toolkit and create an application context.
   XtToolkitInitialize();
   AppContext = XtCreateApplicationContext();
   
   // Open a display connection.
   display =
     XtOpenDisplay( AppContext, 0, "GlgExample", "Glg", 0, 0, &argc, argv );
   
   // Create a shell.
   ac = 0;
   XtSetArg( al[ac], XtNbaseWidth, WIDTH ); ac++;
   XtSetArg( al[ac], XtNbaseHeight, HEIGHT ); ac++;
   XtSetArg( al[ac], XtNwidth, WIDTH ); ac++;
   XtSetArg( al[ac], XtNheight, HEIGHT ); ac++;
   XtSetArg( al[ac], XtNminWidth, 10 ); ac++;
   XtSetArg( al[ac], XtNminHeight, 10 ); ac++;
   XtSetArg( al[ac], XtNallowShellResize, True ); ac++;
   shell =
     XtAppCreateShell( "GlgExample", "Glg", applicationShellWidgetClass,
		      display, al, ac );
   
   // Create an instance of a GLG Wrapper widget.
   glg_wrapper.Create( "bar1.g", shell );
   
   XtRealizeWidget( shell );

   glg_wrapper.GetViewport();    // Get viewport object after realizing.
   
   // Add a work procedure to update the graph.
   XtAppAddWorkProc( AppContext,
		    (XtWorkProc)UpdateGraph, (XtPointer)&glg_wrapper );
   
   XtAppMainLoop( AppContext );
}

/////////////////////////////////////////////////////////////////////////////
// Pushes the next data and label values and updates the graph.
/////////////////////////////////////////////////////////////////////////////
Boolean UpdateGraph( GlgWrapperC * wrapper )
{
   static long iteration_counter = 2; // A counter used to generate labels.
   GlgObject viewport;
   char * label;

   // Push the next data value, let the graph handle scrolling.
   wrapper->SetResource( "DataGroup/EntryPoint", GetData() );

   // Generate the next label to use.
   label = GlgCreateIndexedName( "Value ", iteration_counter );
   ++iteration_counter;

   // Push the next label. The graph handles labels scrolling.
   // To set labels directly, use "XLabelGroup/Xlabel<n>/String" as a
   // name of a resource, where <n> is the sequential zero-based label
   // index. In this case you will be responsible for handling label
   // scrolling.
   //
   wrapper->SetResource( "XLabelGroup/EntryPoint", label );

   GlgFree( label );    // Free the label.

   wrapper->Update();   // Makes changes visible.
   wrapper->Sync();     // Improves interactive response.

   return False;    // Return False to continue updating.
}

/////////////////////////////////////////////////////////////////////////////
// Returns a random number in the [0;1] range. Use a real source of data
// instead of this function.
/////////////////////////////////////////////////////////////////////////////
double GetData()
{
   return GlgRand( 0., 1. );
}

