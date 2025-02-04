#include <stdlib.h>
#include <stdio.h>

#include "wsqueue.h"
#include "waterslide.h"
#include "waterslide_io.h"
#include "init.h"
#include "mimo.h"
#include "shared/mimo_shared.h"

extern uint32_t graph_has_cycle;
extern uint32_t deadlock_firehose_shutoff;

ws_outtype_t * ws_find_outtype(
        ws_outlist_t * olist,
        wsdatatype_t * dtype,
        wslabel_t * label)
{
    if (olist->outtype_q == NULL) {
        return NULL;
    }
    q_node_t * cursor = olist->outtype_q->head;
    ws_outtype_t * otype;
    for (cursor=olist->outtype_q->head;
         cursor!=NULL;
         cursor=cursor->next)
    {
        otype = (ws_outtype_t *)cursor->data; 
        if ( (otype->dtype == dtype) 
             && (otype->label == label))
        {
            return otype;
        }
    }
    return NULL;
}

ws_outtype_t * ws_add_outtype(
        ws_outlist_t * olist,
        wsdatatype_t * dtype,
        wslabel_t * label)
{
    // check if outtype is already specified    
    ws_outtype_t * otype;

    otype = ws_find_outtype(olist, dtype, label);
    if (otype)
        return otype;
    if (olist->outtype_q == NULL)
        olist->outtype_q = queue_init();
    otype = (ws_outtype_t*)calloc(1, sizeof(ws_outtype_t));
    if (!otype) {
        error_print("failed ws_add_outtype calloc of otype");
        return NULL;
    }
    otype->dtype = dtype;
    otype->label = label;
    queue_add(olist->outtype_q, otype);
    return otype; 
}

ws_outtype_t * ws_add_outtype_byname(
        void * type_list,
        ws_outlist_t * olist,
        const char * dname,
        const char * lname)
{
    wsdatatype_t * dtype = wsdatatype_get(type_list, dname);
    wslabel_t * label = wsregister_label(type_list, lname);
    if (!dtype) {
        error_print("trying to register unknown type %s", dname);
        return NULL;
    }
    return ws_add_outtype(olist, dtype, label);
}

int ws_check_subscribers(ws_outtype_t * otype) {
    // this function returns 1 if there is any subscriber 
    // to this outtype (regardless of whether input/output
    // data types match); returns 0 otherwise
    return ( (otype->local_subscribers != NULL)
             || (otype->ext_subscribers != NULL));
}

wsdata_t * ws_get_outdata(ws_outtype_t * otype) {
    wsdata_t * wsd = wsdata_alloc(otype->dtype); 
    if (!wsd) {
        return NULL;
    }
    wsdata_add_label(wsd, otype->label);
    return wsd;
}

