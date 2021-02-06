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
// Write numerical value into the provided database tag. 
//////////////////////////////////////////////////////////////////////////
DemoDataFeed.prototype.WriteDValue = 
    function ( /*String*/ tag_source, /*double*/ value )
{
    if( IsUndefined( tag_source ) )
        return;

    // DEMO only: Set value for a specified tag in the currently loaded drawing.
    MainViewport.SetDTag( tag_source, value );
}

//////////////////////////////////////////////////////////////////////////
// Write string value into the provided database tag. 
//////////////////////////////////////////////////////////////////////////
DemoDataFeed.prototype.WriteSValue = 
    function ( /*String*/ tag_source, /*String*/ value )
{
    if( IsUndefined( tag_source ) )
        return;

    // DEMO only: Set value for a specified tag in the currently loaded drawing.
    MainViewport.SetSTag( tag_source, value );
}

//////////////////////////////////////////////////////////////////////////
DemoDataFeed.prototype.ACKAlarm = /*boolean*/
    function( /*String*/ tag_source )
{
    /* Simulate alarm ACK in the demo mode. In a real application, 
       LiveDataFeed will send alarm ACK to the process data server.
    */
    
    var num_alarms = this.ActiveAlarmList.length;   /* int */ 
    
    // Find the alarm in the active alarm list and reset it's ACK flag.
    for( var i=0; i<num_alarms; ++i )
    {
        var alarm = this.ActiveAlarmList[i];   /* AlarmRecord */
        if( alarm.tag_source == tag_source )
        {
            alarm.ack = true;
            console.log( "Acknowledging alarm: " + tag_source );
            return true;
        }
    }
    return false;
} 

//////////////////////////////////////////////////////////////////////////
DemoDataFeed.prototype.GetAlarms = function( alarm_callback, user_data )
{
    var alarm;   /* AlarmRecord */
    
    /* Simulate alarms. */
    
    /* Ages alarms and removes old alarms from the list for continuous
       simulation.
    */
    this.AgeAlarms( this.ActiveAlarmList );
    var num_active_alarms = this.ActiveAlarmList.length;   /* int */
    
    var alarm_raise_threshold =    /* double */
    ( num_active_alarms == 0 ?
      this.FIRST_ALARM_RAISE_THRESHOLD : this.ALARM_RAISE_THRESHOLD );
    
    // Add new simulated alarm (conditionally).
    if( num_active_alarms < this.NUM_SIMULATED_ALARMS &&          
        GLG.Rand( 0.0, 1.0 ) > alarm_raise_threshold )
    {
        alarm = this.GetAlarmData(); 
        this.ActiveAlarmList.push( alarm );
        num_active_alarms = this.ActiveAlarmList.length;
    }

    var alarm_list = null;
    if( num_active_alarms > 0 )
    {
        // Create a new list of alarms to be returned.
        alarm_list = [];   /* AlarmRecord[] */
        
        /* For simulating alarms in the demo, populate the list with alarms from 
           ActiveAlarmList.
        */
        for( var i=0; i<num_active_alarms; ++i )
        {
            alarm = this.ActiveAlarmList[i];
            alarm_list.push( alarm );
        }
    }

    // Invoke the callback with new alrm list.
    alarm_callback( alarm_list );
} 

//////////////////////////////////////////////////////////////////////////
// If the incoming value is used in a chart's plot, check if the value
// is valid for this datasample. The value must be a double. 
// For demo purposes, the function always returns true. 
// The application should provide a custom  implementation of this method 
// in LiveDataFeed. If the function returns false, the plot's ValidEntryPoint 
// will be 0 and the plot will have a hole for this datasample.
//////////////////////////////////////////////////////////////////////////
DemoDataFeed.prototype.IsValid = /* bool */ 
    function( /*String*/ tag_source, /*int*/ data_type, value )
{
    var result = false;
    switch( data_type )
    {
    case GLG.GlgDataType.D: 
        result = true;
        break;
    default:
        break;
    } 

    return result;
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
        // DEMO only: don't push new data to tags with TagSource="State". 
        if( tag_list[i].tag_source == "State" )
            continue;

        switch( tag_list[i].data_type )
        {
        case GLG.GlgDataType.D:
            // Obtain new numerical data value for a specified tag_source.
            value = this.GetDemoValue( tag_list[i].tag_source, false );
            break;
        case GLG.GlgDataType.S:
            // Obtain new string value for a specified tag_source.
            value = this.GetDemoString( tag_list[i].tag_source );
            break;
        }

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
    
    switch( PageType )
    {
    case RTCHART_PAGE:
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
        break;

    case TEST_COMMANDS_PAGE:
        if( tag_source.toLowerCase().includes( "fuel" ) )
        {
            low = 0.;
            high = 500.;
            period = 1000.0;
            amplitude = 80.;
            center = 350.;
        }
        else if( tag_source.toLowerCase().includes( "temp" ) )
        {
            low = 0.;
            high = 90.;
            period = 1000.0;
            amplitude = 25.;
            center = 50.;
        }
        else  //default
        {
            low = 0.;
            high = 100.;
            period = 500.0;
            amplitude = ( high - low ) / 2.0;
            center = low + amplitude;
        }

        if( tag_source.toLowerCase().includes( "low" ) )
            return low;
        else if( tag_source.toLowerCase().includes( "high" ) )
            return high;

        if( historical_mode )
            // Historical data, data were saved once per second.
            this.counter += 10;
        else
          ++this.counter; 
        break;

    case DEFAULT_PAGE_TYPE:
    default:
        low = 0.;
        high = 100;
        period = 500.0;
        amplitude = ( high - low ) / 2.0;
        center = low + amplitude;

        ++this.counter; 
        break;
    }

    var alpha = 2.0 * Math.PI * this.counter / period;
    var value = center + 
        amplitude * Math.sin( alpha ) * Math.sin( alpha / 30.0 );
    
    return value;
}

