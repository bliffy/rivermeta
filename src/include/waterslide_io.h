#ifndef _WATERSLIDE_IO_H
#define _WATERSLIDE_IO_H

#include "waterslide.h"
#include "wsqueue.h"
#include "shared/shared_queue.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _ws_subscriber_t {
     proc_process_t proc_func;
     ws_proc_instance_t * proc_instance;
     void * local_instance;
     wslabel_t * port;   //name of input port
     wslabel_t * src_label;
     ws_doutput_t * doutput;
     int input_index;
     mimo_sink_t * sink; // used for external sink
     struct _ws_subscriber_t * next;
     //put thread stuff here... so that jobs can be queued
     //up in another thread..
     uint32_t thread_id;
} ws_subscriber_t;

struct _ws_sourcev_t {
     mimo_t * mimo;
     ws_proc_instance_t * proc_instance;
};

struct _ws_outtype_t {
     wsdatatype_t * dtype;
     wslabel_t * label;
     ws_subscriber_t * local_subscribers;
     ws_subscriber_t * ext_subscribers;
};

struct _ws_outlist_t {
     //list of output datatypes
     nhqueue_t * outtype_q; // of ws_outtype_t
};

typedef struct _ws_job_t {
     wsdata_t * data;
     ws_subscriber_t * subscribers;
} ws_job_t;

struct _ws_doutput_t {
     nhqueue_t * local_jobq;  // a thread's job q
     nhqueue_t * local_job_freeq;  /// linked with mimo's jobq

#ifdef WS_PTHREADS
     shared_queue_t ** shared_jobq_array;       // a kid's array of job q's (shared_queue_t). needs to be casted
     void * mimo;
#endif // WS_PTHREADS
};

typedef struct _ws_proctype_t {
     proc_process_t proc_func;
     wsdatatype_t * dtype;
     wslabel_t * port;
     wslabel_t * src_label;
     int input_index;
     struct _ws_proctype_t * next;
} ws_proctype_t;

struct _ws_input_t {
     ws_proctype_t * head;
};

//main processing queue..

typedef struct _ws_source_list_t {
     ws_outtype_t outtype;
     proc_process_t proc_func;
     int input_index;
     ws_proc_instance_t * pinstance;
     struct _ws_source_list_t * next;
} ws_source_list_t;

#ifdef __cplusplus
}
#endif

#endif // _WATERSLIDE_IO_H

