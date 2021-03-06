//////////////////////////////////////////////////////////////////////////////
// GIS Air Combat Simulation Demo
//
// This program simulates a missile fight. It creates a number of planes
// and missiles, and animates them. Every missile is following one plane 
// until it destroys the plane. Initailly, the missiles are going slower
// than the planes, but eventually they are catching up.
//
// The demo is written in pure HTML5 and JavaScript. The source code of the
// demo uses the GLG Toolkit JavaScript Library supplied by the included
// Glg*.js and GlgToolkit*.js files. The GLG library loads a GLG drawing
// and renders it on a web page, providing an API to handle user interaction
// with graphical objects in the drawing.
//
// The drawings are created using the GLG Graphics Builder, an interactive
// editor that allows to create grahical objects and define their dynamic
// behavior without any programming. The airplane and missile icons are dynamic
// GLG objects created with the Graphics Builder.
//
// The background map is generated by the GLG Map Server, which is integrated
// inside of a GLG drawing as a GLG GIS Object. The GIS Object takes a
// complete care of the map display, automatically generating Map Server
// requests to re-generate the image when the map is panned or zoomed.
//
// Except for the changes to comply with the JavaScript syntax, this source
// is identical to the source code of the corresponding C/C++/C# and Java
// desktop versions of the demo.
//
// This demo uses the GLG Map Server to display a map.
// The Map Server has to be installed either on the local host or on a
// remote web server. After the Map Server has been installed, modify the
// source code to set the SuppliedMapServerURL variable to point to the
// Map Server location.
//////////////////////////////////////////////////////////////////////////////

// Get a handle to the GLG Toolkit library.
var GLG = new GlgToolkit();

// Debugging aid: uncomment the next line to throw an exception on a GLG error.
//GLG.ThrowExceptionOnError( true, true, true );

// Set initial size of the drawing.
SetDrawingSize( false );

// Increase canvas resoultion for mobile devices.
SetCanvasResolution();

/* Load a drawing from the aircombat_demo.g file. 
   The LoadCB callback will be invoked when the drawing has been loaded.
*/
GLG.LoadWidgetFromURL( "aircombat.g", null, LoadCB, null );

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

   StartAircombatDemo( drawing );

   drawing.InitialDraw();
}

//////////////////////////////////////////////////////////////////////////////
// Control variables and constants
//////////////////////////////////////////////////////////////////////////////

/* This demo uses the GLG Map Server to display a map.
   The Map Server has to be installed either on the local host or on a
   remote web server. After the Map Server has been installed, set the 
   SuppliedMapServerURL variable to point to the Map Server location:
   for example:
     SuppliedMapServerURL = "http://localhost/cgi-bin/GlmScript";
   or
     SuppliedMapServerURL = "http://www.myserver.com/cgi-bin/GlmScript";
   The SuppliedMapServerURL value overrides the URL of the GIS object in the
   drawing.
*/
var SuppliedMapServerURL = null;    /* String */

const
   NUM_PLANE_MISSILE_PAIRS = 8, // Number of airplains.
   MAX_DISTANCE      = 0.8,     // Max allowed distance from the center,
                                // in relative units.
   DESTROY_DISTANCE  = 0.03,    // How close a missile needs to get to a plane 
                                // to destroy it (relative units).
   CHANGE_THRESHOLD  = 0.99,    // The number in the range [0;1]; defines 
                                // how often planes change direction. 
   PLANE_SPEED       = 0.01,    // Plane's speed 
   MISSILE_SPEED     = 0.6;     // Initial speed relatively to the plane
   UPDATE_INTERVAL   = 30,      // Update interval in msec
   TRANSITION_NUM    = 20,      // The number of steps required for a turn 
   DELAY             =  2,      // Delay after a turn 
   MISSILE_DELAY     = 10;      // Missile adjustment delay 

var
   NumPlanes,            /* int */
   SelectedPlane = -1;   /* int */

var
   PlaneScale = 1.0,     /* double */
   UpdateSpeed = 1.0,    /* double */
   MaxDistance;          /* double */

var
   Drawing,              /* GlgObject */
   MapViewport,          /* GlgObject */
   GISObject,            /* GlgObject */
   PlaneTemplate,        /* GlgObject */
   MissileTemplate,      /* GlgObject */
   InfoDialog;           /* GlgObject */

