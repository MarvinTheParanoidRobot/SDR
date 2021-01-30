#include <stdio.h>
#include <stdlib.h>
#include "GlgClass.h"

class GraphExample : public GlgObjectC
{
 public:
   GraphExample( void );
   virtual ~GraphExample( void );

   // Override to supply custom Input methods to handle window closing.
   void Input( GlgObjectC& callback_viewport, GlgObjectC& message );
};

// Function prototypes
extern "C" GlgBoolean UpdateGraph( GlgObjectC* );
double GetData( void );

// Defines a platform-specific program entry point.
#include "GlgMain.h"

/////////////////////////////////////////////////////////////////////////////
// This is a simple program which creates and displays a bar graph,
// filling it with random data. The program uses Generic C++ API.
// In order to use this program template with a different drawing file,
// replace "bar1.g" with the name of your drawing file and adust resource
// names.
/////////////////////////////////////////////////////////////////////////////
int GlgMain( int argc, char *argv[], GlgAppContext InitAppContext )
{
   GlgSessionC glg_session( False, InitAppContext, argc, argv );
   GraphExample graph_example;	

   // Create an instance of a GLG Wrapper widget.
   graph_example.LoadWidget( "bar1.g" );

   // Enable input and selection callbacks
   graph_example.EnableCallback( GLG_INPUT_CB );
   
   graph_example.InitialDraw();

   // Add a work procedure to update the graph.
   GlgAddWorkProc( glg_session.GetAppContext(), (GlgWorkProc)UpdateGraph,
		  (GlgAnyType)&graph_example );
   
   return (int) GlgMainLoop( glg_session.GetAppContext() );
}

/////////////////////////////////////////////////////////////////////////////
// Pushes the next data and label values and updates the graph.
/////////////////////////////////////////////////////////////////////////////
extern "C" GlgBoolean UpdateGraph( GlgObjectC * graph_example )
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

   GlgFree( label );    // Free the label.

   graph_example->Update();   // Makes changes visible.
   graph_example->Sync();     // Improves interactive response.

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

/////////////////////////////////////////////////////////////////////////////
// Handles window closing
/////////////////////////////////////////////////////////////////////////////
void GraphExample::Input( GlgObjectC& viewport, GlgObjectC& message )
{
   CONST char
     * format,
     * action;
   message.GetResource( "Format", &format );
   message.GetResource( "Action", &action );

   // Handle window closing. May use viewport's name.
   if( strcmp( format, "Window" ) == 0 &&
      strcmp( action, "DeleteWindow" ) == 0 )
     exit( 0 );
}

GraphExample::GraphExample( void )
{
}

GraphExample::~GraphExample( void )
{
}
