import java.awt.event.*;
import javax.swing.*;
import com.genlogic.*;

//////////////////////////////////////////////////////////////////////////
public class GlgGraphDemo extends GlgJBean implements ActionListener
{
   //////////////////////////////////////////////////////////////////////////
   // The main demo class
   //////////////////////////////////////////////////////////////////////////
   static final long serialVersionUID = 0;

   // Graph types
   static final int
     BAR = 0,
     BAR_3D = 1,
     LINE = 2,
     RIBBON = 3;

   boolean
     IsReady = false,
     PerformUpdates = true,
     UpdateAfterEachSample = true;
   int
     GraphType = BAR,
     label_counter = 0;

   Timer timer = null;

   //////////////////////////////////////////////////////////////////////////
   public GlgGraphDemo()
   {
      super();
   }

   //////////////////////////////////////////////////////////////////////////
   // Initializes the simulation and starts updates.
   //////////////////////////////////////////////////////////////////////////
   public void ReadyCallback( GlgObject viewport )
   {
      super.ReadyCallback( viewport );

      // Set the initial number of samples, etc.
      SetDResource( "DataGroup/Factor", 20.0 );
      SetSResource( "XLabelGroup/XLabel/String", "" ); // Initial value
      SetDResource( "XLabelGroup/Factor", 5.0 );        // Num labels and ticks 
      SetDResource( "XLabelGroup/MinorFactor", 4.0 );   // Num minor ticks
      SetDResource( "DataGroup/ScrollType", 0.0 );
      SetSResource( "XAxisLabel/String", "Sample" );
      SetSResource( "YAxisLabel/String", "Value" );

      // Don't set title: let the graph display it's graph type as a title.
      // SetSResource( "Title/String", "Graph Example" );

      // Make the level line invisible
      if( GraphType == BAR || GraphType == LINE )
        SetDResource( "LevelObjectGroup/Visibility", 0.0 );

      SetColorLocal();

      Update();

      if( timer == null )
      {
         // Restart the timer after each update (instead of using repeats)
         // to avoid flooding the event queue with timer events on slow 
         // machines.
         timer = new Timer( 30, this );
         timer.setRepeats( false );
         timer.start();
      }

      IsReady = true;
   }

   //////////////////////////////////////////////////////////////////////////
   // For use as a stand-alone java demo
   //////////////////////////////////////////////////////////////////////////
   public static void main( final String [] arg )
   {
      SwingUtilities.
        invokeLater( new Runnable(){ public void run() { Main( arg ); } } );
   }

   //////////////////////////////////////////////////////////////////////////
   public static void Main( final String [] arg )
   {
      class DemoQuit extends WindowAdapter
      {
         public void windowClosing( WindowEvent e ) { System.exit( 0 ); }
      } 

      JFrame frame = new JFrame( "GLG Graph Demo" );

      frame.setResizable( true );
      frame.setSize( 600, 450 );
      frame.setLocation( 20, 20 );

      GlgGraphDemo graph = new GlgGraphDemo();      
      frame.getContentPane().add( graph );

      frame.addWindowListener( new DemoQuit() );
      frame.setVisible( true );

      // Assign a drawing filename after the frame became visible and 
      // determined its client size to avoid unnecessary resizing of 
      // the drawing.
      // Loading the drawing triggers ReadyCallback which starts updates.
      //
      graph.SetDrawingName( "bar1.g" );
   }

   //////////////////////////////////////////////////////////////////////////
   // Updates the graph with random data
   //////////////////////////////////////////////////////////////////////////
   void UpdateGraphProc()
   {
      if( timer == null )
        return;   // Prevents race conditions

      if( PerformUpdates && IsReady )
      {
         ++label_counter;
         if( label_counter > 9999 )
           label_counter = 0;
         
         // Push next data value
         SetDResource( "DataGroup/EntryPoint", GlgObject.Rand( 0.0, 1.0 ) );

         // Push next label
         SetSResource( "XLabelGroup/EntryPoint", "#" + label_counter );
         
         // Update after each new data iteration or after filling the whole
         // graph
         if( UpdateAfterEachSample )
           Update();
         else
         {
            int num_samples = (int) GetDResource( "DataGroup/Factor" );
            if( ( label_counter % num_samples ) == 0 )
              Update();
         }
      }

      timer.start();   // Restart the update timer
   }

