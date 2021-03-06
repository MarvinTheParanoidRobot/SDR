//////////////////////////////////////////////////////////////////////////////
// Base class that defines methods used by pages of the GLG SCADAViewer.
//////////////////////////////////////////////////////////////////////////////
function HMIPageBase(){}

//////////////////////////////////////////////////////////////////////////////
// Returns an update interval in msec for animating drawing with data.
//////////////////////////////////////////////////////////////////////////////
HMIPageBase.prototype.GetUpdateInterval = function()   /* int */
{
   return 1000;    // Update once a second by default.
}

//////////////////////////////////////////////////////////////////////////////
// A custom update method for animating drawing with data; may be used
// to implement any additional data update logic.
//////////////////////////////////////////////////////////////////////////////
HMIPageBase.prototype.UpdateData = function()   /* boolean */
{
   /* Return false to automatically update all tags defined in the drawing 
      (via the UpdateData method of the GlgSCADAViewer class).
   */
   return false;
}

//////////////////////////////////////////////////////////////////////////////
// A custom input handler for the page. If it returns false, the default
// input handler of the SCADA Viewer will be used to process common
// events and commands.
//////////////////////////////////////////////////////////////////////////////
HMIPageBase.prototype.InputCallback =    /* boolean */
  function( /* GlgObject */ viewport, /* GlgObject */ message_obj )
{
   return false;
}
 
//////////////////////////////////////////////////////////////////////////////
// A custom trace callback for the page. If it returns false, the default 
// trace callback of the SCADA Viewer will be used to process events.
//////////////////////////////////////////////////////////////////////////////   
HMIPageBase.prototype.TraceCallback =    /* boolean */
  function( /* GlgObject */ viewport, /* GlgTraceData */ trace_info )
{
   return false;
}
   
//////////////////////////////////////////////////////////////////////////////
// Returns true if tag sources need to be remapped for the page,
// in which case RemapTagObject() must provide code to perform desired
// remapping logic.
//////////////////////////////////////////////////////////////////////////////
HMIPageBase.prototype.NeedTagRemapping = function()   /* boolean */
{
   return false;
}
   
//////////////////////////////////////////////////////////////////////////////
// Used if NeedTagRemapping() returns true.
// Reassigns TagSource parameter for a given tag object to a new
// TagSource value. tag_source and tag_name parameters are the current 
// TagSource and TagName of the tag_obj.
//////////////////////////////////////////////////////////////////////////////
HMIPageBase.prototype.RemapTagObject =
  function( /* GlgObject */ tag_obj, /* String */ tag_name,
            /* String */ tag_source ){}

//////////////////////////////////////////////////////////////////////////////
// Performs any desired initialization of the drawing before and after
// hierarchy setup.
//////////////////////////////////////////////////////////////////////////////
HMIPageBase.prototype.InitBeforeSetup = function(){}
HMIPageBase.prototype.InitAfterSetup = function(){}

//////////////////////////////////////////////////////////////////////////////
// Invoked when the page has been loaded and the tags have been remapped.
//////////////////////////////////////////////////////////////////////////////
HMIPageBase.prototype.Ready = function(){}

//////////////////////////////////////////////////////////////////////////////
// Performs layout adjustments (if any) to display the page on mobile devices.
//////////////////////////////////////////////////////////////////////////////
HMIPageBase.prototype.AdjustForMobileDevices = function(){}

//////////////////////////////////////////////////////////////////////////////
// Handles zoom commands from HTML buttons.
//////////////////////////////////////////////////////////////////////////////
HMIPageBase.prototype.PerformZoom = function( /* String */ zoom_type )
{
   // Use global PerformZoom function if defined.
   if( typeof PerformZoom !== "undefined" )
     PerformZoom( zoom_type );
}
