/*
   This is an approximate counting algorithm

   This work is based on the Space Saving Algorithm in
   "Efficient Computation of Frequent and Top-k Elements in Data Streams"
   by Ahmed Metwally, Divyakant Agrawal, and Amr El Abbadi

   and informed by analysis in:
   "Methods for Finding Frequent Items in Data Streams"
   by Graham Cormode, Marios Hadjieleftheriou
*/

#ifndef _HEAVYHITTERS_H
#define _HEAVYHITTERS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "error_print.h"
#include "dprint.h"
#include "evahash64.h"
#include "wsheap.h"

#ifdef __cplusplus
extern "C" {
#endif

/* main structures are:
   1. priority queue implemented as doubly-linked list stack
   2. hashtable-list that is accessed sorted

   probably better implemented as a min-heap for priority queue and sorting upon
   output
 */
typedef struct _hh_node_t
{
     uint64_t           key;
     uint64_t           value;
     uint64_t           starting_value;
     struct _hh_node_t* ht_prev;
     struct _hh_node_t* ht_next;
     uint8_t            data[];
} hh_node_t;

static int hh_node_compair(void* vfirst, void* vsecond) {
     hh_node_t* first = (hh_node_t*)vfirst;
     hh_node_t* second = (hh_node_t*)vsecond;

     if (first->value < second->value) {
          return -1;
     }
     else if (first->value == second->value) {
          return 0;
     }
     else {
          return 1;
     }
}

static void hh_node_replace(void* vnode, void* vreplace, void* aux);
typedef void (*heavyhitters_release)(void* /*data*/);

typedef struct _heavyhitters_t
{
     uint64_t     max;
     size_t       data_size;
     hh_node_t**  hashtable;
     wsheap_t*    heap;
     heavyhitters_release release;
     uint32_t     seed;
} heavyhitters_t;


/* allocates & initializes queue memory */
static inline heavyhitters_t* heavyhitters_init(
          uint64_t max,
          size_t data_size,
          heavyhitters_release release)
{
     heavyhitters_t* hh = (heavyhitters_t*)calloc(1, sizeof(heavyhitters_t));
     hh->max = max;
     hh->data_size = data_size;
     
     hh->heap = wsheap_init(
          max, sizeof(hh_node_t) + data_size,
          hh_node_compair,
          hh_node_replace,
          hh);
     if (!hh->heap) {
          free(hh);
          error_print("unable to allocate heavy hitters heap");
          return NULL;
     }
     hh->hashtable = (hh_node_t**)calloc(max, sizeof(hh_node_t*));
     if (!hh->hashtable) {
          wsheap_destroy(hh->heap);
          free(hh);
          error_print("unable to allocate heavy hitters hashtable");
          return NULL;
     }

     hh->release = release;
     hh->seed = rand();

     return hh;
}

static inline void hh_release_all(heavyhitters_t* hh) {
     uint64_t count = hh->heap->count;
     if (!count) {
          return;
     }
     void** list = hh->heap->heap;
     if (!list) {
          return;
     }
     //do callback to release old data
     uint64_t i;
     for (i = 0; i < count; i++) {
          hh_node_t * node = (hh_node_t*)list[i];
          hh->release(node->data);
     }
     return;
}

//the following sort destroys heavyhitters table tracking..
//must call reset or destroy after done with sorted list
static inline hh_node_t** heavyhitters_sort(
          heavyhitters_t* hh,
          uint64_t max,
          uint64_t* count)
{
     wsheap_sort_inplace(hh->heap);
     if (!count) {
          error_print("must have counter for sort");
          return NULL;
     }
     if (max > hh->heap->count) {
          *count = hh->heap->count;
     }
     else {
          *count = max;
     }
     return (hh_node_t**)hh->heap->heap;
}

// reset data structures back to empty
static inline int heavyhitters_reset(heavyhitters_t* hh) {
     if (!hh) {
          return 0;
     }
     hh_release_all(hh);
     wsheap_reset(hh->heap);

     if (hh->hashtable) {
          memset(hh->hashtable, 0, sizeof(hh_node_t*) * hh->max);
     }

     return 1;
}

// destroy all data structures
static inline int heavyhitters_destroy(heavyhitters_t* hh) {
     if (!hh) {
          return 0;
     }
     hh_release_all(hh);
     wsheap_destroy(hh->heap);
     
     if (hh->hashtable) {
          free(hh->hashtable);
     }

     free(hh);

     return 1;
}

static inline void hh_hashtable_detach(
          heavyhitters_t* hh,
          hh_node_t* node)
{
     uint64_t hindex = node->key % hh->max;

     if (hh->hashtable[hindex] == node) {
          hh->hashtable[hindex] = node->ht_next;
     }
     if (node->ht_prev) {
          node->ht_prev->ht_next = node->ht_next;
     }
     if (node->ht_next) {
          node->ht_next->ht_prev = node->ht_prev;
     }

     node->ht_prev = NULL;
     node->ht_next = NULL;
}

static inline void hh_hashtable_attach(
          heavyhitters_t* hh,
          hh_node_t* node,
          uint64_t hindex)
{
     if (!hh->hashtable[hindex]) {
          hh->hashtable[hindex] = node;
     }
     else {
          hh->hashtable[hindex]->ht_prev = node;
          node->ht_next = hh->hashtable[hindex];
          hh->hashtable[hindex] = node;
     }
}

//called to replace existing with new value/data
static void hh_node_replace(
          void* vnode,
          void* vreplace,
          void* vhh)
{
     hh_node_t* node = (hh_node_t*)vnode;
     hh_node_t* replace = (hh_node_t*)vreplace;
     heavyhitters_t* hh = (heavyhitters_t*)vhh;

     //if not empty...
     //remove from hashtable
     if (node->value) {
          if (hh->release) {
               hh->release((void *)node->data);
          }
          //reset data buffer
          memset(node->data, 0, sizeof(hh->data_size));

          hh_hashtable_detach(hh, node);

          //set old value
          node->starting_value = node->value;
     }
           
     node->key = replace->key;
     node->value += replace->value;
     hh_hashtable_attach(hh, node, replace->key % hh->max);
}


//returns data at increment position
static inline hh_node_t* heavyhitters_increment(
          heavyhitters_t* hh,
          const void* key_data,
          size_t key_length,
          uint64_t value) {
     //hash key data
     uint64_t key = evahash64((uint8_t*)key_data, key_length, hh->seed);

     //look up key in hashtable
     uint64_t hindex = key % hh->max;
     hh_node_t * cursor = hh->hashtable[hindex];
     for (;cursor; cursor = cursor->ht_next) {
          if (cursor->key == key) {
               //found key
               cursor->value += value;
               return (hh_node_t*)wsheap_update(hh->heap, cursor);
          }
     }

     hh_node_t node;
     node.value = value;
     node.key = key;

     return (hh_node_t*)wsheap_insert_replace(hh->heap, &node);
}

#ifdef __cplusplus
}
#endif

#endif // _HEAVYHITTERS_H
