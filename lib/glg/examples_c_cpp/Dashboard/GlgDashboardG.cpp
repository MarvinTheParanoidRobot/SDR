#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "GlgClass.h"

/* If set to true, tags defined in the drawing are used for animation.
   Otherwise, object resources are used to push real-time values into the
   drawing.
*/
#define USE_TAGS True

/* Update interval, msec. */
#define  UPDATE_INTERVAL     100

#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950
#endif

GlgAppContext AppContext;

class GlgDashboard : public GlgObjectC
{
 public:   
   GlgDashboard( void );
   virtual ~GlgDashboard( void );

   // Input callback.
   virtual void Input( GlgObjectC& callback_viewport, GlgObjectC& message );

   void InitializeDrawing( void );
   void UpdateDrawing( void );
   void SetSize( GlgLong x, GlgLong y, GlgLong width, GlgLong height );
   void StopUpdates( void );
   void StartUpdates( void );
   double GetData( double low, double high );
   
 public:
   GlgLong TimerID;
};

// Function prototype.
extern "C" void OnTimerEvent( GlgDashboard * dashboard, GlgLong * timer_id );

// Defines a platform-specific program entry point.
#include "GlgMain.h"

/////////////////////////////////////////////////////////////////////////////
// Main Entry point.
/////////////////////////////////////////////////////////////////////////////
int GlgMain( int argc, char *argv[], GlgAppContext InitAppContext )
{
   GlgSessionC glg_session( False, InitAppContext, argc, argv );
   GlgDashboard glg_dashboard;

   AppContext = glg_session.GetAppContext();

   // Load a GLG  drawing from a file.
   glg_dashboard.LoadWidget( "dashboard.g" );

   if( glg_dashboard.IsNull() )
   {
      GlgError( GLG_USER_ERROR, (char *) "Can't load drawing." );
      exit( GLG_EXIT_ERROR );
   }

   /* Set widget dimensions. If not set, default dimensions will 
      be used as set in the GLG editor.
   */
   glg_dashboard.SetSize( 0, 0, 550, 500 );

   // Setting window title. */
   glg_dashboard.SetResource( "ScreenName", "GLG Dashboard Example" );

   // Enable input callback.
   glg_dashboard.EnableCallback( GLG_INPUT_CB );
 
   // Set initial drawing parameters.
   glg_dashboard.InitializeDrawing();

   // Display GLG window.
   glg_dashboard.InitialDraw();

   /* Start periodic dynamic updates. */
   glg_dashboard.StartUpdates();

   return (int) GlgMainLoop( AppContext );
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
   TimerID = GlgAddTimeOut( AppContext, UPDATE_INTERVAL,
                            (GlgTimerProc) OnTimerEvent, 
                            (GlgAnyType) this );
}

/////////////////////////////////////////////////////////////////////////////
// Stop dynamic updates.
/////////////////////////////////////////////////////////////////////////////
void GlgDashboard::StopUpdates()
{
   /* Stop update timers */
   if( TimerID )
   {
      GlgRemoveTimeOut( TimerID );
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
     GlgAddTimeOut( AppContext, UPDATE_INTERVAL, 
                    (GlgTimerProc) OnTimerEvent, dashboard );
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
   
   if( USE_TAGS ) // Use tags for animation.
   {
      // Push values to the objects using tags defined in the drawing.
      SetTag( "Voltage", voltage, GlgTrue /*if_changed*/ );
      SetTag( "Current", current, GlgTrue /*if_changed*/ );
   }
   else // Use resources for animation.
   {
      // Push values to the objects using resource paths.
      SetResource( "DialVoltage/Value", voltage );
      SetResource( "DialAmps/Value", current );
   }
   
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
| Set viewport size in screen cooridnates. 
*/
void GlgDashboard::SetSize( GlgLong x, GlgLong y, 
                            GlgLong width, GlgLong height )
{
   SetResource( "Point1", 0., 0., 0. );
   SetResource( "Point2", 0., 0., 0. );

   SetResource( "Screen/XHint", (double) x );
   SetResource( "Screen/YHint", (double) y );
   SetResource( "Screen/WidthHint", (double) width );
   SetResource( "Screen/HeightHint", (double) height );
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



