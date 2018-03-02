#ifndef _VARIANCE_H
#define _VARIANCE_H 

/**
 * Compute a running variance.  Accumulates
 * the sum and sum of squares along the way so that 
 * variance and mean can be requested at any point
 * with minimum additional computation.
 */

#ifdef __cplusplus
extern "C" {
#endif

/** 
 * Stores information eneded to compute variance.
 * The structure methods are designed to work with
 * this structure passed to them zeroed out initially.
 * In that case, the methods will comput a total
 * variance and mean.  If the window_size and previous_items
 * fields are initialized, then the methods will keep a
 * windowed variance and mean calculation of the last
 * window_size items that it sees.  Note that previous_items
 * should be initialized to an array of doubles the length
 * of window_size.
 *
 */
typedef struct _variance {
     int window_size;
     int start;
     double * previous_items;
     double sum;
     double sumofsquares;
     double sumcubed;
     double count;
} variance;

/**
 * Add an item seen to the variances.
 * structure.
 */
void var_hit(variance*, double);

/**
 * Get the variance represented by
 * the variance object. Note that 
 * it does a final computation, however
 * it does not have to iterate over all
 * the items.
 */
double var_getVariance(const variance*);

/**
 * Get the mean of the items seen
 * Note that it does a final computation,
 * however it does not have to iterate
 * over all the items.
 */
double var_getMean(const variance*);


/**
 * Get the skewness represented by the variance
 * object.  Note that this does a final computation,
 * however it does not have to iterate over all the items.
 */
double var_getSkewness(const variance*);

#ifdef __cplusplus
}
#endif

#endif // _VARIANCE_H
