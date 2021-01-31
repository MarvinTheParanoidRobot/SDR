#include <stdio.h>
#include <stdlib.h> 
#include "GlgClass.h"

// Set this defined constant to be 1 to use code generated by the
// GLG Code Generation Utility. Save the drawing uncompressed to 
// generate code.
//
#define USE_GENERATED_CODE     0

GlgAppContext AppContext;

class GraphExample : public GlgObjectC
{
 public:
 
   GraphExample( void );
   virtual ~GraphExample( void );

   // Time interval for periodic dynamic updates, in millisec. 
   GlgLong TimeInterval; 

   // Override to supply custom Input and Selection methods
   void Input( GlgObjectC& callback_viewport, GlgObjectC& message );
};

// Function prototypes
extern "C" void UpdateGraph( GraphExample*, GlgLong * );
double GetData( void );


#if USE_GENERATED_CODE
// The following symbols should be defined in the file generated by the
// GLG Code Generation Utility.
//
extern long GraphData[];
extern long GraphDataSize;
#endif

// Defines a platform-specific program entry point.
#include "GlgMain.h"

///////////////////////////////////////////////////////////////////////////
//
// This program illustrates using most of the GLG Widget's features with
// GLG generic API. The program creates a GLG widget with a bar graph in it
// and fills it with random data, using resources to change graph's
// attributes. 
// 
// The widget's Input callback is used to handle the user feedback
// via the use of a Custom MouseClickEvent added to the DataSample
// object in the Builder.
//
// When a user selects a particular bar (DataSample) with the
// mouse, the DataSample's name and value get displayed in a 
// text object.
//
// In order to use this program template with a diffrent drawing file,
// replace "bar_graph.g" with the name of your drawing file and adjust 
// resource names.
///////////////////////////////////////////////////////////////////////////
int GlgMain( int argc, char *argv[], GlgAppContext InitAppContext )
{
   GlgSessionC glg_session( False, InitAppContext, argc, argv );
   GraphExample graph_example;
   
   AppContext = glg_session.GetAppContext();

   // Create a GLG object either from a file or a generated image.
#if USE_GENERATED_CODE
   // Use a generated drawing image.
   graph_example.LoadWidget( GraphData, GraphDataSize );
#else
   // Take a drawing from the file.
   graph_example.LoadWidget( "bar_graph.g" );
#endif
   
   // Set some optional resources for the initial appearance of the widget.
   // Doing it before realizing the widget allows setting resources before
   // the drawing hierarchy is created.
   // To avoid setting these resources every time on start up, use the
   // Editor to save the customized version of the widget with the desired
   // values of resources.

   // Set the background color of the widget to be white.
   //
   graph_example.SetResource( "FillColor", 1., 1., 1. );
   
   // Set the number of datasamples in the graph to 30.
   graph_example.SetResource( "DataGroup/Factor", 30. );
   
   // Set the number of X labels and minor ticks
   graph_example.SetResource( "XLabelGroup/Factor", 6. );
   graph_example.SetResource( "XLabelGroup/MinorFactor", 5. );

   // Set the initial value of all bars to be 0 on the initial appearance.
   graph_example.SetResource( "DataGroup/DataSample/Value", 0. );
   
   // Set all labels to display empty strings on the initial appearance.
   graph_example.SetResource( "XLabelGroup/XLabel/String", "" );
   
   // Set the fill color of all bars of a bar graph to be green by
   // setting the color of a template datasample.
   //
   graph_example.SetResource( "DataGroup/DataSample/FillColor", 0., 1., 0. );
   
   // Set the colors of individual bars to be independent.
   graph_example.SetResource( "DataGroup/DataSample/FillColor/Global", 0. );

   // Make  the SelectedBarLabel object invisible.
   graph_example.SetResource( "SelectedBarLabel/Visibility", 0.0 );

   // Set widget dimensions using world coordinates [-1000;1000].
   // If not set, default dimensions will be used as set in the GLG editor.
   graph_example.SetResource( "Point1", -500., -500., 0. );
   graph_example.SetResource( "Point2", 400., 400., 0. );

   // Add input callback for handling user feedback.
   graph_example.EnableCallback( GLG_INPUT_CB );
 
   graph_example.InitialDraw();

   // Set the colors of the second bar to blue. This resource is accessible
   // only after the hierarchy has been setup.
   graph_example.SetResource( "DataGroup/DataSample2/FillColor", 0., 0., 1. );

   // Add a timer procedure to update the graph.
   GlgAddTimeOut( AppContext,  graph_example.TimeInterval, 
		  (GlgTimerProc)UpdateGraph, (GlgAnyType)&graph_example );
   
   return (int) GlgMainLoop( AppContext );
}

