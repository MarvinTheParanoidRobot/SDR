#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950
#endif

typedef enum
{
   BUTTON_PRESS_EVENT = 0,
   MOUSE_MOVE_EVENT,
   LEAVE_EVENT
} EventType;

/* Function prototypes */
void InitChart( void );
void StartUpdate( void );
void UpdateChart( GlgAnyType data, GlgIntervalID * id );
void PushPlotPoint( GlgLong plot_index, DataPoint * data_point );
void Input( GlgObject viewport, GlgAnyType client_data, GlgAnyType call_data );
void Trace( GlgObject viewport, GlgAnyType client_data, GlgAnyType call_data );
void Trace2( GlgObject viewport, GlgAnyType client_data, GlgAnyType call_data );
void ScrollToDataEnd( ShowDataEnd data_end, GlgBoolean show_extra );
double GetExtraSeconds( void );
void DisplaySelection( char * string, GlgBoolean erase_selection_marker );
void ChangeAutoScroll( GlgLong new_state );
void SetChartSpan( GlgLong span_index );
void SetSelectorLabels( void );
void StoreInitialYRanges( void );
void RestoreInitialYRanges( void );
GlgLong ZoomToMode( void );
void AbortZoomTo( void );
void ChangeYAxisLabelType( int new_type );
void error( char * string, GlgBoolean quit );
GlgLong GetAdjustedTimeout( GlgULong sec1, GlgULong microsec1, 
                            GlgLong interval );
double GetPointInterval( void );
void PreFillChartData( void );
void FillHistData( GlgLong plot_index, double start_time, double end_time );
void GetDemoData( GlgLong plot_index, DataPoint * data_point );
void GetDemoPlotValue( GlgLong plot_index, DataPoint * data_point );
double GetCurrTime( void );
void SetMode( GlgLong mode );
void SetTimeFormats( void );
void AdjustButtonColor( void );
void AdjustYAxisSpace( void );
void SetMarkerSize( void );
void SelectPlot( GlgObject plot );
