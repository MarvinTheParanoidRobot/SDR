This example demonstrates how to display and update a GLG realtime
stripchart widget and use its integrated interactive behavior. The
example supports both horizontal and vertical stripchart.

To build and run the example:

Linux
  export CLASSPATH=.:./GlgCE.jar   
Windows
  set CLASSPATH=.:.\GlgCE.jar  

  javac  GlgRTChartExample.java
  java GlgRTChartExample

The example supports the following command line options:
    -random-data         (use simulated demo data)
    -live-data           (use live application date from LiveDataFeed)
    drawing_name         (specifies GLG drawing to be loaded an animated

By default, the example loads stripchart.g drawing containing a
horizontal stripchart. To use a vertical stripchart drawing
stripchart_vertical.g, specify the drawing name on the command line:

  java GlgRTChartExample stripchart_vertical.g

The example uses simulated data generated by DemoDataFeed. 
To use live data for animation:

  - Set GlgRTChartExample.RANDOM_DATA=false, 
    or use command line option -live-data.

  - Provide a custom implementation of LiveDataFeed class.

DETAILS

GlgChart class is derived from GlgJBean and encapsulates methods for
initializing the chart, updating the chart with data and handling user
interaction. It is added to the parent container GlgRTChartExample.

GlgChart loads and displays a drawing passed as an argument to the
LoadDrawing() method. It is expected that the drawing includes a
viewport named ChartViewport as well as interface widgets allowing to
scroll and zoom the chart. ChartViewport contains the Chart object
which plots the chart data.
  
The chart is initilaized in the H and V callbacks and updated with
data supplied by the DataFeed using a timer. The demo uses
DemoDataFeed to generate simulated data. The application should provide
a custom data feed implementation via LiveDataFeed class. Both
DemoDataFeed and LiveDataFeed implement DataFeedInterface.

The X axis labels display current date and time using the time format
defined in the drawing. Data points in the chart are positioned
according to their time stamp. An application may provide a time stamp
for each data point, otherwise the chart will automatically use
current time for the time stamp.

To supply a time stamp explicitly, set GlgChart.SUPPLY_TIME_STAMP = true.
 
This example is written using GLG Intermediate API.