var
     Center = GLG.CreateGlgPoint( 0, 0, 0 ),          /* GlgPoint */
     buf_position1 = GLG.CreateGlgPoint( 0, 0, 0 ),   /* GlgPoint */
     buf_position2 = GLG.CreateGlgPoint( 0, 0, 0 );   /* GlgPoint */

var MapLoaded = false;   /* boolean */

var buf_plane = new Plane();   /* Plane */

// Array that keeps the state of the simulation (both planes and missiles).
var Planes;                    /* Plane[] */

var timer = null;

//////////////////////////////////////////////////////////////////////////////
function StartAircombatDemo( /* GlgObject */ drawing )
{
    Drawing = drawing;
    
   /* Both missiles and airplanes use the same Plane structure.
      NumPlanes is the total number of Plane structires.
   */
      
   NumPlanes = NUM_PLANE_MISSILE_PAIRS * 2;
   Planes = new Array( NumPlanes );

   /* Load plane icons and delete the icon palette from the drawing
      before it becomes visible.
   */
   LoadPlaneIcons();

   MapViewport = Drawing.GetResourceObject( "Map" );
   InfoDialog = MapViewport.GetResourceObject( "Info" );
   GISObject = MapViewport.GetResourceObject( "GISObject" );
   GetMapExtent();
   
   InfoDialog.SetDResource( "Visibility", 0.0 );
   Drawing.SetDResource( "UpdateSpeed/ValueX", UpdateSpeed );

   /* Add Input callback used to handle user interaction. */
   Drawing.AddListener( GLG.GlgCallbackType.INPUT_CB, InputCallback );

   MapViewport.SetDResource( "LoadingMessage/Visibility", 1.0 );

   Drawing.SetupHierarchy();

   if( SetDrawingSize.is_mobile )
     PlaneScale = 1.2;    // Increase initial plane scale on mobile devices.
   
   // Populate the drawing with planes: copy the plane template.
   CreatePlanes( NumPlanes );

   Drawing.Update();

   timer = setTimeout( UpdateBattlefield, UPDATE_INTERVAL );
}

//////////////////////////////////////////////////////////////////////////
function GetMapExtent()
{
   Center = GISObject.GetGResource( "GISCenter" );
   var extent = GISObject.GetGResource( "GISExtent" );   /* GlgPoint */
   MaxDistance = Math.min( extent.x / 2.0, extent.y / 2.0 ) * MAX_DISTANCE;
}

//////////////////////////////////////////////////////////////////////////
// Loads plane icons from the plane palette defined in the drawing and
// select the icons to use depending on the number of planes.
//////////////////////////////////////////////////////////////////////////
function LoadPlaneIcons()
{
   var  /* GlgObject */
     plane_palette, Plane3DFull, Plane3DLight, Plane2D, Missile3D, Missile2D;
    
   /* Find a plane palette. */
   plane_palette = Drawing.GetResourceObject( "PlanePalette" );
   
   // Find a plane's template in the drawing
   Plane3DFull  = plane_palette.GetResourceObject( "plane_3D_full" );
   Plane3DLight = plane_palette.GetResourceObject( "plane_3D_light" );
   Plane2D      = plane_palette.GetResourceObject( "plane_3D_light" );
   Missile3D    = plane_palette.GetResourceObject( "missile_3D_full" );
   Missile2D    = plane_palette.GetResourceObject( "missile_2D" );
    
    // Delete the palette from the drawing 
   Drawing.DeleteThisObject( plane_palette );
   
   if( NumPlanes <= 30 )
   {
      PlaneTemplate = Plane3DFull;
      MissileTemplate = Missile3D; 
   }
   else if( NumPlanes <= 80 )
   {
      PlaneTemplate = Plane3DLight;
      MissileTemplate = Missile3D; 
   }
    else
    {
        PlaneTemplate = Plane2D;
        MissileTemplate = Missile2D; 
    }
}

