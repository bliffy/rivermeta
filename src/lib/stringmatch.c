/* The following code was taken from 
   http://www-igm.univ-mlv.fr/~lecroq/string/
   to implement the Boyer-Moore-Horspool algorithm for string matching

   it was redesigned for speed and for better return values..
*/
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include "stringmatch.h"

stringmatch_t* stringmatch_init(
          const char* str,
          size_t len)
{
     stringmatch_t* sm = (stringmatch_t *)calloc(1, sizeof(stringmatch_t));
     if (!sm) {
          error_print("failed stringmatch_init calloc of sm");
          return 0;
     }
     sm->str = (char *)malloc(len);
     if (!sm->str) {
          error_print("failed stringmatch_init malloc of sm->str");
          return 0;
     }
     memcpy(sm->str, str, len);
     sm->len = len;
     size_t i;
     for (i = 0; i < BMH_CHARSET_SIZE; ++i) {
          sm->bmshift[i] = len;
     }
     for (i = 0; i < (len - 1); ++i) {
          sm->bmshift[(uint8_t)str[i]] = len - i - 1;
     }
     return sm;
}

stringmatch_t* stringmatch_init_nocase(
          const char* str,
          size_t len) {
     stringmatch_t * sm = (stringmatch_t *)calloc(1, sizeof(stringmatch_t));
     if (!sm) {
          error_print("failed stringmatch_init_nocase calloc of sm");
          return 0;
     }
     sm->str = (char *)malloc(len);
     if (!sm->str) {
          error_print("failed stringmatch_init_nocase malloc of sm->str");
          return 0;
     }
     sm->str2 = (char *)malloc(len);
     if (!sm->str2) {
          error_print("failed stringmatch_init_nocase malloc of sm->str2");
          return 0;
     }
     sm->nocase = 1;
     sm->len = len;
     size_t i;
     for (i = 0; i < len; ++i) {
          sm->str[i] = tolower(str[i]);
          sm->str2[i] = toupper(str[i]);
     }
     for (i = 0; i < BMH_CHARSET_SIZE; ++i) {
          sm->bmshift[i] = len;
     }
     for (i = 0; i < (len - 1); ++i) {
          sm->bmshift[(uint8_t)sm->str[i]] = len - i - 1;
          sm->bmshift[(uint8_t)sm->str2[i]] = len - i - 1;
     }
     return sm;
}

void stringmatch_free(stringmatch_t * sm) {
     if (sm->str) {
          free(sm->str);
     }
     if (sm->str2) {
          free(sm->str2);
     }
     free(sm);
}

const char* stringmatch(
          const stringmatch_t* sm,
          const char* haystack,
          size_t haylen)
{
     if (!haystack) {
          return NULL;
     }
     if (haylen < sm->len) {
          return NULL;
     }
     int c;
     size_t j = 0;
     while (j <= (haylen - sm->len)) {
          c = haystack[j + sm->len - 1];
          if ((sm->str[sm->len - 1] == c) && 
              (memcmp(sm->str, haystack + j, sm->len - 1) == 0)) {
               return haystack + j;
          }
          j += sm->bmshift[c];
     }
     return NULL;
}


const char* stringmatch_nocase(
          const stringmatch_t* sm,
          const char* haystack,
          size_t haylen)
{
     if (!haystack)
          return NULL;
     if (haylen < sm->len)
          return NULL;

     int c;
     size_t i=0, j=0;
     while (j <= (haylen - sm->len)) {
          c = haystack[j + sm->len - 1];
          if ((sm->str[sm->len - 1] == c) || (sm->str2[sm->len - 1] == c)) {
               for (i = 0; i < (sm->len - 1); i++) {
                    if ((sm->str[i] != (haystack+j)[i]) &&
                        (sm->str2[i] != (haystack+j)[i])) {
                         break;
                    }
               }
               if (i == (sm->len -1))
                    return haystack + j;
          }
          j += sm->bmshift[c];
     }
     return NULL;
}

size_t stringmatch_offset_nocase(
          const stringmatch_t* sm,
          const char* haystack,
          size_t haylen)
{
     const char* match = stringmatch_nocase(
          sm, haystack, haylen);  
     if (!match)
          return -1;
     return (match - haystack) + sm->len;
}

size_t stringmatch_offset(
          const stringmatch_t* sm,
          const char* haystack,
          size_t haylen)
{
     const char* match = stringmatch(
          sm, haystack, haylen);  
     if (!match)
          return -1;
     return (match - haystack) + sm->len;
}

