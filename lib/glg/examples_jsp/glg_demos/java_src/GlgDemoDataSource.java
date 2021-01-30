package glg_demos;

import java.util.*;
import com.genlogic.*;

// Datasource contains one or more values and a time stamp.
class GlgDemoDataSource
{
   // Inner class, updates a single double value
   class GlgDemoDataValue
   {
      // Data range
      double low;
      double high;

      double value;    // Current value.
      int direction;   // Last increment direction: 1 or -1

      /////////////////////////////////////////////////////////////////////
      GlgDemoDataValue( double low_p, double high_p )
      {
         low = low_p;
         high = high_p;

         if( low >= high )   // Use defaults.
         {
            low = 0.;
            high = 100.;
         }

         // Starting value.
         value = low + GlgObject.Rand( 0.2, 0.8 ) * ( high - low );

         // Initial direction.
         direction = ( GlgObject.Rand( 0., 1. ) > 0.5 ? 1 : -1 );
      }

      /////////////////////////////////////////////////////////////////////
      void GetNewValue()
      {
         boolean change_direction;

         double increment = GlgObject.Rand( 0.02, 0.15 ) * ( high - low );

         // Change direction on every 5th iteractions, on average.
         change_direction = ( GlgObject.Rand( 0., 5. ) < 1. );
         if( change_direction )
           direction *= -1;
         
         double new_value = value + increment * (double) direction;

         if( new_value > high || new_value < low )
         {
            direction *= -1; // Got to the min/max: change direction.
            new_value = value + increment * (double) direction;
         }

         value = new_value;
      }
   }

   long last_time = 0;

   // Used for graph labels.
   int hour;
   int min;
   int sec;

   int update_interval;
   int num_values;    // Number of values, >1 for multi-line graph.

   // Array of values, may contain more than one value for 
   // multi-value datasources.
   GlgDemoDataValue value_array[];

   //////////////////////////////////////////////////////////////////////////
   GlgDemoDataSource( double low, double high, int num_values_p,
                     int update_interval_p )
   {      
      num_values = num_values_p;

      value_array = new GlgDemoDataValue[ num_values ];
      for( int i=0; i<num_values; ++i )
        value_array[ i ] = new GlgDemoDataValue( low, high );

      update_interval = update_interval_p;
      if( update_interval < 0 )
        update_interval = 1000;   // Update once a second by default.
   }

   //////////////////////////////////////////////////////////////////////////
   // Updates simulated data. An application would query its data from
   // any application-specific datasource.
   //////////////////////////////////////////////////////////////////////////
   boolean UpdateData()
   {
      if( update_interval != 0 )
      {
         Calendar right_now = Calendar.getInstance();
         Date date = right_now.getTime();

         long curr_time = date.getTime();   // msec
           
         hour = right_now.get( Calendar.HOUR );
         min = right_now.get( Calendar.MINUTE );
         sec = right_now.get( Calendar.SECOND );      
         
         // Update simulation data only as often as defined by update_interval:
         // once a second.
         if( curr_time < last_time + update_interval )
           return false;
         last_time = curr_time;
      }

      for( int i=0; i<num_values; ++i )
        value_array[i].GetNewValue();

      return true;   // Got a new data value.
   }

   //////////////////////////////////////////////////////////////////////////
   double GetValue()
   {
      return value_array[0].value;
   }

   //////////////////////////////////////////////////////////////////////////
   double GetValue( int index )
   {
      return value_array[ index ].value;
   }
}