//////////////////////////////////////////////////////////////////////////
// Creates planes by copying the template and adds the planes to the
// drawing. Annotates missiles with a different size and color.
//////////////////////////////////////////////////////////////////////////
function CreatePlanes( /* int */ num_planes )
{
   for( var i=0; i<num_planes; ++i )
   {
      // Create a copy of a plane template and add it to the drawing 
      Planes[ i ] = new Plane();
      
      var name;
      if( IS_MISSILE( i ) )
      {         
         Planes[ i ].glg_object = MissileTemplate.CopyObject();
         name = "Missile " + i;
      }
      else
      {
         Planes[ i ].glg_object = PlaneTemplate.CopyObject();
         name = "Plane " + i;
      }
      
      // Store name.
      Planes[ i ].glg_object.SetSResource( "Name", name );
      
      /* Store a plane index used to identify the plane when it is selected 
         with the mouse. It is stored in a custom property attached to the 
         plane icon.
      */
      Planes[ i ].glg_object.SetDResource( "PlaneIndex", i );

      /* Add planes and missiles to GISArray of GISObject to be able to 
         position them using lat/lon coordinates.
      */
      GISObject.AddObjectToBottom( Planes[ i ].glg_object );

      // Set initial simulation parameters 
      Planes[ i ].destroyed = false;
      Planes[ i ].correcting_distance = 0;
      Planes[ i ].missile_delay = 0;
      
      if( IS_PLANE( i ) )
      {
         // It's a plane: set a random initial position
         Planes[ i ].position.x = Center.x + MaxDistance * GetData( -1.0, 1.0 );
         Planes[ i ].position.y = Center.y + MaxDistance * GetData( -1.0, 1.0 );
         Planes[ i ].position.z = Center.z + MaxDistance * GetData( -1.0, 1.0 );
      }
      else
      {
         // It's a missile: set in some proximity of a plane.
         Planes[ i ].position.x =
           Planes[ i - 1 ].position.x + MaxDistance * GetData( -0.3, 0.3 );
         Planes[ i ].position.y =
           Planes[ i - 1 ].position.y + MaxDistance * GetData( -0.3, 0.3 );
         // Set at the same depth.
         Planes[ i ].position.z = Planes[ i - 1 ].position.z;
      }

      if( IS_MISSILE( i ) )   // Missile
        // Set missiles initial speed to be lower 
        Planes[ i ].speed = MISSILE_SPEED * PLANE_SPEED;         
      else   // Plane 
        Planes[ i ].speed = PLANE_SPEED;
      
      Planes[ i ].change_state = 1;
      SetPlane( i );     // Set initial plane parameters. 
      Planes[ i ].change_state = 0;
   }

   AdjustPlanesScale( PlaneScale ); // Set initial plane scale 
}

//////////////////////////////////////////////////////////////////////////
// Simulates a plane or a missile.
//////////////////////////////////////////////////////////////////////////
function AutoPilot( /* int */ plane )
{
   if( Planes[ plane ].destroyed )
     return;
   
   if( IS_PLANE( plane ) )
   {
      if( Distance( Planes[ plane ].position, Center ) > MaxDistance )
      {
         // Change direction to correct the distance from a center.
         if( Planes[ plane ].correcting_distance == 0 )
           ChangeDirection( plane, true );
      }
      else if( GetData( 0.0, 1.0 ) > CHANGE_THRESHOLD )
        ChangeDirection( plane, false );  // Random direction change 
   }
   else   // Missile 
   {
      ++Planes[ plane ].missile_delay;
      if( ( Planes[ plane ].missile_delay % MISSILE_DELAY ) == 0 )
        // Adjust missile to the new plane position 
        ChangeDirection( plane, false );
   }
   
   // Move the plane by one simulation step. 
   MakeStep( plane, 1, null );
   
   if( !Planes[ plane ].destroyed )
     SetPlane( plane );  // Set the plane in the drawing 
}

