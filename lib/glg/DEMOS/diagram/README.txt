This is an example of a simple diagraming editor build using the
Extended API. 

The demo starts in the process diagram mode if the -process-digram option 
(or if process_digram link on Unix) is used to start the demo. Otherwise,
it starts in the diagram editor mode.

The diagramG.c version uses GLG's new Installable Interation Handlers API 
(IIH API) to handle user interaction, which makes it more flexible and 
easier to extend. It also implements Save and Load functionality by saving 
and retrieving the diagram's connectivity information, and using it to 
recreate the diagram's graphics on load.

The diagramG2.c version does not use IIH API. It also implements Save and Load
functionality by saving and retrieving the diagram's drawing as a .g drawing 
file, without saving the diagram's connectivity information.

The demo uses GLG input objects for user interface and the GLG Generic
API for programming interface. This allows the source code to be
compiled and run on both Unix and Windows with no changes.

The demo may be modified to use native MFC interface on Windows and
Motif interface on Unix. This would use different GLG wrappers for
each environment, while the rest of the GLG-related graphics and
related programming logic can still be shared.

The code also shows how to use a Trace callback to handle complex
cases of the user feedback.
