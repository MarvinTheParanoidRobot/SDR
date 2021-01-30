<!--

//////////////////////////////////////////////////////////////////////////
// See glg_script.txt for descriptions of public methods.
//////////////////////////////////////////////////////////////////////////

function GlgSO()
{
   //////////////////////////////////////////////////////////////////////////
   // Do not set the below variables directly - use the corresponding
   // Set methods.
   //////////////////////////////////////////////////////////////////////////
   this.image_object = null;
   this.url_base = null;
   this.size_string = null;
   this.custom_param_func = null;
   this.pre_update_cb = null;
   this.after_update_cb = null;
   this.update_interval = 0;
   this.idle_timeout = 600;
   this.tooltip_object = null;
   this.tooltip_data_object = null;
   this.tooltip_interval = 500;
   this.dialog_object = null;
   this.dialog_data_object = null;
   this.two_stage_dialog = false;
   this.active_dialog_type;
   this.click_data_cb = null;
   this.dialog_data_cb = null;
   this.erase_dialog_cb = null;
   this.start_updates_button = null;
   this.stop_updates_button = null;
   this.tooltip_timeout_id = 0;     
   this.tooltip_active = false;
   this.last_x = -1;
   this.g_last_y = -1;
   this.interval_id = 0;
   this.idle_timeout_id = 0;
   //this.IE  = document.all
   this.ns6 = document.getElementById&&!document.all;
}

// Specifies the name of the HTML image element that receives the servlet's 
// output.
//
GlgSO.prototype.SetImageID = function( name )
{
   this.image_object = document.getElementById( name );
   if( !this.image_object )
     alert( "Invalid image name." );
   
   // Set servlet size to match the size of the image.
   this.SetServletSize( this.image_object.width, this.image_object.height );
}

// Supplies the base name of the servlet URL.
//
GlgSO.prototype.SetServletBase = function( base_name )
{
   this.url_base = base_name;
}

// Specifies servlet's size.
//
GlgSO.prototype.SetServletSize = function( width, height )
{
   this.size_string = "width=" + width + "&height=" + height;
}

// Specifies servlet image update interval.
//
GlgSO.prototype.SetUpdateInterval = function( sec )
{
   this.update_interval = sec;
}

// Specifies the period of inactivity (in seconds) to stop updates after.
//
GlgSO.prototype.SetIdleTimeout = function( sec )
{
   this.idle_timeout = sec;
}

// Specifies the name of the HTML StartUpdates button. The button's enabled
// state will be updated by the StartUpdates and StopUpdates methods.
//
GlgSO.prototype.SetStartButtonID = function( name )
{
   this.start_updates_button = document.getElementById( name );
   if( !this.start_updates_button )
     alert( "Invalid StartUpdates button name." );
}
   
// Specifies the name of the HTML StopUpdates button. The button's enabled
// state will be updated by the StartUpdates and StopUpdates methods.
//
GlgSO.prototype.SetStopButtonID = function( name )
{
   this.stop_updates_button = document.getElementById( name );
   if( !this.stop_updates_button )
     alert( "Invalid StopUpdates button name." );
}

// Specifies names of HTML elements used to display the tooltip:
//   top_name - the top-level container
//   containt_name - the element that receives tooltip data
//
GlgSO.prototype.SetTooltipID = function( top_name, content_name )
{
   this.tooltip_object = document.getElementById( top_name );
   this.tooltip_data_object = document.getElementById( content_name );
   
   if( !this.tooltip_object || !this.tooltip_data_object )
     alert( "Invalid tooltip names." );
}
   
// Specifies tooltip timeout - how long the mouse has to hover over an object
// for the tooltip to be activated.
//
GlgSO.prototype.SetTooltipTimeout = function( millisec )
{
   this.tooltip_interval = millisec;
}
   
// Specifies names of HTML elements used to display the dialog:
//   top_name - the top-level container
//   containt_name - the element that receives dialog data
//
GlgSO.prototype.SetDialogID = function( top_name, content_name )
{
   this.dialog_object = document.getElementById( top_name );
   this.dialog_data_object = document.getElementById( content_name );
   
   if( !this.dialog_object || !this.dialog_data_object )
     alert( "Invalid dialog names." );
}
      
// For two stage dialogs, the result of the MouseClick query 
// (usually the name of the selected object) will be used passed to 
// a second query to fetch dialog data for that object.
// For the one stage dialogs the result of the query will be displayed 
// in the dialog right away.
//
GlgSO.prototype.SetTwoStageDialog = function( true_or_false )
{
   this.two_stage_dialog = true_or_false;
}