//////////////////////////////////////////////////////////////////////////
// For a missile: adjust the missile's direction to point to the plane it
//   follows.
// For a plane:
//   a) if required==True, change the plane direction to correct the
//      distance from the center to avoid going off the screen.
//   b) if required==False, randomly change the plane's direction to make
//      it more complicated for a missile to catch it.
//////////////////////////////////////////////////////////////////////////
function ChangeDirection( /* int */ plane, /* boolean */ required )
{
   var dx, dy, dz, angle_y, angle_z;   /* double */

   if( IS_PLANE( plane ) )
   {
      // Start a new change and set flags to indicate a change in progress.
      Planes[ plane ].change_state = TRANSITION_NUM - 1; 
      if( required )
        Planes[ plane ].correcting_distance = DELAY * TRANSITION_NUM + 1;
      
      while( true )
      {
         // Choose a new random direction 
         Planes[ plane ].angle_change.x =
           60.0 * GetData( -1.0, 1.0 ) / TRANSITION_NUM;
         Planes[ plane ].angle_change.y =
           60.0 * GetData( -1.0, 1.0 ) / TRANSITION_NUM;
         Planes[ plane ].angle_change.z = 
           360.0 * GetData( 0.0, 1.0 ) / TRANSITION_NUM;

         // Test if it lowers the distance from the center, if required. 
         if( !required ||
             DecreaseDistance( plane, 0, DELAY * TRANSITION_NUM + 1 ) )
           break;
      }
   }
   else  // Missile 
   {
      // Calculate the angles to direct the missile at the plane. 
      dx = Planes[ plane - 1 ].position.x - Planes[ plane ].position.x;
      dy = Planes[ plane - 1 ].position.y - Planes[ plane ].position.y;
      dz = Planes[ plane - 1 ].position.z - Planes[ plane ].position.z;
      
      if( dx == 0.0 )
        angle_z = ( dy > 0.0 ? 90.0 : 270.0 );
      else
      {
         angle_z = Math.atan( dy / dx );
         angle_z = DEG( angle_z );
         if( dx < 0.0 )
           angle_z += 180.0;
      }
      
      if( dx == 0.0 )
        angle_y = ( dz > 0.0 ? 270.0 : 90.0 );
      else
      {
         angle_y = Math.atan( dz / dx );
         angle_y = DEG( angle_y );
      }
      
      // Change missile direction instantly (it has a much smaller inertia). 
      
      // Take x angle from a plane. 
      Planes[ plane ].angle.x = Planes[ plane - 1 ].angle.x; 
      Planes[ plane ].angle.y = angle_y;
      Planes[ plane ].angle.z = angle_z;
      Planes[ plane ].change_state = 1;
      
      // Randomly speed the missile up until it is faster then the plane. 
      if( Planes[ plane ].speed < 1.2 * PLANE_SPEED && 
          GetData( 0.0, 1.0 ) > 0.5 )
        Planes[ plane ].speed *= 1.04;
   }
}

//////////////////////////////////////////////////////////////////////////
// Calculate and return a distance between two positions.
//////////////////////////////////////////////////////////////////////////
function Distance( /* GlgPoint */ position1,
                   /* GlgPoint */ position2 )   /* double */
{
   var dx, dy, dz;    /* double */

   dx = position1.x - position2.x;
   dy = position1.y - position2.y;
   dz = position1.z - position2.z;
   
   return Math.sqrt( dx * dx + dy * dy + dz * dz );
}

//////////////////////////////////////////////////////////////////////////
/// Tests whether or not the distance will decrease after the required
/// number of simulation steps. Compares the distance after steps1 number
/// of steps with the distance after steps2 number of steps.
///////////////////////////////////////////////////////////////////////////
function DecreaseDistance( /* int */ plane,
                           /* int */ steps1, /* int */ steps2 )   /* boolean */
{
   MakeStep( plane, steps1, buf_position1 );
   MakeStep( plane, steps2, buf_position2 );
   return ( Distance( buf_position2, Center ) <
            Distance( buf_position1, Center ) );
}

