/***************************************************************************
  This example demonstrates how to display and update a GLG realtime
  stripchart widget.

  GlgJLWBean is added to the parent container and displays a stripchart
  drawing "chart2.g".
  
  The graph is initilaized in the H and V callbacks and updated with 
  simulated data using a timer. 

  GetPlotValue() method, used in this example to generate demo data,
  may be replaced with a custom data feed.

  The X axis labels display current date and time using the time format
  defined in the drawing. By default, the labels are generated automatically 
  by the graph, however the program may supply a time stamp for 
  each data iteration, by setting use_current_time=False in UpdateChart()
  and replacing code in GetTimeStamp() method.

  This example is written using GLG Standard API. 
 ***************************************************************************/

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import java.lang.reflect.*;
import com.genlogic.*;

//////////////////////////////////////////////////////////////////////////
public class GlgRTChartSimple extends JApplet implements ActionListener
{
  static final long serialVersionUID = 0;

  GlgJLWBean glg_bean;
  Timer timer = null;

  final double TIME_SPAN = 60.;          // Time Span in sec.
  final int NUM_PLOTS = 3;               // Number of lines in a chart.

  // Low and High range of the incoming data values.
  double Low = 0.;
  double High = 10.;
  
  int UpdateInterval = 100;       // Update interval in msec.
  double TimeSpan = TIME_SPAN;    // Currently displayed Time axis span in sec.

  // Store object IDs for each plot. 
  // Used for performance optimization in the chart data feed.
  GlgObject Plots[];

  // Number of plots as defined in the drawing.
  int num_plots_drawing;

  boolean IsReady = false;

  // Used in GetPlotValue() to generate demo data.
  double counter = 0;

  /////////////////////////////////////////////////////////////////////
  // Constructor, where Glg bean component is created and added to a 
  // native Java container, an Applet in this case.
  /////////////////////////////////////////////////////////////////////
  public GlgRTChartSimple()
  {
     getContentPane().setLayout( new GridLayout( 1, 1 ) );
     
     glg_bean = new GlgJLWBean();
     getContentPane().add( glg_bean );
     
     // Add event listeners.
     glg_bean.AddListener( GlgObject.H_CB, new HListener() );
     glg_bean.AddListener( GlgObject.V_CB, new VListener() );
     glg_bean.AddListener( GlgObject.READY_CB, new ReadyListener() );
     glg_bean.AddListener( GlgObject.INPUT_CB, new InputListener() );

     // Disable automatic update for input events to avoid slowing down 
     // real-time chart updates.
     glg_bean.SetAutoUpdateOnInput( false );
  }
  
  //////////////////////////////////////////////////////////////////////
   // Invoked by the browser to start the applet
   //////////////////////////////////////////////////////////////////////
   public void start()
   {
      super.start();

      // Set a GLG drawing to be displayed in the GLG bean, line1.g.
      // The drawing name is relative to the current directory 
      // or applet's document base if used in a browser. 
      // SetParentApplet() must be called to pass the document base
      // directory of the applet to the bean.
      //
      glg_bean.SetParentApplet( this );
      glg_bean.SetDrawingName( "chart2.g" );
      StartUpdates();
   }

   ///////////////////////////////////////////////////////////////////////
   // Invoked by the browser asynchronously to stop the applet.
   ///////////////////////////////////////////////////////////////////////
   public void stop()
   {
      // Using invokeLater() to make sure the applet is destroyed in the
      // event thread to avoid threading exceptions.
      SwingUtilities.
        invokeLater( new Runnable(){ public void run() { Stop(); } } );
   }

   ///////////////////////////////////////////////////////////////////////
   // Invoked from the event thread via invokeLater().
   ///////////////////////////////////////////////////////////////////////   
   public void Stop()
   {
      // Stop periodic updates
      StopUpdates();

      super.stop();
   }

   //////////////////////////////////////////////////////////////////////////
   // For use as a stand-alone application
   //////////////////////////////////////////////////////////////////////////
   public static void main( final String arg[] )
   {
      SwingUtilities.
         invokeLater( new Runnable(){ public void run() { Main( arg ); } } );
   }