   //////////////////////////////////////////////////////////////////////////
   public void StopUpdates()
   {
      if( timer != null )
      {
         timer.stop();
         timer = null;
      }
   }

   //////////////////////////////////////////////////////////////////////////
   // Inner class for a Runnable interface.
   // Provides an interface between JavaScript and Java: invokes applet's 
   // methods in a synchronous way as required by Swing.
   //////////////////////////////////////////////////////////////////////////
   class GlgBeanRunnable implements Runnable
   {
      GlgGraphDemo bean;
      String request_name;
      int value;
      String string1, string2, string3;
      double dvalue1, dvalue2, dvalue3;

      public GlgBeanRunnable( GlgGraphDemo bean_p, 
                             String request_name_p, int value_p )
      {
         bean = bean_p;
         request_name = request_name_p;
         value = value_p;
      }

      public void run()
      {
         if( request_name.equals( "NumberOfSamples" ) )
           bean.NumberOfSamples();
         else if( request_name.equals( "SetGraphType" ) )
           bean.SetGraphType( value );
         else if( request_name.equals( "ScrollType" ) )
           bean.ScrollType();
         else if( request_name.equals( "Reverse" ) )
           bean.Reverse();
         else if( request_name.equals( "ChangeRange" ) )
           bean.ChangeRange();
         else if( request_name.equals( "StartUpdate" ) )
           bean.StartUpdate();
         else if( request_name.equals( "StopUpdate" ) )
           bean.StopUpdate();
         else if( request_name.equals( "ChangeYLabels" ) )
           bean.ChangeYLabels();
         else if( request_name.equals( "ChangeXLabels" ) )
           bean.ChangeXLabels( value);
         else if( request_name.equals( "ScrollType" ) )
           bean.ScrollType();
         else if( request_name.equals( "ChangeRange" ) )
           bean.ChangeRange();
         else if( request_name.equals( "ChangeYFormat" ) )
           bean.ChangeYFormat();
         else if( request_name.equals( "WholeFrame" ) )
           bean.WholeFrame();
         else if( request_name.equals( "OneSample" ) )
           bean.OneSample();
         else if( request_name.equals( "Grid" ) )
           bean.Grid( value );
         else if( request_name.equals( "SetTitles" ) )
           bean.SetTitles( string1, string2, string3 );
         else if( request_name.equals( "TitleFont" ) )
           bean.TitleFont( value );
         else if( request_name.equals( "TitleSize" ) )
           bean.TitleSize( value );
         else if( request_name.equals( "MoveTitleLeft" ) )
           bean.MoveTitle( "left" );
         else if( request_name.equals( "MoveTitleRight" ) )
           bean.MoveTitle( "right" );
         else if( request_name.equals( "MoveTitleUp" ) )
           bean.MoveTitle( "up" );
         else if( request_name.equals( "MoveTitleDown" ) )
           bean.MoveTitle( "down" );
         else if( request_name.equals( "TitleColor" ) )
           bean.TitleColor( dvalue1, dvalue2, dvalue3 );
         else if( request_name.equals( "BackColor" ) )
           bean.BackColor( dvalue1, dvalue2, dvalue3 );
         else if( request_name.equals( "GraphColor" ) )
           bean.GraphColor( dvalue1, dvalue2, dvalue3 );
         else if( request_name.equals( "DataColor" ) )
           bean.DataColor( dvalue1, dvalue2, dvalue3 );
         else if( request_name.equals( "Data5Color" ) )
           bean.Data5Color( dvalue1, dvalue2, dvalue3 );
         else if( request_name.equals( "LabelColor" ) )
           bean.LabelColor( dvalue1, dvalue2, dvalue3 );
         else
           PrintToJavaConsole( "Invalid request name: " + 
                              request_name + "\n" );
      }
   }

