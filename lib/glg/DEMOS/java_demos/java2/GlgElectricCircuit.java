import java.awt.*;
import java.awt.event.*;
import java.lang.reflect.*;
import javax.swing.*;
import com.genlogic.*;

//////////////////////////////////////////////////////////////////////////
public class GlgElectricCircuit extends GlgJBean implements ActionListener
{
   //////////////////////////////////////////////////////////////////////////
   // The main demo class
   //////////////////////////////////////////////////////////////////////////
   static final long serialVersionUID = 0;

   // If true, the resource path is used to animate resources of the drawing.
   // If false, stored resource ID is used to set resource directly with 
   // null path using the Extended API. 
   // Alternatively, tags may be used instead of resources.
     //
   final boolean USE_RESOURCE_PATH = false;

   Timer timer = null;
   final int UpdateInterval = 500;    // milliseconds
   boolean UpdateWithData = true;

   // Array of resources to update, queried from the drawing.
   GlgObject ResourceList;

   // Keeps info of a resource to update
   class SimulationResource
   {
      GlgObject glg_object;
      int type;
      double range;
      String resource_path;
   }

   //////////////////////////////////////////////////////////////////////////
   public GlgElectricCircuit()
   {
      super();
   }

   //////////////////////////////////////////////////////////////////////////
   public void StartUpdates()
   {
      if( timer == null )
      {
         // Restart the timer after each update (instead of using repeats)
         // to avoid flooding the event queue with timer events on slow 
         // machines.
         timer = new Timer( UpdateInterval, this );
         timer.setRepeats( false );
         timer.start();
      }
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
   public void ToggleUpdates()
   {
      UpdateWithData = !UpdateWithData;
   }

   //////////////////////////////////////////////////////////////////////////
   // Starts updates
   //////////////////////////////////////////////////////////////////////////
   public void ReadyCallback( GlgObject viewport )
   {
      super.ReadyCallback( viewport );

      InitializeSimulation();

      StartUpdates();
   }

   //////////////////////////////////////////////////////////////////////////
   // For use as a stand-alone java demo.
   // Optional command-line arguments: [-drawing filename]
   //////////////////////////////////////////////////////////////////////////
   public static void main( final String [] arg )
   {
      SwingUtilities.
        invokeLater( new Runnable(){ public void run() { Main( arg ); } } );
   }

   public static void Main( String [] arg )
   {
      String filename = "electric_circuit.g";

      class DemoQuit extends WindowAdapter
      {
         public void windowClosing( WindowEvent e ) { System.exit( 0 ); }
      } 

      int arg_length = Array.getLength( arg );
      int i = 0;

      // Parse options
      while( i < arg_length )
      {
         if( arg[ i ].equals( "-drawing" ) )
         {
            ++i;
            if( i >= arg_length )
              error( "Missing filename after -drawing option", true );
            filename = arg[i];
         }
      }

      GlgObject.Init();

      JFrame frame = new JFrame( "GLG Electrical Circuit Monitoring Demo" );
      frame.setResizable( true );
      frame.setSize( 800, 600 );
      frame.addWindowListener( new DemoQuit() );

      GlgElectricCircuit circuit = new GlgElectricCircuit();

      frame.getContentPane().add( circuit );
      frame.setVisible( true );

      // Assign a drawing filename after the frame became visible and 
      // determined its client size to avoid unnecessary resizing of 
      // the drawing.
      // Loading the drawing triggers ReadyCallback which starts updates.
      //
      circuit.SetDrawingName( filename );
   }

   //////////////////////////////////////////////////////////////////////////
   // Updates display with simulated data.
   //////////////////////////////////////////////////////////////////////////
   void UpdateCircuit()
   {
      if( timer == null || ResourceList == null )
        return;

      if( UpdateWithData )
      {
         int size = ResourceList.GetSize();
         for( int i=0; i<size; ++i )
         {
            SimulationResource resource = (SimulationResource)
              ResourceList.GetElement( i );
            
            if( resource.type != GlgObject.D ) 
              continue;      // Update only resources of D type
            
            double value = GlgObject.Rand( 0.0, resource.range );
            if( USE_RESOURCE_PATH )
              // Use resource path.
              SetDResource( resource.resource_path, value );
            else
              // Use stored resource ID with null path to set the resource 
              // directly using the Extended API.
              resource.glg_object.SetDResource( null, value );

            Update();
         } 
      }

      timer.start();   // Restart the update timer
   }

   //////////////////////////////////////////////////////////////////////////
   // This callback is invoked when user selects some object in the drawing
   // with the mouse. 
   //////////////////////////////////////////////////////////////////////////
   public void SelectCallback( GlgObject viewport, Object[] name_array, int button )
   {
      super.SelectCallback( viewport, name_array, button );

      // Handle mouse selection here
   }

   //////////////////////////////////////////////////////////////////////////
   // This callback is invoked when user interacts with input objects in GLG
   // drawing. 
   //////////////////////////////////////////////////////////////////////////
   public void InputCallback( GlgObject viewport, GlgObject message_obj )
   {
      String
        origin,
        format,
        action;

      super.InputCallback( viewport, message_obj );

      origin = message_obj.GetSResource( "Origin" );
      format = message_obj.GetSResource( "Format" );
      action = message_obj.GetSResource( "Action" );

      // Handle user input here
      if( format.equals( "Button" ) )
      {
         if( action.equals( "Activate" ) )
         {
            if( origin.equals( "Resources" ) )
              PrintResources();
            else if( origin.equals( "ToggleUpdates" ) )
              ToggleUpdates();
         }
      }
   }

   //////////////////////////////////////////////////////////////////////////
   // Prints resources of the drawing to use for animation (resources whose
   // name starts with the "#" character.
   //////////////////////////////////////////////////////////////////////////
   void PrintResources()
   {
      if( ResourceList == null )
      {
         WriteLine( "Found no resources to update!" );
         return;
      }

      WriteLine( "Resource list for updates: resource_path type" );
      
      int size = ResourceList.GetSize();
      for( int i=0; i<size; ++i )
      {
         SimulationResource resource = (SimulationResource)
           ResourceList.GetElement( i );

         String data_type_str;
         switch( resource.type )
         {
          case GlgObject.D: data_type_str = "d"; break;
          case GlgObject.S: data_type_str = "s"; break;
          case GlgObject.G: data_type_str = "g"; break;
          default:
            error( "Invalid resource type.", false ); 
            continue;
         }
      
         WriteLine( resource.resource_path + " " + data_type_str );
      }

      WriteLine( "Resource list: Done." );
   }

   //////////////////////////////////////////////////////////////////////////
   void WriteLine( String line )
   {
      System.out.println( line );
   }

   //////////////////////////////////////////////////////////////////////////
   // Creates a list of resources to animate defined in the drawing.
   // Queries the drawing to include all resources of interest that are 
   // marked by having "#" as the first character of their names. 
   // Alternatively, tags may be used instead of resources.
   //////////////////////////////////////////////////////////////////////////
   GlgObject GetResourceList( GlgObject obj, String res_path, GlgObject list )
   {
      // Using only named resources in this example, no aliases.
      GlgObject res_list = obj.CreateResourceList( true, false, false );
      if( res_list == null )
        return list;
     
      int size = res_list.GetSize();
      for( int i=0; i<size; ++i )
      {
         GlgObject glg_object = (GlgObject) res_list.GetElement( i );

         String name = glg_object.GetSResource( "Name" );
         if( !name.startsWith("#") )
           // We are interested only in resources that start with #
           continue;

         // Accumulate resource path.
         String new_path;
         if( res_path == null )
           new_path = name;
         else
           new_path = res_path + "/" + name;

         int object_type = glg_object.GetDResource( "Type" ).intValue();

         // Data or attribute object: add to the list of resources to animate.
         if( object_type == GlgObject.DATA || 
            object_type == GlgObject.ATTRIBUTE )
         {
            SimulationResource resource = new SimulationResource();
            resource.glg_object = glg_object;
            resource.type = glg_object.GetDResource( "DataType" ).intValue();
            resource.resource_path = new_path; 
            
            // Set range for animating the resource.
            // State resources may have ON (1) and OFF (0) values - 
            // use 1.3 as a range to simulate. Use range=100 for the rest 
            // of resources.
              //
            if( name.equals( "#State" ) )
              resource.range = 1.3;
            else
              resource.range = 1000.0;
            
            // Create a list of does not yet exist.
            if( list == null )
              list = new GlgDynArray( GlgObject.GLG_OBJECT, 0, 0 );
            
            list.AddObjectToBottom( resource );
         }

         double has_resources = 
           glg_object.GetDResource( "HasResources" ).intValue();

         // If object's HasResources=ON, recursively traverse all resources
         // inside it.
         if( has_resources == 1.0 )
           list = GetResourceList( glg_object, new_path, list );
      }
      return list;
   }

   //////////////////////////////////////////////////////////////////////////
   // Creates a list of resources to animate.
   //////////////////////////////////////////////////////////////////////////
   void InitializeSimulation()
   {
      ResourceList = GetResourceList( GetViewport(), null, null );
      if( ResourceList == null )
        error( "No resources to animate.", false );
   }

   //////////////////////////////////////////////////////////////////////////
   static void error( String message, boolean quit )
   {
      System.out.println( message );

      if( quit )
        System.exit( 1 );
   }

   //////////////////////////////////////////////////////////////////////////
   // ActionListener method to use the bean as update timer's ActionListener.
   //////////////////////////////////////////////////////////////////////////
   public void actionPerformed( ActionEvent e )
   {
      UpdateCircuit();
   }

   //////////////////////////////////////////////////////////////////////////
   // Invoked by the browser to stop the applet
   //////////////////////////////////////////////////////////////////////////
   public void stop()
   {
      StopUpdates();
      super.stop();
   }
}
