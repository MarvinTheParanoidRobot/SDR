//////////////////////////////////////////////////////////////////////////////
// GLG Electric Circuit Demo
//
// The demo is written in pure HTML5 and JavaScript. The source code of the
// demo uses the GLG Toolkit JavaScript Library supplied by the included
// Glg*.js and GlgToolkit*.js files.
//
// The library loads a GLG drawing and renders it on a web page, providing
// an API to animate the drawing with real-time data and handle user
// interaction with graphical objects in the drawing.
//
// The drawings are created using the GLG Graphics Builder, an interactive
// editor that allows to create grahical objects and define their dynamic
// behavior without any programming.
//////////////////////////////////////////////////////////////////////////////

// Get a handle to the GLG Toolkit library.
var GLG = new GlgToolkit();

// Debugging aid: uncomment the next line to throw an exception on a GLG error.
//GLG.ThrowExceptionOnError( true, true, true );

/* Load a drawing from the electric_circuit.g file. 
   The LoadCB callback will be invoked when the drawing has been loaded.
*/
GLG.LoadWidgetFromURL( "electric_circuit.g", null, LoadCB, null );

// Set initial size of the drawing.
SetDrawingSize( false );

// Increase canvas resoultion for mobile devices.
var CoordScale = SetCanvasResolution();

//////////////////////////////////////////////////////////////////////////////
function LoadCB( drawing, data, path )
{
   if( drawing == null )
   {
      window.alert( "Can't load drawing, check console message for details." );
      return;
   }
   
   // Define the element in the HTML page to display the drawing in.
   drawing.SetParentElement( "glg_area" );
   
   // Disable viewport border to use the border of the glg_area.
   drawing.SetDResource( "LineWidth", 0 );

   StartCircuitDemo( drawing );
}

//////////////////////////////////////////////////////////////////////////////
// Control variables and constants
//////////////////////////////////////////////////////////////////////////////

// Graphics update interval.
const UPDATE_INTERVAL = 500;    // msec
var viewport;
var timer = null;
var ResourceList;  // Array of resources to update, queried from the drawing.
var UpdateWithData = true;

/* If true, the resource path is used to animate resources of the drawing.
   If false, stored resource ID is used to set resource directly with 
   null path using the Extended API. 
   Alternatively, tags may be used instead of resources.
*/
const USE_RESOURCE_PATH = false;

//////////////////////////////////////////////////////////////////////////////
function StartCircuitDemo( drawing )
{
   viewport = drawing;

   AdjustForMobileDevices();

   viewport.AddListener( GLG.GlgCallbackType.INPUT_CB, InputCallback );

   viewport.InitialDraw();

   // Invoke after InitialDraw, which sets up subdrawings used in the drawing.
   InitializeAnimation( viewport );

   // Start periodic updates.
   timer = setTimeout( UpdateCircuit, UPDATE_INTERVAL );
}

//////////////////////////////////////////////////////////////////////////////
// Creates a list of resources to animate.
//////////////////////////////////////////////////////////////////////////////
function InitializeAnimation( viewport )
{
   ResourceList = GetResourceList( viewport, null, null );
   if( ResourceList == null )
     window.alert( "No resources to animate." );
}

//////////////////////////////////////////////////////////////////////////////
function UpdateCircuit()
{
   if( UpdateWithData )
   {
      var size = ResourceList.GetSize();
      for( var i=0; i<size; ++i )
      {
         var resource = ResourceList.GetElement( i );
         
         if( resource.type != GLG.GlgDataType.D ) 
           continue;      // Update only resources of D type
         
         /* Animate using random data. In a real application, live data will
            be queried and used to animate the drawing.
         */
         var value = GLG.Rand( 0, resource.range );
         if( USE_RESOURCE_PATH )
           // Use resource path.
           viewport.SetDResource( resource.resource_path, value );
         else
           /* Use stored resource ID with null path to set the resource 
              directly using the Extended API.
           */
           resource.glg_object.SetDResource( null, value );

         viewport.Update();
      }
   }

   // Restart update timer
   timer = setTimeout( UpdateCircuit, UPDATE_INTERVAL );
}

