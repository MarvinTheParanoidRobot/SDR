/////////////////////////////////////////////////////////////////////////
// DemoDataFeed provides simulated data for demo, as well as for testing 
// with no LiveDataFeed.
// In an application, data will be coming from LiveDataFeed.
//////////////////////////////////////////////////////////////////////////

/* Set to true to use demo data file in JSON format.
   Set to false to generate demo data in memory.
*/
var USE_DEMO_DATA_FILE = false;

// Demo data file to be used for testing JSON format.
var DEMO_DATA_FILE_JSON = "DemoDataFile_JSON.txt"; 

function DemoDataFeed()
{
    // Initialize datafeed as needed.
    this.Initialize();

    // Used to generate simulated demo data.
    this.counter = 0;
}

//////////////////////////////////////////////////////////////////////////
DemoDataFeed.prototype.Initialize = function()
{
    // Do nothing for the simulated data. For the live data,
    // provide a custom implementation of this method in LiveDataFeed.
}

//////////////////////////////////////////////////////////////////////////
// Query new data values. 
// Parameters:
// tag_list:
//    An array of strings representing tag sources in the drawing, 
//    can be passed to the server to indicate which tags to obtain 
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
        glg.LoadAsset( data_file_url.toString(), 
                       glg.GlgHTTPRequestResponseType.JSON, 
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
DemoDataFeed.prototype.WriteDValue = function ( tag_source, value )
{
    if( IsUndefined( tag_source ) )
        return;

    // DEMO only: Set value for a specified tag in the currently loaded drawing.
    Viewport.SetDTag( tag_source, value );
}

//////////////////////////////////////////////////////////////////////////
// Write string value into the provided database tag. 
//////////////////////////////////////////////////////////////////////////
DemoDataFeed.prototype.WriteSValue = function ( tag_source, value )
{
    if( IsUndefined( tag_source ) )
        return;

    // DEMO only: Set value for a specified tag in the currently loaded drawing.
    Viewport.SetSTag( tag_source, value );
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
    var value;

    for( var i=0; i<tag_list.length; ++i )
    {
        // DEMO only: don't push new data to tags with TagSource="State". 
        if( tag_list[i].tag_source == "State" )
            continue;

        // Obtain new data value for a specified tag_source.
        value = this.GetDemoValue( tag_list[i].tag_source );
    
        // Add new element to the new_data array.
        new_data.push( { tag_source: tag_list[i].tag_source, value: value } );
    }

    // Invoke the callback with new_data.
    data_callback( new_data, user_data );
}

//////////////////////////////////////////////////////////////////////////
// Generate a simulated numerical data value. 
//////////////////////////////////////////////////////////////////////////
DemoDataFeed.prototype.GetDemoValue = function( tag_source )
{
    var low = 0.0;
    var high = 100.0;
    var period = 500;
    var half_amplitude = ( high - low ) / 2.0;
    var center = low + half_amplitude;
    var alpha = 2.0 * Math.PI * this.counter / period;
    
    var value = center + 
        half_amplitude * Math.sin( alpha ) * Math.sin( alpha / 30.0 );
    
    this.counter++;
    return value;
}