   //////////////////////////////////////////////////////////////////////////
   // Provides an interface between JavaScript and Java: invokes applet's 
   // methods in a synchronous way as required by Swing.
   //////////////////////////////////////////////////////////////////////////
   public void SendRequest( String request_name, int value )
   {
      SendRequestLong( request_name, value, null, null, null, 0.0, 0.0, 0.0 );
   }

   public void SendRequestStrings( String request_name,
                                  String string1, String string2, 
                                  String string3 )
   {
      SendRequestLong( request_name, 0, string1, string2, string3, 
                      0.0, 0.0, 0.0 );
   }

   public void SendRequestColor( String request_name,
                                double dvalue1, double dvalue2, 
                                double dvalue3 )
   {
      SendRequestLong( request_name, 0, null, null, null,
                      dvalue1, dvalue2, dvalue3 );
   }

   public void SendRequestLong( String request_name, int value,
                               String string1, String string2, String string3,
                               double dvalue1, double dvalue2, double dvalue3 )
   {
      GlgBeanRunnable runnable = 
        new GlgBeanRunnable( this, request_name, value );
      runnable.string1 = string1;
      runnable.string2 = string2;
      runnable.string3 = string3;
      runnable.dvalue1 = dvalue1;
      runnable.dvalue2 = dvalue2;
      runnable.dvalue3 = dvalue3;

      SwingUtilities.invokeLater( runnable );
   }

   //////////////////////////////////////////////////////////////////////////
   // Changes number of samples
   //////////////////////////////////////////////////////////////////////////
   public void NumberOfSamples()
   {
      int num_samples;

      num_samples = (int) GetDResource( "DataGroup/Factor" );

      num_samples += 10;
      if( num_samples >= 60 )
        num_samples = 20;

      if( timer != null )
        timer.stop();      // Stop updates

      SetDResource( "DataGroup/Factor", (double) num_samples );
      Update();

      // Adjust number of labes to a new number of samples
      ChangeXLabels( 0 ); 

      if( timer != null )
        timer.start();     // Resume updates
   }

   //////////////////////////////////////////////////////////////////////////
   // Changes scroll type
   //////////////////////////////////////////////////////////////////////////
   public void ScrollType()
   {
      int scroll_type;

      scroll_type = (int) GetDResource( "DataGroup/ScrollType" );

      // Toggle between WRAP and SCROLL
      if( scroll_type == GlgObject.WRAPPED )
        scroll_type = GlgObject.SCROLLED; 
      else
        scroll_type = GlgObject.WRAPPED;

      SetDResource( "DataGroup/ScrollType", (double) scroll_type );
      Update();
   }

   //////////////////////////////////////////////////////////////////////////
   // Changes scroll direction
   //////////////////////////////////////////////////////////////////////////
   public void Reverse()
   {
      int inversed;

      if( GraphType != RIBBON )    // No reversing for 3D line
      {
         inversed = (int) GetDResource( "DataGroup/Inversed" );

         // Toggle between 0 and 1
         if( inversed == 0 )
           inversed = 1;
         else
           inversed = 0;

         SetDResource( "DataGroup/Inversed", inversed );
         Update();
      }
   }

   //////////////////////////////////////////////////////////////////////////
   // Changes range
   //////////////////////////////////////////////////////////////////////////
   public void ChangeRange()
   {
      double high_range;

      high_range = GetDResource( "YLabelGroup/YLabel/High" );

      if( high_range == 1.0 )
        high_range = 5;
      else
        high_range = 1.0;

      SetDResource( "YLabelGroup/YLabel/High", high_range );
      Update();
   }

   //////////////////////////////////////////////////////////////////////////
   // Changes the number of digits after the decimal point for Y labels
   //////////////////////////////////////////////////////////////////////////
   public void ChangeYFormat()
   {
      String format;

      format = GetSResource( "YLabelGroup/YLabel0/Format" );
      if( format.equals( "%.1lf" ) )
        format = "%.2lf";
      else
        format = "%.1lf";

      SetSResource( "YLabelGroup/YLabel0/Format", format );
      Update();
   }

