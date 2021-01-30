This example demonstrates how to animate a Glg drawing containing a
panel (dashboard) of GLG controls and handle user interaction in a
GLG widget, such as a button or a slider.

The program loads a GLG drawing dashboard.g created in the GLG
Builder. The drawing contains several GLG widgets, including a few
gauges, a push button, a toggle button and a slider. Each widget was
assigned a unique name, such as "DialVoltage", "DialAmps",
"DialPressure", "StartButton", etc.

The program performs drawing animation using either tags mechanism or
resource mechanism, based on the setting of the USE_TAGS flag defined in the
program. 

Tags are added at design time in the GLG Builder. For example, Voltage
and Amps dials have tags added to the dial's Value resource, with
TagSource="Voltage" and TagSource="Current" respectively. The dials
are updated with dynamic data values, using simulated data generated
in the GetData() function.

If USE_TAGS=true, the dials are animated using tags, such as "Voltage"
and "Current". If USE_TAGS=false, resource paths are used to push
dynamic data to the widgets, such as "DialVotage/Value" and
"DialAmps/Value". The use of resources requires each widget to have a
unique name, so that the program can use a unique resource path to
animate the widget. However, the use of tags doesn't require each widget
to be named, as tags are global and all tags are visible at the top
level viewport of the drawing.

The program also demonstrates handling user interaction in a GLG widget, using
Input callback. For example, clicking on a Start/Stop button toggles animation
on/off, and clicking on a slider controls the value if the pressure dial.