/////////////////////////////////////////////////////////////////////////////
// Pushes the next data and label values and updates the graph.
/////////////////////////////////////////////////////////////////////////////
extern "C" void UpdateGraph( GraphExample * graph_example, GlgLong *timer_id )
{
   static long iteration_counter = 2; // A counter used to generate labels.
   char * label;

   // Push the next data value, let the graph handle scrolling.
   graph_example->SetResource( "DataGroup/EntryPoint", GetData() );

   // Generate the next label to use.
   label = GlgCreateIndexedName( "Value ", iteration_counter );
   ++iteration_counter;

   // Push the next label. The graph handles labels scrolling.
   // To set labels directly, use "XLabelGroup/Xlabel<n>/String" as a
   // name of a resource, where <n> is the sequential zero-based label
   // index. In this case you will be responsible for handling label
   // scrolling.
   //
   graph_example->SetResource( "XLabelGroup/EntryPoint", label );

   // Store the label in a datasample as well to display it when 
   // the datasample is selected. The stock graph was moditied to add 
   // this additional entry point.
   graph_example->SetResource( "DataGroup/SampleNameEntryPoint", label );

   GlgFree( label );           // Free the label.

   graph_example->Update();    // Makes changes visible.

     /* Reinstall the timeout to continue updating */
   GlgAddTimeOut( AppContext, graph_example->TimeInterval, 
				  (GlgTimerProc)UpdateGraph,
				  (GlgAnyType)graph_example );
}

/////////////////////////////////////////////////////////////////////////////
// Returns a random number in the [0;1] range. Use a real source of data
// instead of this function.
/////////////////////////////////////////////////////////////////////////////
double GetData()
{
   return GlgRand( 0., 1. );
}

/////////////////////////////////////////////////////////////////////////////
// In this example, Input callback is used to process Custom MouseClick 
// events for the DataSamples. When the user selects a DataSample with 
// the mouse, its value will be displayed in the text object named 
// SelectedBarLabel.
/////////////////////////////////////////////////////////////////////////////
void GraphExample::Input( GlgObjectC& viewport, GlgObjectC& message )
{
   double bar_value;
   CONST char
      * format,
      * action,
      * sample_name,
      * event_label;

   message.GetResource( "Format", &format );
   message.GetResource( "Action", &action );

   // Handle window closing. May use viewport's name.
   if( strcmp( format, "Window" ) == 0 &&
      strcmp( action, "DeleteWindow" ) == 0 )
     exit( 0 );

   // Process CustomEvents
   if( strcmp( format, "CustomEvent" ) == 0 &&
       strcmp( action, "MouseClick" ) == 0 )
   {
      message.GetResource( "EventLabel", &event_label );
      if( strcmp( event_label, "BarSelected" ) == 0 ) //Bar selected
      {
	 // The DataSample was selected. Retrieve its value and sample name.
         // The stock graph was modified by adding a SampleName custom propery
         // to the graph's datasample. If a stock graph is used, use the 
         // Object/Name resource instead of Object/SampleName to retrieve 
         // the name of the datasample object instead of the stored label.
         //
	 message.GetResource( "Object/Value", &bar_value );
	 message.GetResource( "Object/SampleName", &sample_name );
	 
	 // Display the bar name and value in a text object 
	 // named "SelectedBarLabel".
	 viewport.SetResource( "SelectedBarLabel/DataSampleValue", bar_value );
	 viewport.SetResource( "SelectedBarLabel/DataSampleName", sample_name );
	 
	 // Make SelectedBarLabel object visible.
	 viewport.SetResource( "SelectedBarLabel/Visibility", 
			       1.0 );
      }
   }
   viewport.Update(); //update the display
}

GraphExample::GraphExample( void )
{
	TimeInterval = 500;
}

GraphExample::~GraphExample( void )
{
}