HOW TO RUN DEMOS

Due to the browsers' security restrictions, JavaScript applications have to be
hosted on a web server. Trying to run them from a local file system will generate
JavaScript errors.

To deploy an example to a web server, copy the example directory to a web
server and open the example's HTML file in a browser.

WINDOWS IIS WEB SERVER NOTES

Custom application/x-glg MIME type should be added for the GLG drawing files
with the .g extension. 

To add a MIME type:
  Start IIS Manager
  Select MIME Types
  Right+click, Add
  In the Add MIME Type dialog, specify MIME type "application/x-glg",
    File name extension ".g"

If the project uses subdrawings that were saved using an .sd extension, the
extension would also need to be assigned the application/x-glg MIME type using
the above steps. The .sd extension is filtered by the IIS by default, resulting
in the "File Not Found" error for the subdrawings.

To disable filtering of the .sd extension, a web.config file containing the
following lines needs to be placed in the root directory of the project:

<?xml version="1.0" encoding="UTF-8"?>
<configuration>
  <system.webServer>
    <security>
      <requestFiltering>
        <fileExtensions allowUnlisted="true">
          <remove fileExtension=".sd" />
          <add fileExtension=".sd" allowed="true" />
        </fileExtensions>
      </requestFiltering>
    </security>
  </system.webServer>
</configuration>

A sample web.config file is provided in the src directory of the GLG
installation. 

GLG JAVASCRIPT LIBRARY

The demos use the demo version of the GLG library (GlgDemo.js) and supporting
files listed below. To use the demos with the Community Edition or production
version of the library, replace GlgDemo.js and GlgToolkitDemo.js with the
corresponding versions of the files. Refer to the Using the JavaScript Version
of the Toolkit chapter of the GLG Programming Reference Manual for more
information.

The GLG JavaScript library is deployed in demos in the form of two files:
GlgDemo.js and GlgToolkitDemo.js. The library also uses the gunzip.min.js file
for uncompressing GLG drawings saved in the compressed format. These three
script files must be included in the html file before the application script as
follows:

    <script defer src="GlgDemo.js"></script>
    <script defer src="GlgToolkitDemo.js"></script>
    <script defer src="gunzip.min.js"></script>
    <script defer src="application_script.js"></script>

DEBUGGING AID

To debug any demo, use either the Community Edition or production version of
the debugging library.

The debugging version of the GLG library is provided in the lib directory of the
GLG installation. While the debugging version executes slower than the highly
optimized non-debugging version, it contains various verification checks that
help catch errors at the application development stage.

The debugging version of the library has the Debug suffix added to its name.

For example, for the GLG Community Edition, the GlgCEDebug.js file provides the
debugging version of the GlgCE.js library, and GlgDebug.js, GlgIntDebug.js or
GlgExtDebug.js files are provided with the production version of the Toolkit.

NOTE: The GlgCEDebugSafari.js file provided with the Community Edition can be
used for debugging with the Safari browser on macOS.

The ThrowExceptionOnError method shown in the demos and examples code may be
used to stop execution on the GLG errors: the stack trace will show the code
location that caused the GLG error.

API USAGE

The JavaScript API online documentation may be found on the GLG web site at the
following link:

  http://www.genlogic.com/doc_html/start.html#JavaScriptDoc

The following provides a brief overview of the JavaScript API.

The vast majority of the JavaScript API methods are the same as the
corresponding Java or C# methods. The only exception are methods that
initialize the GLG API and load the GLG drawing: the demos and
examples provide source code examples of using these methods.

The JavaScript API methods use the same arguments as their
corresponding Java or C# equivalents. For example, the SetDResource
method expects a string as the first parameter and a double value as
the second parameter.

For methods that expect integer parameters, such as GetElement, it is
an error to pass a numerical value that is not an integer, such as
3.5. Such errors will be detected if GlgCEDebug.js library is used
during development. If the GlgCE.js library is used, the error may be
undetected and cause errors late in the program.

The static methods are invoked on a GLG handle obtained via a call to the
GlgToolkit() method:

  var glg = new GlgToolkit();     // Get a handle to the GLG Toolkit library.
  glg.LoadWidgetFromURL( "process.g", null, LoadCB, null );  // Load GLG drawing.

The instance methods are invoked on the object instance:

   viewport.SetDResource( "FillColor", 0.7, 0.7, 0.7 );

Some methods, such as GetResourceObject, return GLG objects. The
Equals method should be used instead of the == operator to check if
two objects are the same:

  obj1.Equals( obj2 );

The ObjectsEqual method can also be used to compare two objects, in
case obj1 may be null:

  glg.ObjectEqual( obj1, obj2 );

GLG CONSTANTS

The GLG constants, such as GLG text type, text direction and others, are
defined similar to the C# enums, for example:

  glg.GlgTextType.WRAPPED_TEXT

The WRAPPED_TEXT constant should be used instead of its numerical value
to provide the required integer value.

The complete list of all enums can be found at the end of the
GlgToolkitCE.js file and can also be accessed online at the following
link:
  
  http://www.genlogic.com/doc_html/javascript_doc/namespacemembers_enum.html

ASYNCHRONOUS LOAD

In JavaScript, all operations that load any file from a server, such as a
drawing, data or any other resource, are asynchronous. As a result, the GLG
methods that load files, such as LoadDrawingFromURL, take a callback parameter.
The callback will be invoked synchronously when the loading request has been
completed. A JavaScript application that loads any file from a server should be
aware of the asynchronous nature of the load requests and use an appropriate
design.

A simplified LoadAsset method of the GLG API may be used by an application
instead of handing HTTP requests in JavaScript to load any type of data the
application may need.
