package glg_demos;

import java.io.*;
import java.net.*;
import javax.servlet.*;
import javax.servlet.http.*;
import javax.imageio.*;
import java.awt.image.*;
import com.genlogic.*;

public final class GlgAvionicsServlet extends HttpServlet 
   implements GlgErrorHandler
{
   static final long serialVersionUID = 0;

   static GlgObject drawing = null;

   // Drawing path relative to the servlet app's dir.
   static final String drawing_name = "/drawings/avionics.g";

   // Array of demo datasources to animate drawing's 15 dynamic resources.
   GlgDemoDataSource datasource_array[] = new GlgDemoDataSource[ 15 ];

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
      int width = GetIntegerParameter( request, "width", 750 );
      int height = GetIntegerParameter( request, "height", 600 );

      // Limit max. size to avoid running out of heap space creating an image.
      if( width > 1000 ) width = 1000;       
      if( height > 1000 ) height = 1000;       

      // This servlet reuses the same drawings for all servlets.
      // Therefore lock to synchronize and prevent other servlets from 
      // changing the drawing size, etc., before we are done.
      GlgObject.Lock();

      // Load the drawing just once and share it between all servlets and 
      // threads. Alternatively, each servlet may load its own drawing.
      //
      if( drawing == null )    // First time: load the drawing.
      {
         drawing = LoadDrawing( drawing_name );
         drawing.SetImageSize( width, height );
         drawing.SetupHierarchy();    // Setup to prepare to receive data
 
         CreateDemoData();
      }
      else   // Already loaded, reuse the drawing.
        drawing.SetImageSize( width, height );
         
      UpdateDrawingWithData();

      // Setup after data update to prepare to generate image.
      drawing.SetupHierarchy();

      // Create an image of the drawing's graphics.
      BufferedImage image = (BufferedImage) drawing.CreateImage( null );

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
      getServletContext().log( "GlgAvionicsServlet: " + msg );
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
   // Create demo datasources for animation. In the application, any
   // custom source of data may be used.
   /////////////////////////////////////////////////////////////////
   void CreateDemoData()
   {
      datasource_array[ 0 ] = new GlgDemoDataSource( 20., 80., 1, 1000 );
      datasource_array[ 1 ] = new GlgDemoDataSource( 30., 85., 1, 1000 );
      datasource_array[ 2 ] = new GlgDemoDataSource( 3.5, 7.5, 1, 1000 );
      datasource_array[ 3 ] = new GlgDemoDataSource( 4.5, 7.8, 1, 1000 );
      datasource_array[ 4 ] = new GlgDemoDataSource( 2000., 8000., 1, 1000 );
      datasource_array[ 5 ] = new GlgDemoDataSource( 0.6, 1., 1, 1000 );
      datasource_array[ 6 ] = new GlgDemoDataSource( 0.7, 1.2, 1, 1000 );
      datasource_array[ 7 ] = new GlgDemoDataSource( 0., 20., 1, 1000 );
      datasource_array[ 8 ] = new GlgDemoDataSource( 0., 1., 1, 1000 );
      datasource_array[ 9 ] = new GlgDemoDataSource( 0., 1., 1, 1000 );
      datasource_array[ 10 ] = new GlgDemoDataSource( -10., 10., 1, 1000 );
      datasource_array[ 11 ] = new GlgDemoDataSource( -10., 10., 1, 1000 );
      datasource_array[ 12 ] = new GlgDemoDataSource( -20., 20., 1, 1000 );
      datasource_array[ 13 ] = new GlgDemoDataSource( -10., 10., 1, 1000 );
      datasource_array[ 14 ] = new GlgDemoDataSource( 30., 160., 1, 1000 );
   }

   /////////////////////////////////////////////////////////////////
   // Create demo datasources for animation. In the application, any
   // custom source of data may be used.
   /////////////////////////////////////////////////////////////////
   void UpdateDrawingWithData()
   {
      for( int i=0; i<15; ++i )
        datasource_array[i].UpdateData();   // Get new data for all variables.

      drawing.SetDResource( "RPM/Value",     datasource_array[0].GetValue() );
      drawing.SetDResource( "RPM/Value2",    datasource_array[1].GetValue() );
      drawing.SetDResource( "EGT/Value",     datasource_array[2].GetValue() );
      drawing.SetDResource( "EGT/Value2",    datasource_array[3].GetValue() );
      drawing.SetDResource( "FUEL/Value",    datasource_array[4].GetValue() );
      drawing.SetDResource( "MACH/Value",    datasource_array[5].GetValue() );
      drawing.SetDResource( "MACH/Value2",   datasource_array[6].GetValue() );
      drawing.SetDResource( "ADA/Value",     datasource_array[7].GetValue() );
      drawing.SetDResource( "NOZ/Value",     datasource_array[8].GetValue() );
      drawing.SetDResource( "NOZ/Value2",    datasource_array[9].GetValue() );
      drawing.SetDResource( "HORIZON/Pitch", datasource_array[10].GetValue() );
      drawing.SetDResource( "HORIZON/Roll",  datasource_array[11].GetValue() );
      drawing.SetDResource( "HORIZON/LeftRudder", 
                           datasource_array[12].GetValue() );
      drawing.SetDResource( "HORIZON/RightRudder", 
                           datasource_array[13].GetValue() );
      drawing.SetDResource( "COMPASS/Value", 
                           datasource_array[14].GetValue() );
   }
}
