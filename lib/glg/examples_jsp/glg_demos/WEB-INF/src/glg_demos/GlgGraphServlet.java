package glg_demos;

import java.io.*;
import java.net.*;
import javax.servlet.*;
import javax.servlet.http.*;
import javax.imageio.*;
import java.awt.image.*;
import com.genlogic.*;

public final class GlgGraphServlet extends HttpServlet 
   implements GlgErrorHandler
{
   static final long serialVersionUID = 0;

   // GLG graph types used in the demo.
   final static int BAR_GRAPH = 0;
   final static int LINE_GRAPH = 1;
   final static int FILLED_LINE_GRAPH = 2;
   final static int MULTI_LINE_GRAPH = 3;
   final static int STACKED_BAR_GRAPH = 4;

   // Inner class to keep all properties of the loaded graph object.
   class GraphData
   {
      GlgObject graph;      // Drawing
      int glg_graph_type;   // GLG graph type
      char graph_time_axis; // 'X' for horiz. graphs or 'Y' for vertical.
      int num_values;       // Number of values in a multi-value graphs.
      GlgDemoDataSource datasource;   // Graph's datasource.
   }

   // The servlet handles several graph types, using a different drawing
     // for each graph.
   static final int NUM_GRAPHS = 5;

   // Drawing filenames.
   static String drawing_names[] = 
     { "graph1.g", "graph2.g", "graph3.g", "graph4.g", "graph5.g" };

   // Arrays to keep loaded graph drawing objects and their properties.
   static GraphData graph_array[] = new GraphData[ NUM_GRAPHS ];

   // Legend labels for multi-value graphs.
   static final String multi_line_labels[] = { "server1", "server2", "server3" };
   static final String stacked_bar_labels[] = { ".com", ".net", "other" };

   /////////////////////////////////////////////////////////////////
   // A wrapper around the main method, doGet2(), to properly handle
   // the access synchronization and unlocking on an error.
   /////////////////////////////////////////////////////////////////
   public void doGet( HttpServletRequest request, 
                      HttpServletResponse response ) 
      throws ServletException
   { 
      try
      {
         doGet2( request, response );
      } 
      catch( Exception e ) 
      {
         // Unlock if was interrupted by the exception while locked.
         GlgObject.UnlockThread();

         throw new ServletException( e );  // Re-throw to log an error
      }

      // Unlock just in case the code did not do it due to a programming error.
      GlgObject.UnlockThread();
   }

   /////////////////////////////////////////////////////////////////
   // Main servlet's method: everything is handled here.
   /////////////////////////////////////////////////////////////////
   public void doGet2( HttpServletRequest request, 
                      HttpServletResponse response ) 
   {
      InitGLG();   // Init the Toolkit

      // Get requested width/height of the image.
      int width = GetIntegerParameter( request, "width", 500 );
      int height = GetIntegerParameter( request, "height", 300 );

      // Limit max. size to avoid running out of heap space creating an image.
      if( width > 1000 ) width = 1000;       
      if( height > 1000 ) height = 1000;       

      // Get graph type.
      int graph_type = GetIntegerParameter( request, "graph_type", 0 );
      if( graph_type < 0 || graph_type >= NUM_GRAPHS )
      {
         Log( "Invalid graph type, using 0." );
         graph_type = 0;
      }

      // This servlet reuses the same drawings between all servlets' threads.
      // Therefore lock to synchronize and prevent other servlets from 
      // changing the drawing size, etc., before we are done.
      GlgObject.Lock();

      GlgObject graph;      
      GraphData graph_data = graph_array[ graph_type ];

      // Load the drawing just once and share it between all servlets and 
      // threads. Alternatively, each servlet may load its own drawing.
      //
      if( graph_data == null )    // First time: load the drawing.
      {
         // Load the graph using a path relative to the servlet app's dir.
         String drawing_name = "/drawings/" + drawing_names[ graph_type ];
         graph = LoadDrawing( drawing_name );
         graph.SetImageSize( width, height );

         // Set graph's properties and create a demo datasource to update it.
         graph_data = InitializeGraph( graph, graph_type );

         // Store the drawing and graph data for reuse.
         graph_array[ graph_type ] = graph_data;         
      }
      else   // Already loaded, reuse the drawing.
      {
         graph = graph_data.graph;
         graph.SetImageSize( width, height );
      }

      // Use title from the request if supplied as a parameter.
      String title = request.getParameter( "title" );
      if( title != null )
        graph.SetSResource( "Title/String", title );
      else
        graph.SetSResource( "Title/String", "Server Load" );

      // Push new data into the graph.
      UpdateGraphData( graph_type );

      // Setup after data update to prepare to generate image.
      graph.SetupHierarchy();

      // Create an image of the graph's graphics.
      BufferedImage image = (BufferedImage) graph.CreateImage( null );

      GlgObject.Unlock();
         
      // Write the image
      try
      {
         response.setContentType("image/png");
         OutputStream out_stream = response.getOutputStream();
         ImageIO.write( image, "png", out_stream );
         out_stream.close();
      }
      catch( IOException e )
      {
         // Log( "Aborted writing of image file." );
      }
   }

   /////////////////////////////////////////////////////////////////
   // Helper methods
   /////////////////////////////////////////////////////////////////

   int GetIntegerParameter( HttpServletRequest request, String name, 
                           int default_value )
   {
      String parameter_string = request.getParameter( name );
      if( parameter_string == null || parameter_string.equals( "" ) )
        return default_value;

      try
      {
         return Integer.parseInt( parameter_string );
      }
      catch( NumberFormatException e )
      {
         Log( "Invalid parameter value for: " + name + 
             " = " + parameter_string );
         return default_value;
      }
   }

   /////////////////////////////////////////////////////////////////
   void Log( String msg )
   {
      getServletContext().log( "GlgGraphServlet: " + msg );
   }

   // GlgErrorHandler interface method for error handling.
   public void Error( String message, int error_type, Exception e )
   {
      Log( message );   // Log errors

      Log( GlgObject.GetStackTraceAsString() ); // Print stack for debugging
   }

   /////////////////////////////////////////////////////////////////
   void InitGLG()
   {
      // Set an error handler to log errors.
      GlgObject.SetErrorHandler( this );

      GlgObject.Init();    // Init GlgToolkit
   }

   /////////////////////////////////////////////////////////////////
   GlgObject LoadDrawing( String drawing_name )
   {
      GlgObject drawing;

      // Get drawing URL relative to the servlet's directory.
      URL drawing_url = null; 
      try
      {
         drawing_url = 
            getServletConfig().getServletContext().getResource( drawing_name );
      }
      catch( MalformedURLException e )
      {
         Log( "Malformed URL: " + drawing_name );
         return null;
      }

      if( drawing_url == null )
      {
         Log( "Can't find drawing: " + drawing_name );
         return null;
      }

      // Load drawing from the URL
      drawing = GlgObject.LoadWidget( drawing_url.toString(), GlgObject.URL );
      if( drawing == null )
      {
         Log( "Can't load drawing: " + drawing_name );
         return null;
      }

      // Disable viewport border in the image: let html define it if needed.
      drawing.SetDResource( "LineWidth", 0. );

      return drawing;
   }

   /////////////////////////////////////////////////////////////////
   // Demonstrates how to set various graph properties: 
   // number of datasamples, labels, major and minor ticks, etc. 
   // Alternatively, all graph properties other than the data may be 
   // defined in the drawing.
   /////////////////////////////////////////////////////////////////
   GraphData InitializeGraph( GlgObject graph, int graph_index )
   {
      // Hardcoded data just to demo a graph
      int num_major_ticks = 5;
      int num_minor_ticks = 4;
      double low = 0.;
      double high = 100.;

      GraphData graph_data = new GraphData();
      graph_data.graph = graph;

      DetermineGraphType( graph_data );

      int num_datasamples = num_major_ticks * num_minor_ticks;

      // Number of labels and major ticks.
      graph.SetDResource( "XLabelGroup/Factor", num_major_ticks );

      // Number of minor ticks.
      graph.SetDResource( "XLabelGroup/MinorFactor", num_minor_ticks );

      // Set the number of datasamples, 
      // should match num_major_tick * num_minor_ticks.
      switch( graph_data.glg_graph_type )
      {
       default:
         graph.SetDResource( "DataGroup/Factor", num_datasamples );
         break;

       case MULTI_LINE_GRAPH:
         graph.SetDResource( "DataGroupOne/DataGroup/Factor", num_datasamples );
         break;
      }

      // Set the range of graph's value axis. 
      if( graph_data.graph_time_axis == 'X' )   // Value axis is Y
      {
         graph.SetDResource( "YLabelGroup/YLabel/Low", low );
         graph.SetDResource( "YLabelGroup/YLabel/High", high );
      }
      else   // Value axis is X
      {
         graph.SetDResource( "XLabelGroup/XLabel/Low", low );
         graph.SetDResource( "XLabelGroup/XLabel/High", high );
      }

      // For multi-line graph, the range each line may be different and
        // is set separately from the range of the value axis.
      if( graph_data.glg_graph_type == MULTI_LINE_GRAPH )
      {
         graph.SetDResource( "DataGroupOne/DataGroup/Marker/DataSample/Low", 
                            low );
         graph.SetDResource( "DataGroupOne/DataGroup/Marker/DataSample/High", 
                            high );
      }

      // Set initial labels to ""
      if( graph_data.graph_time_axis == 'X' )
        graph.SetSResource( "XLabelGroup/XLabel/String", "" );
      else
        graph.SetSResource( "YLabelGroup/YLabel/String", "" );

      // A value outside the graph.
      double out_value = low - ( high - low );

      // Set initial values.
      switch( graph_data.glg_graph_type )
      {         
       default:
       case BAR_GRAPH:
         graph.SetDResource( "DataGroup/DataSample/Value", low );
         break;

       case LINE_GRAPH:
         graph.SetDResource( "DataGroup/Marker/DataSample/Value", out_value );
         break;

       case FILLED_LINE_GRAPH:
         graph.SetDResource( "DataGroup/Marker/DataSample/Value", low );
         break;

       case MULTI_LINE_GRAPH:
         graph.SetDResource( "DataGroupOne/DataGroup/Marker/DataSample/Value",
                            out_value );
         break;       

       case STACKED_BAR_GRAPH:
         graph.SetDResource( "DataGroup/Pack/DataSample/Low", low );
         break;
      }

      // Set title and X/Y axis labels
      graph.SetSResource( "Title/String", "Server Load" );
      
      if( graph_data.graph_time_axis == 'X' )
      {
         graph.SetSResource( "YAxisLabel/String", "Load, %" );
         graph.SetSResource( "XAxisLabel/String", "Time" );
      }
      else
      {
         graph.SetSResource( "XAxisLabel/String", "Time" );
         graph.SetSResource( "YAxisLabel/String", "Load, %" );
      }

      // Set legend for multi-value graphs (stacked bar and multi-line)
      if( graph_data.num_values > 1 )        
        for( int i=0; i<graph_data.num_values; ++i )
          graph.SetSResource( "LegendObject/LegendGroup/Legend/Text" + i,
                             graph_data.glg_graph_type == STACKED_BAR_GRAPH ?
                             stacked_bar_labels[ i ] : multi_line_labels[ i ] );
      // Enable display of one level line at 90% of max. value.    
      if( graph_data.glg_graph_type != STACKED_BAR_GRAPH )
      {
         graph.SetDResource( "LevelObjectGroup/Factor", 1. );
         graph.SetDResource( "LevelObjectGroup/LevelObject/Low", low );
         graph.SetDResource( "LevelObjectGroup/LevelObject/High", high );
         graph.SetDResource( "LevelObjectGroup/LevelObject/Value", 
                            low + 0.9 * ( high - low ) );

         graph.SetDResource( "LevelObjectGroup/LevelObject/LineWidth",  1. );
         graph.SetGResource( "LevelObjectGroup/LevelObject/EdgeColor", 
                            0.7, 0., 0. );         
         graph.SetDResource( "LevelObjectGroup/Visibility", 1. );
      }

      // Create datasource to fill graph with demo data.
      graph_data.datasource = 
        new GlgDemoDataSource( low, high, graph_data.num_values, 1000 );

      graph.SetupHierarchy();    // Setup to prepare to receive data

      return graph_data;
   }

   /////////////////////////////////////////////////////////////////
   void UpdateGraphData( int graph_index )
   {
      int i;

      GraphData graph_data = graph_array[ graph_index ];
      GlgObject graph = graph_data.graph;
      GlgDemoDataSource datasource = graph_data.datasource;

      // If got a new data value, push it into the graph.
      if( datasource.UpdateData() )
      {
         // Push data into the graph.
         switch( graph_data.glg_graph_type )
         {         
          default:
            graph.SetDResource( "DataGroup/EntryPoint", datasource.GetValue() );
            break;

          case MULTI_LINE_GRAPH:   // Push several values (3)
            for( i=0; i<graph_data.num_values; ++i )
              graph.SetDResource( "DataGroupOne/EntryPoint", 
                                 datasource.GetValue( i ) );
            break;

          case STACKED_BAR_GRAPH:
            for( i=0; i<graph_data.num_values; ++i )
              graph.SetDResource( "DataGroup/EntryPoint", 
                                 datasource.GetValue( i ) / 3. );
            break;
         }

         // Push graph label as well.
         String label = "" + datasource.hour + ":" + 
             GlgObject.Printf( "%02d", datasource.min ) + ":" +
             GlgObject.Printf( "%02d", datasource.sec );

         if( graph_data.graph_time_axis == 'X' )
           graph.SetSResource( "XLabelGroup/EntryPoint", label );      
         else
           graph.SetSResource( "YLabelGroup/EntryPoint", label );      
      }
   }

   /////////////////////////////////////////////////////////////////
   // Determine the type of the graph depending on its resources.
   // Alternatively, a custom "GraphType" property may be added to the 
   // drawing of each graph and queried at run time.
   /////////////////////////////////////////////////////////////////
   void DetermineGraphType( GraphData graph_data )
   {
      GlgObject graph = graph_data.graph;

      if( graph.HasResourceObject( "DataGroup/Marker" ) )
      {
         if( ( graph.GetDResource( "DataGroup/Polygon/FillType" ).intValue()
            & GlgObject.FILL ) == 0 )
           graph_data.glg_graph_type = LINE_GRAPH;
         else
           graph_data.glg_graph_type = FILLED_LINE_GRAPH;

         graph_data.num_values = 1;
      }
      else if( graph.HasResourceObject( "DataGroupOne/DataGroup/Marker" ) )
      {
         graph_data.glg_graph_type = MULTI_LINE_GRAPH;
         graph_data.num_values = 
           graph.GetDResource( "DataGroupOne/Factor" ).intValue();
      }
      else if( graph.HasResourceObject( "DataGroup/Pack" ) )
      {
         graph_data.glg_graph_type = STACKED_BAR_GRAPH;
         graph_data.num_values = 
           graph.GetDResource( "DataGroup/Pack/Factor" ).intValue();
      }
      else
      {
         graph_data.glg_graph_type = BAR_GRAPH;
         graph_data.num_values = 1;
      }

      // Determine graph's time axis.        
      if( graph.HasResourceObject( "XLabelGroup/EntryPoint" ) )
        graph_data.graph_time_axis = 'X';
      else
        graph_data.graph_time_axis = 'Y';
   }
}