//////////////////////////////////////////////////////////////////////////
// Makes the requested number of simulation steps and returns the
// calculated postion of a plane after that.
// If the position_value parameter is not NULL, it is a trial step; the
// calculated values are returned in this structure and the plane
// parametrs are restored afterwards.
// If the parameter is NULL, it is a real step affecting the plane.
//////////////////////////////////////////////////////////////////////////
function MakeStep( /* int */ plane, /* int */ num_steps,
                   /* GlgPoint */ position )
{
   if( position != null )
     buf_plane.CopyPlaneInfoFrom( Planes[ plane ] );     // Save
   
   for( var i=0; i<num_steps; ++i )
   {
      // Make a step in the current direction 
      Planes[ plane ].position.x +=
        Planes[ plane ].speed * UpdateSpeed *
        Math.cos( RAD( Planes[ plane ].angle.z ) ) *
        Math.cos( RAD( Planes[ plane ].angle.y ) );
      Planes[ plane ].position.y +=
        Planes[ plane ].speed * UpdateSpeed *
        Math.sin( RAD( Planes[ plane ].angle.z ) ) *
        Math.cos( RAD( Planes[ plane ].angle.y ) );
      Planes[ plane ].position.z +=
        Planes[ plane ].speed * UpdateSpeed *
        Math.sin( RAD( Planes[ plane ].angle.y ) ) *
        Math.cos( RAD( Planes[ plane ].angle.z ) );

      // If a direction change is in progress, adjust the plane angles. 
      if( Planes[ plane ].change_state != 0 && IS_PLANE( plane ) )
      {
         Planes[ plane ].angle.x += Planes[ plane ].angle_change.x;
         Planes[ plane ].angle.y += Planes[ plane ].angle_change.y;
         Planes[ plane ].angle.z += Planes[ plane ].angle_change.z;
         
         if( Planes[ plane ].angle.x >= 360.0 )
           Planes[ plane ].angle.x -= 360.0;
         if( Planes[ plane ].angle.y >= 360.0 )
           Planes[ plane ].angle.y -= 360.0;
         if( Planes[ plane ].angle.z >= 360.0 )
           Planes[ plane ].angle.z -= 360.0;
         
         --Planes[ plane ].change_state;
      }
      
      if( Planes[ plane ].correcting_distance != 0 )
        --Planes[ plane ].correcting_distance;
   }

   if( position != null ) // Just checking the distance: restore and return.
   {
      // Return the result 
      position.x = Planes[ plane ].position.x;
      position.y = Planes[ plane ].position.y;
      position.z = Planes[ plane ].position.z;
      
      Planes[ plane ].CopyPlaneInfoFrom( buf_plane );     // Restore
   }
   else  // Real step: if it is a missile, check if it got its plane. 
     if( IS_MISSILE( plane ) &&
         Distance( Planes[ plane - 1 ].position, Planes[ plane ].position ) <
         DESTROY_DISTANCE * MaxDistance )
     {
        // Got it: Delete both the plane and the missile. 
        DeletePlane( plane - 1, false );
        DeletePlane( plane, false );
        
        // Restart if no more planes left. 
        if( NoPlanesLeft() )
        {
           CreatePlanes( NumPlanes );
           Drawing.Update();
        }
     }
}

//////////////////////////////////////////////////////////////////////////
// Deletes the plane or missile from the drawing. Flashes the size of a
// missile for an explosive effect.
//////////////////////////////////////////////////////////////////////////
function DeletePlane( /* int */ plane, /* boolean */ deleting_all )
{
   if( plane == SelectedPlane )
     UnselectPlane();

   if( IS_MISSILE( plane ) )         /* Create an explosive effect. */
   {
      /* Speed it up for large number of planes when deleting all - 
      do it only a few times.
      */
      if( !deleting_all ||
          NumPlanes > 2 && ( ( plane - 1 ) % ( NumPlanes / 3 ) ) == 0 )
      {
         for( var i=0; i<6; ++i )
         {
            var plane_graphics = Planes[ plane ].glg_object;   /* GlgObject */
               
            var scale = plane_graphics.GetDResource( "Scale" );  /* double */
            if( ( (i+1) % 2 ) != 0 )
              plane_graphics.SetDResource( "Scale", scale );
            else
              plane_graphics.SetDResource( "Scale", scale * 1.3 );
            MapViewport.UpdateImmediately();
         }
      }
   }
   
   // Find and delete the plane from the drawing 
   if( !GISObject.DeleteThisObject( Planes[ plane ].glg_object ) )
     GLG.Error( GlgErrorType.WARNING, "Cannot find the plane." );

   Planes[ plane ].destroyed = true;
}

