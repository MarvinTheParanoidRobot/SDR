/* Custom chart filter coding example. */

#include <math.h>
#include "GlgApi.h"

/* Convenience macros for data access. */
#define GOdsValue( ds )         ( ds->value )
#define GOdsTime( ds )          ( (ds)->time )
#define GOdsMarkerVis( ds )     ( (ds)->marker_vis )
#define GOdsValid( ds )         ( (ds)->valid )
#define GOdsExtendedData( ds )  ( (ds)->extended_data )

#define GOdsSavedType( dse )    ( (dse)->saved_type )
#define GOdsYLow( dse )         ( (dse)->y_low )

#define GOfiFilterType( fi )    ( (fi)->filter_type )
#define GOfiNumAccum( fi )      ( (fi)->num_accum )
#define GOfiDrawMarkers( fi )   ( (fi)->draw_markers )
#define GOfiAccumX( fi )        ( (fi)->accum_x )
#define GOfiAccumY( fi )        ( (fi)->accum_y )
#define GOfiAccumYLow( fi )     ( (fi)->accum_y_low )
#define GOfiMinX( fi )          ( (fi)->min_x )
#define GOfiMinY( fi )          ( (fi)->min_y )
#define GOfiMaxX( fi )          ( (fi)->max_x )
#define GOfiMaxY( fi )          ( (fi)->max_y )
#define GOfiX1( fi )            ( (fi)->x1 )
#define GOfiY1( fi )            ( (fi)->y1 )
#define GOfiY1Low( fi )         ( (fi)->y1_low )
#define GOfiY1LowSet( fi )      ( (fi)->y1_low_set )
#define GOfiX2( fi )            ( (fi)->x2 )
#define GOfiY2( fi )            ( (fi)->y2 )
#define GOfiMarkerVisAcc( fi )  ( (fi)->marker_vis_acc )
#define GOfiMarkerVisMin( fi )  ( (fi)->marker_vis_min )
#define GOfiMarkerVisMax( fi )  ( (fi)->marker_vis_max )
#define GOfiMarkerVis1( fi )    ( (fi)->marker_vis1 )
#define GOfiMarkerVis2( fi )    ( (fi)->marker_vis2 )
#define GOfiDataSampleMin( fi ) ( (fi)->data_sample_min )
#define GOfiDataSampleMax( fi ) ( (fi)->data_sample_max )
#define GOfiDataSample1( fi )   ( (fi)->data_sample1 )
#define GOfiDataSample2( fi )   ( (fi)->data_sample2 )
#define GOfiDataSample( fi )    ( (fi)->data_sample )
#define GOfiDisableAttributes( fi ) ( (fi)->disable_attributes )

#ifdef _WINDOWS
#define fmax   max
#endif

/* Function prototypes. */
GlgLong CustomFilterStart( GlgChartFilterData * filter_data, 
                           void * client_data );
void CustomFilterAddSample( GlgChartFilterData * filter_data, 
                            void * client_data );
GlgLong CustomFilterFlush( GlgChartFilterData * filter_data, 
                           void * client_data );
GlgChartFilterType GetCustomFilterType( GlgChartFilterData * filter_data );

/*-----------------------------------------------------------------------
| This is an example of implemeting the MIN_MAX, AVERAGE, DISCARD and
| RANGE_MIN_MAX data filters via a custom chart filter. The code uses 
| the value of the FilterType attribute to determine the type of filter, 
| nd treats the values of CUSTOM_FILTER, CUSTOM_FILTER+1, CUSTOM_FILTER+2
| and CUSTOM_FILTER+3 as MIN_MAX, AVERAGE, DISCARD and RANGE_MIN_MAX 
| filter types. To use this custom filter, set a plot's FilterType in 
| the drawing to one of the above custom filter types (101, 102, 103 
| or 104) and install this function as a custom plot filter using 
| GlgSetChartFilter().
*/
GlgLong CustomChartFilter( GlgChartFilterCBReason reason, 
                           GlgChartFilterData * filter_data, 
                           void * client_data )
{
   switch( reason )
   {
      /* Invoked every time a plot's rendering operation begins. */
    case GLG_FILTER_CB_START:
      return CustomFilterStart( filter_data, client_data );

      /* Invoked to push a new data sample into the filter. */
    case GLG_FILTER_CB_ADD_SAMPLE:
      CustomFilterAddSample( filter_data, client_data );
      return 0;

      /* Invoked to flush the data accumulated by the filter. */
    case GLG_FILTER_CB_FLUSH:
      return CustomFilterFlush( filter_data, client_data );

      /* Invoked every time a plot's rendering operation finishes. */
    case GLG_FILTER_CB_FINISH: 
      return 0;

      /* Invoked when the plot with a custom filter is destroyed; may be used
         to free any allocated client data passed to the filter via 
         GlgSetChartFilter().
      */
    case GLG_FILTER_CB_DESTROY:
      return 0;
   }
   return 0;
}

