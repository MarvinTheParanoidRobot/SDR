package glg_demos;

import java.util.*;
import com.genlogic.*;

// Simulated data for the demo. In an application, the data may come from 
// any datasource, such as a PLC or process database.
  //
public class GlgCircuitDemoData
{
   // Constants
   static final int UPDATE_INTERVAL = 1000;   // milliseconds

   long last_time = 0;

   // Contains a list of all resources to animate, which was queried 
     // from the drawing.
   GlgObject resource_list;

   GlgCircuitDemoData()
   {
   }

   //////////////////////////////////////////////////////////////////////////
   // Updates drawing with simulated data. An application would query its 
   // data from a custom data source using JDBC or Java OPC.
   //////////////////////////////////////////////////////////////////////////
   void UpdateCircuitData()
   {
      long curr_time = new Date().getTime();

      // Update simulation data only as often as defined by UPDATE_INTERVAL:
      // once a second.
      if( curr_time < last_time + UPDATE_INTERVAL )
         return;

      IterateCircuit();

      last_time = curr_time;
   }

   //////////////////////////////////////////////////////////////////////////
   // Update each resource with new data value.
   //////////////////////////////////////////////////////////////////////////
   void IterateCircuit()
   {
      if( resource_list == null )
        return;

      int size = resource_list.GetSize();
      for( int i=0; i<size; ++i )
      {
         GlgSimulationResource resource = (GlgSimulationResource)
           resource_list.GetElement( i );

         if( resource.type != GlgObject.D ) 
            continue;      // Update only resources of D type

         // Get new resource value.
         resource.value = resource.range * Math.random();
      } 
   }
}
