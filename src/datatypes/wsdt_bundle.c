#include "wsdt_bundle.h"
#include "datatypeloader.h"
#include "waterslidedata.h"

//fast init function.. just zero out the length, not the string
void wsdt_init_bundle(wsdata_t * wsdata, wsdatatype_t * dtype) {
     if (wsdata->data) {
          wsdt_bundle_t * ls = (wsdt_bundle_t*)wsdata->data;
          ls->len = 0;
     }
}

//copied directly from wsdatatype_default_delete func
static void wsdt_delete_bundle(wsdata_t * wsdata) {
     int tmp = wsdata_remove_reference(wsdata);

     // no more references.. we can move this data to free q
     if (tmp <= 0) {

          wsdt_bundle_t * my_bundle = (wsdt_bundle_t *)wsdata->data;
          
          int i;
          for (i=0; i < my_bundle->len; i++) {
               wsdata_delete(my_bundle->wsd[i]);
          }

          if (wsdata->dependency) {
               wsdata_t * parent;
               //remove references to parent
               while ((parent = wsstack_remove(wsdata->dependency)) != NULL) {
                    parent->dtype->delete_func(parent);
               }
          }
          //add to child free_q
          wsdata_moveto_freeq(wsdata);
          //fprintf(stderr,"waterslidedata: data dereferenced\n");
     }
}


int datatypeloader_init(void * type_list) {
     wsdatatype_t *fsdt = wsdatatype_register_generic(type_list,
                                                      WSDT_BUNDLE_STR,
                                                      sizeof(wsdt_bundle_t));
     fsdt->init_func = wsdt_init_bundle;
     fsdt->delete_func = wsdt_delete_bundle;    
     return 1;
}
