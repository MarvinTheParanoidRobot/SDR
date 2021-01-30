import com.genlogic.*;

/*--------------------------------------------------------------------
| This is an example of implementing the MIN_MAX, AVERAGE and DISCARD 
| data filters via a custom chart filter. The code uses the value of 
| the FilterType attribute to determine the type of filter, and treats 
| the values of CUSTOM_FILTER, CUSTOM_FILTER+1 and CUSTOM_FILTER+2 
| as MIN_MAX, AVERAGE and DISCARD filter types.
| To use this custom filter, set a plot's FilterType in the drawing
| to one of the above custom filter types (101, 102 or 103) and
| install this class as a custom plot filter using SetChartFilter().
*/
class CustomChartFilter implements GlgChartFilter
{
   /*--------------------------------------------------------------------
   | Invoked every time a plot's rendering operation begins, may be used 
   | to initialize variables used for accumulating data samples.
   */
   public int Start( GlgChartFilterData filter_data, Object client_data )
   {
      /* Initialize variables used to accumulate samples. */
      filter_data.num_accum = 0;     /* Number of accumulated samples. */

      switch( GetCustomFilterType( filter_data ) )
      {
       case GlgObject.MIN_MAX_FILTER:
         filter_data.disable_attributes = false;
         break;
         
       case GlgObject.BAR_MIN_MAX_FILTER:
         filter_data.accum_x = 0.0;
         filter_data.disable_attributes = true;
         break;
         
       case GlgObject.AVERAGE_FILTER:
         filter_data.accum_x = 0.0;
         filter_data.accum_y = 0.0;
         filter_data.accum_y_low = 0.0;
         filter_data.marker_vis_acc = 0.0f;
         filter_data.disable_attributes = true;
         break;
         
       case GlgObject.DISCARD_FILTER:
         filter_data.marker_vis_acc = 0.0f;
         filter_data.disable_attributes = false;
         break;
      }
      
      return GlgObject.CHART_FILTER_VERSION;   /* Versioning check. */
   }

   /*--------------------------------------------------------------------
   | Invoked to push a new data sample into the filter. The filter
   | accumulates multiple samples within the horizontal interval
   | defined by the plot's FilterPrecision attribute, and combines 
   | them according to the filter type into either one or two 
   | aggregate samples to be drawn.
   */
   public void AddSample( GlgChartFilterData filter_data, Object client_data )
   {
      GlgDataSample data_sample = filter_data.data_sample;
      GlgDataSampleExt data_sample_ext;
      double x, y, y_low;

      if( !data_sample.valid )
        return;      /* Nothing to store if not valid. */

      x = data_sample.time;
      y = data_sample.value;

      /* Using existing filter_data's fields to store min, max and
         average values depending on the filter type. 
         Alternatively, a custom structure passed as an client_data 
         argument may be used to store accumulated values.
      */
      switch( GetCustomFilterType( filter_data ) )
      {
       case GlgObject.MIN_MAX_FILTER:
         if( filter_data.num_accum == 0 || y < filter_data.min_y )
         {
            filter_data.min_x = x;
            filter_data.min_y = y;

            filter_data.data_sample_min = data_sample; 

            /* If the plot has markers, store visibility of the min/max 
               markers. */
            if( filter_data.draw_markers )
              filter_data.marker_vis_min = data_sample.marker_vis;
         }
         
         if( filter_data.num_accum == 0 || y > filter_data.max_y )
         {
            filter_data.max_x = x;
            filter_data.max_y = y;

            filter_data.data_sample_max = data_sample; 

            if( filter_data.draw_markers )
              filter_data.marker_vis_max = data_sample.marker_vis;
         }
         
         ++filter_data.num_accum; /* Number of accumulated samples. */
         break;

       case GlgObject.BAR_MIN_MAX_FILTER:
         /* Used only for a floating bar with extended data. */
         if( !data_sample.extended_data )
           break;
         
         data_sample_ext = (GlgDataSampleExt) data_sample;
         if( data_sample_ext.saved_type != GlgObject.BAR_EXT_DATA )
           break;
         
         y_low = data_sample_ext.y_low;
         if( filter_data.num_accum == 0 || y_low < filter_data.min_y )
           filter_data.min_y = y_low;
         
         if( filter_data.num_accum == 0 || y > filter_data.max_y )
           filter_data.max_y = y;
         
         /* Accumulate x positions for averaging. */
         filter_data.accum_x += x;
         ++filter_data.num_accum;
         break; 
         
       case GlgObject.AVERAGE_FILTER:
         filter_data.accum_x += x;
         filter_data.accum_y += y;
         
         if( data_sample.extended_data )
         {
            data_sample_ext = (GlgDataSampleExt) data_sample;
            
            if( data_sample_ext.saved_type == GlgObject.BAR_EXT_DATA )
              filter_data.accum_y_low += data_sample_ext.y_low;
         }
            
         /* Store visibility of the marker with the highest alpha. */
         if( filter_data.draw_markers &&
             data_sample.marker_vis > filter_data.marker_vis_acc )
           filter_data.marker_vis_acc = data_sample.marker_vis;
         
         ++filter_data.num_accum; /* Number of accumulated samples. */
         break;
         
       case GlgObject.DISCARD_FILTER:
         /* Store only a single last sample for flushing. */
         filter_data.x1 = x;
         filter_data.y1 = y;
         
         filter_data.data_sample1 = data_sample;
         
         /* Store visibility of the marker with the highest alpha. */
         if( filter_data.draw_markers &&
             data_sample.marker_vis > filter_data.marker_vis_acc )
           filter_data.marker_vis_acc = data_sample.marker_vis;
         
         filter_data.num_accum = 1; /* Number of accumulated samples. */
         break;
         
       default:
         GlgObject.Error( GlgObject.USER_ERROR, "Invalid filter type.", null );
         break;
      }
   }

