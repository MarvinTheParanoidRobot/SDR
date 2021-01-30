import com.genlogic.*;

///////////////////////////////////////////////////////////////////////      
public class DataPoint
{
   public double d_value;
   public String s_value;
   public int data_type; // 'D' or 'S'

   public DataPoint()
   {
      data_type = GlgObject.D; //default data type.
   }
                 
   public DataPoint( int type )
   {
      data_type = type;
   }
}