   //////////////////////////////////////////////////////////////////////////
   // Changes the number of Y labels
   //////////////////////////////////////////////////////////////////////////
   public void ChangeYLabels()
   {
      int num_labels, num_minor_ticks;

      num_labels = (int) GetDResource( "YLabelGroup/Factor" );

      if( num_labels == 5 )
      {
         num_labels = 4;
         num_minor_ticks = 3;
      }
      else
      {
         num_labels = 5;
         num_minor_ticks = 2;
      }
      
      SetDResource( "YLabelGroup/Factor", (double) num_labels );
      SetDResource( "YLabelGroup/MinorFactor", (double) num_minor_ticks );
      Update();
   }

   //////////////////////////////////////////////////////////////////////////
   // Changes the number of X labels
   //////////////////////////////////////////////////////////////////////////
   public void ChangeXLabels( int change )
   {
      int num_samples, num_labels, num_minor_ticks;

      num_samples = (int) GetDResource( "DataGroup/Factor" );
      num_labels = (int) GetDResource( "XLabelGroup/Factor" );

      if( change != 0 )
        if( num_labels == 5 )
          num_labels = 10;
        else
          num_labels = 5;
      
      if( num_labels == num_samples )
        // No minor ticks: we have a lable for each sample
        num_minor_ticks = 1;
      else
        num_minor_ticks = num_samples / num_labels;

      SetDResource( "XLabelGroup/Factor", (double) num_labels );
      SetDResource( "XLabelGroup/MinorFactor", (double) num_minor_ticks );
      Update();
   }

   //////////////////////////////////////////////////////////////////////////
   public void OneSample()
   {
      UpdateAfterEachSample = true;
   }

   //////////////////////////////////////////////////////////////////////////
   public void WholeFrame()
   {
      UpdateAfterEachSample = false;
   }

   //////////////////////////////////////////////////////////////////////////
   public void StopUpdate()
   {
      PerformUpdates = false;
   }

   //////////////////////////////////////////////////////////////////////////
   public void StartUpdate()
   {
      PerformUpdates = true;
   }

   //////////////////////////////////////////////////////////////////////////
   public void SetTitles( String title, String titleX, String titleY )
   {
      SetSResource( "Title/String", title );
      SetSResource( "XAxisLabel/String", titleX );
      SetSResource( "YAxisLabel/String", titleY );
      Update();
   }

   //////////////////////////////////////////////////////////////////////////
   public void BackColor( double red, double green, double blue )
   {
      SetGResource( "FillColor", red, green, blue );
      Update();
   }

   //////////////////////////////////////////////////////////////////////////
   public void GraphColor( double red, double green, double blue )
   {
      SetGResource( "DataArea/FillColor", red, green, blue );
      Update();
   }

   //////////////////////////////////////////////////////////////////////////
   public void  DataColor( double red, double green, double blue )
   {
      if( GraphType == BAR || GraphType == RIBBON )
        SetGResource( "DataGroup/DataSample%/FillColor", red, green, blue );

      else if( GraphType == BAR_3D )
        SetGResource( "DataGroup/DataSample%/Element/FillColor",
                     red, green, blue );

      else if( GraphType == LINE )
      {
         SetGResource( "DataGroup/Polygon/EdgeColor", red, green, blue );
         SetGResource( "DataGroup/Markers/Marker%/FillColor",
                      red, green, blue );
      }

      Update();
   }
   
   //////////////////////////////////////////////////////////////////////////
   // Set the template's color to Local to allow coloring individual data
   // samples
   //////////////////////////////////////////////////////////////////////////
   public void SetColorLocal()
   {
      if( GraphType == BAR || GraphType == RIBBON )
        SetDResource( "DataGroup/DataSample/FillColor/Global", 0.0 );
      else if( GraphType == BAR_3D )
        SetDResource( "DataGroup/DataSample/Element/FillColor/Global", 0.0 );
      else if( GraphType == LINE )
      {
         SetDResource( "DataGroup/Marker/FillColor/Global", 0.0 );
         // Using size and marker type to annotate too
         SetDResource( "DataGroup/Marker/MarkerSize/Global", 0.0 );
         SetDResource( "DataGroup/Marker/MarkerType/Global", 0.0 );
      }
   }

