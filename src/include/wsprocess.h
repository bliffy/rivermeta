#ifndef _WSPROCESS_H
#define _WSPROCESS_H

#ifdef __cplusplus
extern "C" {
#endif

// Macros for noop serial functions
#ifdef WS_PTHREADS
#define WS_DO_EXTERNAL_JOBS(mimo,shared_jobq) \
     ws_do_external_jobs(mimo,shared_jobq)
#else // !WS_PTHREADS
#define WS_DO_EXTERNAL_JOBS(mimo,shared_jobq) 0
#endif // WS_PTHREADS

//functions defined in wsprocess.c and invoked in mimo.c
int ws_execute_graph(mimo_t *);
int ws_execute_exiting_graph(mimo_t *);
int ws_add_local_job_source(
     mimo_t *,
     wsdata_t *,
     ws_subscriber_t *);
void ws_destroy_graph(mimo_t *);

#ifdef __cplusplus
}
#endif

#endif // _WSPROCESS_H
