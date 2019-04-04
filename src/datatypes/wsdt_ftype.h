#ifndef _WSDT_FTYPE_H
#define _WSDT_FTYPE_H

#define WSDT_FTYPE_STR "FTYPE_TYPE"

typedef enum {
     unknown,
     text,
     image,
     executable,
     compressed,
     pdf
} wsdt_ftype_t;

#endif
