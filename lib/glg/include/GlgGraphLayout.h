#ifndef _Glg_Graph_Layout_h
#define _Glg_Graph_layout_h

#ifdef __cplusplus
extern "C" {
#endif

typedef struct GlgGraphNodeStruct * GlgGraphNode;
typedef struct GlgGraphEdgeStruct * GlgGraphEdge;
typedef struct GlgGraphLayoutStruct * GlgGraphLayout;

#define GlgGraphMaxIteration( graph )     graph->MaxIteration 
#define GlgGraphSpikesEndTemp( graph )    graph->SpikesEndTemp 
#define GlgGraphChainFixTemp1( graph )    graph->ChainFixTemp1 
#define GlgGraphChainFixTemp2( graph )    graph->ChainFixTemp2 
#define GlgGraphAttractionC1( graph )     graph->AttractionC1 
#define GlgGraphAttractionC2( graph )     graph->AttractionC2 
#define GlgGraphRepulsionC3( graph )      graph->RepulsionC3 
#define GlgGraphStepC4( graph )           graph->StepC4 
#define GlgGraphOscillationC5( graph )    graph->OscillationC5 
#define GlgGraphSkewC6( graph )           graph->SkewC6 
#define GlgGraphRotationC7( graph )       graph->RotationC7 
#define GlgGraphGravitationC8( graph )    graph->GravitationC8 
#define GlgGraphMassC9( graph )           graph->MassC9 
#define GlgGraphRandConstant( graph )     graph->RandConstant 
#define GlgGraphConnectC10( graph )       graph->ConnectC10 
#define GlgGraphRandSpikeThreshold( graph )  graph->RandSpikeThreshold 
#define GlgGraphSpikeValue( graph )   graph->SpikeValue 
#define GlgGraphOffsetC11( graph )        graph->OffsetC11 
#define GlgGraphOffsetC12( graph )        graph->OffsetC12 
#define GlgGraphAttractionExtC13( graph ) graph->AttractionExtC13 
#define GlgGraphSingleRepulsionC14( graph ) graph->SingleRepulsionC14 
#define GlgGraphSingleGravityC15( graph ) graph->SingleGravityC15 

#define GlgGraphPalette( graph )      graph->palette
#define GlgGraphCenter( graph )       graph->center
#define GlgGraphNodeIcons( graph )    graph->node_icons
#define GlgGraphEdgeIcon( graph )     graph->edge_icon
#define GlgGraphViewportIcon( graph ) graph->viewport_icon
#define GlgGraphViewport( graph )     graph->viewport
#define GlgGraphGroup( graph )        graph->group
#define GlgGraphNodeArray( graph )    graph->node_array
#define GlgGraphEdgeArray( graph )    graph->edge_array
#define GlgGraphNodeGroup( graph )    graph->node_group
#define GlgGraphEdgeGroup( graph )    graph->edge_group
#define GlgGraphNodeCount( graph )    graph->node_count
#define GlgGraphEdgeCount( graph )    graph->edge_count
#define GlgGraphExtent( graph )       graph->extent
#define GlgGraphExtentSet( graph )    graph->extent_set
#define GlgGraphOldExtent( graph )    graph->old_extent
#define GlgGraphFixChains( graph )    graph->fix_chains
#define GlgGraphFixChains1( graph )   graph->fix_chains1
#define GlgGraphFixChains2( graph )   graph->fix_chains2
#define GlgGraphFinished( graph )     graph->finished
#define GlgGraphEnableSpikes( graph ) graph->enable_spikes
#define GlgGraphIteration( graph )    graph->iteration
#define GlgGraphUpdateRate( graph )   graph->update_rate
#define GlgGraphEndTemperature( graph ) graph->end_temperature
#define GlgGraphDimensions( graph )   graph->dimensions
#define GlgGraphRandomList( graph )   graph->random_list
#define GlgGraphRandomListResult( graph )  graph->random_list_result
#define GlgGraphRandomSize( graph )   graph->random_size
#define GlgGraphEdgesOnTop( graph )   graph->edges_on_top
#define GlgGraphZ( graph )            graph->z

#define GlgGraphNumNodes( graph ) \
  ( GlgGraphNodeArray( graph ) ? GlgGetSize( GlgGraphNodeArray( graph ) ) : 0 )

#define GlgGraphNumLinks( graph ) \
  ( GlgGraphEdgeArray( graph ) ? GlgGetSize( GlgGraphEdgeArray( graph ) ) : 0 )

#define GlgNodeNumLinks( node ) \
  ( GlgNodeLinkArray( node ) ? GlgGetSize( GlgNodeLinkArray( node ) ) : 0 )

struct GlgGraphLayoutStruct
{
   int MaxIteration;
   double SpikesEndTemp;
   double ChainFixTemp1;
   double ChainFixTemp2;
   double AttractionC1;
   double AttractionC2;
   double RepulsionC3;
   double StepC4;
   double OscillationC5;
   double SkewC6;
   double RotationC7;
   double GravitationC8;
   double MassC9;
   double RandConstant;
   double ConnectC10;
   double RandSpikeThreshold;
   double SpikeValue;
   double OffsetC11;
   double OffsetC12;
   double AttractionExtC13;
   double SingleRepulsionC14;
   double SingleGravityC15;

   GlgObject palette;
   GlgPoint center;
   GlgObject node_icons;
   GlgObject edge_icon;
   GlgObject viewport_icon;
   GlgObject viewport;
   GlgObject group;
   GlgObject node_group;
   GlgObject edge_group;
   GlgObject node_array;
   GlgObject edge_array;
   GlgLong node_count;
   GlgLong edge_count;
   GlgCube extent;
   GlgCube old_extent;
   char extent_set;
   char fix_chains;
   char fix_chains1;
   char fix_chains2;
   char finished;
   char enable_spikes;
   char edges_on_top;
   GlgLong iteration;
   GlgLong update_rate;
   double end_temperature;
   GlgCube dimensions;
   GlgLong * random_list;
   GlgGraphNode * random_list_result;
   GlgLong random_size;
   double z;
};

#define GlgNodeData( node )           node->data
#define GlgNodeType( node )           node->type
#define GlgNodeTemplate( node )       node->template
#define GlgNodeGraphics( node )       node->graphics
#define GlgNodePosition( node )       node->position
#define GlgNodeDisplayPosition( node ) node->display_position
#define GlgNodeImpulse( node )        node->impulse
#define GlgNodeOldImpulse( node )     node->old_impulse
#define GlgNodeTemperature( node )    node->temperature
#define GlgNodeSkew( node )           node->skew
#define GlgNodeLinkArray( node )      node->link_array
#define GlgNodeProbability( node )    node->probability
#define GlgNodeAveragePos( node )     node->average_pos
#define GlgNodeAnchor( node )         node->anchor
#define GlgNodeExternal( node )       ( GlgNodeType( node ) < 0 )
#define GlgNodeUsed( node )           node->used

struct GlgGraphNodeStruct
{
   void * data;
   int type;
   GlgObject template;
   GlgObject graphics;
   GlgPoint position;
   GlgPoint display_position;
   GlgPoint impulse;
   GlgPoint old_impulse;
   double temperature;
   double skew;
   GlgObject link_array;
   double probability;
   char anchor;
   char used;
};

#define GlgEdgeData( edge )         edge->data
#define GlgEdgeType( edge )         edge->type
#define GlgEdgeTemplate( edge )     edge->template
#define GlgEdgeGraphics( edge )     edge->graphics
#define GlgEdgeStartNode( edge )    edge->start_node
#define GlgEdgeEndNode( edge )      edge->end_node
#define GlgEdgeAttractionC1( edge ) edge->AttractionC1 
#define GlgEdgeAttractionC2( edge ) edge->AttractionC2 
#define GlgEdgeFlag( edge )         edge->flag
#define GlgEdgeExternal( node )     ( GlgEdgeType( node ) < 0 )

struct GlgGraphEdgeStruct
{
   void * data;
   int type;
   GlgObject template;
   GlgObject graphics;
   GlgGraphNode start_node;
   GlgGraphNode end_node;
   double AttractionC1;
   double AttractionC2;
   char flag;
};

typedef enum _GraphType
{
   RANDOM_GRAPH = 0,
   CIRCULAR_GRAPH,
   STAR_GRAPH
} GraphType;

extern GlgObject GlgGraphDefNodeIconsDef;
extern GlgObject GlgGraphDefEdgeIconDef;
extern GlgObject GlgGraphDefViewportIconDef;

#define GlgGraphDefNodeIcons()    GlgGraphDefNodeIconsDef
#define GlgGraphDefEdgeIcon()     GlgGraphDefEdgeIconDef
#define GlgGraphDefViewportIcon() GlgGraphDefViewportIconDef

GlgGraphLayout GlgGraphCreateRandom( GlgLong num_nodes, GlgLong num_node_types,
				    GraphType type );
GlgGraphLayout GlgGraphCreateFromData( GlgLong num_nodes, GlgLong node_type[], 
				      GlgLong num_connections,
				      GlgLong connections_list[][2] );
void GlgGraphSetDefPalette( GlgObject palette_drawing );
void GlgGraphUnloadDefPalette( void );
void GlgGraphTerminate( void );
GlgGraphLayout GlgGraphCreate( void );
void GlgGraphSetPalette( GlgGraphLayout graph, GlgObject palette_drawing );
void GlgGraphDestroy( GlgGraphLayout graph );
GlgBoolean GlgGraphCreateGraphics( GlgGraphLayout graph, GlgObject viewport,
				  GlgObject group );
void GlgGraphDestroyGraphics( GlgGraphLayout graph );
GlgObject GlgGraphGetViewport( GlgGraphLayout graph );
GlgObject GlgGraphGetGroup( GlgGraphLayout graph );
GlgBoolean GlgGraphSpringIterate( GlgGraphLayout graph );
void GlgGraphCircularLayout( GlgGraphLayout graph );
GlgBoolean GlgGraphGetUntangle( GlgGraphLayout graph );
void GlgGraphSetUntangle( GlgGraphLayout graph, GlgBoolean untangle );
void GlgGraphIncreaseTemperature( GlgGraphLayout graph, GlgBoolean init );
void GlgGraphScramble( GlgGraphLayout graph );
void GlgGraphUpdate( GlgGraphLayout graph );
void GlgGraphError( char * string );

GlgGraphNode GlgGraphAddNode( GlgGraphLayout graph, GlgObject graphics,
			     GlgLong node_type, void * data );
GlgGraphEdge GlgGraphAddEdge( GlgGraphLayout graph, 
			     GlgGraphNode start_node, GlgGraphNode end_node,
			     GlgObject graphics, GlgLong edge_type, 
			     void * data );
void GlgGraphDeleteNode( GlgGraphLayout graph, GlgGraphNode node );
void GlgGraphDeleteEdge( GlgGraphLayout graph, GlgGraphEdge edge );
GlgGraphNode GlgGraphFindNode( GlgGraphLayout graph, GlgObject node_graphics );
GlgGraphEdge GlgGraphFindEdge( GlgGraphLayout graph, GlgObject edge_graphics );
GlgBoolean GlgGraphNodesConnected( GlgGraphNode node1, GlgGraphNode node2 );
void GlgGraphGetNodePosition( GlgGraphLayout graph, GlgGraphNode node, 
			     double * x, double * y, double * z );
void GlgGraphSetNodePosition( GlgGraphLayout graph, GlgGraphNode node, 
			     double x, double y, double z );
GlgGraphNode GlgGraphGetNode( GlgGraphLayout graph, GlgLong i );
GlgGraphEdge GlgGraphGetEdge( GlgGraphLayout graph, GlgLong i );
GlgLong GlgGraphGetNodeIndex( GlgGraphLayout graph, GlgGraphNode node );
GlgLong GlgGraphGetEdgeIndex( GlgGraphLayout graph, GlgGraphEdge edge );


#ifdef __cplusplus
}
#endif

#endif   /* End of _Glg_Graph_layout_h */
