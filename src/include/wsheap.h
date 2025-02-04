#ifndef _WSHEAP_H
#define _WSHEAP_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "error_print.h"
#include "dprint.h"

#ifdef __cplusplus
extern "C" {
#endif

//return -1 if less than, 0 if equal, 1 if greater than
typedef int (*wsheap_compair)(void * first, void * second);
typedef void (*wsheap_replace)(void * olddata, void * newdata, void * aux);

/* stack implemented as single-linked list */
typedef struct _wsheap_t
{
     uint64_t max;
     uint64_t count;
     size_t data_size;
     void * buffer;
     void ** heap;
     wsheap_compair cmp;
     wsheap_replace replace;
     void * aux;
} wsheap_t;

static inline void wsheap_swap(void ** first, void ** second) {
     void * tmp = *first;
     *first = *second;
     *second = tmp;
}


/* allocates & initializes heap */
static inline wsheap_t * wsheap_init(
          uint64_t max,
          size_t data_size,
          wsheap_compair cmp,
          wsheap_replace replace,
          void * aux)
{
     if (!cmp || !replace) {
          error_print("failed wsheap_init - callback");
          return NULL;
     }
     wsheap_t * h;

     h = (wsheap_t*) calloc(1, sizeof(wsheap_t));
     if (!h) {
          error_print("failed wsheap_init calloc");
          return NULL;
     }
     h->data_size = data_size;
     h->max = max;
     h->cmp = cmp;
     h->replace = replace;
     h->aux = aux;
     h->buffer = calloc(max, data_size);
     if (!h->buffer) {
          error_print("failed wsheap_init calloc buffer");
          return NULL;
     }
     h->heap = (void **)calloc(max, sizeof(void*));
     if (!h->heap) {
          error_print("failed wsheap_init calloc heap pointers");
          return NULL;
     }
     //point heap at buffers
     for (uint64_t i = 0; i < max; i++) {
          h->heap[i] = h->buffer + (i*data_size);
     }

     return h;
}

static inline void wsheap_reset(wsheap_t * h)
{
     memset(h->buffer, 0, h->max * h->data_size); 
     for (uint64_t i = 0; i < h->max; i++) {
          h->heap[i] = h->buffer + (i*h->data_size);
     }
     h->count = 0;
}


/* frees stack memory */
static inline void wsheap_destroy(wsheap_t * h)
{
     if (h->buffer)
          free(h->buffer);
     if (h->heap)
          free(h->heap);
     free(h);
}

//maintenace function
static inline void wsheap_promote(
          wsheap_t * h,
          uint64_t pos)
{
     //assert(pos < h->count);
     uint64_t child = pos;
     uint64_t parent;
     while (child > 0) {
          parent = ((child + 1) / 2) - 1;
          if (h->cmp(h->heap[child], h->heap[parent]) < 0) {
               wsheap_swap(&h->heap[child], &h->heap[parent]);
               child = parent;
          }
          else
               break;
     } 
}

//maintenace function
static inline void wsheap_demote(wsheap_t *h, uint64_t pos) {
     //assert(pos < h->count);
     uint64_t parent = pos;
     uint64_t child2;
     uint64_t child1;
     uint64_t min_child;
     while (1) {
          child2 = (parent + 1) * 2;
          child1 = child2 - 1;
          min_child = child1;

          if (child1 >= h->count)
               break;

          if ((child2 < h->count) && (h->cmp(h->heap[child1], h->heap[child2]) > 0)) {
               min_child = child2;
          }
          
          if (h->cmp(h->heap[min_child],
                     h->heap[parent]) >= 0)
          {
               break;
          }
          wsheap_swap(&h->heap[min_child], &h->heap[parent]);
          parent = min_child;
     }
}


/***** 
 * returns pointer to node or NULL on error
 *****/
static inline void * wsheap_insert(wsheap_t * h, void * data)
{
     void * rtn_data = NULL;
     if (h->count < h->max) {
          h->replace(h->heap[h->count], data, h->aux);
          uint64_t pos = h->count;
          rtn_data = h->heap[pos];
          h->count++;
          wsheap_promote(h, pos);
     }
     return rtn_data; 
}

static inline void * wsheap_update(
          wsheap_t * h,
          void * data)
{
     uint64_t pos = (data - h->buffer)/h->data_size;
     if (pos >= h->count) {
          return NULL;
     }
     wsheap_demote(h, pos);
     return data;
}


static inline void * wsheap_replace_root(
          wsheap_t * h,
          void * data)
{
     void * rtn_data = NULL;
     if (h->count) {
          h->replace(h->heap[0], data, h->aux);
          rtn_data = h->heap[0];
          wsheap_demote(h, 0);
     }
     return rtn_data;
}

static inline void * wsheap_insert_replace(
          wsheap_t * h,
          void * data)
{
     void * rtn = wsheap_insert(h, data);
     if (rtn) return rtn;
     return wsheap_replace_root(h, data);
}

//destroys heap property
static inline void wsheap_sort_inplace(wsheap_t * h) {
     uint64_t total = h->count;
     while (h->count > 1) {
        h->count--;
        wsheap_swap(&h->heap[0], &h->heap[h->count]);
        wsheap_demote(h, 0);
     }
     h->count = total;
}

#ifdef __cplusplus
}
#endif

#endif // _WSSTACK_H