//////////////////////////////////////////////////////////////////////////
// Updates the plane in the drawing with the new values. Ignores 3D
// rotation for a missle.
//////////////////////////////////////////////////////////////////////////
function SetPlane( /* int */ plane )
{
   // Plane is changing its direction. 
   if( Planes[ plane ].change_state != 0 )
   {
      Planes[ plane ].glg_object.SetDResource( "AngleX",
                                               Planes[ plane ].angle.x );
      Planes[ plane ].glg_object.SetDResource( "AngleY",
                                               Planes[ plane ].angle.y );
      Planes[ plane ].glg_object.SetDResource( "AngleZ",
                                               Planes[ plane ].angle.z );

      if( IS_MISSILE( plane ) )     // Missile: an instant change 
        Planes[ plane ].change_state = 0;
   }
   
   Planes[ plane ].glg_object.SetGResource( "Position",
                                            Planes[ plane ].position.x,
                                            Planes[ plane ].position.y,
                                            Planes[ plane ].position.z );
}

//////////////////////////////////////////////////////////////////////////
// Makes one simulation step for every plane or missile.
//////////////////////////////////////////////////////////////////////////
function UpdateBattlefield()
{
   if( !MapLoaded && Math.trunc( GISObject.GetDResource( "ImageLoaded" ) ) != 0 )
   {
      // Erase map loading message.
      MapViewport.SetDResource( "LoadingMessage/Visibility", 0 );
      MapLoaded = true;
   }

   for( var i=0; i<NumPlanes; ++i )
     AutoPilot( i );
   
   DisplaySelectedPlaneInfo();
   
   // Update the drawing after changes. 
   Drawing.Update();

   // Restart update timer.
   timer = setTimeout( UpdateBattlefield, UPDATE_INTERVAL );
}

//////////////////////////////////////////////////////////////////////////
// Returns a random number in the requested range.
//////////////////////////////////////////////////////////////////////////
function GetData( /* double */ min, /* double */ max )   /* double */
{
   return GLG.Rand( min, max );
}

//////////////////////////////////////////////////////////////////////////
function DisplaySelectedPlaneInfo()
{
   if( SelectedPlane == -1 )
     return;
   
   var roll = Planes[ SelectedPlane ].angle.x;
   var pitch = Planes[ SelectedPlane ].angle.y;
   var yaw = Planes[ SelectedPlane ].angle.z - 90.0;
   
   while( roll > 180.0 )
     roll -= 360.0;
   while( pitch > 180.0 )
     pitch -= 360.0;
   while( yaw > 180.0 )
     yaw -= 360.0;
   
   InfoDialog.SetDResource( "Roll",  roll );
   InfoDialog.SetDResource( "Pitch", pitch );
   InfoDialog.SetDResource( "Yaw",   yaw );
   
   var lat_string =
     CreateLocationString( Planes[ SelectedPlane ].position.y, true );
   var lon_string = 
     CreateLocationString( Planes[ SelectedPlane ].position.x, false );

   InfoDialog.SetSResource( "Lat", lat_string);
   InfoDialog.SetSResource( "Lon", lon_string );
}

//////////////////////////////////////////////////////////////////////////
function SelectPlane( /* int */ plane_index )
{
   if( SelectedPlane == -1 )
     // No previously selected plane: activate an information popup dialog.
     InfoDialog.SetDResource( "Visibility", 1.0 );
   else 
     /* The dialog is already active, just unselect the previously selected 
        plane.
     */
     if( !Planes[ SelectedPlane ].destroyed )
       Planes[ SelectedPlane ].glg_object.SetDResource( "SelectedState", 0.0 );

   SelectedPlane = plane_index;
   
   // Display selected plane or missile info stored as its name.
   var plane_name = Planes[ SelectedPlane ].glg_object.GetSResource( "Name" );
   InfoDialog.SetSResource( "PlaneName", plane_name );

   // Set the SelectedState resource which controls an icon's color. 
   Planes[ plane_index].glg_object.SetDResource( "SelectedState", 1.0 );
}

//////////////////////////////////////////////////////////////////////////
function UnselectPlane()
{
   if( SelectedPlane == -1 )
     return;
   
   if( !Planes[ SelectedPlane ].destroyed )
     Planes[ SelectedPlane ].glg_object.SetDResource( "SelectedState", 0.0 );
   
   InfoDialog.SetDResource( "Visibility", 0.0 );
   SelectedPlane = -1;
}