   ///////////////////////////////////////////////////////////////////////
   public static void Main( final String arg[] )
   {
      class DemoQuit extends WindowAdapter
      {
         public void windowClosing( WindowEvent e ) { System.exit( 0 ); }
      }

      JFrame frame = new JFrame();
      frame.setResizable( true );
      frame.setSize( 650, 500 );
      frame.addWindowListener( new DemoQuit() );
      
      GlgRTChartSimple graph = new GlgRTChartSimple();
      frame.getContentPane().add( graph );
      frame.setVisible( true );
      
      // Set a GLG drawing to be displayed in the GLG bean
      // Set after layout negotiation has finished.
      //
      graph.glg_bean.SetDrawingName( "chart2.g" );

      // Start periodic updates.
      graph.StartUpdates();
   }

   //////////////////////////////////////////////////////////////////////
   // StartUpdates() creates a timer to perform periodic updates.
   // The timer invokes the bean's UpdateGraphProc() method to update
   // drawing's resoures with new data values.
   /////////////////////////////////////////////////////////////////////
   void StartUpdates()
   {
      if( timer == null )
      {
         timer = new Timer( UpdateInterval, this );
         timer.setRepeats( true );
         timer.start();
      }
   }

   ///////////////////////////////////////////////////////////////////////
   // StopUpdate() method stops periodic updates
   ///////////////////////////////////////////////////////////////////////
   void StopUpdates()
   {
      if( timer != null )
      {
         timer.stop();
         timer = null;
      }

      // Disable dynamic updates of the drawing
      IsReady = false;
   }

   ////////////////////////////////////////////////////////////////////////
   // Define a class HListener which implements GlgHListener interface. 
   // This class should provide an implementation of the HCallback method.
   // HCallback is invoked after the Glg drawing is loaded, but before the 
   // hierarchy setup takes place and before the drawing is drawn for the 
   // first time. 
   ////////////////////////////////////////////////////////////////////////
   class HListener implements GlgHListener
   {
      public void HCallback( GlgObject viewport )
      {
         double major_interval, minor_interval;

         // Retrieve the number of plots defined in the drawing.
         num_plots_drawing = 
           viewport.GetDResource( "Chart/NumPlots" ).intValue();

         // Set number of plots as needed.
         viewport.SetDResource( "Chart/NumPlots", NUM_PLOTS );
         
         // Set Time Span for the X axis.
         viewport.SetDResource( "Chart/XAxis/Span", TimeSpan );

         // Set tick intervals for the Time axis.
         // Use positive values for absolute time interval, for example
         // set major_interval = 10 for a major tick every 10 sec.
         //
         major_interval = -6;     // 6 major intervals
         minor_interval = -5;     // 5 minor intervals
         viewport.SetDResource( "Chart/XAxis/MajorInterval", major_interval );
         viewport.SetDResource( "Chart/XAxis/MinorInterval", minor_interval );
         
         // Set data value range. Since the graph has one Y axis and
         // common data range for the plots, Low/High data range is
         // set on the YAxis level.
         //
         viewport.SetDResource( "Chart/YAxis/Low", Low );
         viewport.SetDResource( "Chart/YAxis/High", High );
      }
   }

   //////////////////////////////////////////////////////////////////////
   // Define a class VListener which implements GlgVListener interface 
   // and implements its VCallback() method. VCallback() is invoked after 
   // the drawing is loaded and setup, but before it is drawn for the
   // first time. 
   /////////////////////////////////////////////////////////////////////
   class VListener implements GlgVListener
   {
      public void VCallback( GlgObject viewport )
      {
         int i;

         // Store object IDs for each plot. Assign line color and/or
         // line annotation as needed.
         //
         Plots = new GlgObject[ NUM_PLOTS ];
         for( i=0; i<NUM_PLOTS; ++i )
         {
            Plots[i] = viewport.GetNamedPlot( "Chart", "Plot#" + i ); 
         }

         // For the existing plots, use color and line annotation setting 
         // from the drawing; initialize new plots using random colors and 
         // strings for demo purposes.
         //
         if( num_plots_drawing < NUM_PLOTS )
           for( i=num_plots_drawing; i < NUM_PLOTS; ++i )
           {
              // Using a random color for a demo.
              Plots[i].SetGResource( "EdgeColor", GlgObject.Rand(0., 1.), 
                                     GlgObject.Rand(0., 1.), 
                                     GlgObject.Rand(0., 1.) );
              Plots[i].SetSResource( "Annotation", "Var" + i );
           }
      }
   }

   ////////////////////////////////////////////////////////////////////////
   // Define a class ReadyListener which implements GlgReadyListener 
   // interface. This class should provide an implementation of the
   // ReadyCallback method. ReadyCallback is invoked after the drawing 
   // is loaded, setup and initially drawn.
   ////////////////////////////////////////////////////////////////////////
   class ReadyListener implements GlgReadyListener
   {
      public void ReadyCallback( GlgObject viewport )
      {
         IsReady = true;
      }
   }

