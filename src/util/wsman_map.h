#ifndef _WSMAN_MAP_H
#define _WSMAN_MAP_H

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "wsman_util.h"


typedef struct _map_t {
     char * key;
     char * val;
     struct _map_t *next;
} map_t;


map_t * map_create();
void    map_destroy(map_t *map);
const char *  map_find(const map_t * map, const char *key);
int     map_insert(map_t * map, char * key, char * val);
void    map_print(map_t *map);


map_t * map_create() {
     map_t * map = (map_t *) malloc(sizeof(map_t));
     if (!map) return NULL;

     map->key  = NULL;
     map->val  = NULL;
     map->next = NULL;

     return map;
}

void map_destroy(map_t *map) {
     if (!map) return;

     map_t * tmp1 = map;
     map_t * tmp2;

     while (tmp1 != NULL) {
          tmp2 = tmp1;
          tmp1 = tmp1->next;
          free(tmp2->key);
          free(tmp2->val);
          free(tmp2);

     }
}

const char * map_find(const map_t * map, const char * key) {
     const map_t * tmp = map;
     while(tmp != NULL && tmp->key) {
          if (strcmp(tmp->key, key) == 0) {
               return tmp->val;
          }
          tmp = tmp->next;
     }

     return NULL;
}

int map_insert(map_t * map, char * key, char * val) {
     if (!map) return 1;

     // is this map empty?
     if (map->key == NULL) {
          // insert the key/val pair
          map->key = strdup(key);
          map->val = strdup(val);

          if (map->key && map->val) {
               // success
               return 0;
          } else {
               // failure in strdup
               return 1;
          }
     }

     map_t * tmp = map;

     while (tmp->next != NULL) {
          tmp = tmp->next;
     }

     map_t * new_map = (map_t *) malloc(sizeof(map_t));
     if (!new_map) return 1;

     new_map->key = strdup(key);
     new_map->val = strdup(val);

     new_map->next = NULL;
     tmp->next = new_map;

     if (new_map->key && new_map->val) {
          // success
          return 0;
     } else {
          // failure in strdup
          return 1;
     }
}

void map_print(map_t * map) {
     if (!map) return;

     map_t * tmp = map;
     while(tmp != NULL) {
          printf("%p, key: %s, value: %s, next: %p\n",
               tmp, tmp->key, tmp->val, tmp->next);
          tmp = tmp->next;
     }
}

#endif // _WSMAN_MAP_H
