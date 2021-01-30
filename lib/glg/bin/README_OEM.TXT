The OEM version of the Graphics Builder is invoked either by using the "-oem" 
command line option of the Enterprise Edition, or by starting the Builder using
the symbolic link with the "_OEM" suffix.

The OEM version of the Builder provides interfaces for defining public
properties via the Export Tags (the ET button in the Attribute dialog).
Public Properties are used to define a custom set of properties visible 
to the user in the HMI Configurator.

When a component with public properties is used in the HMI Configurator, 
the HMI Configurator's Properties dialog displays only the component's 
properties defined as public. The HMI Editor Widget Samples palette 
provides samples of components with public properties.

The OEM version of the Builder has two buttons for editing properties: 
the Object Properties button displays regular properties, while 
the Public Propertries button displays public properties (if any).

The HMI Configurator has a single Property button by default. If an object 
has public properties defined, it displays a list of public propeties; 
otherwise it displays regular properties. 

The glg_hmi_config file contains configuration variables that enable or 
disable buttons and menus in the HMI Configurator. For example, to enable 
two properties buttons in the HMI configurator, set HMISinglePropertyButton=0.
To disable editing resources of an object, set HMIDisableResources=1.