   ///////////////////////////////////////////////////////////////////////
   // timer's ActionListener method to be invoked peridically 
   //////////////////////////////////////////////////////////////////////
   public void actionPerformed( ActionEvent e )
   {
      UpdateGraphProc();
   }
   
   ///////////////////////////////////////////////////////////////////////
   // Update procedure to update a graph with random data. This procedure 
   // is invoked periodically by a timer.
   ///////////////////////////////////////////////////////////////////////
   void UpdateGraphProc()
   {
      // Perform dynamic updates only if the drawing is ready and 
      // the timer is active.
      if( !IsReady )
         return;

      boolean use_current_time = true; // automatic X axis labeling.
            
      // Supply demo data to update plot lines. 
      // In this example, current time is automatically supplied
      // by the chart. The application may supply a time stamp instead,
      // by replacing code in GetTimeStamp() method. 
      //
      GetChartData( use_current_time ? 0. : GetTimeStamp(), 
                    use_current_time);
      
      glg_bean.Update();
   }

   ///////////////////////////////////////////////////////////////////////
   // Supplies chart data for each plot.
   ///////////////////////////////////////////////////////////////////////
   void GetChartData( double time_stamp, boolean use_current_time )
   {
      double value;
      
      for( int i=0; i<NUM_PLOTS; ++i )
      {
         // Get new data value. The example uses simulated data, while
         // an application will replace code in GetPlotValue() to
         // supply application specific data for a given plot index.
         //
         value = GetPlotValue( i );
         
         // Supply plot value for the chart via ValueEntryPoint.
         Plots[i].SetDResource( "ValueEntryPoint", value );
                 
         // Supply an optional time stamp. If not supplied, the chart will 
         // automatically generate a time stamp using current time. 
         //
         if( !use_current_time )  
         {   
            Plots[i].SetDResource( "TimeEntryPoint", time_stamp );
         }
         
         // Set ValidEntryPoint resource only if a graph needs to display
         // holes for invalid data points.
         //
         Plots[i].SetDResource( "ValidEntryPoint", 1. /*valid*/ );
      }
   }

   ///////////////////////////////////////////////////////////////////////
   // Handle user interaction.
   ///////////////////////////////////////////////////////////////////////
   class InputListener implements GlgInputListener
   {
      public void InputCallback( GlgObject viewport, GlgObject message_obj )
      {
         String
           origin,
           format,
           action;
         
         origin = message_obj.GetSResource( "Origin" );
         format = message_obj.GetSResource( "Format" );
         action = message_obj.GetSResource( "Action" );
         
         // Handle window closing if run stand-alone. 
         if( format.equals( "Window" ) && action.equals( "DeleteWindow" ) )
           System.exit( 0 );
      }
   }
   
   /////////////////////////////////////////////////////////////////////// 
   // Supplies plot values for the demo. In a real application, these data 
   // will be coming from an application-specific data source. 
   // time_stamp parameter is not used for demo data, but in a real
   // application the plot's data value may be retrieved based
   // on a given plot index.
   /////////////////////////////////////////////////////////////////////// 
   double GetPlotValue( int plot_index )
   {
      double 
        half_amplitude, center,
        period,
        value,
        alpha;
      
      half_amplitude = ( High - Low ) / 2.;
      center = Low + half_amplitude;
      
      period = 100. * ( 1. + plot_index * 2. );
      alpha = 2. * Math.PI * counter / period;
      
      value = center + 
        half_amplitude * Math.sin( alpha ) * Math.sin( alpha / 30. );

      ++counter;
      return value;
   }
   
   /////////////////////////////////////////////////////////////////////// 
   // For demo purposes, returns current time in seconds. 
   // Place application specific code here to return a time stamp as needed.
   /////////////////////////////////////////////////////////////////////// 
   double GetTimeStamp( )
   {
      return GetCurrTime(); // for demo purposes.
   }
   
   /////////////////////////////////////////////////////////////////////// 
   // Return exact time including fractions of seconds.
   /////////////////////////////////////////////////////////////////////// 
   double GetCurrTime()
   {
      return System.currentTimeMillis() / 1000.;
   }
   
   /////////////////////////////////////////////////////////////////////// 
   void error( String string, boolean quit )
   {
      System.out.println( string );
      if( quit )
         System.exit( 0 );
   }  
}
