OVERVIEW

This example provides a sample implementation of a generic GLG viewer
application that loads a GLG drawing created in the GLG Builder or GLG
HMI Configurator, animates the drawing using tags defined in the
drawing and handles user interaction, including commands attached to
objects in the editor.

DESCRIPTION OF FEATURES

GlgViewer example demonstrates the following functionality:

  - Navigation between various drawings using buttons on a web
    page. The drawing name for each button is specified in
    glg_viewer.html.
  
  - Drawing animation is implemented using tags defined in the drawing
    at design time in the GLG Builder or HMI Configurator. A tag is
    attached to a dynamic data driven parameter of a graphical
    object. The tag's TagSource property represents a data source
    variable from the back-end data acquisition system that drives the
    animation.

  - DataFeed object is used to supply dynamic data values for
    animation. The example uses simulated data generated by
    DemoDataFeed. The application can provide a custom implementation
    of LiveDataFeed to query realtime data from a custom data
    acquisition system. To use live data for animation, set
    RANDOM_DATA flag in GlgViewer.js to false.

  - Upon drawing loading, the program obtains a list of tags defined
    in the drawing and builds TagRecords array with objects of type
    GlgTagRecord containing tag information. After the drawing is
    displayed, the program periodically queries new data values for
    all tags in the list and pushes the new values into the graphics.

  - User interaction includes object selection with the mouse or a
    touch on a mobile device. The example demonstrates how to use a
    generic approach to handle custom events and commands attached to
    objects in the GLG editor. Commands shown in the example include
    PopupMenu, PopupDialog, GoTo, WriteValue and WriteValueFromWidget
    and others.

 - Commands button loads commands.g drawing that demonstrates how to
    handle various commands attached to objects in the loaded
    drawing, such as WriteValue, WriteValueFromWidget, GOTO, PopupDialog,
    PopupMenu commands. 

  - PopupMenu and PopupDialog commands use a viewport named "PopupViewport"
    embedded in the loaded drawing to load and display a popup drawing specified
    by the individual command. This is implemented by using a DrawingArea
    subwindow object inside a PopupViewport. If GLG Extended API is used (and
    HAS_EXTENDED_API flag is set to true), the PopupViewport is added on the fly
    to each loaded drawing, using popup_viewport.g as a template. Otherwise,
    each drawing with PoupMenu or PopupDialog command should store PopupViewport
    in the drawing at design time, using popup_viewport.g as a template and
    naming the viewport "PopupViewport". 
    
  - PopupMenu and PopupDialog commands are context sensitive to the selected
    object. For example, clicking on one of the tanks in commands.g displays a
    floating dialog with a RealTimeChart showing data for the selected
    object. The popup chart plots data specified by the PlotValue tag defined in
    the selected object. The chart displays YAxis label (%, GAL, etc.) as
    defined by the command (using command's ParamS parameter). Likewise,
    clicking on one of the valves in commands.g will bring a popup menu allowing
    to open or close the selected valve.

  - Popup object may have a WidgetType property allowing to initialize
    the popup object accordingly. For example, RTChartDialog in commands.g
    drawing has WidgetType = "RTChart".

  - If a loaded drawing contains an embedded RealTimeChart widget, the
    example shows how to supply a time stamp for each data sample
    explicitly, in addition to the value. By default, the time stamp
    for the chart is generated automatically using current time.
    To enable explicit time stamp, set SUPPLY_PLOT_TIME_STAMP=true.

  - PageType property of the loaded drawing may be used to enable
    special handling for this drawing. If PageType is not present
    or empty, Default page type is used and the drawing is handled in
    a generic way.

  - RealTimeChart button loads rt_chart.g drawing with
    PageType = "RealTimeChart". GlgChart.js module provided special
    handling for this page, including chart scrolling or zooming,
    chart dragging, changing the chart's Span to show historical data,
    etc.

  - Alarms button displays a list of alarms using a floating
    dialog. The loaded alarms.g drawing contains a custom table
    created in the GLG Builder, which is populated with a list of
    alarms at run-time. An alarm can be acknowledged by a Ctrl+click
    on the alarm row. On a mobile device, an alarm can be acknowledged
    by touching the alarm row. The Alarms button is highlighted in red
    if unacknowledged alarms are detected. The alarms dialog
    functionality is included in GlgAlarms.js module.

  - Pressing the Escape key closes a popup dialog and Alarm dialog, if any.

  - When a web page is used on a mobile device, adjustments are made
    to take into account devicePixelRatio of the device:

    SetCanvasResolution() sets scaling factors for the GLG rendering engine.
    AdjustForMobileDevices() makes desired adjustments in the loaded drawing. 
    AdjustChartForMobileDevices() makes adjustments in the RealTimeChart page.
    AdjustAlarmDialogForMobileDevices() makes adjustments in the alarm dialog.

USING LIVE DATA FOR ANIMATION

1. Provide custom implementation of the Read/Write methods in LiveDataFeed.java.
2. Set RANDOM_DATA flag in GlgViewer.js to false.

MODULES

GlgViewer.js 
   Main module, handles an arbitrary GLG drawing with the Default
   PageType. If PageType is not present in the loaded drawing or it is
   empty, Default PageType is used.

GlgChart.js
   Handles a drawing with PageType = "RealTimeChart". 
   Sample drawing in the example: rt_chart.g

GlgAlarms.js
   Loads alarms.g drawing containing a table populated with a list of
   system alarms.

DemoDataFeed.js
   Supplies simulated data for animation.

LiveDataFeed.js 
   A custom implementation of the data acquisition mechanism to
   read/write real-time data. This module is expected to be provided
   by the application developer.

glg_viewer.html
   Example's web page.

GLG API
  This example is written using GLG Intermediate API.


