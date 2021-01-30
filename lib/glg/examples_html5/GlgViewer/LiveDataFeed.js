/////////////////////////////////////////////////////////////////////////
// Application should provide a custom implementation of LiveDataFeed
// to query real-time data from a custom data source.
/////////////////////////////////////////////////////////////////////////
function LiveDataFeed()
{
    // Initialize datafeed as needed.
    this.Initialize();
}

//////////////////////////////////////////////////////////////////////////
LiveDataFeed.prototype.Initialize = function()
{
    // Place custom initialization code here.
}

//////////////////////////////////////////////////////////////////////////
// Query new data values. 
// Parameters:
// tag_list:
//    An array of objects with the following properties:
//      tag_source - tag source as stored in the drawing, 
//      data_type  - tag data type, for example GLG.GlgDataType.D,
//                   GLG.GlgDataType.S, GLG.GlgDataType.G.
//    The tag_list can be passed to the server to indicate which tags to obtain 
//    new data values for.
// data_callback:
//    The callback function to be invoked when the data query is finished.
//    The callback should be invoked with the new_data array containing
//    an array of objects with the following properties:
//    new_data[i].tag_source
//    new_data[i].value
// user_data:
//    User data to be passed to the data_callback.
// 
// GLG LoadAsset function can be used to invoke the provided URL, and 
// upon completion, the specified data_callback will be invoked with 
// new_data formed from the URL response. 
//////////////////////////////////////////////////////////////////////////
LiveDataFeed.prototype.ReadData = function( tag_list, data_callback, user_data )
{
    /* Create a JSON object from the provided tag_list and pass it to
       the server to query real-time data. 
    */
    var tag_list_JSON = JSON.stringify( tag_list );

    // Build a custom URL as needed, passing tag_list_JSON.
    var data_url = "http://myserver/pathname?action=read&tags=" + tag_list_JSON;

    /* Issue http request to get new data. 
       GLG LoadAsset method can be used to issue the request of a 
       specified type and invoke data_callback when the data has been received,
       for example:

        GLG.LoadAsset( data_url, glg.GlgHTTPRequestResponseType.JSON, 
                       data_callback, user_data );

       If LoadAsset is not used, the application should issue an HTTP request
       and invoke data_callback with the received data and user_data.

       The received data should contain new_data JSON object containing an array 
       of objects with the following properties: {tag_source,value,time_stamp}.
    */
}
    
//////////////////////////////////////////////////////////////////////////
// Write numerical value into the provided database tag. 
//////////////////////////////////////////////////////////////////////////
LiveDataFeed.prototype.WriteDValue = 
    function ( /*String*/ tag_source, /*double*/ value )
{
    if( IsUndefined( tag_source ) )
        return;

    /* Example:
       var tag_JSON = JSON.stringify( { tag_source: tag_source, value: value } );
       var data_url = "http://myserver/pathname?action=write&tag=" + tag_JSON;
       
       // Place code here to issue http request.
    */
}

//////////////////////////////////////////////////////////////////////////
// Write string value into the provided database tag. 
//////////////////////////////////////////////////////////////////////////
LiveDataFeed.prototype.WriteSValue = 
    function ( /*String*/ tag_source, /*String*/ value )
{
    if( IsUndefined( tag_source ) )
        return;

    // Place code here to write a string value to the specified tag.
}


//////////////////////////////////////////////////////////////////////////
// Place custom code here to build alarm_list, each item is an AlarmRecord.
//////////////////////////////////////////////////////////////////////////
LiveDataFeed.prototype.GetAlarms = function( alarm_callback, user_data )
{
    // Create a new list of alarms to be returned.
    var alarm_list = [];   /* AlarmRecord[] */
    
    /*
    var num_active_alarms = .. 
    for( var i=0; i<num_active_alarms; ++i )
    {
       var alarm = new AlarmRecord();
       alarm.time = alarm_time;
       alarm.tag_source = 
       alarm.description = 
       alarm.string_value = 
       alarm.double_value = 
       alarm.status = 
       alarm.ack = false;
       alarm_list.push( alarm );
    }
    */    

    // Invoke the callback with new alrm list.
    alarm_callback( alarm_list );
} 

//////////////////////////////////////////////////////////////////////////
// Plcae custom code to process alarm acknowledgement for a specified
// tag source.
//////////////////////////////////////////////////////////////////////////
LiveDataFeed.prototype.ACKAlarm = /*boolean*/
    function( /*String*/ tag_source )
{
    var result = false;

    // Write a new value to the back end system to acknowledge an alarm.
    // result = 

    return result;
} 

/////////////////////////////////////////////////////////////////////// 
// Get histrorical data for the plot with a specified tag.
/////////////////////////////////////////////////////////////////////// 
LiveDataFeed.prototype.GetPlotData = /* PlotDataPoint[] */
    function ( /*String*/ tag_source, /*double*/ start_time, 
               /*double*/ end_time, /*int*/ max_num_samples,
               /*callback*/ data_callback, /*user data*/ user_data)
{
    /* Place custom application code here to obtain historical
       data. Build data_array[] with PlotDataPoint objects and
       invoke data_callback.
    */

    /* EXAMPLE
    var data_array = [];
    for( var i=0; i<num_samples; ++i )
    {
       var value =
       var time_stamp =
       var value_valid =

       var data_point = new PlotDataPoint( value, time_stamp, value_valid );
       data_array.push( data_point );
    }

    data_callback( data_array, user_data );
    */
}

//////////////////////////////////////////////////////////////////////////
// If the incoming value is used in a chart's plot, check if the value
// is valid for this datasample. If the function returns false, 
// the plot will have a hole for this datasample (plot's ValidEntryPoint=0).
//////////////////////////////////////////////////////////////////////////
LiveDataFeed.prototype.IsValid = /* bool */ 
    function( /*String*/ tag_source, /*int*/ data_type, value )
{
    var result = false;
    switch( data_type )
    {
    case GLG.GlgDataType.D: 
        /* Place custom application code here to validate the value of
           type double.
        */
        result = true;

    default:
        break;
    } 

    return result;
}