//////////////////////////////////////////////////////////////////////////////
// Handles user input: in this demo, it handles clicks on the buttons.
//////////////////////////////////////////////////////////////////////////////
function InputCallback( vp, message_obj )
{
   var origin = message_obj.GetSResource( "Origin" );
   var format = message_obj.GetSResource( "Format" );
   var action = message_obj.GetSResource( "Action" );
   // var subaction = message_obj.GetSResource( "SubAction" );

   if( format== "Button" )         /* Handle button clicks */
   {
      if( action == "Activate" )
      {
         if( origin == "Resources" )
           PrintResources();
         else if( origin == "ToggleUpdates" )
           ToggleUpdates();
      }
   }
}

//////////////////////////////////////////////////////////////////////////////
function ToggleUpdates()
{
   UpdateWithData = !UpdateWithData;
}

//////////////////////////////////////////////////////////////////////////////
// Prints to the console resources of the drawing used for animation
// (resources whose name starts with the "#" character).
//////////////////////////////////////////////////////////////////////////////
function PrintResources()
{
   if( ResourceList == null )
   {
      window.alert( "Found no resources to update!" );
      return;
   }

   console.log( "Resource list for updates: resource_path type" );
   
   var size = ResourceList.GetSize();
   for( i=0; i<size; ++i )
   {
      var resource = ResourceList.GetElement( i );

      var data_type_str;
      switch( resource.type )
      {
       case GLG.GlgDataType.D: data_type_str = "d"; break;
       case GLG.GlgDataType.S: data_type_str = "s"; break;
       case GLG.GlgDataType.G: data_type_str = "g"; break;
       default:
         console.error( "Invalid resource type." ); 
         continue;
      }
      
      console.log( resource.resource_path + " " + data_type_str );
   }

   console.log( "Resource list: Done." );
   window.alert( "Printed resource list to the console." );
}

//////////////////////////////////////////////////////////////////////////////
// Creates a list of resources in the drawing that need to be animated
// by traversing all resources of the drawing and creating a list of resources
// of interest that are marked by the "#" characted at the beginning of their
// names. 
//
// Alternatively, tags may be used instead of resources. Tags are reported
// as a flat list and are easier to process for a typical process control
// application.
//////////////////////////////////////////////////////////////////////////////
function GetResourceList( obj, res_path, list )
{
   /* Using only named resources in this example, no aliases or 
      default attribute names.
   */
   var res_list = obj.CreateResourceList( true, false, false );
   if( res_list == null )
     return list;

   var size = res_list.GetSize();
   for( var i=0; i<size; ++i )
   {
      var glg_object = res_list.GetElement( i );

      var name = glg_object.GetSResource( "Name" );
      if( !name.startsWith( "#" ) )
        continue;  // We are interested only in resources that start with #
       
      // Accumulate resource path.
      var new_path;
      if( res_path == null )
        new_path = name;
      else
        new_path = res_path + "/" + name;

      var object_type = glg_object.GetDResource( "Type" );

      // Data or attribute object: add to the list of resources to animate.
      if( object_type == GLG.GlgObjectType.DATA || 
          object_type == GLG.GlgObjectType.ATTRIBUTE )
      {
         var data_type = glg_object.GetDResource( "DataType" );

         /* Set range for animating the resource.
            State resources may have ON (1) and OFF (0) values - 
            use 1.3 as a range to simulate. Use range=100 for the rest 
            of resources.
         */
         var range;
         if( name == "#State" )
           range = 1.3;
         else
           range = 1000.0;

         var resource =
           new AnimatedResource( glg_object, data_type, new_path, range );
         
         // Create a list of does not yet exist.
         if( list == null )
           list =
             GLG.CreateObject( GLG.GlgObjectType.ARRAY,
                               GLG.GlgContainerType.GLG_OBJECT, 0, 0, null );

         list.AddObjectToBottom( resource );
      }

      var has_resources = glg_object.GetDResource( "HasResources" );

      /* If object's HasResources=ON, recursively traverse all resources
         inside it.
      */
      if( has_resources == 1 )
         list = GetResourceList( glg_object, new_path, list );
   }
   return list;
}

//////////////////////////////////////////////////////////////////////////////
// This object keeps parameters of resources that will be animated with data.
// This demo uses simulated data to animate these resources.
// In a real application, live process data will be queried and used for
// animation.
//////////////////////////////////////////////////////////////////////////////
function AnimatedResource( glg_object, data_type, resource_path, range )
{
   this.glg_object = glg_object;
   this.type = data_type;
   this.resource_path = resource_path;
   this.range = range;
}

