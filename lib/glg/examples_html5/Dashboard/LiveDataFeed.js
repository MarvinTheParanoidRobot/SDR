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

         glg.LoadAsset( data_url, glg.GlgHTTPRequestResponseType.JSON, 
                        data_callback, user_data );


       If LoadAsset is not used, the application should issue an HTTP request
       and invoke data_callback with the received data and user_data.

       The received data should contain new_data JSON object containing an array 
       of {tag_source,value} pairs.
    */
}
    
//////////////////////////////////////////////////////////////////////////
// Write numerical value into the provided database tag. 
//////////////////////////////////////////////////////////////////////////
LiveDataFeed.prototype.WriteDValue = function ( tag_source, value )
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
LiveDataFeed.prototype.WriteSValue = function ( tag_source, value )
{
    if( IsUndefined( tag_source ) )
        return;

    // Place code here to write a string value to the specified tag.
}