   //////////////////////////////////////////////////////////////////////////
   public void Data5Color( double red, double green, double blue )
   {
      if( GraphType == BAR || GraphType == RIBBON )
        SetGResource( "DataGroup/DataSample5/FillColor", red, green, blue );

      else if( GraphType == BAR_3D )
        SetGResource( "DataGroup/DataSample5/Element/FillColor",
                     red, green, blue );

      else if( GraphType == LINE )
      {
         SetGResource( "DataGroup/Markers/Marker5/FillColor",
                      red, green, blue );
         SetDResource( "DataGroup/Markers/Marker5/MarkerSize", 8.0 );
         SetDResource( "DataGroup/Markers/Marker5/MarkerType", 24.0 );
      }
      
      Update();
   }

   //////////////////////////////////////////////////////////////////////////
   public void LabelColor( double red, double green, double blue )
   {
      SetGResource( "XLabelGroup/XLabel0/TextColor", red, green, blue );
      SetGResource( "XAxisLabel/TextColor", red, green, blue );

      SetGResource( "YLabelGroup/YLabel0/TextColor", red, green, blue );
      SetGResource( "YAxisLabel/TextColor", red, green, blue );

      Update();
   }

   //////////////////////////////////////////////////////////////////////////
   public void TitleColor( double red, double green, double blue )
   {
      SetGResource( "Title/TextColor", red, green, blue );
      Update();
   }

   //////////////////////////////////////////////////////////////////////////
   public void TitleFont( int font )
   {
      SetDResource( "Title/FontType", font );
      Update();
   }

   //////////////////////////////////////////////////////////////////////////
   public void TitleSize( int size )
   {
      SetDResource( "Title/FontSize", size );
      Update();
   }

   //////////////////////////////////////////////////////////////////////////
   public void MoveTitle( String direction )
   {
      double x, y;
      
      x = GetXResource( "Title/Point1" );
      y = GetYResource( "Title/Point1" );

      if( direction.equals( "left" ) )
        x -= 100;
      else if( direction.equals( "right" ) )
        x += 100;
      else if( direction.equals( "up" ) )
        y += 100;
      else if( direction.equals( "down" ) )
        y -= 100;

      SetGResource( "Title/Point1", x, y, 0.0 );
      Update();
   }

   //////////////////////////////////////////////////////////////////////////
   public void Grid( int x_grid )
   {
      int grid_on;
      String resource_name;

      if( x_grid != 0 )
        resource_name = "XGridGroup/Visibility";
      else
        resource_name = "YGridGroup/Visibility";

      grid_on = (int) GetDResource( resource_name );

      // Toggle between 0 and 1
      if( grid_on == 0 )
        grid_on = 1;
      else
        grid_on = 0;
        
      SetDResource( resource_name, (double) grid_on );
      Update();
   }
   
   //////////////////////////////////////////////////////////////////////////
   public void SetGraphType( int type )
   {
      String graph_drawing;

      if( GraphType == type )
        return;

      if( type == BAR )
        graph_drawing = "bar1.g";
      else if( type == BAR_3D ) 
        graph_drawing = "bar101.g";
      else if( type == LINE ) 
        graph_drawing = "line1.g";
      else  // RIBBON
        graph_drawing = "line101.g";

      GraphType = type;
      IsReady = false;
      SetDrawingURL( graph_drawing );
   }

   //////////////////////////////////////////////////////////////////////////
   // ActionListener method to use the bean as update timer's ActionListener.
   //////////////////////////////////////////////////////////////////////////
   public void actionPerformed( ActionEvent e )
   {
      UpdateGraphProc();
   }

   //////////////////////////////////////////////////////////////////////////
   // Invoked by the browser to stop the applet
   //////////////////////////////////////////////////////////////////////////
   public void stop()
   {
      StopUpdates();
      GraphType = BAR;
      IsReady = false;
      super.stop();
   }
}
