#include <math.h>
#include "GlgApi.h"

#ifdef __cplusplus
#  define CONST const
#  define STRING_CAST      (char*)
#  define STRING_PTR_CAST  (char**)
#else
#  define CONST 
#  define STRING_CAST 
#  define STRING_PTR_CAST 
#endif

typedef enum _AnimationType
{
   SIN = 0,
   INCR,
   RANDOM,
   RANDOM_INT

} AnimationType;

typedef struct _AnimationData
{
   AnimationType type;
   int counter;
   int period;
   double min;
   double max;
   CONST char * resource_name;
} AnimationData;

AnimationData AnimationDataArray[] =
{
   /* 3 top gages */
   { SIN, 0, 47, 50., 180., "Dial1/Value" },
   { SIN, 0, 63, 30.0, 90.0, "Dial2/Value" },
   { SIN, 0, 91, 1.2, 3.5, "Dial3/Value" },
     
     /* Two small gauges in the middle. */
   { SIN, 0, 36, 2.0, 8.0, "Dial4/Value" },
   { SIN, 0, 59, 2.0, 8.0, "Dial5/Value" },
     
     /* Sliders */
   { SIN, 0, 75, 0.2, 0.8, "Slider1/ValueY" },
   { SIN, 0, 45, 0.2, 0.8, "Slider2/ValueY" },
   { SIN, 0, 84, 0.2, 0.8, "Slider3/ValueY" },
   { SIN, 0, 65, 0.2, 0.8, "Slider4/ValueY" },
   { SIN, 0, 32, 0.2, 0.8, "Slider5/ValueY" },
   { SIN, 0, 58, 0.2, 0.8, "Slider6/ValueY" },
     
     /* Thermometer */
   { SIN, 0, 80, -20.0, 105.0, "Thermometer/ValueY" },
     
     /* 3 knobs to the right of the thermometer */
   { SIN, 0, 46, 5.0, 80.0, "Knob1/Value" },
   { SIN, 0, 65, 1.0, 9.0, "Knob2/Value" },
   { SIN, 0, 38, 1.0, 9.0, "Knob3/Value" },
     
     /* 2 switches */
   { RANDOM_INT, 0, 100, 0.0, 2.99, "Switch1/ValueX" },
   { RANDOM_INT, 0, 100, 0.0, 1.99, "Switch2/OnState" },
     
     /* Lights */
   { RANDOM, 0, 100, 0.0, 3.0, "Light1/Value" },
   { RANDOM, 0, 100, 0.0, 3.0, "Light2/Value" },
   { RANDOM, 0, 100, 0.0, 3.0, "Light3/Value" },
     
     /* Toggle switches */
   { RANDOM_INT, 0, 100, 0.0, 1.99, "Toggle1/OnState" },
   { RANDOM_INT, 0, 100, 0.0, 1.99, "Toggle2/OnState" },
   { RANDOM_INT, 0, 100, 0.0, 1.99, "Toggle3/OnState" },
     
     /* Joystick */
   { SIN, 0, 23, 0.2, 0.8, "Joystick/ValueX" },
   { SIN, 0, 19, 0.2, 0.8, "Joystick/ValueY" }
};

#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950
#endif

int NumAnimationResources = 
  sizeof( AnimationDataArray ) / sizeof( AnimationData );

/*----------------------------------------------------------------------
  Animates gauges in the control panel with random data defined in the 
  animation array. The array_index parameter supplies the index of the 
  array element to animate.
 */
void AnimateOneResource( GlgObject viewport, int array_index )
{ 
   double angle, value;

   AnimationData * data = &AnimationDataArray[ array_index ];
   
   if( data->period < 1 )
   {
      GlgError( GLG_WARNING, STRING_CAST "Invalid period." );
      return;
   }
   
   switch( data->type )
   {
    case SIN:
      angle = M_PI * data->counter / data->period;	 
      value = data->min + data->max * sin( angle );	 
      break;
      
    case INCR:
      value = data->min + ( data->counter / (double) ( data->period - 1 ) ) * 
        ( data->max - data->min );
      break;
      
    case RANDOM:
      value = GlgRand( data->min, data->max );
      break;

    case RANDOM_INT:
      value = (double) ( (int) GlgRand( data->min, data->max ) );
      break;
      
    default:
      GlgError( GLG_WARNING, STRING_CAST "Invalid animation type." );
      value = 0.0;
      break;
   }

   /* Update resource in the drawing.*/
   GlgSetDResource( viewport, STRING_CAST data->resource_name, value );

   /* Increment counter */
   ++data->counter;
      if( data->counter >= data->period )
        data->counter = 0;    /* Reset counter */
}
