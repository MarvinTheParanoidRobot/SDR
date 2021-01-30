package glg_demos;

import java.io.*;
import java.net.*;
import javax.servlet.*;
import javax.servlet.http.*;
import javax.imageio.*;
import java.awt.image.*;
import com.genlogic.*;

public final class GlgProcessServlet extends HttpServlet 
   implements GlgErrorHandler
{
   static final long serialVersionUID = 0;

   /* Demonstrates updating the drawing using either tags (true) or 
      resources (false).
   */
   static final boolean UseTags = true;

   static GlgObject viewport = null;

   // Drawing path relative to the servlet app's dir.
   static final String drawing_name = "/drawings/process2.g";

   // Global simulated data used by all servlets.
   static GlgProcessDemoData data = new GlgProcessDemoData();

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
   // Supported actions (action parameter):
   //    GetImage - generates image of the monitored process
   //    ProcessEvent - processes object selection on MouseClick or 
   //                   Tooltip event types.
   //    GetDialogData - returns data for requested dialogs.
   /////////////////////////////////////////////////////////////////
   public void doGet2( HttpServletRequest request, 
                       HttpServletResponse response ) 
   {
      InitGLG();   // Init the Toolkit

      String action = GetStringParameter( request, "action", "GetImage" );
      if( action.equals( "GetDialogData" ) )
      {
         // Simply return requested dialog data: no drawing required.
         ProcessDialogData( request, response );
         return;
      }

      // The rest of actions (GetImage, ProcessEvent) require a drawing -
      // load it (if first time) and update with data.

      // Get requested width/height of the image: need for all other actions.
      int width = GetIntegerParameter( request, "width", 500 );
      int height = GetIntegerParameter( request, "height", 400 );

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
      }
      else   // Already loaded, reuse the drawing.
        viewport.SetImageSize( width, height );
       
      ShowPipes( request );      // Show pipes and flow lines if requested.
           
      data.UpdateProcessData();  // Get the new data values.
             
      UpdateDrawingWithData();   // Updates drawing with current data.
             
      // Setup after data update to prepare to generate image.
      viewport.SetupHierarchy();

      // Main action: Generate Image.
      if( action.equals( "GetImage" ) )
      {
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
      // Secondary action: ProcessEvent.
      else if( action.equals( "ProcessEvent" ) )
      {
         String selection_info = null;

         String event_type = GetStringParameter( request, "event_type", "" );

         int selection_type;
         if( event_type.equals( "MouseClick" ) )      // Get selected object
         {
            // Find object with the MouseClick custom event.
            selection_type = GlgObject.CLICK_SELECTION;
         }
         else if( event_type.equals( "Tooltip" ) )    // Get object's tooltip
         {
            // Find object with the TooltipString property.
            selection_type = GlgObject.TOOLTIP_SELECTION;
         }
         else
         {
            selection_type = 0;
            selection_info = "Unsupported event_type";
         }

         if( selection_type != 0 )
         {
            // Get x and y coordinates of the mouse click.
            int x = GetIntegerParameter( request, "x", -1 );
            int y = GetIntegerParameter( request, "y", -1 );
            
            // Selection rectangle around the mouse click.
            GlgCube click_box = new GlgCube();
            int selection_sensitivity = 3;   // in pixels
            click_box.p1.x = x - selection_sensitivity;
            click_box.p1.y = y - selection_sensitivity;
            click_box.p2.x = x + selection_sensitivity;
            click_box.p2.y = y + selection_sensitivity;

            // Find selected object with MouseClick custom event attached.
            if( x > 0 && y > 0 )
            {
               // Select using MouseClick custom events.
               GlgObject selection_message = null;
               selection_message =
                 GlgObject.CreateSelectionMessage( viewport, 
                                                  click_box, viewport, 
                                                  selection_type, 1 );
               
               if( selection_message != null )
                 switch( selection_type )
                 {
                  default:
                  case GlgObject.CLICK_SELECTION:   // Return object name.
                    selection_info = 
                      selection_message.GetSResource( "Object/Name" );
                    break;
                     
                  case GlgObject.TOOLTIP_SELECTION:
                    /* Return tooltip string, which is an event label
                       of the tooltip action message.
                    */
                    selection_info = 
                      selection_message.GetSResource( "EventLabel" );
                    break;
                 }
            }
            else
              selection_info = "Invalid x/y coordinates.";
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
      getServletContext().log( "GlgProcessServlet: " + msg );
   }

   // GlgErrorHandler interface method for error handling.
   public void Error( String message, int error_type, Exception e )
   {
      Log( message );   // Log errors

      Log( GlgObject.GetStackTraceAsString() );   // Print stack
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

   //////////////////////////////////////////////////////////////////////////
   // Updates drawing state with data.
   //////////////////////////////////////////////////////////////////////////
   void UpdateDrawingWithData()
   {
      // The drawing can be updated using either tags or resources.
      if( UseTags )
        UpdateProcessTags();
      else
        UpdateProcessResources();
   }

   //////////////////////////////////////////////////////////////////////////
   // Updates drawing using resources.
   //////////////////////////////////////////////////////////////////////////
   public void UpdateProcessTags()
   {
      viewport.SetDTag( "SolventValveValue", data.SolventValve, true );
      viewport.SetDTag( "SteamValveValue", data.SteamValve, true );
      viewport.SetDTag( "CoolingValveValue", data.CoolingValve, true );
      viewport.SetDTag( "WaterValveValue", data.WaterValve, true );
      
      viewport.SetDTag( "SteamTemperature", data.SteamTemperature, true );
      viewport.SetDTag( "HeaterTemperature", data.HeaterTemperature, true );
      viewport.SetDTag( "BeforePreHeaterTemperature", 
                        data.BeforePreHeaterTemperature, true );
      viewport.SetDTag( "PreHeaterTemperature", 
                        data.PreHeaterTemperature, true );
      viewport.SetDTag( "AfterPreHeaterTemperature", 
                        data.AfterPreHeaterTemperature, true );
      viewport.SetDTag( "CoolingTemperature", data.CoolingTemperature, true );
      
      viewport.SetDTag( "HeaterLevel", data.HeaterLevel, true );
      viewport.SetDTag( "WaterLevel", data.WaterLevel, true );
      
      viewport.SetDTag( "HeaterAlarm", data.HeaterAlarm ? 1. : 0., true );
      viewport.SetDTag( "WaterAlarm", data.WaterAlarm ? 1. : 0., true );
      
      /* Pass if_changed=false to move the chart even if the value did not 
         change. The rest of resources use true to update them only if their 
         values changed.
      */
      viewport.SetDTag( "PlotValueEntryPoint", data.HeaterTemperature, false );
      
      viewport.SetDTag( "PressureValue", 5. * data.HeaterPressure, true );
   }   
   
   //////////////////////////////////////////////////////////////////////////
   // Updates drawing using resources.
   //////////////////////////////////////////////////////////////////////////
   public void UpdateProcessResources()
   {
      viewport.SetDResource( "SolventValve/Angle", data.SolventValve, true );
      viewport.SetDResource( "SteamValve/Angle",   data.SteamValve, true );
      viewport.SetDResource( "CoolingValve/Angle", data.CoolingValve, true );
      viewport.SetDResource( "WaterValve/Angle",   data.WaterValve, true );
      
      viewport.SetDResource( "Heater/SteamTemperature", 
                             data.SteamTemperature, true );
      viewport.SetDResource( "Heater/HeaterTemperature",
                             data.HeaterTemperature, true );
      viewport.SetDResource( "BeforePreHeaterTemperature",
                             data.BeforePreHeaterTemperature, true );
      viewport.SetDResource( "PreHeaterTemperature",
                             data.PreHeaterTemperature, true );
      viewport.SetDResource( "AfterPreHeaterTemperature",
                             data.AfterPreHeaterTemperature, true );
      viewport.SetDResource( "CoolingTemperature", 
                             data.CoolingTemperature, true );
      
      viewport.SetDResource( "Heater/HeaterLevel",
                             data.HeaterLevel, true );
      viewport.SetDResource( "WaterSeparator/WaterLevel",
                             data.WaterLevel, true );
      
      viewport.SetDResource( "HeaterAlarm", data.HeaterAlarm ? 1. : 0, true );
      viewport.SetDResource( "WaterAlarm", data.WaterAlarm ? 1. : 0, true );
      
      /* Pass if_changed=false to move the chart even if the value did not 
         change. The rest of resources use true to update them only if their 
         values changed.
      */
      viewport.SetDResource( "ChartVP/Chart/Plots/Plot#0/ValueEntryPoint",
                             data.HeaterTemperature, false );

      viewport.SetDResource( "PressureGauge/Value", 
                             5. * data.HeaterPressure, true );
   }

   /////////////////////////////////////////////////////////////////
   // Shows pipes and flow lines if requested.
   // Constraints in the drawing take care of updating associated
   // toggle buttons when the pipe or flow line display state is 
   // changed. The last parameter of SetDResource() is set to true
   // to update the drawing only if the resource value gets changed.
   /////////////////////////////////////////////////////////////////
   void ShowPipes( HttpServletRequest request )
   {
      // Show pipes if requested, default 0.
      int show_pipes = GetIntegerParameter( request, "show_pipes", 0 );
      viewport.SetDResource( "3DPipes/Visibility", (double)show_pipes, true );

      // Show flow if requested, default 1.
      int show_flow = GetIntegerParameter( request, "show_flow", 1 );
      viewport.SetDResource( "FlowGroup/Visibility", (double)show_flow, true );
   }

   /////////////////////////////////////////////////////////////////
   // Returns data for a requested dialog.
   /////////////////////////////////////////////////////////////////
   void ProcessDialogData( HttpServletRequest request, 
                          HttpServletResponse response )
   {
      String dialog_type = 
        GetStringParameter( request, "dialog_type", "None" );

      if( dialog_type.equals( "Heater" ) )
      {
         WriteAsPlainText( response, 
            "<b>Solvent Heater</b><br>" +
            "Level: " + Format( data.HeaterLevel * 100. ) + " %<br>" +
            "Pressure: " + Format( data.HeaterPressure * 5. ) + " ATM.<br>" +
            "Temperature: " + Format( 50. + data.HeaterTemperature * 100. )
                            + " C\u00B0" );
      }
      else if( dialog_type.equals( "WaterSeparator" ) )
      {
         WriteAsPlainText( response, 
            "<b>Water Heater</b><br>" +
            "Level: " + Format( data.WaterLevel * 100. ) + " %<br>" +
            "Temperature: " + Format( 50 + data.CoolingTemperature * 30. )
                            + " C\u00B0" );
      }
      else if( dialog_type.equals( "SolventValve" ) )
      {
         WriteAsPlainText( response, 
               "<b>Solvent Valve</b><br>" +
               "Open: " + Format( data.SolventValve * 100. ) + " %" );
      }
      else if( dialog_type.equals( "SteamValve" ) )
      {
         WriteAsPlainText( response, 
               "<b>Steam Valve</b><br>" +
               "Open: " + Format( data.SteamValve * 100. ) + " %" );
      }
      else if( dialog_type.equals( "CoolingValve" ) )
      {
         WriteAsPlainText( response, 
               "<b>Cooling Valve</b><br>" +
               "Open: " + Format( data.CoolingValve * 100. ) + " %" );
      }
      else if( dialog_type.equals( "WaterValve" ) )
      {
         WriteAsPlainText( response, 
               "<b>Water Valve</b><br>" +
               "Open: " + Format( data.WaterValve * 100. ) + " %" );
      }
      else
        WriteAsPlainText( response, "None" );
   }

   String Format( double value )
   {
      return GlgObject.Printf( "%.2f", value );
   }
}