/*--------------------------------------------------------------------
| Invoked every time a plot's rendering operation begins, may be used 
| to initialize variables used for accumulating data samples.
*/
GlgLong CustomFilterStart( GlgChartFilterData * filter_data, 
                           void * client_data )
{
   /* Initialize variables used to accumulate samples. */
   GOfiNumAccum( filter_data ) = 0;     /* Number of accumulated samples. */

   switch( GetCustomFilterType( filter_data ) )
   {
    case GLG_MIN_MAX_FILTER:
      GOfiDisableAttributes( filter_data ) = False;
      break;

    case GLG_BAR_MIN_MAX_FILTER:
      GOfiAccumX( filter_data ) = 0.;
      GOfiDisableAttributes( filter_data ) = True;
      break;

    case GLG_AVERAGE_FILTER:
      GOfiAccumX( filter_data ) = 0.;
      GOfiAccumY( filter_data ) = 0.;
      GOfiAccumYLow( filter_data ) = 0.;
      GOfiMarkerVisAcc( filter_data ) = 0.;
      GOfiDisableAttributes( filter_data ) = True;
      break;

    case GLG_DISCARD_FILTER:
      GOfiMarkerVisAcc( filter_data ) = 0.;
      GOfiDisableAttributes( filter_data ) = False;
      break;
   }
   return GLG_CHART_FILTER_VERSION;   /* Versioning check. */
}

/*--------------------------------------------------------------------
| Invoked to push a new data sample into the filter. The filter 
| accumulates multiple samples within the horizontal interval
| defined by the plot's FilterPrecision attribute, and combines 
| them according to the filter type into either one or two 
| aggregate samples to be drawn.
*/
void CustomFilterAddSample( GlgChartFilterData * filter_data, 
                            void * client_data )
{
   GlgDataSample * data_sample; 
   GlgDataSampleExt * data_sample_ext;
   double x, y, y_low;

   data_sample = GOfiDataSample( filter_data );
   if( !GOdsValid( data_sample ) )
     return;     /* Nothing to store if not valid. */

   x = GOdsTime( data_sample );
   y = GOdsValue( data_sample );

   /* Using existing filter_data's fields to store min, max and
      average values depending on the filter type. 
      Alternatively, a custom structure passed as an client_data argument 
      may be used to store accumulated values.
   */
   switch( GetCustomFilterType( filter_data ) )
   {
    case GLG_MIN_MAX_FILTER:
      if( !GOfiNumAccum( filter_data ) || y < GOfiMinY( filter_data ) )
      {
         GOfiMinX( filter_data ) = x;
         GOfiMinY( filter_data ) = y;            

         GOfiDataSampleMin( filter_data ) = data_sample;
          
         /* If the plot has markers, store visibility of the min/max markers. */
         if( GOfiDrawMarkers( filter_data ) )
           GOfiMarkerVisMin( filter_data ) = GOdsMarkerVis( data_sample );
      }
      
      if( !GOfiNumAccum( filter_data ) || y > GOfiMaxY( filter_data ) )
      {
         GOfiMaxX( filter_data ) = x;
         GOfiMaxY( filter_data ) = y;
         
         GOfiDataSampleMax( filter_data ) = data_sample;

         if( GOfiDrawMarkers( filter_data ) )
           GOfiMarkerVisMax( filter_data ) = GOdsMarkerVis( data_sample );
      }
      
      ++GOfiNumAccum( filter_data );  /* Number of accumulated samples. */
      break;
      
    case GLG_BAR_MIN_MAX_FILTER:
      /* Used only for a floating bar with extended data. */
      if( !GOdsExtendedData( data_sample ) )
        break;

      data_sample_ext = (GlgDataSampleExt*) data_sample;
      if( GOdsSavedType( data_sample_ext ) != GLG_BAR_EXT_DATA )
        break;
      
      y_low = GOdsYLow( data_sample_ext );
      if( !GOfiNumAccum( filter_data ) || y_low < GOfiMinY( filter_data ) )
        GOfiMinY( filter_data ) = y_low;
      
      if( !GOfiNumAccum( filter_data ) || y > GOfiMaxY( filter_data ) )
        GOfiMaxY( filter_data ) = y;
         
      /* Accumulate x positions for averaging. */
      GOfiAccumX( filter_data ) += x;
      ++GOfiNumAccum( filter_data );
      break;        

    case GLG_AVERAGE_FILTER:
      GOfiAccumX( filter_data ) += x;
      GOfiAccumY( filter_data ) += y;
      
      if( GOdsExtendedData( data_sample ) )
      {
         data_sample_ext = (GlgDataSampleExt*) data_sample;

         if( GOdsSavedType( data_sample_ext ) == GLG_BAR_EXT_DATA )
           GOfiAccumYLow( filter_data ) += GOdsYLow( data_sample_ext );
      }
      
      /* Store visibility of the marker with the highest alpha. */
      if( GOfiDrawMarkers( filter_data ) && 
          GOdsMarkerVis( data_sample ) > GOfiMarkerVisAcc( filter_data ) )
        GOfiMarkerVisAcc( filter_data ) = GOdsMarkerVis( data_sample );
      
      ++GOfiNumAccum( filter_data );  /* Number of accumulated samples. */
      break;
      
    case GLG_DISCARD_FILTER:     
      /* Store only a single last sample for flushing. */
      GOfiX1( filter_data ) = x;
      GOfiY1( filter_data ) = y;
      
      GOfiDataSample1( filter_data ) = data_sample;

      /* Store visibility of the marker with the highest alpha. */
      if( GOfiDrawMarkers( filter_data ) && 
          GOdsMarkerVis( data_sample ) > GOfiMarkerVisAcc( filter_data ) )
        GOfiMarkerVisAcc( filter_data ) = GOdsMarkerVis( data_sample );
      
      GOfiNumAccum( filter_data ) = 1;  /* Number of accumulated samples. */
      break;
      
    default: GlgError( GLG_USER_ERROR, "Invalid filter type." ); break;
   }  
}