// Specifies the method to call to obtain additional request parameters.
// This parameters will be passed to the servlet to alter the generated 
// image.
//
GlgSO.prototype.SetCustomRequestParam = function( func )
{
   this.custom_param_func = func;
}
 
// Specifies the method to be invoked before each image update.
//
GlgSO.prototype.SetPreUpdateCB = function( func )
{
   this.pre_update_cb = func;
}
   
// Specifies the method to be invoked after each image update.
//
GlgSO.prototype.SetAfterUpdateCB = function( func )
{
   this.after_update_cb = func;
}
   
// Specifies the method to be invoked before processing returned click data.
//
GlgSO.prototype.SetClickDataCB = function( func )
{
   this.click_data_cb = func;
}
   
// Specifies the method to be invoked before processing returned dialog data.
//
GlgSO.prototype.SetDialogDataCB = function( func )
{
   this.dialog_data_cb = func;
}

// Specifies the method to be invoked when a click dialog is erased.
//
GlgSO.prototype.SetEraseDialogCB = function( func )
{
   this.erase_dialog_cb = func;
}
   
// Fetches a new servlet image.
//
GlgSO.prototype.UpdateImage = function()
{
   if( this.pre_update_cb && !pre_update_cb() )
     return;
   
   var custom_request_param = "";
   if( this.custom_param_func )
     custom_request_param = this.custom_param_func();
   
   // Append current time to the URL prevent caching of the image.
   var time_string = "&time=" + new Date().getTime()
   
   // Form the URL string.
   var url = this.url_base + "?" + this.size_string + 
             custom_request_param + time_string;
   
   // Reload image with new data.
   this.image_object.src = url;         

   this.UpdateActiveDialog();
   
   if( this.after_update_cb )
     this.after_update_cb();
}

// Stops servlet image updates, disables the Stop button and enables the
// Start button (if defined).
//
GlgSO.prototype.StopUpdates = function()
{
   if( this.interval_id )
   {
      clearInterval( this.interval_id );
      this.interval_id = 0;
   }
   
   if( this.idle_timeout_id )
   {
      clearTimeout( this.idle_timeout_id );
      this.idle_timeout_id = 0;
   }
   
   if( this.start_updates_button )
     this.start_updates_button.disabled = false; // enable
   if( this.stop_updates_button )
     this.stop_updates_button.disabled = true;   // disable
}
   
// Starts servlet image updates, disables the Start button and enables the
// Start button (if defined). Also invokes UpdateImage() if update_now is true.
//
GlgSO.prototype.StartUpdates = function( update_now )
{
   this.StopUpdates();
   
   if( this.update_interval < 30 )
   {
      if( this.update_interval > 0 )
        alert( "Update interval is too small, ignoring updates!" );
      return;
   }

   if( update_now )
     this.UpdateImage();
   
   var _this = this;    // Bind a proper this
   
   this.interval_id = setInterval( function() { _this.UpdateImage(); }, 
                                   this.update_interval );
   
   // Update state of Start/Stop buttons.
   if( this.start_updates_button )
     this.start_updates_button.disabled = true;   // disable
   if( this.stop_updates_button )
     this.stop_updates_button.disabled = false;   // enable
   
   // Stop updates after 10 minutes of inactivity.
   if( this.idle_timeout > 0 )
     this.idle_timeout_id = 
       setTimeout( function() { _this.LongIdling(); }, 
                   this.idle_timeout * 1000 );
}

// Stops updates after 10 minutes of inactivity to save bandwidth,
// and displays a warning.
//
GlgSO.prototype.LongIdling = function()
{
   this.idle_timeout_id = 0;   /* reset stored id */
   this.StopUpdates();
   alert( "Stopped updates after 10 minutes of inactivity. Click OK to resume." );

   this.UpdateImage();    // Update now
   this.StartUpdates();
}

// Processes mouse click events: queries the servlet to find selected object 
// and displays a popup dialog with returned detailed data.
//
GlgSO.prototype.ProcessClick = function( event )
{
   if( !this.dialog_object )
   {
      alert( "No dialog object defined." );
      return;
   }

   this.EraseTooltip();
   this.EraseDataDialog();
   
   // Coordinates of the mouse click
   var position_string = 
         "&x=" + this.GetEventX( event ) + "&y=" + this.GetEventY( event );
      
   var custom_request_param = "";
   if( this.custom_param_func )
     custom_request_param = this.custom_param_func();

   // Append current time to the URL to prevent caching.
   var time_string = "&time=" + new Date().getTime()
      
   var url = 
      this.url_base + "?action=ProcessEvent&event_type=MouseClick&" + 
      this.size_string +            // Process selection for the curr. size!
      custom_request_param + position_string + time_string;

   this.GetData( this, url, "click" );
}