/////////////////////////////////////////////////////////////////////// 
DemoDataFeed.prototype.GetDemoString =    /* String */
    function( /*String*/ tag_source )
{
    return "DEMO";
}

/////////////////////////////////////////////////////////////////////// 
// Ages alarms and removes old alarms from the list for continuous
// simulation.
///////////////////////////////////////////////////////////////////////
DemoDataFeed.prototype.AgeAlarms = function( /* AlarmRecord[] */ alarm_list )
{
    var num_alarms = alarm_list.length;
    for( var i=0; i<num_alarms; ++i )
    {
        var alarm = alarm_list[i];    /* AlarmRecord */
        ++alarm.age;
        
        if( alarm.ack && alarm.age > this.OLD_ACK_ALARM_AGE ||
            !alarm.ack && alarm.age > this.OLD_NON_ACK_ALARM_AGE )
        {
            this.RemoveArrayElement( alarm_list, alarm );
            return;
        }
    }
}

////////////////////////////////////////////////////////////////////////
DemoDataFeed.prototype.RemoveArrayElement = function( array, data )
{
   for( var i=0; i<array.length; ++i )
     if( array[i] == data )
     {
        array.splice( i, 1 );
        return;
     }
}  

//////////////////////////////////////////////////////////////////////////
DemoDataFeed.prototype.GetAlarmData = function()   /* AlarmRecord */
{
    /* Simulate alarms for the demo. In a real application, LiveDataSource
       will query the list of alarms from the process data server.
    */
    
    var alarm_status = Math.trunc( GLG.Rand( 1.0, 3.99 ) );   /* int */
    
    var ch = ( 'A' + Math.trunc( GLG.Rand( 0.0, 26.9 ) ) );          /* String */
    var alarm_source = ch + Math.trunc( GLG.Rand( 100.0, 999.0 ) );  /* String */
    
    var description;          /* String */
    var string_value = null;  /* String  */
    var double_value = 0.0;   /* double */
    var random_message;       /* int */
    
    if( alarm_status < 3 )
    {
        random_message = ( GLG.Rand( 0.0, 10.0 ) < 5.0 ? 0 : 1 );
        
        switch( random_message )
        {
        default:
        case 0:
            description = "Tank #% low";
            double_value = GLG.Rand( 100.0, 150.0 ); 
            break;
            
        case 1: 
            description = "Tank #% high";
            double_value = GLG.Rand( 600.0, 900.0 ); 
            break;
        }
    }
    else
    {
        random_message = Math.trunc( GLG.Rand( 0.0, 2.99 ) );
        switch( random_message )
        {
        default:
        case 0: description = "Breaker #%"; string_value = "TRIPPED"; break;
        case 1: description = "Fuse #%"; string_value = "BLOWN"; break;
        case 2: description = "Tank #%"; string_value = "OVERFLOW"; break;
        }
    }
    
    var random_number = Math.trunc( GLG.Rand( 1.0, 100.0 ) );   /* int */
    description = GLG.CreateIndexedName( description, random_number );
    
    var alarm_time = GetCurrTime();   /* double */
    
    var alarm = new AlarmRecord();
    alarm.time = alarm_time;
    alarm.tag_source = alarm_source;
    alarm.description = description;
    alarm.string_value = string_value;
    alarm.double_value = double_value;
    alarm.status = alarm_status;
    alarm.ack = false;
    return alarm;
}

/////////////////////////////////////////////////////////////////////// 
// Get histrorical data for the plot with a specified tag.
/////////////////////////////////////////////////////////////////////// 
DemoDataFeed.prototype.GetPlotData = /* PlotDataPoint[] */
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
