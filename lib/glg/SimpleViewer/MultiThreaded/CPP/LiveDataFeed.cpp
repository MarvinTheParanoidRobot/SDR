#include "stdafx.h"
#include "LiveDataFeed.h"
#include "GlgViewer.h"

#ifdef _WINDOWS
# pragma warning( disable : 4996 )    /* Allow cross-platform strcpy */
#endif

/*--------------------------------------------------------------------
| LiveDataFeed class. The application should provide custom implementation
| for the virtual methods to communicate with real-time data
| acquisition system, including Init(), Terminate(), ProcessData(),
| WriteDValue, WriteSValue().
*/
LiveDataFeed::LiveDataFeed( GlgViewer * viewer ) : 
   DataFeedBase( viewer )
{
}

LiveDataFeed::~LiveDataFeed( void )
{
}

/*--------------------------------------------------------------------
| 
*/
void LiveDataFeed::Init( void )
{
   // Place custom data initialization code here as needed.
}

/*--------------------------------------------------------------------
| 
*/
void LiveDataFeed::Terminate( void )
{
   // Place custom data termination code here as needed.
}

/*--------------------------------------------------------------------
| Provide a custom implementation of this method to process incoming
| data and store application specific data structures in the 
| AccumulatedData vector. 
|
| Thread locking is used inside DataFeedBase::StoreRawData() to ensure 
| synchronization for AccumulatedData object.
*/
void LiveDataFeed::ProcessData( void )
{
   /*
      Get data and pass it to the GUI thread using StoreRawData() function.
      For example:

      MyData * data = GetMyData();
      if( data )
        StoreRawData( data );

      Adjust code in GlgViewer::PushData() to process application 
      specific data structures.

      Add code to put the data thread to sleep after processing data if
      the method that receives data is not blocking.
   */
}

/*--------------------------------------------------------------------
| 
*/
GlgBoolean LiveDataFeed::WriteDValue( SCONST char * tag_source, double d_value )
{
   /* Set new tag value of D-type. */
   return GlgTrue;
}


/*--------------------------------------------------------------------
| Simulte the "Write" opearion for S value. For demo purposes, set the value
| of the specified tag source in the drawing. 
*/
GlgBoolean LiveDataFeed::WriteSValue( SCONST char * tag_source, 
                                      SCONST char * s_value )
{
   /* Set new tag value of S-type. */
   return GlgTrue;
}

