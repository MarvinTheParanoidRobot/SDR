package glg_demos;

import java.io.*;
import java.net.*;
import javax.servlet.*;
import javax.servlet.http.*;
import java.util.ArrayList;
import javax.imageio.*;
import java.awt.image.*;
import com.genlogic.*;

public final class GlgSimpleViewerServlet extends HttpServlet 
   implements GlgErrorHandler, ServletLogInterface
{
   static final long serialVersionUID = 0;

   // Using random demo data, change to false to use a custom data source.
   static final boolean RANDOM_DATA = true;

   /* Number of cached recent drawings. Cached drawings are kept in memory
      and do not require reloading to process a request. When this number
      is exceeded, the oldest drawing is discarded to free space for a new
      drawing.
   */
   static final int MAX_NUM_DRAWINGS = 20;

   // Drawing path relative to the servlet app's dir.
   static final String default_drawing_name = "plant_ajax.g";

   static GlgObject DrawingArray = null;

   // Used for the popup dialog updates.
   static GlgTagRecord tag_record = new GlgTagRecord();

   /* DataFeed object is used to supply data for animation:
      DemoDataFeed or LiveDataFeed.
   */
   static DataFeedInterface DataFeed;

   // Stores information about a drawing and its list of tags.
   class DrawingData
   {
      String drawing_name;
      GlgObject viewport;
      ArrayList<GlgTagRecord> tag_records;
      int num_tags;

      DrawingData( String drawing_name, GlgObject viewport, 
                   ArrayList<GlgTagRecord> tag_records )
      {
         this.drawing_name = drawing_name;
         this.viewport = viewport;
         this.tag_records = tag_records;
         if( tag_records == null )
           this.num_tags = 0;
         else
           this.num_tags = tag_records.size();
      }
   }
   
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

      /* DataFeed object is used to supply data for animation:
         DemoDataFeed or LiveDataFeed.
      */
      if( DataFeed == null )
      {
         if( RANDOM_DATA )
           DataFeed = new DemoDataFeed( this );
         else
           DataFeed = new LiveDataFeed( this );
      }

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

      String drawing_name = "drawings/" +
        GetStringParameter( request, "drawing", default_drawing_name );

      // The drawings are reused between all servlets' threads.
      // Therefore lock to synchronize and prevent other servlets from 
      // changing the drawing size, etc., before we are done.
      GlgObject.Lock();

      // Load the drawing just once and share it between all servlets and 
      // threads. Alternatively, each servlet may load its own drawing.
      //
      DrawingData drawing = GetDrawing( drawing_name );      
      
      GlgObject viewport;
      if( drawing == null )    // First time: load the drawing.
      {         
         viewport = LoadDrawing( drawing_name ); 
         viewport.SetImageSize( width, height );
         viewport.SetupHierarchy();    // Setup to prepare to receive data

         ArrayList<GlgTagRecord> tag_list = CreateTagRecords( viewport );
         drawing = new DrawingData( drawing_name, viewport, tag_list );
         StoreDrawing( drawing_name, drawing );
      }
      else   // Already loaded, reuse the drawing.
      {
         viewport = drawing.viewport;
         viewport.SetImageSize( width, height );
      }

      UpdateDrawing( drawing );  // Update drawing with data.
             
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
                    String label = 
                      selection_message.GetSResource( "EventLabel" );
                    if( label != null && label.equals( "PopupDialog" ) )
                    {
                       String tag_path =
                         selection_message.GetSResource( "ActionObject/TagPath" );
                       selection_info = 
                         selection_message.GetSResource( "Object/" + tag_path +
                                                         "/TagSource" );
                    }
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

   /////////////////////////////////////////////////////////////////
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
   public void Log( String msg )
   {
      getServletContext().log( "GlgSimpleViewerServlet: " + msg );
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
   DrawingData GetDrawing( String drawing_name )
   {
      if( DrawingArray == null )
        return null;

      int size = DrawingArray.GetSize();
      for( int i=0; i<size; ++i )
      {
         DrawingData drawing = (DrawingData) DrawingArray.GetElement( i );
         if( drawing_name.equals( drawing.drawing_name ) )
         {
            /* Found an already loaded drawing.
               Reorder the drawing to the bottom of DrawingArray to indicate 
               it is the most recent.
            */
            if( drawing != (DrawingData) DrawingArray.GetElement( size - 1 ) )
            {
               DrawingArray.DeleteObject( drawing );
               DrawingArray.AddObjectToBottom( drawing );
            }
            return drawing;
         }
      }
      return null;   // Not found.
   }

   /////////////////////////////////////////////////////////////////
   void StoreDrawing( String drawing_name, DrawingData drawing )
   {
      drawing.drawing_name = drawing_name;

      if( DrawingArray == null )
        DrawingArray = new GlgDynArray( GlgObject.GLG_OBJECT, 0, 0 );

      // Free space if needed. 
      if( DrawingArray.GetSize() > MAX_NUM_DRAWINGS )
      {
         DrawingData old_drawing = (DrawingData) DrawingArray.GetElement( 0 );
         old_drawing.viewport.ResetHierarchy();
         DrawingArray.DeleteTopObject();
      }

      DrawingArray.AddObjectToBottom( drawing );
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

   /////////////////////////////////////////////////////////////////////
   // Create and populate TagRecordArray
   /////////////////////////////////////////////////////////////////////
   public ArrayList<GlgTagRecord> CreateTagRecords( GlgObject viewport )
   {
      GlgObject tag_obj;
      String tag_source;
      String tag_name;
      String tag_comment;
      int tag_access_type;
      GlgObject tag_list;

      /* Retrieve a tag list from the drawing. Include tags with unique
         tag sources.
      */
      tag_list = viewport.CreateTagList( /*unique tag sources*/ true );
      if( tag_list == null )
        return null;
      
      int size = tag_list.GetSize();
      if( size == 0 )
        return null; // no tags found 

      /* Create an array of tag records by traversing the tag list
         and retrieving information from each tag object
         in the list.
      */
      ArrayList<GlgTagRecord> tag_records = new ArrayList<GlgTagRecord>();
            
      for( int i=0; i<size; ++i )
      {
         tag_obj = (GlgObject) tag_list.GetElement( i );
         tag_source = tag_obj.GetSResource( "TagSource" );
         tag_name = tag_obj.GetSResource( "TagName" );
         tag_comment = tag_obj.GetSResource( "TagComment" );
         
         // Skip undefined tags.
         if( tag_source == null || tag_source.equals("") || 
             tag_source.equals("unset") )
           continue;
         
         tag_access_type = tag_obj.GetDResource( "TagAccessType" ).intValue();
         
         // Handle special tags according to their tag access type.
         switch( tag_access_type )
         {
          case GlgObject.OUTPUT_TAG: continue;   // Skip OUTPUT tags.
            
          case GlgObject.INIT_ONLY_TAG:
            /* Initialize INIT_ONLY tags; do not include them in tag_records, 
               so that they will not be subject to contineous data animation.
            */
            InitTag( viewport, tag_source );
            continue;
          
          default: break;
         }
         
         // Tag source is valid, add a new tag record to tag_records.
         GlgTagRecord tag_record = new GlgTagRecord();
         tag_record.tag_obj = tag_obj;
         tag_record.tag_source = tag_source;
         tag_record.data_type = tag_obj.GetDResource( "DataType" ).intValue();
         
         /* For further performance optimization, set if_changed=true which 
            will push the value into the tag only if the value has changed. 
            Set if_changed = false for the tags assigned to the entry 
            points of a chart object.
         */
         if( tag_comment != null && tag_comment.contains( "Chart" ) )
           tag_record.if_changed = false;
         else
           tag_record.if_changed = true;      
         
         // Add a valid tag record to the list.
         tag_records.add( tag_record );
      }

      return tag_records;
   }

   //////////////////////////////////////////////////////////////////////////
   // Initialize tag with a given tag source.
   //////////////////////////////////////////////////////////////////////////
   public void InitTag( GlgObject viewport, String tag_source )
   {
      double value = -1.0;
      
      if( RANDOM_DATA )
      {
         // Initialize "State" tag.
         if( tag_source.equals( "State" ) )
           value = 1.0;
      }
      else
      {
         // Place custom code here to set value as needed.
      }
      
      // Set the tag value in the drawing.
      viewport.SetDTag( tag_source, value, false );
   }

   //////////////////////////////////////////////////////////////////////////
   // Update the drawing with new dynamic data values.
   //////////////////////////////////////////////////////////////////////////
   public void UpdateDrawing( DrawingData drawing )
   {
      /* For each tag in the tag_records, obtain a new data value using 
         DataFeed object and push a new value into the graphics.  
      */
      GlgObject viewport = drawing.viewport;
      for( int i = 0; i<drawing.num_tags; ++i )
      {
         GlgTagRecord tag_record = drawing.tag_records.get(i);
         switch( tag_record.data_type )
         {
          case GlgObject.D:
            if( !DataFeed.ReadDValue( tag_record ) )
            {
               Log( "Failed to read data value for " + tag_record.tag_source );
               continue;
            }

            // Push new data value into the graphics.
            viewport.SetDTag( tag_record.tag_source,  tag_record.d_value, 
                              tag_record.if_changed );
            break;

          case GlgObject.S:
            if( !DataFeed.ReadSValue( tag_record ) )
            {
               Log( "Failed to read data value for " + tag_record.tag_source );
               continue;
            }

            // Push new data value into the graphics.
            viewport.SetSTag( tag_record.tag_source, tag_record.s_value, 
                              tag_record.if_changed );
            break;

          case GlgObject.G:      // Not used in this example.
          default:
            break;
         }
      }
   }
   
   /////////////////////////////////////////////////////////////////
   // Returns data for a requested dialog.
   /////////////////////////////////////////////////////////////////
   void ProcessDialogData( HttpServletRequest request, 
                          HttpServletResponse response )
   {
      String tag_source = 
        GetStringParameter( request, "dialog_type", null );

/*
      // This is an example of a response that provides dialog data for 
      // the selected object.
      if( dialog_type.equals( "Heater" ) )
      {
         WriteAsPlainText( response, 
            "<b>Solvent Heater</b><br>" +
            "Level: " + Format( data.HeaterLevel * 100. ) + " %<br>" +
            "Pressure: " + Format( data.HeaterPressure * 5. ) + " ATM.<br>" +
            "Temperature: " + Format( 50. + data.HeaterTemperature * 100. )
                            + " C\u00B0" );
         return;
      }
*/

      if( tag_source == null )
      {
         WriteAsPlainText( response, "None" );
         return;
      }

      tag_record.tag_source = tag_source;
      tag_record.data_type = GlgObject.D;

      if( !DataFeed.ReadDValue( tag_record ) )
      {
         Log( "Failed to read data value for " + tag_record.tag_source );

         WriteAsPlainText( response, "None" );
         return;
      }

      WriteAsPlainText( response, 
                        "<b>" + tag_source + "</b><br>" +
                        "Value: " + GlgObject.Printf( "%.1f", 
                                                      tag_record.d_value ) );
   }

   /////////////////////////////////////////////////////////////////
   String Format( double value )
   {
      return GlgObject.Printf( "%.2f", value );
   }
}