/*--------------------------------------------------------------------
| Invoked to flush the data accumulated by the filter when the
| next data sample is outside the horizontal interval defined by
| the plot's FilterPrecision attribute. It may also be invoked
| if the next data sample has a marker and FilterMarkers=OFF.
*/
GlgLong CustomFilterFlush( GlgChartFilterData * filter_data, 
                           void * client_data )
{
   GlgLong rval;

   if( !GOfiNumAccum( filter_data ) )       /* No accumulated samples. */
     return GLG_SKIP_DATA;

   /* GOfiNumAccum( filter_data ) != 0 - has some samples to flush. */
   
   /* Fill filter_data's output fields used to draw accumulated points. */
   switch( GetCustomFilterType( filter_data ) )
   {
    case GLG_MIN_MAX_FILTER:
      /* Sort min/max point based on time to ensure the proper ordering. */
      if( GOfiMinX( filter_data ) >= GOfiMaxX( filter_data ) )
      {
         GOfiX1( filter_data ) = GOfiMaxX( filter_data );
         GOfiY1( filter_data ) = GOfiMaxY( filter_data );
         
         GOfiX2( filter_data ) = GOfiMinX( filter_data );
         GOfiY2( filter_data ) = GOfiMinY( filter_data );
         
         GOfiDataSample1( filter_data ) = GOfiDataSampleMax( filter_data );
         GOfiDataSample2( filter_data ) = GOfiDataSampleMin( filter_data );

         if( GOfiDrawMarkers( filter_data ) )
         {
            GOfiMarkerVis1( filter_data ) = GOfiMarkerVisMax( filter_data );
            GOfiMarkerVis2( filter_data ) = GOfiMarkerVisMin( filter_data );
         }
      }
      else   /* GOfiMinX( filter_data ) < GOfiMaxX( filter_data ) */
      {
         GOfiX1( filter_data ) = GOfiMinX( filter_data );
         GOfiY1( filter_data ) = GOfiMinY( filter_data );
         
         GOfiX2( filter_data ) = GOfiMaxX( filter_data );
         GOfiY2( filter_data ) = GOfiMaxY( filter_data );
         
         GOfiDataSample1( filter_data ) = GOfiDataSampleMin( filter_data );
         GOfiDataSample2( filter_data ) = GOfiDataSampleMax( filter_data );

         if( GOfiDrawMarkers( filter_data ) )
         {
            GOfiMarkerVis1( filter_data ) = GOfiMarkerVisMin( filter_data );
            GOfiMarkerVis2( filter_data ) = GOfiMarkerVisMax( filter_data );
         }
      }
      
      GOfiY1LowSet( filter_data ) = False;

      if( GOfiY1( filter_data ) == GOfiY2( filter_data ) )
      {
         /* MinY = MaxY - draw single sample at the averaged position,
            but still use DataSample1 for possible extended data.
         */
         rval = GLG_USE_DATA1; 

         GOfiX1( filter_data ) = 
           ( GOfiX1( filter_data ) + GOfiX2( filter_data ) ) / 2.;

         GOfiMarkerVis1( filter_data ) = fmax( GOfiMarkerVis1( filter_data ), 
                                               GOfiMarkerVis2( filter_data ) );
      }
      else   /* Draw two samples. */
        rval = GLG_USE_DATA2;
      break;

    case GLG_BAR_MIN_MAX_FILTER:
      GOfiY1( filter_data ) = GOfiMaxY( filter_data );
      GOfiY1Low( filter_data ) = GOfiMinY( filter_data );
      GOfiY1LowSet( filter_data ) = True;

      /* Position the final bar at the averaged x position. */
      GOfiX1( filter_data ) = 
        GOfiAccumX( filter_data ) / GOfiNumAccum( filter_data );
      GOfiAccumX( filter_data ) = 0.;

      GOfiDataSample1( filter_data ) = NULL;
      GOfiMarkerVis1( filter_data ) = 0.;    /* No markers for this filter. */

      rval = GLG_USE_DATA1;
      break;
      
    case GLG_AVERAGE_FILTER:        
      /* Calculate averaged values. */
      GOfiX1( filter_data ) = 
        GOfiAccumX( filter_data ) / GOfiNumAccum( filter_data );
      GOfiY1( filter_data ) = 
        GOfiAccumY( filter_data ) / GOfiNumAccum( filter_data );
      /* Always calculate for simplicity, non-zero only if used. */
      GOfiY1Low( filter_data ) = 
        GOfiAccumYLow( filter_data ) / GOfiNumAccum( filter_data );
      GOfiY1LowSet( filter_data ) = True;
 
      /* Reset. */
      GOfiAccumX( filter_data ) = 0.;
      GOfiAccumY( filter_data ) = 0.;
      GOfiAccumYLow( filter_data ) = 0.;

      GOfiDataSample1( filter_data ) = NULL;

      if( GOfiDrawMarkers( filter_data ) )
      {
         GOfiMarkerVis1( filter_data ) = GOfiMarkerVisAcc( filter_data );
         GOfiMarkerVisAcc( filter_data ) = 0.;  /* Reset for the next. */
      }

      rval = GLG_USE_DATA1;
      break;
      
    case GLG_DISCARD_FILTER: 
      /* X1/Y1/DataSample1 output is ready. */

      if( GOfiDrawMarkers( filter_data ) )
      {
         GOfiMarkerVis1( filter_data ) = GOfiMarkerVisAcc( filter_data );
         GOfiMarkerVisAcc( filter_data ) = 0.;  /* Reset for the next. */
      }
      GOfiY1LowSet( filter_data ) = False;

      rval = GLG_USE_DATA1; 
      break;
      
    default: 
      GlgError( GLG_USER_ERROR, "Invalid filter type." ); 
      rval = GLG_SKIP_DATA;
      break;
   }
   
   /* Flushed: reset the number of accumulated samples. */
   GOfiNumAccum( filter_data ) = 0;
   return rval;
}   

/*--------------------------------------------------------------------
| Determines the type of a custom filter based on the passed value
| of the FilterType attribute.
*/
GlgChartFilterType GetCustomFilterType( GlgChartFilterData * filter_data )
{
   /* Map the four consecutive values starting with CUSTOM_FILTER 
      (101, 102, 103 and 104) to MIN_MAX_FILTER, AVERAGE_FILTER,
      DISCARD_FILTER and BAR_MIN_MAX_FILTER.
   */
   return GOfiFilterType( filter_data ) - GLG_CUSTOM_FILTER + 1;
}
