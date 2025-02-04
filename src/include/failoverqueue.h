#ifndef _FAILOVER_QUEUE_H
#define _FAILOVER_QUEUE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "error_print.h"

#ifdef __cplusplus
extern "C" {
#endif

/* queue implemented as doubly-linked list with two data per node (one data could be NULL, if desired)*/
typedef struct _fq_node_t
{
     void*              data1;
     void*              data2;
     struct _fq_node_t* prev;
     struct _fq_node_t* next;
} fq_node_t;

typedef struct _failoverqueue_t
{
     fq_node_t*    head;  /* records removed from here */
     fq_node_t*    tail;  /* records added here */
     fq_node_t*    freeq;
     unsigned int  size;
} failoverqueue_t;


/* allocates & initializes queue memory */
static inline failoverqueue_t* fqueue_init(void)
{
     failoverqueue_t *new_queue;

     new_queue = (failoverqueue_t*) calloc(1, sizeof(failoverqueue_t));
     if (!new_queue) {
          error_print("failed fqueue_init calloc of new_queue");
          return NULL;
     }

     return new_queue;
}

/* frees queue memory */
static inline void fqueue_exit(failoverqueue_t *qp)
{
     fq_node_t* qnode;
     fq_node_t* this_node;

     if(qp == NULL) {
          return;
     }

     /* free all freeq nodes */
     qnode = qp->freeq;
     while(qnode != NULL) {
          this_node = qnode;	
          qnode = qnode->next;
          free(this_node);
     }

     /* free queue */
     free(qp);
}


/***** 
 * adds new node to the queue with pointer to data
 * allocates memory for this node
 * returns pointer to node or NULL on error
 *****/
static inline fq_node_t* fqueue_add(
          failoverqueue_t* qp,
          void* data1,
          void* data2)
{
     fq_node_t* new_node = NULL;

     if (qp == NULL) {
          return new_node;
     }

     if (qp->freeq) {
          new_node = qp->freeq;
          qp->freeq = new_node->next;
          memset(new_node, 0, sizeof(fq_node_t));
     }
     else {
          /* allocate memory and initialize */
          new_node = (fq_node_t*)calloc(1, sizeof(fq_node_t));
          if (!new_node) {
               error_print("failed fqueue_add calloc of new_node");
               return NULL;
          }
     }
     new_node->data1 = data1;
     new_node->data2 = data2;

     /* maintain queue */
     qp->size++;

     if(qp->head == NULL) {
          qp->head = new_node;
          qp->tail = new_node;
     }
     else {
          qp->tail->next = new_node;
          new_node->prev = qp->tail;
          qp->tail = new_node;
     }

     return new_node;
}


/***** 
 * adds new node to the queue with pointer to data; however, this node is added in 
 * a stack fashion (added to the head not tail)
 * allocates memory for this node
 * returns pointer to node or NULL on error
 *****/
static inline fq_node_t* fqueue_add_front(
          failoverqueue_t* qp,
          void* data1,
          void *data2)
{
     fq_node_t* new_node = NULL;

     if(qp == NULL) {
          return new_node;
     }

     if (qp->freeq) {
          new_node = qp->freeq;
          qp->freeq = new_node->next;
          memset(new_node, 0, sizeof(fq_node_t));
     }
     else {
          /* allocate memory and initialize */
          new_node = (fq_node_t*)calloc(1, sizeof(fq_node_t));
          if (!new_node) {
               error_print("failed fqueue_add_front calloc of new_node");
               return NULL;
          }
     }
     new_node->data1 = data1;
     new_node->data2 = data2;

     /* maintain queue */
     qp->size++;

     if (qp->head == NULL) {
          qp->head = new_node;
          qp->tail = new_node;
     }
     else {
          qp->head->prev = new_node;
          new_node->next = qp->head;
          qp->head = new_node;
     }

     return new_node;
}

// GCC pragma diagnostics only work for gcc 4.2 and later
#define GCC_VERSION (  __GNUC__       * 10000  \
                       + __GNUC_MINOR__ *   100   \
                       + __GNUC_PATCHLEVEL__)
#if GCC_VERSION >= 40200
#pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif
#undef GCC_VERSION
/***** 
 * removes node from the queue and saves content in data1 and data2
 * frees memory allocated for this node
 * returns 1 on success or 0 on error
 *****/
static inline int fqueue_remove(
          failoverqueue_t* qp,
          void** data1,
          void** data2)
{
     fq_node_t* rm_node = NULL;

     if((qp == NULL) || (qp->head == NULL)) {
          return 0;
     }

     /* check for head == NULL? */
     *data1 = qp->head->data1;
     *data2 = qp->head->data2;

     /* maintain queue */
     qp->size--;

     rm_node = qp->head;
     qp->head = qp->head->next;

     if (qp->head != NULL) {
          qp->head->prev = NULL;
     }
     else {
          qp->tail = NULL;
     }

     /* free memory to list of freeq available*/
     rm_node->next  = qp->freeq;
     rm_node->data1 = NULL;
     rm_node->data2 = NULL;
     qp->freeq = rm_node;

     return ((NULL != *data1) || (NULL != *data2));
}

static inline unsigned int fqueue_size(failoverqueue_t* qp)
{
     if (NULL == qp) {
          return 0;
     }
     return qp->size;
}


static inline int fqueue_clear(failoverqueue_t* qp)
{
     fq_node_t* qnode;
     fq_node_t* this_node;

     if (qp == NULL) {
          return -1;
     }

     /* place all nodes in freeq */
     qnode = qp->head;
     while(qnode != NULL) {
          this_node = qnode;	
          qnode = qnode->next;
          this_node->next = qp->freeq;
          qp->freeq = this_node;
     }

     qp->size = 0;
     qp->head = NULL;
     qp->tail = NULL;

     return 0;
}

#ifdef __cplusplus
}
#endif

#endif // _FAILOVER_QUEUE_H
