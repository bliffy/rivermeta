#ifndef _WSTYPES_H
#define _WSTYPES_H

#include <unistd.h>
#include <errno.h>

#include "waterslide.h"
#include "waterslidedata.h"
#include "timeparse.h"

#include "datatypes/wsdt_string.h"
#include "datatypes/wsdt_binary.h"
#include "datatypes/wsdt_fixedstring.h"
#include "datatypes/wsdt_massivestring.h"
#include "datatypes/wsdt_hugeblock.h"
#include "datatypes/wsdt_bigstring.h"
#include "datatypes/wsdt_mediumstring.h"
#include "datatypes/wsdt_smallstring.h"
#include "datatypes/wsdt_tinystring.h"
#include "datatypes/wsdt_flush.h"
#include "datatypes/wsdt_mmap.h"
#include "datatypes/wsdt_uint.h"
#include "datatypes/wsdt_uint64.h"
#include "datatypes/wsdt_uint16.h"
#include "datatypes/wsdt_uint8.h"
#include "datatypes/wsdt_int.h"
#include "datatypes/wsdt_int64.h"
#include "datatypes/wsdt_double.h"
#include "datatypes/wsdt_ts.h"
#include "datatypes/wsdt_monitor.h"
#include "datatypes/wsdt_vector_double.h"
#include "datatypes/wsdt_vector_uint32.h"

#ifndef _WSUTIL
#define EXT extern
#else
#define EXT
#endif

// Globals
EXT wsdatatype_t * dtype_uint;
EXT wsdatatype_t * dtype_uint8;
EXT wsdatatype_t * dtype_uint16;
EXT wsdatatype_t * dtype_uint64;
EXT wsdatatype_t * dtype_double;
EXT wsdatatype_t * dtype_int;
EXT wsdatatype_t * dtype_int64;
EXT wsdatatype_t * dtype_string;
EXT wsdatatype_t * dtype_str;
EXT wsdatatype_t * dtype_binary;
EXT wsdatatype_t * dtype_fstr;
EXT wsdatatype_t * dtype_fixedstring;
EXT wsdatatype_t * dtype_bigstr;
EXT wsdatatype_t * dtype_bigstring;
EXT wsdatatype_t * dtype_mediumstring;
EXT wsdatatype_t * dtype_massivestring;
EXT wsdatatype_t * dtype_hugeblock;
EXT wsdatatype_t * dtype_ts;
EXT wsdatatype_t * dtype_tuple;
EXT wsdatatype_t * dtype_monitor;
EXT wsdatatype_t * dtype_labelset;
EXT wsdatatype_t * dtype_label;
EXT wsdatatype_t * dtype_mmap;
EXT wsdatatype_t * dtype_flush;
EXT wsdatatype_t * dtype_array_uint;
EXT wsdatatype_t * dtype_array_double;
EXT wsdatatype_t * dtype_smallstring;
EXT wsdatatype_t * dtype_tinystring;
EXT wsdatatype_t * dtype_vector_double;
EXT wsdatatype_t * dtype_vector_uint32;

// get a string buffer from a data type
static inline int dtype_string_buffer(
          const wsdata_t * member,
          const char ** buf,
          int * len)
{ return member->dtype->to_string(member, buf, len); }

static inline int dtype_get_uint(
          const wsdata_t * member,
          uint64_t * o64)
{ return member->dtype->to_uint64(member, o64); }

static inline int dtype_get_uint64(
          const wsdata_t * member,
          uint64_t * o64)
{ return member->dtype->to_uint64(member, o64); }

static inline int dtype_get_uint32(
          const wsdata_t * member,
          uint32_t * o32)
{ return member->dtype->to_uint32(member, o32); }

static inline int dtype_get_int(
          const wsdata_t * member,
          int64_t * i64)
{ return member->dtype->to_int64(member, i64); }

static inline int dtype_get_int64(
          const wsdata_t * member,
          int64_t * i64)
{ return member->dtype->to_int64(member, i64); }

