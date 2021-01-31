import com.genlogic.*;

//////////////////////////////////////////////////////////////////////////
public class GlgAnimationValue
{
   static final int 
     INCR = 0,
     SIN = 1,
     RANDOM = 2,
     RANDOM_INT = 3;
   int
     counter,
     period;
   int type;
   double
     min,
     max;
   String name;
   GlgJBean bean;

   //////////////////////////////////////////////////////////////////////////
   public GlgAnimationValue( GlgJBean bean_p, int type_p,
                            int counter_p, int period_p,
                            double min_p, double max_p, String name_p )
   {
      bean = bean_p;
      type = type_p;
      counter = counter_p;
      period = period_p;
      min = min_p;
      max = max_p;
      name = name_p;
   }

   //////////////////////////////////////////////////////////////////////////
   public void Iterate()
   { 
      double angle, value;

      if( period < 1 )
      {
         System.out.println( "Invalid period." );
         return;
      }

      switch( type )
      {
       case SIN:
         angle = Math.PI * counter / period;	 
         value = min + max * Math.sin( angle );	 
         break;

       case INCR:
         value = min + ( counter / (double) ( period - 1 ) ) * ( max - min );
         break;

       case RANDOM:
         value = GlgObject.Rand( min, max );
         break;

       case RANDOM_INT:
         value = (double) ( (int) GlgObject.Rand( min, max ) );
         break;

       default:
         System.out.println( "Invalid animation type." );
         value = 0.0;
         break;
      }

      if( bean != null )
        bean.SetDResource( name, value );

      // Increment counter
      ++counter;
      if( counter >= period )
        counter = 0;    // Reset counter
   }
}