int ws_set_outdata(
        wsdata_t * wsdata,
        ws_outtype_t * outtype,
        ws_doutput_t * doutput)
{
    ws_job_t * job;

// XXX: The #ifdef...#else...#endif below has duplicate code,
// so remember to modify both branches when making any
// applicable changes. Code here is duplicated for
// performance optimization reasons.

#ifdef WS_PTHREADS
    int has_subscriber = 0;
    if (outtype->local_subscribers) {
        has_subscriber = 1;
        job = queue_remove(
            (nhqueue_t*) doutput->local_job_freeq);
        if ( job == NULL ) {
            job = (ws_job_t*)malloc(sizeof(ws_job_t));
            if (!job) {
                error_print("failed ws_set_outdata malloc of job");
                return 0;
            }
        }
        job->data = wsdata;
        if (wsdata->references == 0)
            wsdata->references = 1;
        else
            wsdata_add_reference(wsdata);

        job->subscribers = outtype->local_subscribers;
        queue_add(doutput->local_jobq, job);
    }

    if (outtype->ext_subscribers) {
        has_subscriber = 1;
        ws_subscriber_t * scur;
        mimo_t * omimo = (mimo_t *)doutput->mimo;

        if (wsdata->references == 0)
            wsdata->references = 1;
        else
            wsdata_add_reference(wsdata);

        const int nrank = GETRANK();
        for (scur = outtype->ext_subscribers;
             scur != NULL;
             scur = scur->next)
        {
            if ( !scur->src_label
                 || wsdata_check_label(wsdata, scur->src_label))
            {
                wsdata_add_reference(wsdata);
                if ( !graph_has_cycle
                     || !mimo_thread_in_cycle(omimo, scur->proc_instance->thread_id))
                {
                    // there is no cycle in the graph OR the
                    // queue we are writing to does not
                    // belong to a cycle

                    // do not proceed until we enqueue
                    register uint32_t tid = scur->proc_instance->thread_id;
                    while( ! shared_queue_add(
                        doutput->shared_jobq_array[tid],
                        wsdata,
                        scur) );
                }
                else {
                    // we belong to a cycle in the graph...
                    // recover from any potential deadlock
                    int ret;
                    register uint32_t tid = scur->proc_instance->thread_id;
                    while(1) {
                        ret = shared_queue_add(
                            doutput->shared_jobq_array[tid],
                            wsdata,
                            scur);
                        if (ret) {
                            // successful in shared queue
                            // add/enqueu, so mark the 
                            // thread's fullshq as false
                            omimo->mgc->fullshq[tid] = 0;
                            break;
                        }
                        omimo->mgc->fullshq[tid] = 1;
                        if (mimo_cycle_deadlock_exist(omimo))
                        {
                            deadlock_firehose_shutoff = 1;
                            fqueue_add(
                                omimo->failoverq[nrank],
                                wsdata,
                                scur);
                            break;
                        }
                    }
                }
            }
        }
        wsdata_delete(wsdata);
    }

    if (!has_subscriber) {
        // at first, this may seem *foolish* because we are
        // performing a function and its inverse at the same
        // time. However, wsdata references can be 0 and not
        // be deleted (i.e., newly created wsdata).
        // This process ensures proper accounting of memory
        // both when there is no subscriber to a newly
        // created data (within a kid) or when the data is
        // being forwarded from a preceding kid in its
        // pipeline.
        wsdata_add_reference(wsdata);
        wsdata_delete(wsdata);
    }

#else // ! WS_PTHREADS
    job = queue_remove((nhqueue_t*)doutput->local_job_freeq);
    if ( job == NULL ) {
        job = (ws_job_t*)malloc(sizeof(ws_job_t));
        if (!job) {
            error_print("failed ws_set_outdata malloc of job");
            return 0;
        }
    }
    job->data = wsdata;
    wsdata_add_reference(wsdata);
    job->subscribers = outtype->local_subscribers;
    queue_add(doutput->local_jobq, job);
#endif // WS_PTHREADS

    return 1;
}

ws_outtype_t * ws_register_source(
        wsdatatype_t * dtype,
        proc_process_t pfunc,
        ws_sourcev_t * sv)
{
    // somehow create a list of sources
    ws_source_list_t * source = (ws_source_list_t*)calloc(
        1, sizeof(ws_source_list_t));
    if (!source) {
        error_print("failed ws_register_source calloc of source");
        return NULL;
    }
    source->outtype.dtype  = dtype;
    source->proc_func      = pfunc;
    source->pinstance      = sv->proc_instance;
    source->next           = sv->mimo->proc_sources;
    source->input_index    = sv->proc_instance->input_index;
    sv->mimo->proc_sources = source;
    sv->proc_instance->input_index++;

    if (!sv->mimo->verbose)
        return &source->outtype;
    // else
    fprintf(stderr, "registering source %s\n", 
           source->pinstance->name);
    return &source->outtype;
}

ws_outtype_t * ws_register_monitor_source(
        proc_process_t pfunc,
        ws_sourcev_t * sv)
{
    // somehow create a list of sources
    ws_source_list_t * source = (ws_source_list_t*)calloc(
        1, sizeof(ws_source_list_t));
    if (!source) {
        error_print("failed ws_register_monitor_source calloc of source");
        return NULL;
    }
    source->outtype.dtype   = dtype_monitor;
    source->proc_func       = pfunc;
    source->pinstance       = sv->proc_instance;
    source->next            = sv->mimo->proc_monitors;
    source->input_index     = sv->proc_instance->input_index;
    sv->mimo->proc_monitors = source;
    sv->proc_instance->input_index++;

    if (!sv->mimo->verbose)
        return &source->outtype;
    // else
    fprintf(stderr, "registering monitor %s\n", 
            source->pinstance->name);
    return &source->outtype;
}

ws_outtype_t * ws_register_source_byname(
        void * type_list,
        const char * name,
        proc_process_t pfunc,
        ws_sourcev_t * sv)
{
    // create a list of sources
    wsdatatype_t * dtype = wsdatatype_get(type_list, name);
    if (!dtype)
        return NULL;
    return ws_register_source(dtype, pfunc, sv);
}

