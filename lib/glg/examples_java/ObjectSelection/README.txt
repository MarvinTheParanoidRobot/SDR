
FEATURES DEMONSTARTED IN THE EXAMPLE

1. Glg object selection events handling in the GLG Input
Callback, using one the following methods:

- Process Command Actions attached to an object in the drawing at
  design time (ActionType = SEND COMMAND).

- Process Custom Event Actions attached to an object in the drawing at
  design time (ActionType = SEND EVENT).

- Process generic object selection events for objects without
  Actions. 
 
2. Handling of the Mouse Feedback Actions attached to an object at
design time, with no programming.

Upon start-up, the program loads the GLG drawing obj_selection.g.
MouseFeedback actions, such as MouseOver Highlight or MouseClick
Toggle, are added to the valve objects on the right. At run-time,
these actions are handled automatically, with no programming.

The two dial widgets have Actions of type SEND EVENT, which are
handled at run-time in the Input Callback, Format="CustomEvent". The
dial on the left handles selection event on MouseRelease (right
click), while the dial of the right handles widget selection event on
MouseClick (left click).

The button widgets have Actions of type SEND COMMAND, with
CommandType=WriteValue forthe "SetValue" button, and Commandtype=GoTo
for the "More" button widget. Clicking on the More button will load
another drawing obj_selection2.g, showing additional widgets with
other command types. At run-time, the Commands added to the widgets are
handled in the Input Callback, Format="Command".
 
The tank widget has Actions of type SEND COMMAND, with a custom
command type CommandType="TankSelected". The command is handled in the
Input Callback, Format="Command".

This example is written using GLG Intermediate API. 