//////////////////////////////////////////////////////////////////////////////
function AdjustForMobileDevices()
{
   /* Increase button lengths for small screens to fit labels.
      Scale transformations attached to both buttons are constrained: 
      setting the scale for one button increases lengths of both buttons.
   */
   if( screen.width <= 600 || CoordScale != 1.0 )
     viewport.SetDResource( "ToggleUpdates/XScale", 1.15 );
   
   // Erase "Print Resources to Console" button on mobile devices.   
   if( CoordScale != 1.0 )
     viewport.SetDResource( "Resources/Visibility", 0 );
}
          
//////////////////////////////////////////////////////////////////////////////
// Changes drawing size while maintaining width/height aspect ratio.
//////////////////////////////////////////////////////////////////////////////
function SetDrawingSize( next_size )
{
   const ASPECT_RATIO = 800 / 700;

   // Settings for desktop displays.
   const MIN_WIDTH = 500;
   const MAX_WIDTH = 1000;
   const SCROLLBAR_WIDTH = 15;
   
   if( SetDrawingSize.size_index == undefined )   // first time
   {
      SetDrawingSize.size_index = 0;

      SetDrawingSize.small_sizes       = [ 1, 1.5,  2.,   2.5 ];
      SetDrawingSize.medium_sizes      = [ 1, 0.75, 1.25, 1.5 ];
      SetDrawingSize.large_sizes       = [ 1, 0.6,  1.25, 1.5 ];
      SetDrawingSize.num_sizes = SetDrawingSize.small_sizes.length;
      SetDrawingSize.is_mobile = ( screen.width <= 760 );

      window.addEventListener( "resize", ()=>{ SetDrawingSize( false ) } );
   }
   else if( next_size )
   {
      ++SetDrawingSize.size_index;
      SetDrawingSize.size_index %= SetDrawingSize.num_sizes;
   }

   var drawing_area = document.getElementById( "glg_area" );
   if( SetDrawingSize.is_mobile )
   {
      /* Mobile devices use constant device-width, adjust only the height 
         of the drawing to keep the aspect ratio.
      */
      drawing_area.style.height =
        "" + Math.trunc( drawing_area.clientWidth / ASPECT_RATIO ) + "px";
   }
   else   /* Desktop */
   {
      var span = document.body.clientWidth; 
      if( !SetDrawingSize.is_mobile )
        span -= SCROLLBAR_WIDTH;

      var start_width;
      if( span < MIN_WIDTH )
        start_width = MIN_WIDTH;
      else if( span > MAX_WIDTH )
        start_width = MAX_WIDTH;
      else
        start_width = span;

      var size_array;
      if( span < 600 )
        size_array = SetDrawingSize.small_sizes;
      else if( span < 800 )
        size_array = SetDrawingSize.medium_sizes;
      else
        size_array = SetDrawingSize.large_sizes;

      var size_coeff = size_array[ SetDrawingSize.size_index ];
      var width = Math.trunc( Math.max( start_width * size_coeff, MIN_WIDTH ) );
   
      drawing_area.style.width = "" + width + "px";
      drawing_area.style.height = "" + Math.trunc( width / ASPECT_RATIO ) + "px";
   }
}

//////////////////////////////////////////////////////////////////////////////
// Increases canvas resolution for mobile devices with HiDPI displays.
// Returns chosen coordinate scale factor.
//////////////////////////////////////////////////////////////////////////////
function SetCanvasResolution()
{
   // Set canvas resolution only for mobile devices with devicePixelRatio != 1.
   if( window.devicePixelRatio == 1. || !SetDrawingSize.is_mobile )
     return 1.0;   // Use coord scale = 1.0 for desktop.
   
   /* The first parameter defines canvas coordinate scaling with values 
      between 1 and devicePixelRatio. Values greater than 1 increase 
      canvas resolution and result in sharper rendering. The value of 
      devicePixelRatio may be used for very crisp rendering with very thin lines.

      Canvas scale > 1 makes text smaller, and the second parameter defines
      the text scaling factor used to increase text size.

      The third parameter defines the scaling factor that is used to
      scale down text in native widgets (such as native buttons, toggles, etc.)
      to match the scale of the drawing.
   */
   var coord_scale = 2.0;
   GLG.SetCanvasScale( coord_scale, 1.75, 0.6 );
   
   // Mobile devices use fixed device-width: disable Change Drawing Size button.
   var change_size_button = document.getElementById( "change_size" );
   if( change_size_button != null )
     change_size_button.parentNode.removeChild( change_size_button );

   return coord_scale;      // Chosen coord scale for mobile devices.
}
