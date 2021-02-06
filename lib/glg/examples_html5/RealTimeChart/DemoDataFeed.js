/////////////////////////////////////////////////////////////////////////
// DemoDataFeed provides simulated data for demo, as well as for testing 
// with no LiveDataFeed.
// In an application, data will be coming from LiveDataFeed.
//////////////////////////////////////////////////////////////////////////

/* Set to true to use demo data file in JSON format.
   Set to false to generate demo data in memory.
*/
var USE_DEMO_DATA_FILE = false;

// Demo data file to be used for testing JSON format for process_overview.g.
var DEMO_DATA_FILE_JSON = "DemoDataFile_JSON.txt"; 

function DemoDataFeed()
{
    // Initialize datafeed as needed.
    this.Initialize();

    // Used to generate simulated demo data.
    this.counter = 0;
    
    this.FIRST_ALARM_RAISE_THRESHOLD = 0.6;
    this.ALARM_RAISE_THRESHOLD = 0.8;
    
    this.OLD_ACK_ALARM_AGE = 15;
    this.OLD_NON_ACK_ALARM_AGE = 30;
    this.NUM_SIMULATED_ALARMS = 30;
    
    // Keeps a list of active alarms for simulation.
    this.ActiveAlarmList = [];   /* AlarmRecord[] */
}

//////////////////////////////////////////////////////////////////////////
DemoDataFeed.prototype.Initialize = function()
{
    // Do nothing for the simulated data. For the live data,
    // provide a custom implementation of this method in LiveDataFeed.
}

//////////////////////////////////////////////////////////////////////////
// Query new data values. 
// tag_list
//    An array of tag sources (strings) attached to the plots in a chart
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
//////////////////////////////////////////////////////////////////////////
DemoDataFeed.prototype.ReadData = function( tag_list, data_callback, user_data )
{
    if( USE_DEMO_DATA_FILE )
    {
        // Get data from a URL (file is used for demo).
    
        /* Use absolute URL path. Relative file path can be used as well
           and passed to LoadAsset.
        */
        var data_file_url = new URL( DEMO_DATA_FILE_JSON, window.location.href );
        GLG.LoadAsset( data_file_url.toString(), 
                       GLG.GlgHTTPRequestResponseType.JSON, 
                       data_callback, user_data );
    }
    else
    {
        /* Create a JSON object from tag_list to be sent to the server.
           For demo purposes, the new data values are generated in
           memory in JSON format.
        */
        var tag_list_JSON = JSON.stringify( tag_list );
        
        //  Generate random data values in memory.
        this.GetDemoData( tag_list_JSON, data_callback, user_data  );
    }
}

//////////////////////////////////////////////////////////////////////////
// Check if the value is valid for this datasample. The value must be a double. 
// For demo purposes, the function always returns true. 
// The application should provide a custom  implementation of this method 
// in LiveDataFeed. If the function returns false, the plot's ValidEntryPoint 
// will be 0 and the plot will have a hole for this datasample.
//////////////////////////////////////////////////////////////////////////
DemoDataFeed.prototype.IsValid = /* bool */ 
    function( /*String*/ tag_source, /*double*/ value )
{
    return true;
}

//////////////////////////////////////////////////////////////////////////
// Generate simulated demo data for all tags listed in tag_list_JSON.
// Simulates the http response the application will create
// using custom http request for data acquisition.
//////////////////////////////////////////////////////////////////////////
DemoDataFeed.prototype.GetDemoData = 
    function( tag_list_JSON, data_callback, user_data )
{
    var tag_list = JSON.parse( tag_list_JSON );
    var new_data = [];
    var 
      value,
      time_stamp;

    for( var i=0; i<tag_list.length; ++i )
    {
        value = this.GetDemoValue( tag_list[i].tag_source, false );
        time_stamp = GetCurrTime();

        // Add new element to the new_data array.
        new_data.push( { tag_source: tag_list[i].tag_source, 
                         value: value, time_stamp: time_stamp } );
    }

    // Invoke the callback with new_data.
    data_callback( new_data, user_data );
}

//////////////////////////////////////////////////////////////////////////
// Generate a simulated numerical data value. 
//////////////////////////////////////////////////////////////////////////
DemoDataFeed.prototype.GetDemoValue =    /* double */
    function( /*String*/ tag_source, /*bool*/ historical_mode )
{
    var  /* double */
      center,
      amplitude,
      period,
      low,
      high;
    
    if( tag_source.startsWith( "Volts" ) )
    {
        low = 0.;
        high = 500.;
        period = 300.0;
        amplitude = 40.0;
        center = 380.0;
    }
    else if( tag_source.startsWith( "Amp" ) )
    {
        low = 0.;
        high = 50.;
        period = 1000.0;
        amplitude = 15.0;
        center = 30.0;
    }
    else 
    {
        low = 0.;
        high = 50.;
        period = 1000.0;
        amplitude = 30.0;
        center = 50.0;
    }
        
    if( historical_mode )
        // Historical data, data were saved once per second.
        this.counter += 10;  
    else
        // Real-time mode.
        ++this.counter; 
    
    var alpha = 2.0 * Math.PI * this.counter / period;
    var value = center + 
        amplitude * Math.sin( alpha ) * Math.sin( alpha / 30.0 );
    
    return value;
}

/////////////////////////////////////////////////////////////////////// 
// Get histrorical data for the plot with a specified tag.
/////////////////////////////////////////////////////////////////////// 
DemoDataFeed.prototype.GetHistPlotData = /* PlotDataPoint[] */
    function ( /*String*/ tag_source, /*double*/ start_time, 
               /*double*/ end_time, /*int*/ max_num_samples,
               /*callback*/ data_callback, /*user data*/ user_data)
{
    /* In a real application, the number of data points to be queried
       is determined by the start and end time. For the demo, return
       the requested max number of points.
    */
    if( max_num_samples < 1 )
        max_num_samples = 1;
    var num_samples = max_num_samples;   /* int */
    
    var interval = ( end_time - start_time ) / max_num_samples;   /* double */

    var data_array = [];   /* PlotDataPoint[] */
    for( var i=0; i<num_samples; ++i )
    {
        /* Generate demo data. */
        var value = this.GetDemoValue( tag_source, /*historical*/ true );
        var time_stamp = start_time + interval * i;
        var value_valid = true;
        
        var data_point = new PlotDataPoint( value, time_stamp, value_valid );
        data_array.push( data_point );
    }
    
    // Invoke data callback.
    data_callback( data_array, user_data );
}
