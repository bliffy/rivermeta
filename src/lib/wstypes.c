#include "waterslide.h"
#include "wstypes.h"

// globals
wsdatatype_t * dtype_uint;
wsdatatype_t * dtype_uint64;
wsdatatype_t * dtype_uint8;
wsdatatype_t * dtype_uint16;
wsdatatype_t * dtype_int;
wsdatatype_t * dtype_int64;
wsdatatype_t * dtype_double;
wsdatatype_t * dtype_string;
wsdatatype_t * dtype_str;
wsdatatype_t * dtype_binary;
wsdatatype_t * dtype_fstr;
wsdatatype_t * dtype_fixedstring;
wsdatatype_t * dtype_mediumstring;
wsdatatype_t * dtype_bigstring;
wsdatatype_t * dtype_bigstr;
wsdatatype_t * dtype_ts;
wsdatatype_t * dtype_tuple;
wsdatatype_t * dtype_monitor;
wsdatatype_t * dtype_labelset;
wsdatatype_t * dtype_label;
wsdatatype_t * dtype_massivestring;
wsdatatype_t * dtype_hugeblock;
wsdatatype_t * dtype_mmap;
wsdatatype_t * dtype_flush;
wsdatatype_t * dtype_smallstring;
wsdatatype_t * dtype_tinystring;

void init_wstypes(void * tl) {

     dtype_uint = wsdatatype_get(
          tl, "UINT_TYPE");
     dtype_int = wsdatatype_get(
          tl, "INT_TYPE");
     dtype_int64 = wsdatatype_get(
          tl, "INT64_TYPE");
     dtype_uint64 = wsdatatype_get(
          tl, "UINT64_TYPE");
     dtype_uint16 = wsdatatype_get(
          tl, "UINT16_TYPE");
     dtype_uint8 = wsdatatype_get(
          tl, "UINT8_TYPE");
     dtype_double = wsdatatype_get(
          tl, "DOUBLE_TYPE");
     dtype_string = wsdatatype_get(
          tl, "STRING_TYPE");
     dtype_str = wsdatatype_get(
          tl, "STRING_TYPE");
     dtype_fstr = wsdatatype_get(
          tl, "FSTR_TYPE");
     dtype_fixedstring = wsdatatype_get(
          tl, "FIXEDSTRING_TYPE");
     dtype_mediumstring = wsdatatype_get(
          tl, "MEDIUMSTRING_TYPE");
     dtype_bigstring = wsdatatype_get(
          tl, "BIGSTRING_TYPE");
     dtype_bigstr = wsdatatype_get(
          tl, "BIGSTRING_TYPE");
     dtype_binary = wsdatatype_get(
          tl, "BINARY_TYPE");
     dtype_ts = wsdatatype_get(
          tl, "TS_TYPE");
     dtype_tuple = wsdatatype_get(
          tl, "TUPLE_TYPE");
     dtype_monitor = wsdatatype_get(
          tl, "MONITOR_TYPE");
     dtype_labelset = wsdatatype_get(
          tl, "LABELSET_TYPE");
     dtype_label = wsdatatype_get(
          tl, "LABEL_TYPE");
     dtype_massivestring = wsdatatype_get(
          tl, "MASSIVESTRING_TYPE");
     dtype_hugeblock = wsdatatype_get(
          tl, "HUGEBLOCK_TYPE");
     dtype_mmap = wsdatatype_get(
          tl, "MMAP_TYPE");
     dtype_flush = wsdatatype_get(
          tl, "FLUSH_TYPE");
     dtype_smallstring = wsdatatype_get(
          tl, "SMALLSTRING_TYPE");
     dtype_tinystring = wsdatatype_get(
          tl, "TINYSTRING_TYPE");
}

