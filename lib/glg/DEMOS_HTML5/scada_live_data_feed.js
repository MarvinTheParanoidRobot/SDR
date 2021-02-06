//////////////////////////////////////////////////////////////////////////
// Provide custom code to read and write real-time data values.
//
// A sample code that shows how to use asynchronous HTTP requests to query
// live data from a server in a JSON format may be found in the GlgViewer
// example in the examples_html5 directory of the GLG installation.
//////////////////////////////////////////////////////////////////////////
function LiveDataFeedObj()
{
   // Place custom code here to initialize data feed.
   //
   // InitDataBase();
}

/////////////////////////////////////////////////////////////////////// 
LiveDataFeedObj.prototype.ReadDTag =    /* boolean */
  function( /* GlgTagRecord */ tag_record, /* DataPoint */ data_point )
{
   // Place code here to query and return double tag value.
   // The name of the tag is provided in tag_record.tag_source.
   
   data_point.d_value = 0.0;
   data_point.time_stamp = GetCurrTime();
   return true;
}

/////////////////////////////////////////////////////////////////////// 
LiveDataFeedObj.prototype.ReadSTag =    /* boolean  */
  function( /* GlgTagRecord */ tag_record, /* DataPoint */ data_point )
{
   // Place code here to query and return string tag value.
   // The name of the tag is provided in tag_record.tag_source.
   
   data_point.s_value = "no value";
   data_point.time_stamp = GetCurrTime();
   return true;
}

/////////////////////////////////////////////////////////////////////// 
LiveDataFeedObj.prototype.WriteDTag =    /* boolean  */
  function( /* String */ tag_source, /* DataPoint */ data_point )
{
   // Place code here to write double tag value.
   // The tag value is provided in data_point.d_value.
   
   return true;
}
   
/////////////////////////////////////////////////////////////////////// 
LiveDataFeedObj.prototype.WriteSTag =    /* boolean */
  function( /* String */ tag_source, /* DataPoint */ data_point )
{
   // Place code here to write double tag value.
   // The tag value is provided in data_point.s_value.
   
   return true;
}

//////////////////////////////////////////////////////////////////////////
// Obtain an array of historical data for a given tag source.
//////////////////////////////////////////////////////////////////////////
LiveDataFeedObj.prototype.GetPlotData =    /* PlotDataPoint[] */
  function( /* String */ tag_source, /* double */ start_time,
            /* double */ end_time, /* int */ max_num_samples )
{
   /* Place code here to query historical data for the plot specified by
      the provided tag_source.
      See the GetPlotData() method of DemoDataFeed for an example of 
      creating an array of plot data points.
   */
   return null;
}

//////////////////////////////////////////////////////////////////////////
// Queries alarm data from the controlled process.
// 
//////////////////////////////////////////////////////////////////////////
LiveDataFeedObj.prototype.GetAlarms = function()   /* AlarmRecord[] */
{
   /* Place code here to query a list of alarms.
      See the GetAlarms() method of DemoDataFeed for an example of 
      creating an array of alarm records.
   */
   return null;
}

//////////////////////////////////////////////////////////////////////////
// Send alarm acknowledgement for the alarm associated with the specified 
// tag.
//////////////////////////////////////////////////////////////////////////
LiveDataFeedObj.prototype.ACKAlarm =    /* boolean */
  function( /* String */ tag_source )
{
   // Place code here to send alarm ACK for the specified tag.
   return true;
}
