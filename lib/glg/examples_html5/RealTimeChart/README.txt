GLG RealTimeChart example demonstrates how to use a custom chart widget
containing a toolbar allowing to scroll and zoom the chart, as well as change
the time interval on the X axis for viewing the chart data.

The example loads stripchart.g drawing containing a horizontal stripchart.

DemoDataFeed.js supplies simulated data for animation. An application should
provide a custom implementation of LiveDataFeed.js to supply real-time
application data to the chart.

To use live data for animation, set RANDOM_DATA=false in RealTimeChart.js.

Real-time data for the chart are quiried based on tags assigned to the plots in
a chart. Tags are added to the ValueEntryPoint of each plot in the drawing at
design time, and a valid TagSource should be assigned for each tag. Tag sources
represent data source variables used to query data for the plots and are used by
the DataFeed object to query real-time data for for the chart.

The X axis labels display current date and time using the time format defined in
the drawing. Data points in the chart are positioned according to their time
stamp. An application may provide a time stamp for each data point, otherwise
the chart will automatically use current time for the time stamp.

To supply a time stamp explicitly, set SUPPLY_TIME_STAMP=true in
RealTimeChart.js.
 
This example is written using GLG JavaScript Intermediate API. 
