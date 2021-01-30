package glg_demos;

import java.io.*;
import java.net.*;
import javax.servlet.*;
import javax.servlet.http.*;
import javax.imageio.*;
import java.awt.image.*;
import com.genlogic.*;

public final class GlgGraphSelectionServlet extends HttpServlet 
   implements GlgErrorHandler
{
   static final long serialVersionUID = 0;

   // Drawing path relative to the servlet app's dir.
   static final String drawing_name = "/drawings/packed_bar_graph.g";

   static GlgObject graph;               // Drawing
   static GlgDemoDataSource datasource;  // Graph's datasource.

   static final String legend_labels[] = { "server 1", "server 2", "server 3" };
   static final String axis_labels[] =
   { "Jan", "Feb", "Mar", "Apr", "May", "Jun", 
     "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

   static final String time_stamp_labels[] =
   { "January", "February", "March", "April", "May", "June", 
     "July", "August", "September", "October", "November", "December" };

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
      boolean update_data;

      InitGLG();   // Init the Toolkit

      // Get requested width/height of the image.
      int width = GetIntegerParameter( request, "width", 500 );
      int height = GetIntegerParameter( request, "height", 300 );

      // Limit max. size to avoid running out of heap space creating an image.
      if( width > 1000 ) width = 1000;       
      if( height > 1000 ) height = 1000;       

      // This servlet reuses the same drawings between all servlets' threads.
      // Therefore lock to synchronize and prevent other servlets from 
      // changing the drawing size, etc., before we are done.
      GlgObject.Lock();

      // Load the drawing just once and share it between all servlets and 
      // threads. Alternatively, each servlet may load its own drawing.
      //
      if( graph == null )    // First time: load the drawing.
      {
         // Load graph using a path relative to the servlet app's dir.
         graph = LoadDrawing( drawing_name );
         graph.SetImageSize( width, height );
            
         // Set graph's properties and create a demo datasource.
         InitializeGraph( graph );

         update_data = true;   // Fill data just once
      }
      else   // Already loaded, reuse the drawing.
      {
         graph.SetImageSize( width, height );
         
         // Update data only if requested.
         // Don't update data when changing size. 
         update_data = 
           ( GetIntegerParameter( request, "update_data", 0 ) != 0 );
      }

      // Use title from the request if supplied as a parameter.
      String title = request.getParameter( "title" );
      if( title == null )
        title = "Used Bandwidth";   // Default
      graph.SetSResource( "Title/String", title );
           
      // Push new data into the graph.
      if( update_data )
        UpdateGraphData();

      // Setup after data update to prepare to generate image.
      graph.SetupHierarchy();

      String action = GetStringParameter( request, "action", "GetImage" );
      if( action.equals( "GetImage" ) )
      {
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
      else if( action.equals( "ProcessEvent" ) )
      {
         // Get x and y coordinates of the mouse click.
         int x = GetIntegerParameter( request, "x", -1 );
         int y = GetIntegerParameter( request, "y", -1 );

         // Selection rectangle around the mouse click.
         GlgCube click_box = new GlgCube();
         int selection_sensitivity = 0; // Extend by a few pixels if necessary.
         click_box.p1.x = x - selection_sensitivity;
         click_box.p1.y = y - selection_sensitivity;
         click_box.p2.x = x + selection_sensitivity;
         click_box.p2.y = y + selection_sensitivity;

         // Find selected object using named object selection.
         String selection_info = null;
         if( x > 0 && y > 0 )
         {
            // Query array of selected objects.
            GlgObject selection = null;
            selection = GlgObject.CreateSelection( graph, click_box, graph );

            // Get requested selection type, use Tooltip as default if omitted.
            String event_type = 
              GetStringParameter( request, "event_type", "Tooltip" );
            // Find selected bar and extract its value.
            selection_info = GetSelectionInfo( selection, event_type );
         }

         GlgObject.Unlock();

         WriteAsPlainText( response, 
                          selection_info == null ? "None" : selection_info );
      }           
      else
      {
         Log( "Unsupported action!" );
         GlgObject.Unlock();
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

   String GetStringParameter( HttpServletRequest request, String name, 
                             String default_value )
   {
      String parameter_string = request.getParameter( name );
      if( parameter_string == null )
        return default_value;
      else
        return parameter_string;
   }

   /////////////////////////////////////////////////////////////////
   void Log( String msg )
   {
      getServletContext().log( "GlgGraphSelectionServlet: " + msg );
   }

   // GlgErrorHandler interface method for error handling.
   public void Error( String message, int error_type, Exception e )
   {
      Log( message );   // Log errors

      Log( GlgObject.GetStackTraceAsString() ); // Print stack for debugging
   }

   /////////////////////////////////////////////////////////////////
   void WriteAsPlainText( HttpServletResponse response, String string )
   {
      try
      {
         response.setContentType("text/plain");
         PrintWriter out_stream = 
            new PrintWriter( response.getOutputStream() );
         out_stream.write( string );
         out_stream.close();
      }
      catch( IOException e )
      {
         // Log( "Aborted writing of text response." );
      }
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
   void InitializeGraph( GlgObject graph )
   {
      // Hardcoded data just to demo a graph
      double low = 0.;
      double high = 10.;

      // Use 12 labels and major ticks.
      graph.SetDResource( "XLabelGroup/Factor", 12. );

      // Disable minor ticks
      graph.SetDResource( "XLabelGroup/MinorFactor", 1. );

      // Set the number of datasamples to 12.
      graph.SetDResource( "DataGroup/Factor", 12. );

      // Set the number of values in each pack of the packed bar graph to 3.
      graph.SetDResource( "DataGroup/Pack/Factor", 3. );

      // Set the range of graph's value axis.
      graph.SetDResource( "YLabelGroup/YLabel/Low", low );
      graph.SetDResource( "YLabelGroup/YLabel/High", high );

      // Set value axis format
      graph.SetSResource( "YLabelGroup/YLabel/Format", "%.0lf GB" );

      // Set title
      graph.SetSResource( "Title/String", "Used Bandwidth" );
      
      // Disable axis labels and vertical grid.
      graph.SetDResource( "YAxisLabel/Visibility", 0. );
      graph.SetDResource( "XAxisLabel/Visibility", 0. );
      graph.SetDResource( "XGridGroup/Visibility", 0. );

      // Set the packed bar graph's legend.
      for( int i=0; i<3; ++i )
        graph.SetSResource( "LegendObject/LegendGroup/Legend/Text" + i,
                           legend_labels[ i ] );

      // Create datasource to fill graph with demo data.
      datasource = 
        new GlgDemoDataSource( low + 0.4 * ( high - low ), 
                              low + 0.7 * ( high - low ), 3, 0 );

      graph.SetupHierarchy();    // Setup to prepare to receive data
   }

   /////////////////////////////////////////////////////////////////
   void UpdateGraphData()
   {
      int i, j;

      // Push data into the graph. The packed bar graph used in the demo
      // displays 12 samples, each sample containing a pack of 3 values.
      for( i=0; i<12; ++i )
      {
         datasource.UpdateData();
         for( j=0; j<3; ++j )
           graph.SetDResource( "DataGroup/EntryPoint", 
                              datasource.GetValue( j ) );

         // For each datasample, fill the time stamp custom property
           // For this demo, it matches the time axis labels.
        graph.SetSResource( "DataGroup/Pack" + i + "/TimeStamp",
                           time_stamp_labels[i] );
      }

      // Push graph labels as well.
      for( i=0; i<12; ++i )
        graph.SetSResource( "XLabelGroup/EntryPoint", axis_labels[i] );      
   }

   /////////////////////////////////////////////////////////////////
   String GetSelectionInfo( GlgObject selection, String event_type )
   {
      if( selection == null )
        return null;

      // Process selection by requesting a list of all selected objects.
      // This allows to use out of the box graph drawing, with no need
      // to attach tooltip and mouse click actions to the chart's datasample
      // template.
      
      int size = selection.GetSize();
      for( int i=0; i<size; ++i )
      {
         GlgObject sel_obj = (GlgObject) selection.GetElement( i );
         String name = sel_obj.GetSResource( "Name" );

         // Accept selection of graph's datasamples only.
         if( name == null || !name.startsWith( "DataSample" ) )
           continue;

         // Get the month pack the datasample belongs too. 
         // The pack of datasamples for each month has a "TimeStamp" custom
           // property attached to it.
         GlgObject parent = sel_obj.GetParent();
         while( parent != null && !parent.HasResourceObject( "TimeStamp" ) )
           parent = parent.GetParent();

         String month;
         if( parent != null )
           month = parent.GetSResource( "TimeStamp" );
         else
           month = "Undefined";

         // Tooltip: Display value of the selected server.
         if( event_type.equals( "Tooltip" ) )
         {
            // Extract datasample index from "DataSampleN" name.
            int server_index;
            String server_index_string = 
              name.substring( "DataSample".length() );
            try
            {
               server_index = Integer.parseInt( server_index_string );
            }
            catch( NumberFormatException e )
            {
               Log( "Can't parse server index: " + name );
               return null;
            }
            
            double value = sel_obj.GetDResource( "Value" ).doubleValue();

            return "<b>Server:</b> " + ( server_index + 1 ) + 
              "<br><b>Month:</b> " + month + 
              "<br><b>Bandwidth:</b> " + GlgObject.Printf( "%.2f GB", value );
         }
         // MouseClick: display values for all three servers.
         else if( event_type.equals( "MouseClick" ) )
         {
            double value0 = 
              parent.GetDResource( "DataSample0/Value" ).doubleValue();
            double value1 = 
              parent.GetDResource( "DataSample1/Value" ).doubleValue();
            double value2 = 
              parent.GetDResource( "DataSample2/Value" ).doubleValue();

            return "<b>" + month + " Bandwidth</b>" +
              "<hr style=\"width: 100%; height: 2px;\">" +
              "<b>Server 1:</b> " + GlgObject.Printf( "%.2f GB", value0 ) +
              "<br><b>Server 2:</b> " + GlgObject.Printf( "%.2f GB", value1 ) +
              "<br><b>Server 3:</b> " + GlgObject.Printf( "%.2f GB", value2 );
         }
         else
           return "Unsupported event_type";
      }
      return null;
   }
}