static inline int dtype_get_int32(
          const wsdata_t * member,
          int32_t * i32)
{ return member->dtype->to_int32(member, i32); }

static inline int dtype_get_double(
          const wsdata_t * member,
          double * dbl)
{ return member->dtype->to_double(member, dbl); }

static inline wsdata_t * wsdata_create_hugebuffer(
          uint64_t,
          char **,
          uint64_t *);

// create a wsdata type with a buffer big enough to hold
// the specified length
static inline wsdata_t * wsdata_create_buffer(
          int len,
          char ** pbuf,
          int * plen)
{
     wsdata_t * dep = NULL;
     if (len <= WSDT_TINYSTRING_LEN)
     {
          if ((dep = wsdata_alloc(dtype_tinystring))) {
               wsdt_tinystring_t * tstr =
                    (wsdt_tinystring_t*)dep->data;
               *plen = len;
               *pbuf = tstr->buf;
          }
     }
     else if (len <= WSDT_FIXEDSTRING_LEN)
     {
          if ((dep = wsdata_alloc(dtype_fstr))) {
               wsdt_fixedstring_t * fstr =
                    (wsdt_fixedstring_t*)dep->data;
               *plen = len;
               *pbuf = fstr->buf;
          }
     }
     else if (len <= WSDT_SMALLSTRING_LEN) {
          if ((dep = wsdata_alloc(dtype_smallstring))) {
               wsdt_smallstring_t * smlstr =
                    (wsdt_smallstring_t*)dep->data;
               *plen = len;
               *pbuf = smlstr->buf;
          }
     }
     else if (len <= WSDT_MEDIUMSTRING_LEN) {
          if ((dep = wsdata_alloc(dtype_mediumstring))) {
               wsdt_mediumstring_t * mstr =
                    (wsdt_mediumstring_t*)dep->data;
               *plen = len;
               *pbuf = mstr->buf;
          }
     }
     else if (len <= WSDT_BIGSTRING_LEN) {
          if ((dep = wsdata_alloc(dtype_bigstring))) {
               wsdt_bigstring_t * bstr =
                    (wsdt_bigstring_t*)dep->data;
               *plen = len;
               *pbuf = bstr->buf;
          }
     }
     else if (len <= WSDT_MASSIVESTRING_LEN) {
          if ((dep = wsdata_alloc(dtype_massivestring))) {
               // char * buf = (char *)malloc(len);
               void * buf;
               long mem_page_sz = sysconf(_SC_PAGESIZE);
               int verify = posix_memalign(&buf, mem_page_sz, len);
               if (0 != verify)
               {
                    // show reason for error
                    fprintf(stderr, "wsdata_create_buffer: ERROR! posix_memalign failed...");
                    if (EINVAL == verify)
                    {
                         fprintf(stderr, "mem_page_sz = %ld is not a power of two or not a multiple of sizeof(void*)", mem_page_sz);
                    }
                    else if (ENOMEM == verify)
                    {
                         fprintf(stderr, "available memory is insufficient for len = %d bytes", len);
                    }
                    fprintf(stderr, "\n");

                    wsdata_delete(dep);
                    return NULL;
               }
               if (buf) {
                    wsdt_massivestring_t * str = (wsdt_massivestring_t *)dep->data;
                    str->buf = (char *)buf;
                    str->len = len;
                    *plen = len;
                    *pbuf = (char *)buf;
               }
               else {
                    wsdata_delete(dep);
                    return NULL;
               }
          }
     }
     else {
          fprintf(stderr, "len = %d exceeds WSDT_MASSIVESTRING_LEN...returning NULL\n", len);
          fprintf(stderr, "consider using wsdata_create_hugebuffer instead\n");
          return NULL;
     }

     return dep;
}