//////////////////////////////////////////////////////////////////////////
// Processes buttons' and sliders' events.
//////////////////////////////////////////////////////////////////////////
function InputCallback( /* GlgObject */ vp, /* GlgObject */ message_obj )
{
   var
     origin,
     format,
     action,
     subaction;
   
   origin = message_obj.GetSResource( "Origin" );
   format = message_obj.GetSResource( "Format" );
   action = message_obj.GetSResource( "Action" );
   subaction = message_obj.GetSResource( "SubAction" );

   if( format == "Slider" )
   {
      if( action != "ValueChanged" )
        return;
      
      if( origin == "UpdateSpeed" )
        // Query the new value of the UpdateSpeed Slider (relative value).
        UpdateSpeed = message_obj.GetDResource( "ValueX" ); 
   }
   else if( format == "Button" )
   {
      if( action != "Activate" )
        return;
      
      if( origin == "Restart" )
      {
         for( var i=0; i<NumPlanes; ++i )
           if( !Planes[ i ].destroyed )
             DeletePlane( i, true );
         
         CreatePlanes( NumPlanes );
         Drawing.Update();
      }
      else if( origin == "BigPlanes" )  // Increase plane size
      {
         PlaneScale *= 1.5;
         AdjustPlanesScale( 1.5 );
      }
      else if( origin == "SmallPlanes" )  // Decrease plane size
      {
         PlaneScale /= 1.5;
         AdjustPlanesScale( 1.0 / 1.5 );
      }
      else if( origin == "ClosePlaneInfo" )
      {
         // Unselect the plane and close its information popup dialog.
         UnselectPlane();
         Drawing.Update();
      }
   }
   /* Process mouse clicks on plane icons, implemented as an Action with
      the PlaneSelection label attached to an icon and activated on a 
      mouse click. 
   */
   else if( format == "CustomEvent" )
   {
      var event_label = message_obj.GetSResource( "EventLabel" );   /* String */
      
      /* Plane icon was selected */
      if( event_label == "PlaneSelection" )
      {
         // Get plane index (int).
         var plane_index =
           Math.trunc( message_obj.GetDResource( "Object/PlaneIndex" ) );
         SelectPlane( plane_index );
         Drawing.Update(); 
      }
   }
}

//////////////////////////////////////////////////////////////////////////
function NoPlanesLeft()   /* boolean */
{
   for( var i=0; i<NumPlanes; ++i )
     if( !Planes[ i ].destroyed )
       return false;
   
   return true;
}

//////////////////////////////////////////////////////////////////////////
function AdjustPlanesScale( /* double */ factor )
{
   const MAX_PLANE_SCALE = 3.375;
   var scale;   /* double */
   
   for( i=0; i<NumPlanes; ++i )
     if( !Planes[ i ].destroyed )
     {
        scale = Planes[ i ].glg_object.GetDResource( "Scale" );
        scale *= factor;        
        if( scale > MAX_PLANE_SCALE )
          scale = MAX_PLANE_SCALE;
        
        Planes[ i ].glg_object.SetDResource( "Scale", scale );
     }
   
   Drawing.Update();
}

//////////////////////////////////////////////////////////////////////////
// Generate a location info string by converting +- sign info into the
// N/S, E/W suffixes, and decimal fraction to deg, min, sec.
//////////////////////////////////////////////////////////////////////////
function CreateLocationString( /* double */ value,
                               /* boolean */ return_lat )   /* String */
{
   var deg, min, sec;    /* int */
   var suffix;           /* String */

   if( return_lat )
   {
      if( value < 0.0 )
      {
         value = -value;
         suffix = 'S';
      }
      else
        suffix = 'N';
      
      deg = Math.trunc( value );
      min = Math.trunc( ( ( value - deg ) * 60.0 ) );
      sec = Math.trunc( ( ( value - deg - min / 60.0 ) * 3600.0 ) );
      
      return ( "" + deg + "\u00B0" + padded( min ) + "'" + padded( sec ) +
               '"' + suffix );
   }
   else   // return lon
   {
      if( value < 0.0 )
      {
         value = -value;
         suffix = 'W';
      }
      else if( value >= 360.0 )
      {
         value -= 360.0;
         suffix = 'E';
      }
      else if( value >= 180.0 )
      {
         value = 180.0 - ( value - 180.0 );
         suffix = 'W';
      }
      else
        suffix = 'E';
      
      deg = Math.trunc( value );
      min = Math.trunc( ( ( value - deg ) * 60.0 ) );
      sec = Math.trunc( ( ( value - deg - min / 60.0 ) * 3600.0 ) );
      return ( "" + deg + "\u00B0" + padded( min ) + "'" + padded( sec ) +
               '"' + suffix );
   }
}

