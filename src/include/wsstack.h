#ifndef _WSSTACK_H
#define _WSSTACK_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "error_print.h"

#ifdef __cplusplus
extern "C" {
#endif

/* stack implemented as single-linked list */
typedef struct _wsstack_node_t {
     void * data;
     struct _wsstack_node_t * next;
} wsstack_node_t;

typedef struct _wsstack_t {
     wsstack_node_t * head; /* records removed from here */
     wsstack_node_t * freeq;
     unsigned int size;
     unsigned int max;
} wsstack_t;

/* allocates & initializes stack memory */
static inline wsstack_t* wsstack_init(void)
{
     wsstack_t * s;
     if (!(s = (wsstack_t*)calloc(1, sizeof(wsstack_t))) ) {
          error_print("failed wsstack_init calloc of s");
          return NULL;
     }
     return s;
}

/* frees stack memory */
static inline void wsstack_destroy(wsstack_t * s)
{
     wsstack_node_t * cursor;
     wsstack_node_t * next;
     if(s == NULL) {
          return;
     }
     /* free all nodes */
     next = s->head;
     while (next != NULL) {
          cursor = next;
          next = next->next;
          free(cursor);
     }
     next = s->freeq;
     while (next != NULL) {
          cursor = next;
          next = next->next;
          free(cursor);
     }
     /* free stack */
     free(s);
}

/***** 
 * adds new node to the stack with pointer to data
 * allocates memory for this node
 * returns pointer to node or NULL on error
 *****/
static inline int wsstack_add(wsstack_t * s, void * data)
{
     if ( s==NULL || (s->max && s->size >= s->max ) )
          return 0;
     wsstack_node_t * new_node = NULL;
     if (s->freeq) {
          new_node = s->freeq;
          s->freeq = new_node->next;
     } else {
          /* allocate memory and initialize */
          new_node = (wsstack_node_t*)malloc(sizeof(wsstack_node_t));
          if (!new_node) {
               error_print("failed wsstack_add malloc of new_node");
               return 0;
          }
     }
     new_node->data = data;
     /* maintain stack */
     s->size++;
     new_node->next = s->head;
     s->head = new_node;
     return 1;
}

/***** 
 * removes node from the stack
 * frees memory allocated for this node
 * returns pointer to node data or NULL on error
 *****/
static inline void* wsstack_remove(wsstack_t * s)
{
     void * retval  = NULL;
     wsstack_node_t * rm_node = NULL;

     if ( s==NULL || s->head==NULL )
          return NULL;
     /* check for head == NULL? */
     retval = s->head->data;
     /* maintain stack */
     s->size--;

     rm_node = s->head;
     s->head = s->head->next;

     rm_node->next = s->freeq;
     s->freeq = rm_node;

     return retval;
}

static inline unsigned int wsstack_size(wsstack_t *s)
{ return ( s ? s->size : 0 ); }

static inline int wsstack_clear(wsstack_t *s)
{
     wsstack_node_t * qnode;
     wsstack_node_t * this_node;

     if ( s == NULL )
          return 0;

     /* place all nodes in freeq */
     qnode = s->head;
     int cnt = 0;
     while( qnode ) {
          this_node = qnode;	
          qnode = qnode->next;
          this_node->next = s->freeq;
          s->freeq = this_node;
          cnt++;
     }

     s->size = 0;
     s->head = NULL;
     return cnt;
}

#ifdef __cplusplus
}
#endif

#endif // _WSSTACK_H