// create a wsdata type with a HUGE buffer; we envision
// using this function when size is bigger than we have with
// wsdt_massivestring_t in the latter part of
// wsdata_create_buffer.
static inline wsdata_t * wsdata_create_hugebuffer(
          uint64_t len,
          char ** pbuf,
          uint64_t * plen)
{
     wsdata_t * dep = wsdata_alloc(dtype_hugeblock);
     if (!dep)
          return NULL;
     wsdt_hugeblock_t * hb = (wsdt_hugeblock_t *)dep->data;
     if (hb->buf) {
          if (hb->len == len) {
               *plen = len;
               *pbuf = hb->buf;
               return dep;
          }

          // free the existing (unlucky) buf that came to us
          // and proceed with creating a new one
          free(hb->buf);
          hb->buf = NULL;
     }

     void * buf;
     int verify = posix_memalign(
          &buf, sysconf(_SC_PAGESIZE), len);
     if (0 != verify) {
          // show reason for error
          fprintf(stderr, "wsdata_create_hugebuffer: ERROR! posix_memalign failed...");
          if (EINVAL == verify)
          {
               fprintf(stderr,
                    "alignment field = %ld is not a power of"
                    " two or not a multiple of sizeof(void*)",
                    sysconf(_SC_PAGESIZE) );
          }
          else if (ENOMEM == verify)
          {
               fprintf(stderr, "available memory is insufficient for len = %" PRIu64 " bytes", len);
          }
          fprintf(stderr, "\n");
          wsdata_delete(dep);
          return NULL;
     }

     if (buf) {
          hb->buf = (char *)buf;
          hb->len = len;
          *plen = len;
          *pbuf = (char *)buf;
          return dep;
     }
     wsdata_delete(dep);
     return NULL;
}

static inline wsdata_t * wsdata_create_string(
          const char * cpy_buffer,
          int len)
{
     int plen;
     char * pbuf;

     wsdata_t * wsbuf = wsdata_create_buffer(
          len, &pbuf, &plen);
     if (!wsbuf)
          return NULL;
     if (plen < len) {
          wsdata_delete(wsbuf);
          return NULL;
     }
     wsdata_t * wsstr = wsdata_alloc(dtype_string);
     if (!wsstr) {
          wsdata_delete(wsbuf);
          return NULL;
     }
     wsdata_assign_dependency(wsbuf, wsstr);
     wsdt_string_t * str = (wsdt_string_t *)wsstr->data;
     str->buf = pbuf; 
     str->len = len; 
     memcpy((void* __restrict__)pbuf,
            (void* __restrict__)cpy_buffer,
            len);
     return wsstr;
}

static inline wsdata_t * dtype_alloc_binary(int len) {
     int plen;
     char * pbuf;
     wsdata_t * wsbuf = wsdata_create_buffer(
          len, &pbuf, &plen);
     if (!wsbuf)
          return NULL;
     if (plen < len) {
          wsdata_delete(wsbuf);
          return NULL;
     }
     wsdata_t * wsstr = wsdata_alloc(dtype_binary);
     if (!wsstr) {
          wsdata_delete(wsbuf);
          return NULL;
     }
     wsdata_assign_dependency(wsbuf, wsstr);
     wsdt_binary_t * str = (wsdt_binary_t *)wsstr->data;
     str->buf = pbuf; 
     str->len = len; 
     return wsstr;
}


static inline wsdata_t * dtype_str2ts(
          const char * str,
          int len)
{
     if (timeparse_detect_date(str, len) != 2)
          return NULL;
     wsdata_t * wsd = wsdata_alloc(dtype_ts);
     if ( wsd == NULL )
          return NULL;
     wsdt_ts_t * ts = (wsdt_ts_t*)wsd->data;
     if (len > 19) {
          char temp[20];
          memcpy((void* __restrict__)temp,
                 (void* __restrict__)str,
                 19);
          temp[19] = '\0';
          if (len > 20) {
               const char * ustr = str + 20;
               ts->usec = atoi(ustr);
          }
          ts->sec = timeparse_str2time(
               temp, STR2TIME_WSSTD_FORMAT);
          return wsd;
     }
     ts->sec = timeparse_str2time(str, STR2TIME_WSSTD_FORMAT);
     return wsd;
}

