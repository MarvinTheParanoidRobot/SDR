//////////////////////////////////////////////////////////////////////////
// DemoDataFeed provides SIMULATED DATA for demo, as well as for testing 
// with no LiveDataFeed.
//
// In an application, data will be coming from LiveDataFeed, which will use
// live data obtained from the server via asynchronous HTTP requests.
//
// An example of using asynchronous HTTP requests to query live data from a
// server in a JSON format may be found in the GlgViewer example in the
// examples_html5 directory of the GLG installation.
//////////////////////////////////////////////////////////////////////////
function DemoDataFeedObj()
{
   this.counter = 0;
   this.tag_index = 0;

   this.FIRST_ALARM_RAISE_THRESHOLD = 0.9;
   this.ALARM_RAISE_THRESHOLD = 0.8;
      
   this.OLD_ACK_ALARM_AGE = 15;
   this.OLD_NON_ACK_ALARM_AGE = 30;
   this.NUM_SIMULATED_ALARMS = 30;

   // Keeps a list of active alarms for simulation.
   this.ActiveAlarmList = [];   /* AlarmRecord[] */
}

/////////////////////////////////////////////////////////////////////// 
DemoDataFeedObj.prototype.ReadDTag =   /* boolean */
  function( /* GlgTagRecord */ tag_record, /* DataPoint*/ data_point )
{
   data_point.d_value = this.GetDemoValue( tag_record.tag_source, false );
   data_point.time_stamp = GetCurrTime();
   return true;
}

/////////////////////////////////////////////////////////////////////// 
DemoDataFeedObj.prototype.ReadSTag =   /* boolean  */
  function( /* GlgTagRecord */ tag_record, /* DataPoint */ data_point )
{
   data_point.s_value = this.GetDemoString( tag_record.tag_source );
   data_point.time_stamp = GetCurrTime();
   return true;
}

/////////////////////////////////////////////////////////////////////// 
DemoDataFeedObj.prototype.WriteDTag =    /* boolean */
  function( /* String */ tag_source, /* DataPoint */ data_point )
{
   /* In demo mode, set the tag in the drawing to simulate round trip
      after the write operation.
   */
   return MainViewport.SetDTag( tag_source, data_point.d_value, false );
}

DemoDataFeedObj.prototype.WriteSTag =    /* boolean */
  function( /* String */ tag_source, /* DataPoint */ data_point )
{
   /* In demo mode, set the tag in the drawing to simulate round trip
      after the write operation.
   */
   return MainViewport.SetSTag( tag_source, data_point.s_value, false );
}

//////////////////////////////////////////////////////////////////////////
// Queries alarm data from the controlled process, uses simulated alarms
// for the demo.
//////////////////////////////////////////////////////////////////////////
DemoDataFeedObj.prototype.GetAlarms = function()  /* AlarmRecord[] */
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

   if( num_active_alarms == 0 )
     return null;   // No alarms.

   // Create a new list of alarms to be returned.
   var alarm_list = [];   /* AlarmRecord[] */
     
   /* For simulating alarms in the demo, populate the list with alarms from 
      ActiveAlarmList.
   */
   for( var i=0; i<num_active_alarms; ++i )
   {
      alarm = this.ActiveAlarmList[i];
      alarm_list.push( alarm );
   }
   
   return alarm_list;
}
   
/////////////////////////////////////////////////////////////////////// 
// Ages alarms and removes old alarms from the list for continuous
// simulation.
///////////////////////////////////////////////////////////////////////
DemoDataFeedObj.prototype.AgeAlarms = function( /* AlarmRecord[] */ alarm_list )
{

   var num_alarms = alarm_list.length;
   for( var i=0; i<num_alarms; ++i )
   {
      var alarm = alarm_list[i];    /* AlarmRecord */
      ++alarm.age;

      if( alarm.ack && alarm.age > this.OLD_ACK_ALARM_AGE ||
          !alarm.ack && alarm.age > this.OLD_NON_ACK_ALARM_AGE )
      {
         RemoveArrayElement( alarm_list, alarm );
         return;
      }
   }
}

////////////////////////////////////////////////////////////////////////
function RemoveArrayElement( array, data )
{
   for( var i=0; i<array.length; ++i )
     if( array[i] == data )
     {
        array.splice( i, 1 );
        return;
     }
}  
   
/////////////////////////////////////////////////////////////////////// 
// Generates demo data value.
///////////////////////////////////////////////////////////////////////
DemoDataFeedObj.prototype.GetDemoValue =   /* double */
  function( /* String */ tag_source, /* boolean */ historical_mode )
{
   switch( PageType )
   {
    case AERATION_PAGE: 
      if( tag_source.includes( "Speed" ) )
        return GLG.Rand( 300.0, 1500.0 );
      else
        return GLG.Rand( 0.0, 10.0 );

    case CIRCUIT_PAGE:
      if( tag_source.endsWith( "State" ) )
        return GLG.Rand( 0.0, 1.3 );
      else
        return GLG.Rand( 0.0, 1000.0 );
    
    case RT_CHART_PAGE:
      var  /* double */
        center,
        amplitude,
        period, alpha,
        value;

      if( tag_source.startsWith( "Volts" ) )
      {
         center = 380.0;
         amplitude = 40.0;
         period = 300.0;
      }
      else
      {
         center = 30.0;
         amplitude = 15.0;
         period = 1000.0;
      }
    
      alpha = 2.0 * Math.PI * this.counter / period;        
      value = center + 
      amplitude * Math.sin( alpha ) * Math.sin( alpha / 30.0 );
    
      if( historical_mode )
        this.counter += 10;  // Historical data, data were saved once per second.
      else
        ++this.counter;      // Real-time mode: updates 10 times per second.
      return value;

    case TEST_COMMANDS_PAGE:
      /* Generate demo data in the range [10,90] for the real-time chart
         in the popup dialog.
      */
      period = 100.0 * ( 1.0 + this.tag_index * 2.0 );
      alpha = 2.0 * Math.PI * this.counter / period;
      value = 50.0 + 40.0 * Math.sin( alpha ) * Math.sin( alpha / 30.0 );
            
      ++this.tag_index;
      if( this.tag_index >= NumTagRecords )
        this.tag_index = 0;
            
      ++this.counter;
      return value;
            
    default: return GLG.Rand( 0.0, 100.0 );
   }
}

/////////////////////////////////////////////////////////////////////// 
// Obtain an array of historical data for a given tag source.
/////////////////////////////////////////////////////////////////////// 
DemoDataFeedObj.prototype.GetPlotData =    /* PlotDataPoint[] */
  function( /* String */ tag_source, /* double */ start_time,
            /* double */ end_time, /* int */ max_num_samples )
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
      var value = this.GetDemoValue( tag_source, true );
      var time_stamp = start_time + interval * i;
      var value_valid = true;

      var data_point = new PlotDataPoint( value, time_stamp, value_valid );
      data_array.push( data_point );
   }
   return data_array;
}

/////////////////////////////////////////////////////////////////////// 
DemoDataFeedObj.prototype.GetDemoString =    /* String */
  function( /* String */ tag_source )
{
   return ( GLG.Rand( 0.0, 1.0 ) > 0.5 ? "On" : "Off" );
}

//////////////////////////////////////////////////////////////////////////
DemoDataFeedObj.prototype.GetAlarmData = function()   /* AlarmRecord */
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

//////////////////////////////////////////////////////////////////////////
DemoDataFeedObj.prototype.ACKAlarm =     /* boolean */
  function( /* String */ tag_source )
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
