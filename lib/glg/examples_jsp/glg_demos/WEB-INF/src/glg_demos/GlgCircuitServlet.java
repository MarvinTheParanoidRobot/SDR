package glg_demos;

import java.io.*;
import java.net.*;
import javax.servlet.*;
import javax.servlet.http.*;
import javax.imageio.*;
import java.awt.image.*;
import com.genlogic.*;

public final class GlgCircuitServlet extends HttpServlet 
   implements GlgErrorHandler
{
   static final long serialVersionUID = 0;

   // If true, the resource path is used to animate resources of the drawing.
   // If false, stored resource ID is used to set resource directly with 
   // null path using the Extended API. 
   // Alternatively, tags may be used instead of resources.
     //
   final boolean USE_RESOURCE_PATH = false;

   static GlgObject viewport = null;

   // Drawing path relative to the servlet app's dir.
   static final String drawing_name = "/drawings/electric_circuit2.g";

   // Global simulated data used by all servlets.
   static GlgCircuitDemoData data = new GlgCircuitDemoData();

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

      // Load the drawing (if first time) and update with data.

      // Get requested width/height of the image.
      int width = GetIntegerParameter( request, "width", 800 );
      int height = GetIntegerParameter( request, "height", 600 );

      // Limit max. size to avoid running out of heap space creating an image.
      if( width > 1000 ) width = 1000;       
      if( height > 1000 ) height = 1000;       

      // This example reuses the same drawing between all servlets' threads.
      // Therefore lock to synchronize and prevent other servlets from 
      // changing the drawing size, etc., before we are done.
      GlgObject.Lock();

      // Load the drawing just once and share it between all servlets and 
      // threads. Alternatively, each servlet may load its own drawing.
      //
      if( viewport == null )    // First time: load the drawing.
      {
         viewport = LoadDrawing( drawing_name );
         viewport.SetImageSize( width, height );
         viewport.SetupHierarchy();    // Setup to prepare to receive data

         // Create a list of resources to animate by querying them from the
           // drawing.
         GlgObject resource_list = GetResourceList( viewport, null, null );
         if( resource_list == null )
           Error( "Found no resources to animate.", 0, null );
         else
           data.resource_list = resource_list;
      }
      else   // Already loaded, reuse the drawing.
        viewport.SetImageSize( width, height );
         
      // Get the new data values from a custom data source.
      data.UpdateCircuitData();

      UpdateDrawingWithData();   // Push new data into the drawing.

      // Setup after data update to prepare to generate image.
      viewport.SetupHierarchy();

      // Create an image of the viewport's graphics.
      BufferedImage image = (BufferedImage) viewport.CreateImage( null );

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
      getServletContext().log( "GlgCircuitServlet: " + msg );
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

   //////////////////////////////////////////////////////////////////////////
   // Creates a list of resources to animate defined in the drawing.
   // Queries the drawing to include all resources of interest that are 
   // marked by having "#" as the first character of their names. 
   // Alternatively, tags may be used instead of resources.
   //////////////////////////////////////////////////////////////////////////
   GlgObject GetResourceList( GlgObject obj, String res_path, GlgObject list )
   {
      // Using only named resources in this example, no aliases.
      GlgObject res_list = obj.CreateResourceList( true, false, false );
      if( res_list == null )
        return list;
     
      int size = res_list.GetSize();
      for( int i=0; i<size; ++i )
      {
         GlgObject object = (GlgObject) res_list.GetElement( i );

         String name = object.GetObjectName();
         if( !name.startsWith( "#" ) )
           // We are interested only in resources that start with #
           continue;

         // Accumulate resource path.
         String new_path;
         if( res_path == null )
           new_path = name;
         else
           new_path = res_path + "/" + name;

         int object_type = object.GetObjectType();

         // Data or attribute object: add to the list of resources to animate.
         if( object_type == GlgObject.DATA || 
             object_type == GlgObject.ATTRIBUTE )
         {
            GlgSimulationResource resource = new GlgSimulationResource();
            resource.object = object;
            resource.type = object.GetDataType();
            resource.resource_path = new_path;

            // Set range for animating the resource.
            // State resources may have ON (1) and OFF (0) values - 
            // use 1.3 as a range to simulate. Use range=100 for the rest 
            // of resources.
              //
            if( name.equals( "#State" ) )
              resource.range = 1.3;
            else
              resource.range = 1000.;
            
            // Create a list if does not yet exist.
            if( list == null )
              list = new GlgDynArray( GlgObject.NATIVE_OBJECT, 0, 0 );

            list.AddObjectToBottom( resource );
         }

         int has_resources = object.GetDResource( "HasResources" ).intValue();

         // If object's HasResources=ON, recursively traverse all resources
           // inside it.
         if( has_resources == 1 )
           list = GetResourceList( object, new_path, list );
      }
      return list;
   }

   //////////////////////////////////////////////////////////////////////////
   // Updates drawing state with data.
   //////////////////////////////////////////////////////////////////////////
   void UpdateDrawingWithData()
   {
      GlgObject resource_list = data.resource_list;
      if( resource_list == null )
        return;

      int size = resource_list.GetSize();
      for( int i=0; i<size; ++i )
      {
         GlgSimulationResource resource = (GlgSimulationResource)
           resource_list.GetElement( i );

         if( resource.type != GlgObject.D ) 
           continue;      // Update only resources of D type

         if( USE_RESOURCE_PATH )
           // Use resource path.
           viewport.SetDResource( resource.resource_path, resource.value );
         else
           // Use stored resource ID with null path to set the resource 
             // directly using the Extended API.
           resource.object.SetDResource( null, resource.value );
      } 
   }
}
