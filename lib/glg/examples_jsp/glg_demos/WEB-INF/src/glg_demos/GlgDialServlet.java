package glg_demos;

import java.io.*;
import java.net.*;
import javax.servlet.*;
import javax.servlet.http.*;
import javax.imageio.*;
import java.awt.image.*;
import com.genlogic.*;

public final class GlgDialServlet extends HttpServlet 
   implements GlgErrorHandler
{
   static final long serialVersionUID = 0;

   // The servlet handles several dial types, using a different drawing
     // for each dial.
   static final int NUM_DIAL_TYPES = 5;

   // Drawing filenames.
   static final String drawing_names[] = 
     { "dial1.g", "dial2.g", "dial3.g", "dial4.g", "dial5.g" };

   // Array to keep loaded dial drawing objects.
   static GlgObject dial_array[] = new GlgObject[ NUM_DIAL_TYPES ];

   // The servlet uses a separate datasource for each dial.
   static GlgDemoDataSource datasource_array[] = 
     new GlgDemoDataSource[ NUM_DIAL_TYPES ];

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
      int width = GetIntegerParameter( request, "width", 200 );
      int height = GetIntegerParameter( request, "height", 200 );

      // Limit max. size to avoid running out of heap space creating an image.
      if( width > 1000 ) width = 1000;       
      if( height > 1000 ) height = 1000;       

      // Get dial type.
      int dial_type = GetIntegerParameter( request, "dial_type", 0 );
      if( dial_type < 0 || dial_type >= NUM_DIAL_TYPES )
      {
         Log( "Invalid dial type, using 0." );
         dial_type = 0;
      }

      // This servlet reuses the same drawings between all servlets' threads.
      // Therefore lock to synchronize and prevent other servlets from 
      // changing the drawing size, etc., before we are done.
      GlgObject.Lock();

      GlgObject dial = dial_array[ dial_type ];

      // Load the drawing just once and share it between all servlets and 
      // threads. Alternatively, each servlet may load its own drawing.
      //
      if( dial == null )    // First time: load the drawing.
      {
         // Load the dial using a path relative to the servlet app's dir.
         String drawing_name = "/drawings/" + drawing_names[ dial_type ];
         dial = LoadDrawing( drawing_name );
         dial.SetImageSize( width, height );
         dial.SetupHierarchy();    // Setup to prepare to receive data
           
         dial_array[ dial_type ] = dial;   // Store the drawing for reuse.
      }
      else   // Already loaded, reuse the drawing.
        dial.SetImageSize( width, height );
         
      // Get data to be displayed in the dial.
      double value = GetData( request, dial, dial_type );

      // Check value range
      value = CheckRange( dial, value );

      // Updates dial with current data.
      dial.SetDResource( "Value", value );

      // Use label from the request if supplied as a parameter.
      String label = request.getParameter( "label" );
      if( label == null )
        label = "Load";   // Default
      dial.SetSResource( "Units", label );      

      // Setup after data update to prepare to generate image.
      dial.SetupHierarchy();

      // Create an image of the dial's graphics.
      BufferedImage image = (BufferedImage) dial.CreateImage( null );

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
         Log( "Invalid parameter value for: " + name + " = " + parameter_string );
         return default_value;
      }
   }

   /////////////////////////////////////////////////////////////////
   void Log( String msg )
   {
      getServletContext().log( "GlgDialServlet: " + msg );
   }

   // GlgErrorHandler interface method for error handling.
   public void Error( String message, int error_type, Exception e )
   {
      Log( message );   // Log errors

      Log( GlgObject.GetStackTraceAsString() );   // Print stack
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
   // Get data from the value parameter of the request if supplied,
   // to demonstrate supplying data from html. If the value is not 
   // supplied in the request, use a server-side datasource.
   /////////////////////////////////////////////////////////////////     
   double GetData( HttpServletRequest request, GlgObject dial, int dial_type )
   {
      // Use the value parameter if supplied.
      String value_string = request.getParameter( "value" );
      if( value_string != null )
      {
         try
         {
            return Double.parseDouble( value_string );
         }
         catch( NumberFormatException e )
         {
            Log( "Invalid parameter value for: value = " + value_string );
         }
      }

      // If value was not supplied in the request, use demo data.

      // Each dial uses its own datasource.
      GlgDemoDataSource datasource = datasource_array[ dial_type ];
      if( datasource == null )   // First time: create.
      {
         // Get dial high and low ranges used to create demo datasource.
         double low = dial.GetDResource( "Low" ).doubleValue();
         double high = dial.GetDResource( "High" ).doubleValue();

         datasource = new GlgDemoDataSource( low, high, 1, 1000 );
         datasource_array[ dial_type ] = datasource; // Store for reuse
      }

      datasource.UpdateData();  // Update data value.
      return datasource.GetValue();
   }

   /////////////////////////////////////////////////////////////////     
   // Sets the value to dial's max. value if it is too 
   /////////////////////////////////////////////////////////////////     
   double CheckRange( GlgObject dial, double value )
   {
      double low = dial.GetDResource( "Low" ).doubleValue();
      double high = dial.GetDResource( "High" ).doubleValue();

      if( value < low )
        value = low;
      else if( value > high )
        value = high + 0.05 * ( high - low );

      return value;
   }
}
