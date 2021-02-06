#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>

#include "GlgClass.h"

#define WIDTH     550
#define HEIGHT    500

/* Update interval, msec. */
#define  UPDATE_INTERVAL     100

#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950
#endif

XtAppContext AppContext;         // Appplication context

class GlgDashboard : public GlgWrapperC
{
 private:
   // Disallow assigments and copying a widget.
   GlgDashboard& operator= ( const GlgDashboard& object );
   GlgDashboard( GlgDashboard& object );

 public:
   GlgDashboard( void );
   virtual ~GlgDashboard( void );

   // Override to supply custom Input method
   void Input( GlgObjectC& callback_viewport, GlgObjectC& message );

   void InitializeDrawing( void );
   void UpdateDrawing( void );
   void StopUpdates( void );
   void StartUpdates( void );
   double GetData( double low, double high );

public:
   GlgLong TimerID;
};

// Function prototype.
extern "C" void OnTimerEvent( GlgDashboard * dashboard, GlgLong * timer_id );

//////////////////////////////////////////////////////////////////////////
// Main Entry point.
//////////////////////////////////////////////////////////////////////////
int main( int argc, char *argv[] )
{
   Display * display;
   Widget shell, form;
   Cardinal ac;
   Arg al[20];
   GlgDashboard glg_dashboard;
 
   // Initialize X Toolkit and create an application context.
   XtToolkitInitialize();
   AppContext = XtCreateApplicationContext();
   
   // Open a display connection.
   display =
     XtOpenDisplay( AppContext, 0, "GlgControlPanel", "Glg", 0, 0, &argc, argv );
   
   // Create a shell.
   ac = 0;
   XtSetArg( al[ac], XtNbaseWidth, WIDTH ); ac++;
   XtSetArg( al[ac], XtNbaseHeight, HEIGHT ); ac++;
   XtSetArg( al[ac], XtNwidth, WIDTH ); ac++;
   XtSetArg( al[ac], XtNheight, HEIGHT ); ac++;
   XtSetArg( al[ac], XtNminWidth, 10 ); ac++;
   XtSetArg( al[ac], XtNminHeight, 10 ); ac++;
   XtSetArg( al[ac], XtNallowShellResize, True ); ac++;
   XtSetArg( al[ac], XtNinput, TRUE ); ac++;
   shell =
     XtAppCreateShell( "GlgExample", "Glg", applicationShellWidgetClass,
		      display, al, ac );
   
   // Create a GLG widget and specify a drawing filename to be displayed
   glg_dashboard.Create( "dashboard.g", shell );
   
   // Enable Input callback
   glg_dashboard.EnableCallback( GLG_INPUT_CB );
         
   XtRealizeWidget( shell );

   glg_dashboard.GetViewport();   // Get the viewport object after realizing.
   glg_dashboard.InitializeDrawing();

   // Start periodic dynamic updates.
   glg_dashboard.StartUpdates();

   XtAppMainLoop( AppContext );
}

/////////////////////////////////////////////////////////////////////////////
GlgDashboard::GlgDashboard( void )
{
   TimerID = 0;
}

/////////////////////////////////////////////////////////////////////////////
GlgDashboard::~GlgDashboard( void )
{
   StopUpdates();
}

/////////////////////////////////////////////////////////////////////////////
// Start periodic dynamic updates.
/////////////////////////////////////////////////////////////////////////////
void GlgDashboard::StartUpdates()
{
   TimerID = XtAppAddTimeOut( AppContext, UPDATE_INTERVAL, 
                              (XtTimerCallbackProc) OnTimerEvent, 
                              (XtPointer) this );
}

/////////////////////////////////////////////////////////////////////////////
// Stop dynamic updates.
/////////////////////////////////////////////////////////////////////////////
void GlgDashboard::StopUpdates()
{
   /* Stop update timers */
   if( TimerID )
   {
      XtRemoveTimeOut( TimerID );
      TimerID = 0;
   }
}

