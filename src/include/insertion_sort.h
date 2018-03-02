#ifndef _INSERTION_SORT_H
#define _INSERTION_SORT_H

#include <stdlib.h>
#include <stdio.h>
#include "error_print.h"
 
#ifdef __cplusplus
extern "C" {
#endif

static inline void InsertionSort(
          double array[],
          uint32_t index[],
          uint32_t len)
{
     uint32_t indx, indx2, cur_indx, prev_indx, temp_indx;
     double cur_val, prev_val, temp_val;

     if (len <= 1) {
          error_print("InsertionSort length was %u\n",len);
          return;
     }

     prev_val = array[0];
     prev_indx = 0;

     for (indx = 1; indx < len; ++indx) {
          cur_val = array[indx];
          cur_indx = index[indx];
          if (prev_val > cur_val) {
               /* out of order: array[indx-1] > array[indx] */
               array[indx] = prev_val; /* move up the larger item first */
               index[indx] = prev_indx;

               /* find the insertion point for the smaller item */
               for (indx2 = indx - 1; indx2 > 0;) {
                    temp_val = array[indx2-1];
                    temp_indx = index[indx2-1];
                    if (temp_val > cur_val) {
                         index[indx2] = temp_indx;
                         array[indx2--] = temp_val;
                         /* still out of order, move up 1 slot to make room */
                    }
                    else
                         break;
               }
               index[indx2] = cur_indx;
               array[indx2] = cur_val; /* insert the smaller item right here */
          }
          else {
               /* in order, advance to next element */
               prev_val = cur_val;
               prev_indx = cur_indx;
          }
     }
}

#ifdef __cplusplus
}
#endif

#endif // _INSERTION_SORT_H
