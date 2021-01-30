#include <stdio.h>
#include <stdlib.h>

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>

#include "GlgWrapper.h"

#define WIDTH     800
#define HEIGHT    600

/* Function prototypes */
Boolean UpdateGraph( Widget widget );
double GetData( void );

/*----------------------------------------------------------------------
|
| This is a simple program which creates a GLG widget with a bar graph
| in it and fills it with random data. The program uses X Windows API.
| In order to use this program template with a diffrent drawing file,
| replace "bar1.g" with the name of your drawing file and adust resource
| names.
|
*/
int main( int argc, char *argv[] )
{
   XtAppContext AppContext;
   Display * display;
   Widget shell, glgWrapper;
   GlgObject viewport;
   Cardinal ac;
   Arg al[20];
   
   /* Initialize X Toolkit and create an application context. */
   XtToolkitInitialize();
   AppContext = XtCreateApplicationContext();
   
   /* Open a display connection. */
   display =
     XtOpenDisplay( AppContext, NULL, "GlgExample", "Glg", 0, 0, &argc, argv );
   
   /* Create a shell */
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
   
   ac = 0;
   
   /* Take a drawing from a file. */
   XtSetArg( al[ac], XtNglgDrawingFile, "bar1.g" ); ac++;

   /* Create an instance of a GLG Wrapper widget. */
   glgWrapper =
     XtCreateWidget( "GlgWrapper", glgWrapperWidgetClass, shell, al, ac );
   
   XtManageChild( glgWrapper ); 
   XtRealizeWidget( shell );
   
   /* Add a work procedure to update the graph. */
   XtAppAddWorkProc( AppContext,
		    (XtWorkProc)UpdateGraph, (XtPointer)glgWrapper );
   
   XtAppMainLoop( AppContext );
}

/*----------------------------------------------------------------------
| Pushes the next data and label values and updates the graph.
*/
Boolean UpdateGraph( widget )
     Widget widget;
{
   static long iteration_counter = 2; /* A counter used to generate labels. */
   GlgObject viewport;
   char * label;

   viewport = XglgGetWidgetViewport( widget );

   /* Push the next data value, let the graph handle scrolling. */
   XglgSetDResource( viewport, "DataGroup/EntryPoint", GetData() );

   /* Generate the next label to use. */
   label = XglgCreateIndexedName( "Value ", iteration_counter );
   ++iteration_counter;

   /* Push the next label. The graph handles labels scrolling.
    * To set labels directly, use "XLabelGroup/Xlabel<n>/String" as a
    * name of a resource, where <n> is the sequential zero-based label
    * index. In this case you will be responsible for handling label
    * scrolling.
    */
   XglgSetSResource( viewport, "XLabelGroup/EntryPoint", label );

   XglgFree( label );    /* Free the label. */

   XglgUpdate( viewport );               /* Makes changes visible. */
   XSync( XtDisplay( widget ), False );  /* Improves interactive response */

   return False;    /* Return False to continue updating. */
}

/*----------------------------------------------------------------------
| Returns a random number in the [0;1] range. Use a real source of data
| instead of this function.
*/
double GetData()
{
   return GlgRand( 0., 1. );
}