// Displays a dialog with returned data.
//
GlgSO.prototype.ProcessClickData = function( xml_http )
{
   if( xml_http.readyState != 4 )
     return;
      
   if( xml_http.status == 200 )
     this.DisplayDataDialog( xml_http );
   else
     alert( "Can't process selection, returned status: " + xml_http.status );
}
   
// Displays a dialog with returned data.
//
GlgSO.prototype.DisplayDataDialog = function( xml_http )
{
   if( xml_http.responseText != "None" )
   {
      if( this.click_data_cb && 
          !this.click_data_cb( xml_http.responseText ) )
        return;   // Skip dialog display if click_data_cb returned false.
      
      if( this.two_stage_dialog )
      {
         this.PositionObject( this.dialog_object );
         
         // Two-stage: send the result of the first request to fetch dialog 
         // data.
         this.active_dialog_type = "&dialog_type=" + xml_http.responseText;
         this.UpdateActiveDialog();
      }
      else
      {
         this.PositionObject( this.dialog_object );
         this.dialog_object.style.display = "";
         this.dialog_data_object.innerHTML = xml_http.responseText;
      }
   }
   else
     this.EraseDataDialog();
}
   
// Fetches dialog data for two-stage dialogs.
//
GlgSO.prototype.ProcessDialogData = function( xml_http )
{
   if( xml_http.readyState != 4 )
     return;
   
   if( xml_http.status == 200 )   
   {  
      if( xml_http.responseText != "None" )
      {
         if( this.dialog_data_cb && 
             !this.dialog_data_cb( xml_http.responseText ) )
           return;   // Skip data display if dialog_data_cb returned false.
         
         this.dialog_object.style.display = "";
         document.getElementById( "dialog_data" ).innerHTML = 
           xml_http.responseText;
      }
      else
        this.EraseDataDialog();
   }
   else
   {
      alert( "Can't process dialog data, returned status: " + xml_http.status );
      this.EraseDataDialog();
   }
}

// Updates a currently active two-stage dialog (if any) with new data.
//
GlgSO.prototype.UpdateActiveDialog = function( xml_http )
{
   if( !this.active_dialog_type )
     return;    // No active dialog.
   
   // Append current time to the URL to prevent caching.
   var time_string = "&time=" + new Date().getTime()
   
   var url = this.url_base + "?action=GetDialogData&" + 
             this.active_dialog_type + time_string;
   
   this.GetData( this, url, "dialog_data" );
}

// Erases the currently active dialog.
//
GlgSO.prototype.EraseDataDialog = function()
{
   if( this.dialog_object != null )
   {
      this.dialog_object.style.display = "none";
      if( this.erase_dialog_cb )
        this.erase_dialog_cb();
      
      this.active_dialog_type = null;
   }
}
   
// Starts a tooltip timer.
//
GlgSO.prototype.ProcessTooltip = function( event )
{
   if( !this.tooltip_object )
   {
      alert( "No tooltip object defined." );
      return;
   }
   
   this.tooltip_active = false;  // Mouse moved: discard any pending tooltip.
   
   this.EraseTooltip();          // Erase any displayed tooltips.
   
   if( !event ) // Mouse moved out: don't start tooltip timer.
     return;
   
   // Store tooltip coordinates and start tooltip timeout.
   this.last_x = this.GetEventX( event );
   this.last_y = this.GetEventY( event );
   
   this.StartTooltipTimeout();
}

GlgSO.prototype.StartTooltipTimeout = function()
{
   var _this = this;    // Bind a proper this

   this.tooltip_timeout_id = 
      setTimeout( function(){ _this.QueryTooltip(); }, this.tooltip_interval );
}

GlgSO.prototype.StopTooltipTimeout = function()
{
   if( this.tooltip_timeout_id )
   {
      clearInterval( this.tooltip_timeout_id );
      this.tooltip_timeout_id = 0;
   }
}

// Queries the servlet to find an object with a tooltip at the mouse location
// and displays the tooltip data, if any.
//
GlgSO.prototype.QueryTooltip = function()
{
   this.tooltip_active = true;
   
   // Tooltip coordinates: last pos.
   var position_string = "&x=" + this.last_x + "&y=" + this.last_y;
   
   var custom_request_param = "";
   if( this.custom_param_func )
     custom_request_param = this.custom_param_func();

   // Append current time to the URL to prevent caching of the request url.
   var time_string = "&time=" + new Date().getTime();
      
   // Find tooltip at the requested location.
   var url = 
      this.url_base + "?action=ProcessEvent&event_type=Tooltip&" + 
      this.size_string +         // Process selection for the curr. size!
      custom_request_param + position_string + time_string;

   this.GetData( this, url, "tooltip" );
}

