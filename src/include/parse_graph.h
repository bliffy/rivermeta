#ifndef _PARSE_GRAPH_H
#define _PARSE_GRAPH_H

#include <stdint.h>
#include "waterslide.h"
#include "waterslide_io.h"
#include "waterslidedata.h"
#include "init.h"
#include "mimo.h"
#include "wsqueue.h"
#include "listhash.h"

#ifdef __cplusplus
extern "C" {
#endif

//name of an instance to parse
typedef struct _parse_node_proc_t {
     const char*  name;
     uint32_t  version;
     uint32_t  argc;
     char** argv;
     uint64_t num_calls;
     int     rank;
     double  time;
     double  pct;
     ws_proc_instance_t* pinst;
     int     thread_context;
     int     mimo_source : 1; /// is this a mimo src
     int     mimo_sink : 1; /// is this a mimo sink
} parse_node_proc_t;

typedef struct _parse_edge_t {
     uint32_t edgetype;
     void* src;
     void* dst;
     const char* port; //aka port name -- used for proc dst
     const char* src_label;
     int thread_trans;
     int thread_context;
     int twoD_placement;
} parse_edge_t;

// could be name only, name+label, transform only
//  name + transform, name + label + transform
typedef struct _parse_node_var_t {
     const char* name;
     nhqueue_t*  dst; // of type parse_edge
} parse_node_var_t;

#define PARSE_NODE_TYPE_PROC 0x0
#define PARSE_NODE_TYPE_VAR 0x1

#define PARSE_EDGE_TYPE_PROCPROC ((PARSE_NODE_TYPE_PROC << 4) | PARSE_NODE_TYPE_PROC)
#define PARSE_EDGE_TYPE_PROCVAR  ((PARSE_NODE_TYPE_PROC << 4) | PARSE_NODE_TYPE_VAR)
#define PARSE_EDGE_TYPE_VARPROC  ((PARSE_NODE_TYPE_VAR  << 4) | PARSE_NODE_TYPE_PROC)
#define PARSE_EDGE_TYPE_VARVAR   ((PARSE_NODE_TYPE_VAR  << 4) | PARSE_NODE_TYPE_VAR)


#define PARSE_MAX_ARGS 500
#define PARSE_GRAPH_MAXLIST 5000


struct _parse_graph_t {
     nhqueue_t*    edges; // of type parse_edge
     listhash_t*   vars;
     listhash_t*   procs;
     uint32_t  state;
     int       thread_context; 
     uint8_t   twoD_placement; // indicates whether %thread(x,y) or %thread(x) used; used in conjuction with thread_context
     double    max_pct;
     uint32_t  max_num_calls;
     uint64_t  total_calls;
     int       error;
     char*     port;
     char*     src_label;
     mimo_t*   mimo;
};

void wsprint_graph_dot(parse_graph_t* pg, FILE*);

#ifdef __cplusplus
}
#endif

#endif // _PARSE_GRAPH_H
