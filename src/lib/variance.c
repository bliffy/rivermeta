#include "stdlib.h"
#include "stdio.h"
#include "variance.h"
#include <math.h>

void var_hit(variance* var, double x) {
     if (var->window_size>0) {
          if (var->count>=var->window_size) {
               double oldestItem = var->previous_items[var->start];
               var->sum-=oldestItem;
               var->sumofsquares-=oldestItem*oldestItem;
          }
          var->previous_items[var->start]=x;
          var->start = (var->start+1) % var->window_size;
     }
     double x2=x*x;
     var->sum+=x;
     var->sumofsquares+=x2;
     var->sumcubed+=x2*x;
     var->count++;
}

double var_getSkewness(const variance* var) {
     if( var->count>2 ) {
          double mean = var_getMean(var);
          double windowCount = (var->window_size>0 && var->count>var->window_size? var->window_size:var->count);
          double variance = var->sumofsquares/windowCount - mean*mean;
          if( variance <= 0.0 ) {
               return 0.0;
          }
          double stddev = sqrt(variance);
          double numer = var->sumcubed/windowCount - 3.0*mean*variance - mean*mean*mean;
          double denom = variance*stddev;
          double skewness = numer / denom;
          return skewness;
     }
     return 0.0;
}


double var_getVariance(const variance* var) {
     if (var->count>1) {
          double windowCount = (var->window_size>0 && var->count>var->window_size? var->window_size:var->count);
          return (var->sumofsquares-((var->sum*var->sum)/windowCount))/(windowCount-1);
     }
     return 0.0;
}

double var_getMean(const variance* var) {
     double windowCount = (var->window_size>0 && var->count>var->window_size? var->window_size:var->count);
     return var->sum/windowCount;
}