//////////////////////////////////////////////////////////////////////////
// Pads the value with 0 if needed to have a constant field width. 
//////////////////////////////////////////////////////////////////////////
function padded( /* int */ value )   /* String */
{
   if( value < 10 )
     return "0" + value;
   else
     return "" + value;
}

//////////////////////////////////////////////////////////////////////////
function IS_MISSILE( /* int */ plane )    /* boolean */
{
   /* Missiles use odd indices, planes use even ones. */
   return ( plane % 2 ) != 0;
}

//////////////////////////////////////////////////////////////////////////
function IS_PLANE( /* int */ plane )      /* boolean */
{
   return !IS_MISSILE( plane );
}

//////////////////////////////////////////////////////////////////////////
// Converts degrees to radians.
//////////////////////////////////////////////////////////////////////////
function RAD( /* double */ angle )   /* double */
{
   return angle / 180.0 * Math.PI;
}

//////////////////////////////////////////////////////////////////////////
// Converts radians to degrees.
//////////////////////////////////////////////////////////////////////////
function DEG( /* double */ angle )   /* double */
{
   return angle / Math.PI * 180.0;
}

//////////////////////////////////////////////////////////////////////////////
// Changes drawing size while maintaining width/height aspect ratio.
//////////////////////////////////////////////////////////////////////////////
function SetDrawingSize( next_size )
{
   const ASPECT_RATIO = 800 / 700;

   // Settings for desktop displays.
   const MIN_WIDTH = 600;
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
   GLG.SetCanvasScale( coord_scale, 1.3, 0.6 );
   
   /* Mobile devices use fixed device-width: disable Change Drawing Size
      button.
   */
   var change_size_button = document.getElementById( "change_size" );
   if( change_size_button != null )
     change_size_button.parentNode.removeChild( change_size_button );

   return coord_scale;      // Chosen coord scale for mobile devices.
}

//////////////////////////////////////////////////////////////////////////
// Support class, keeps data for one airplane or missile.
//////////////////////////////////////////////////////////////////////////
function Plane()
{
   this.glg_object = null;        /* GlgObject */
   this.speed = 0;                /* double */
   this.change_state = 0;         /* int */
   this.correcting_distance = 0;  /* int */

   /* True initially: no graphics created for the plane yet. */
   this.destroyed = true;         /* boolean */

   /* Prevents from changing for some time after a turn to let the plane 
      correct the distance.
   */
   this.missile_delay = 0;        /* int */

   this.position     = GLG.CreateGlgPoint( 0, 0, 0 );   /* GlgPoint */
   this.direction    = GLG.CreateGlgPoint( 0, 0, 0 );   /* GlgPoint */
   this.angle        = GLG.CreateGlgPoint( 0, 0, 0 );   /* GlgPoint */
   this.angle_change = GLG.CreateGlgPoint( 0, 0, 0 );   /* GlgPoint */
}

///////////////////////////////////////////////////////////////////////
// A partial copy.
///////////////////////////////////////////////////////////////////////
Plane.prototype.CopyPlaneInfoFrom = function( /* Plane */ from )
{
   this.position.x = from.position.x;
   this.position.y = from.position.y;
   this.position.z = from.position.z;
   
   this.angle.x = from.angle.x;
   this.angle.y = from.angle.y;
   this.angle.z = from.angle.z;
   
   this.angle_change.x = from.angle_change.x;
   this.angle_change.y = from.angle_change.y;
   this.angle_change.z = from.angle_change.z;
   
   this.change_state = from.change_state;
   this.correcting_distance = from.correcting_distance;
}