   /*--------------------------------------------------------------------
   | Invoked to flush the data accumulated by the filter when the
   | next data sample is outside the horizontal interval defined by
   | the plot's FilterPrecision attribute. It may also be invoked
   | if the next data sample has a marker and FilterMarkers=OFF.
   */
   public int Flush( GlgChartFilterData filter_data, Object client_data )
   {
      if( filter_data.num_accum == 0 ) /* No accumulated samples. */
        return GlgObject.SKIP_DATA;

      /* GOfiNumAccum( filter_data ) != 0 - has some samples to flush. */
      
      /* Fill filter_data's output fields used to draw accumulated points. */
      int rval;
      switch( GetCustomFilterType( filter_data ) )
      {
       case GlgObject.MIN_MAX_FILTER:
         /* Sort min/max point based on time to ensure proper ordering. */
         if( filter_data.min_x >= filter_data.max_x )
         {
            filter_data.x1 = filter_data.max_x;
            filter_data.y1 = filter_data.max_y;

            filter_data.x2 = filter_data.min_x;
            filter_data.y2 = filter_data.min_y;

            filter_data.data_sample1 = filter_data.data_sample_max;
            filter_data.data_sample2 = filter_data.data_sample_min;

            if( filter_data.draw_markers )
            {
               filter_data.marker_vis1 = filter_data.marker_vis_max;
               filter_data.marker_vis2 = filter_data.marker_vis_min;
            }
         }
         else /* filter_data.min_x < filter_data.max_x */
         {
            filter_data.x1 = filter_data.min_x;
            filter_data.y1 = filter_data.min_y;

            filter_data.x2 = filter_data.max_x;
            filter_data.y2 = filter_data.max_y;

            filter_data.data_sample1 = filter_data.data_sample_min;
            filter_data.data_sample2 = filter_data.data_sample_max;

            if( filter_data.draw_markers )
            {
               filter_data.marker_vis1 = filter_data.marker_vis_min;
               filter_data.marker_vis2 = filter_data.marker_vis_max;
            }
         }

         filter_data.y1_low_set = false;

         if( filter_data.y1 == filter_data.y2 )
         {
            /* MinY = MaxY - draw single sample at the averaged position,
               but still use DataSample1 for possible extended data.
            */
            rval = GlgObject.USE_DATA1;
            
            filter_data.x1 = ( filter_data.x1  + filter_data.x2 ) / 2.0;
            
            if( filter_data.draw_markers )
              filter_data.marker_vis1 = Math.max( filter_data.marker_vis1, 
                                                  filter_data.marker_vis2 );
         }
         else /* Draw two samples. */
           rval = GlgObject.USE_DATA2;
         break;

       case GlgObject.BAR_MIN_MAX_FILTER:
         filter_data.y1 = filter_data.max_y;
         filter_data.y1_low = filter_data.min_y;
         filter_data.y1_low_set = true;
            
         /* Position the final bar at the averaged x position. */
         filter_data.x1 = filter_data.accum_x / filter_data.num_accum;
         filter_data.accum_x = 0.0;
         
         filter_data.data_sample1 = null;
         filter_data.marker_vis1 = 0.0f;    /* No markers for this filter. */
         
         rval = GlgObject.USE_DATA1;
         break;

       case GlgObject.AVERAGE_FILTER:
         /* Calculate averaged values. */
         filter_data.x1 = filter_data.accum_x / filter_data.num_accum;
         filter_data.y1 = filter_data.accum_y / filter_data.num_accum;
         /* Always calculate for simplicity, non-zero only if used. */
         filter_data.y1_low = filter_data.accum_y_low / filter_data.num_accum;
         filter_data.y1_low_set = true;

         /* Reset. */
         filter_data.accum_x = 0.0;
         filter_data.accum_y = 0.0;
         filter_data.accum_y_low = 0.0;
         
         filter_data.data_sample1 = null;

         if( filter_data.draw_markers )
         {
            filter_data.marker_vis1 = filter_data.marker_vis_acc;
            filter_data.marker_vis_acc = 0.0f; /* Reset for the next. */
         }

         rval = GlgObject.USE_DATA1;
         break;

       case GlgObject.DISCARD_FILTER:
         /* X1/Y1/DataSample1 output is ready. */

         if( filter_data.draw_markers )
           filter_data.marker_vis1 = filter_data.marker_vis_acc;

         filter_data.y1_low_set = false;
            
         rval = GlgObject.USE_DATA1;
         break;

       default:
         GlgObject.Error( GlgObject.USER_ERROR, "Invalid filter type.", null );
         rval = GlgObject.SKIP_DATA;
         break;
      }

      /* Flushed: reset the number of accumulated samples. */
      filter_data.num_accum = 0;
      return rval;
   }

   /*--------------------------------------------------------------------
   | Invoked every time a plot's rendering operation finishes.
   */
   public void Finish( GlgChartFilterData filter_data, Object client_data )
   {
   }

   /*--------------------------------------------------------------------
   | Determines the type of a custom filter based on the passed value
   | of the FilterType attribute.
   */
   int GetCustomFilterType( GlgChartFilterData filter_data )
   {
      /* Map the three consecutive values starting with CUSTOM_FILTER 
         (101, 102 and 103) to MIN_MAX_FILTER, AVERAGE_FILTER and 
         DISCARD_FILTER.
      */
      return ( filter_data.filter_type - GlgObject.CUSTOM_FILTER + 1 );
   }
}
