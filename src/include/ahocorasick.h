/*
     ahocorasick.h
     Nov 2006
     Aho Corasick string searching API
*/

#ifndef _AHOCORASICK_H
#define _AHOCORASICK_H

#include <stdint.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/** match node information */
typedef struct _term_info_t {
     int          keymapval;
     char*        key;   /* keyword */
     size_t       len;   /* size of keyword */
     void*        udata;
} term_info_t;

/** AC tree node */
typedef struct _treenode_t {
  unsigned char       alpha;      /* node alphabet value */
  uint8_t             children;   /* number of child nodes */
  term_info_t*        match;      /* keyword info */
  struct _treenode_t* nodes[sizeof(char)];
  struct _treenode_t* fail_node;  /* pointer to fail node */
  uint8_t             buflen;     /* max buffer len */
} treenode_t;

/** queue node */
typedef struct _qnode_t {
     struct _qnode_t*    next;
     struct _treenode_t* nodeptr;
} qnode_t;


/** AC type */
typedef struct _ahoc_t {
     treenode_t* root;
     uint32_t    max_shift;
     uint32_t    cshift[sizeof(char)];
     uint32_t    max_pattern_len;
     uint8_t     case_insensitive;
     uint8_t     below_threshold;
} ahoc_t;

typedef uint8_t     boolean;
typedef treenode_t* ahoc_state_t;

/**
  callback function type on keyword matches in files (for testing)
  return non-zero to match only once
  return 0 to match multiple keywords
*/
typedef int (*aho_filecallback)(
     const char* /* matchstr */,
     FILE* /* pointer after match in file */,
     void* /* callback data */);

/**
  callback function type on keyword matches in buffer (normal use)
  return non-zero to match only once
  return 0 to match multiple keywords
*/
typedef int (*aho_buffercallback)(
     const char*  /* matchstr */,
     const char*  /* pointer after match in buffer */,
     size_t       /* remaining chars in buffer */,
     void*        /* callback data */);

/**
  \brief initializes Aho Corasick type

  must be called before loading keywords
  \return new ahoc_t* on success
  \return NULL on failure
 */
ahoc_t* ac_init(void);

/**
  insert keyword into Aho Corasick tree

  use keymap value for singlesearch returns (for use in 'switch' statements)
  return 1 on success
  return 0 on failure
*/
int ac_loadkeyword(
     ahoc_t*      /* initialized ahoc tree */,
     const char*  /* keyword */, 
     size_t       /* keywordlen */,
     int          /* key map value */);

/**
  remove keyword from Aho Corasick tree

  return 1 on success
  return 0 on failure
*/
int ac_remove_keyword(
     ahoc_t*     /* initialized ahoc tree */,
     const char* /* keyword */, 
     size_t      /* keywordlen */);

/**
  call after all keywords loaded 

  determines fail nodes on mismatched characters,
  don't need to call for _skip() APIs
  also deteremine which algorithm to use

  return 1 on success
  return 0 on failure
*/
int ac_finalize(ahoc_t* /* init & loaded ahoc tree */);

/**
  searches file for keyword matches
  return -1 on failure
  return 1 on match
  return 0 on no match
*/
int ac_searchfile(
     const ahoc_t*    /* init & loaded ahoc tree */,
     ahoc_state_t* /* state pointer in keyword tree */,
     const char*      /* filename */,
     aho_filecallback /* saved file callback function */,
     void*            /* callback_data */);

/**
  searches buffer for keyword matches, calls callback on each match
 
  will call straight aho-corasick or bmh/aho-corasick depending on number of signatures 
  and length of signatures

  return -1 on failure
  return 1 on match
  return 0 on no match
  int (*ac_search)(
       const ahoc_t*
       ahoc_state_t*
       const char*  
       size_t
       aho_buffercallback
       void*
*/




/**
  searches buffer for keyword matches, calls callback on each match

  return -1 on failure
  return 1 on match
  return 0 on no match
*/
int ac_searchstr(
     const ahoc_t*       /* init & loaded ahoc tree */,
     ahoc_state_t* /* state pointer in keyword tree */,
     const char*         /* search space */,
     size_t              /* length of search space */,
     aho_buffercallback  /* match callback function */,
     void*               /* callback_data */);

void ac_free(ahoc_t* ac);

/**
  searches buffer for keyword matches
  starts searching from the end of buffer and does BM skipping,
  does not need state and will not handle cross-buffer matches
  return -1 on failure
  return 1 on match
  return 0 on no match
*/
int ac_searchstr_skip(
     const ahoc_t*       /* init & loaded ahoc tree */, 
     ahoc_state_t* /* state pointer in keyword tree */,
     const char*         /* search space */,
     size_t              /* length of search space */,
     aho_buffercallback  /* match callback function */,
     void*               /* callback_data */);

/**
  prints Aho-Corasick tree for debugging
*/
void ac_print_tree(const ahoc_t*);

const term_info_t *ac_singlesearch_trans(
     const ahoc_t* ac,
     ahoc_state_t* sPtr,
     const char* buf,
     size_t buflen,
     char const ** retbuf,
     size_t* retlen,
     uint8_t mask_len,
     uint8_t case_ins,
     uint8_t binary_op);

/**
  searches buffer for first keyword
  return -1 on failure or no match
  return 'mapval' on match
*/
int ac_singlesearch(
     const ahoc_t* ac,
     ahoc_state_t* sPtr,
     const char* buf,
     size_t buflen,
     char const ** retbuf,
     size_t* retlen);

/**
  searches buffer for first keyword
  return -1 on failure or no match
  return 'mapval' on match
*/
int ac_singlesearch_skip(
     const ahoc_t* ac,
     ahoc_state_t* sPtr,
     const char* buf,
     size_t buflen,
     char const ** retbuf,
     size_t* retlen);

#ifdef __cplusplus
}
#endif

#endif // _AHOCORASICK_H
