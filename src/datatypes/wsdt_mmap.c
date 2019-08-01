
#include <stdint.h>
#include <stddef.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "waterslidedata.h"
#include "wsdt_mmap.h"
#include "datatypeloader.h"
#include "sysutil.h"

#if !(defined _WIN32 || defined _WIN64 || defined WINDOWS)
#include <sys/mman.h>
#else
#include "win_mman.h"
#endif

ws_hashloc_t * wsdt_mmap_hash(wsdata_t*);

static int wsdt_print_mmap_wsdata(FILE * stream, wsdata_t * wsdata,
                                    uint32_t printtype) {
     wsdt_mmap_t * mm = (wsdt_mmap_t*)wsdata->data;
     if (!mm->buf) {
          return 0;
     }
     int rtn = 0;
     switch (printtype) {
     case WS_PRINTTYPE_HTML:
          fprintf(stream,"\n");
          sysutil_print_content_web(stream, mm->buf, mm->len); 
          return 1;
          break;
     case WS_PRINTTYPE_TEXT:
          fprintf(stream,"\n");
          sysutil_print_content(stream, mm->buf, mm->len); 
          return 1;
          break;
     case WS_PRINTTYPE_BINARY:
          rtn = fwrite(&mm->len, sizeof(int), 1, stream);
          rtn += fwrite(mm->buf, mm->len, 1, stream);
          return rtn;
          break;
     default:
          return 0;
     }
}

static void wsdt_cleanup_wsdata(wsdata_t * wsdata)
{
     wsdt_mmap_t * mm = (wsdt_mmap_t*)wsdata->data;
     if (mm) {
          if (mm->buf && (mm->srcfd >= 0)) {
               //unmmap the file
               munmap((void *)mm->buf, mm->len);
               close(mm->srcfd);
          }
          // Set to invalid
          mm->len = 0;
          mm->buf = NULL;
          mm->srcfd = -1;
          mm->filename[0] = 0x00;
     }
}

//close the old mmap before re-using..
void wsdt_init_mmap(wsdata_t * wsdata, wsdatatype_t * dtype) {
     // quick init and resets if necessary
     wsdt_cleanup_wsdata(wsdata);
}


static int wsdt_to_string_mmap(
          wsdata_t * wsdata,
          const char ** buf,
          size_t * len)
{
     wsdt_mmap_t *str = (wsdt_mmap_t*)wsdata->data;

     *buf = str->buf; 
     *len = str->len;
     return 1;           
}

int datatypeloader_init(void * tl) {
     wsdatatype_t * bdt = wsdatatype_register(tl,
                                              WSDT_MMAP_STR, sizeof(wsdt_mmap_t),
                                              wsdt_mmap_hash,
                                              wsdt_init_mmap,
                                              wsdatatype_default_delete,
                                              wsdt_print_mmap_wsdata,
                                              wsdatatype_default_snprint,
                                              wsdatatype_default_copy,
                                              wsdatatype_default_serialize);

     bdt->to_string = wsdt_to_string_mmap;

     wsdatatype_register_alias(tl, bdt, "MMAP_TYPE");

     wsdatatype_register_subelement(bdt, tl,
                                    "LENGTH", "INT_TYPE",
                                    offsetof(wsdt_mmap_t, len));

     return 1;
}

ws_hashloc_t* wsdt_mmap_hash(wsdata_t * wsdata) {
     if (!wsdata->has_hashloc) {
          wsdt_mmap_t *strd = (wsdt_mmap_t*)wsdata->data;
          wsdata->hashloc.offset = strd->buf; 
          wsdata->hashloc.len = strd->len;
          wsdata->has_hashloc = 1;
     }
     return &wsdata->hashloc;
}