static inline wsdata_t * dtype_detect_strtype(
          const char * str,
          int len)
{
     if (!str || !len)
          return NULL;
     if (str[0] == ' ') {
          str++;
          len--;
          if (len == 0) {
               return NULL;
          }
     }

     int offset = 0;
     int is_neg = 0;
     if (str[0] == '-') {
          is_neg = 1; 
          offset++;
          if (len <= offset) {
               return NULL;
          }
     }
     if (!isxdigit(str[offset]))
          return NULL;
     wsdata_t * wsd = dtype_str2ts(str, len);
     if (wsd) return wsd;
     
     int is_int = 1;
     int dots = 0, colons = 0, ees = 0;
     int is_double = 1;

     //test for pure uints and doubles
     for (int i = offset; i < len; i++) {
          if (!isdigit(str[i])) {
               is_int = 0;
               if (str[i] == '.') {
                    dots++;
                    if (dots > 1) {
                         is_double = 0;
                         if ((i < (len - 1))
                             && !isdigit(str[i+1])) {
                              break;
                         }
                    }
               }
               else if (str[i] == ':') {
                    is_double = 0;
                    colons++;
                    if (colons > 7) {
                         break;
                    }
               }
               else if (str[i] == 'E' || str[i] == 'e') {
                    ees++;
                    if (i == (len - 1) || ees > 1) {
                         is_double = 0;
                    }
               }
               else if (str[i] == '+' || str[i] == '-') {
                    if ( i<=0 || (str[i-1]!='E' 
                              && str[i-1]!='e')
                        || i==(len-1) || !isdigit(str[i+1]) )
                    {
                         is_double = 0;
                         break;
                    }
               }
               else {
                    is_double = 0;
                    if(!isxdigit(str[i])) {
                         break;
                    }
               }
          }
     }
     if (is_int) {
          if (is_neg) {
               if (len >= 10) {
                    if ((wsd = wsdata_alloc(dtype_int64))) {
                         wsdt_int64_t * ti = (wsdt_int64_t*)wsd->data; 
                         *ti = (wsdt_int64_t)strtoll(str, NULL, 10);
                    }
               }
               else if ((wsd = wsdata_alloc(dtype_int))) {
                    wsdt_int_t * ti = (wsdt_int_t*)wsd->data; 
                    *ti = (wsdt_int_t)strtol(str, NULL, 10);
               }
          }
          else if (len >= 10) {
               if ((wsd = wsdata_alloc(dtype_uint64))) {
                    wsdt_uint64_t * u64 = (wsdt_uint64_t*)wsd->data; 
                    *u64 = (wsdt_uint64_t)strtoull(str, NULL, 10);
               }
          }
          else if ((wsd = wsdata_alloc(dtype_uint))) {
               wsdt_uint_t * u = (wsdt_uint_t*)wsd->data; 
               *u = (wsdt_uint_t)strtoul(str, NULL, 10);
          }
     }
     else if (is_double) {
          if ((wsd = wsdata_alloc(dtype_double))) {
               wsdt_double_t * dbl = (wsdt_double_t*)wsd->data; 
               *dbl = (wsdt_double_t)atof(str);
          }
     }

     return wsd;
}

static inline int dtype_is_exit_flush(wsdata_t * wsd) {
     if (wsd->dtype == dtype_flush) {
          wsdt_flush_t * flush = (wsdt_flush_t*)wsd->data; 
          if (flush->flag == WSDT_FLUSH_EXIT_MSG) {
               return 1;
          }
     }
     return 0;
}

#ifdef __cplusplus
extern "C" {
#endif

void init_wstypes(void *);

#ifdef __cplusplus
}
#endif

#endif // _WSTYPES_H