/////////////////////////////////////////////////////////////////////////////
// Timer procedure, invoked periodically on a timer to update the
// drawing with new data values.
/////////////////////////////////////////////////////////////////////////////
void OnTimerEvent( GlgDashboard * dashboard, GlgLong * timer_id )
{
   dashboard->UpdateDrawing();

   /* Restart the timer. */
   dashboard->TimerID = 
     XtAppAddTimeOut( AppContext, UPDATE_INTERVAL, 
                    (XtTimerCallbackProc) OnTimerEvent, dashboard );
}

/////////////////////////////////////////////////////////////////////////////
// Set initial parameters for GLG objects/widgets.
/////////////////////////////////////////////////////////////////////////////
void GlgDashboard::InitializeDrawing()
{
   // Set initial patameters as needed.
   SetResource( "DialPressure/Low", 0. );
   SetResource( "DialVoltage/Low", 0. );
   SetResource( "DialAmps/Low", 0. );
   SetResource( "SliderPressure/Low", 0. );
   
   SetResource( "DialPressure/High", 50. );
   SetResource( "DialVoltage/High", 120. );
   SetResource( "DialAmps/High", 10. );
   SetResource( "SliderPressure/High", 50. );
}

/////////////////////////////////////////////////////////////////////////////
// Periodic dynamic updates.
/////////////////////////////////////////////////////////////////////////////
void GlgDashboard::UpdateDrawing()
{
   /* Obtain simulated demo data values in a specified range.
      The application should provide a custom implementation
      of the data acquisition interface to obtain real-time
      data values.
   */
   double voltage = GetData( 0.0, 120.0 );
   double current = GetData( 0.0, 10.0 );
   
   // Push values to the objects using resource paths.
   SetResource( "DialVoltage/Value", voltage );
   SetResource( "DialAmps/Value", current );
   
   Update();    /* Make changes visible. */
   Sync();
}

/////////////////////////////////////////////////////////////////////////////
// This callback is invoked when user interacts with input objects in GLG
// drawing, such as a slider, dial or a button.
/////////////////////////////////////////////////////////////////////////////
void GlgDashboard::Input( GlgObjectC& viewport, GlgObjectC& message )
{
   CONST char
     * format,
     * action,
     * origin;
      
   // Get the message's format, action and origin.
   message.GetResource( "Format", &format );
   message.GetResource( "Action", &action );
   message.GetResource( "Origin", &origin );

   // Handle window closing. May use viewport's name.
   if( strcmp( format, "Window" ) == 0 &&
      strcmp( action, "DeleteWindow" ) == 0 )
     exit( 0 );

   // Input event occurred in a button.
   if( strcmp( format, "Button" ) == 0 )
   {
      if( strcmp( action, "Activate" ) == 0 )         //Push button events.
      {
         // User selected a Quit button: exit the program.
         if( strcmp( origin , "QuitButton" ) == 0 )
           exit( 0 );
      }
      else if( strcmp( action, "ValueChanged" ) == 0 ) //Toggle button events.
      {
         if( strcmp( origin, "StartButton" ) == 0 )
         {
            double value;
            message.GetResource( "OnState", &value );

            switch( (int) value)
            {
             case 0:
               StopUpdates();   //Stop updates.
               break;
             case 1:
               StartUpdates();  //Start updates.
               break;
             default: break;
            }
         }
      }
      
      Update();
   }
   // Input occurred in a slider. 
   else if( strcmp( format, "Slider" ) == 0 &&
            strcmp( origin, "SliderPressure" ) == 0 )
   {
      double slider_value;
	      
      // Retreive current slider value from a message object.
      message.GetResource( "ValueY", &slider_value );

      // Set a data value for a dial control
      SetResource( "DialPressure/Value", slider_value );

      // Update the viewport to reflect new resource settings
      Update();  
   }
}

/*----------------------------------------------------------------------
| Generates demo data value within a specified range. 
| An application can replace code in this function to supply 
| real-time data from a custom data source.
*/
double GlgDashboard::GetData( double low, double high )
{
   double
     half_amplitude, center,
     period,
     value,
     alpha;
   
   static int counter = 0;

   half_amplitude = ( high - low ) / 2.0;
   center = low + half_amplitude;
   
   period = 100.0;
   alpha = 2.0 * M_PI * counter / period;
   
   value = center +
     half_amplitude * sin( alpha ) * sin( alpha / 30.0 );
   
   ++counter;
   return value;
}