// Displays a tooltip that shows returned data.
//
GlgSO.prototype.ProcessTooltipData = function( xml_http )
{
   if( xml_http.readyState != 4 )
     return;
   
   if( xml_http.status == 200 )   
     this.PopupTooltip( xml_http.responseText );
   else
     alert( "Can't process selection, returned status: " + xml_http.status );
}

// Popup tooltip if the mouse hasn't moved while the data were being queried.
//
GlgSO.prototype.PopupTooltip = function( tooltip_string )
{
   if( !this.tooltip_active )   // Mouse moved: don't popup.
     return;
   
   if( tooltip_string == "None" )
     return;   // No tooltip found in the drawing at requested location.
   
   this.PositionObject( this.tooltip_object, this.last_x, this.last_y );
   
   this.tooltip_object.style.display = "";
   this.tooltip_data_object.innerHTML = tooltip_string;
}

// Erases an active toolip, if any.
//
GlgSO.prototype.EraseTooltip = function()
{
   if( !this.tooltip_object )
     return;
      
   this.StopTooltipTimeout();
   this.tooltip_object.style.display = "none";
}
   
// Positions tooltip or dialog on top of the servlet's image.
//
GlgSO.prototype.PositionObject = function( object )
{
   var x = this.last_x + this.GetOffsetX( this.image_object ) + 10;
   var y = this.last_y + this.GetOffsetY( this.image_object ) + 10;
      
   if( this.ns6 )
   {
      object.style.left = x;
      object.style.top = y;
   }
   else
   {
      object.style.pixelLeft = x;
      object.style.pixelTop = y;
   }
}
   
// Returns the X mouse position relatively to the image.
//
GlgSO.prototype.GetEventX = function( event )
{
   return event.offsetX ? event.offsetX : 
   event.pageX - this.GetOffsetX( this.image_object );
}

// Returns the Y mouse position relatively to the image.
//
GlgSO.prototype.GetEventY = function( event )
{
   return event.offsetY ? event.offsetY : 
   event.pageY - this.GetOffsetY( this.image_object );
}
   
// Returns X offset of the element relative to the document.
GlgSO.prototype.GetOffsetX = function( element )
{
   var offset_x = 0;

   for( var curr = element; curr; curr = curr.offsetParent )
      offset_x += curr.offsetLeft;

   return offset_x;
}

// Returns Y offset of the element relative to the document.
GlgSO.prototype.GetOffsetY = function( element )
{
   var offset_y = 0;

   for( var curr = element; curr; curr = curr.offsetParent )
      offset_y += curr.offsetTop;

   return offset_y;
}

// Queries dialog or tooltip data at the selected image location defined
// by the url string.
//
GlgSO.prototype.GetData = function( gso, url, event_type )
{
   var xml_http = this.GetXmlHttp();
   if( !xml_http )
     return false;
   
   xml_http.open( "GET", url, true );
   
   if( event_type === "tooltip" )
     xml_http.onreadystatechange = 
       function (){ gso.ProcessTooltipData( xml_http ); };
   else if( event_type === "click" )
     xml_http.onreadystatechange = 
       function (){ gso.ProcessClickData( xml_http ); };
   else if( event_type === "dialog_data" )
     xml_http.onreadystatechange = 
       function (){ gso.ProcessDialogData( xml_http ); };
   else
     alert( "Invalid event type in GetData()." );
   
   xml_http.send( null );
   return true;
}
   
// Executes HTTP query.
GlgSO.prototype.GetXmlHttp = function()
{
   var xml_http = null;
   
   if( window.XMLHttpRequest )   // Mozilla and new IE
   {
      xml_http = new XMLHttpRequest();
   }
   else if( window.ActiveXObject )  // Older IE
   {
      var i;
      var version_strings = 
      new Array( "MSXML2.XMLHttp.7.0",
                 "MSXML2.XMLHttp.6.0",
                 "MSXML2.XMLHttp.5.0",
                 "MSXML2.XMLHttp.4.0",
                 "MSXML2.XMLHttp.3.0",
                 "MSXML2.XMLHttp",
                 "Microsoft.XMLHttp" );
      
      for( i=0; i < version_strings.length; i++ )
      {
         try
         {
            xml_http = new ActiveXObject( version_strings[i] );
            if( xml_http )
            {
               //alert( "Using " + version_strings[i] );
               break;
            }
         }
         catch( err )
         {
            //alert( version_strings[i] + " not supported." );
         }
      }
   }
   
   if( !xml_http )
     alert( "Your browser does not support XMLHttpRequest, please upgrade." );
   return xml_http;
}

//-->
