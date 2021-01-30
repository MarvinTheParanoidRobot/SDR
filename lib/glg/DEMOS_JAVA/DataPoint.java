import com.genlogic.*;

///////////////////////////////////////////////////////////////////////      
public class DataPoint
{
   double d_value;
   String s_value;
   double time_stamp;
   int data_type; // 'D' or 'S'
              
   public DataPoint()
   {
      data_type = 'D'; //default data type.
   }
                 
   public DataPoint( int type )
   {
      data_type = type;
   }
}

