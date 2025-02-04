#ifndef _KMEANS_H
#define _KMEANS_H

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
   cluster an array of points.. must pass in functions for
   distance and update this is a genertic library for kmeans
   */

// for multidimensinal data, may want to consider magitude
// normalization.. linear, log norms

//These functions have to be defined by the user of the kmeans
//get distance from data to cetroid
typedef double (* kmeans_distance_f)(void *, void *, void *);
//check if cetroids should merge
typedef int (* kmeans_merge_f)(void *, void *, void *);
//update centroid
typedef void (* kmeans_update_f)(void *, void *, double, void *);
// settle centroid
typedef int (* kmeans_settle_f)(void *, void *);
//assign cluster id
typedef void (* kmeans_assign_f)(void *, int, void *);
//get cluster id
typedef int (* kmeans_get_f)(void *, void *);

// must be len number of elements in A,
// must be k elements in centroids
// keep running until settled
static inline int kmeans(
          void** A, int len, int k,
          void** centroids, kmeans_distance_f kdist,
          kmeans_update_f kupdate,
          kmeans_settle_f ksettle,
          kmeans_assign_f kassign,
          kmeans_get_f kget,
          void* vdata, int maxpasses)
{
     //find closes cluster
     int i, j;
     int change;
     int passes = 0;

     do {
          passes++;
          change = 0;
          for (i = 0; i < len; i++) {
               double min;
               int clust = 0;
               int old = 0;
               //see if cluster assignment changed -- first get old cluster
               if (!change) {
                    old = kget(A[i], vdata);
               }

               min = kdist(A[i], centroids[0], vdata);
               //get distance to each cluster
               for (j = 1; j < k; j++) {
                    double d;
                    d = kdist(A[i], centroids[j], vdata);
                    if (d < min) {
                         min = d;
                         clust = j;
                    }
               }

               //see if cluster assignment changed -- first get old cluster
               if (!change && old != clust) {
                    change = 1;
               }
               kassign(A[i], clust, vdata);
               kupdate(centroids[clust], A[i], min, vdata);
          }
          if (!change || (maxpasses && passes >= maxpasses)) {
               return passes;
          }

          for (j = 0; j < k; j++) {
               ksettle(centroids[j], vdata);
          }

     } while (change);

     return passes;
}

//just use existing centroids to set cluster
static inline void kmeans_set(
          void** A, int len, int k,
          void** centroids,
          kmeans_distance_f kdist,
          kmeans_assign_f kassign,
          void* vdata)
{
     //find closes cluster
     int i, j;

     for (i = 0; i < len; i++) {
          double min;
          int clust = 0;

          min = kdist(A[i], centroids[0], vdata);
          //get distance to each cluster
          for (j = 1; j < k; j++) {
               double d;
               d = kdist(A[i], centroids[j], vdata);
               if (d < min) {
                    min = d;
                    clust = j;
               }
          }
          kassign(A[i], clust, vdata);
     }
}


//this is a variant that takes empty clusters and assigns them to furthest data
//elements
// must be len number of elements in A,
// must be k elements in centroids
// keep running until settled
static inline int kmeans_recluster(
          void** A, int len, int k,
          void** centroids,
          kmeans_distance_f kdist,
          kmeans_update_f kupdate,
          kmeans_settle_f ksettle,
          kmeans_assign_f kassign,
          kmeans_get_f kget,
          kmeans_merge_f kmerge,
          void* vdata, int maxpasses)
{
     //find closes cluster
     int i, j;
     int change;
     int passes = 0;
     void* furthest;

     do {
          passes++;
          dprint("passes %d", passes);
          change = 0;
          furthest = NULL;
          double max = 0;
          for (i = 0; i < len; i++) {
               double min;
               int clust = 0;
               int old = 0;
               //see if cluster assignment changed -- first get old cluster
               if (!change) {
                    old = kget(A[i], vdata);
               }

               min = kdist(A[i], centroids[0], vdata);

               //get distance to each cluster
               for (j = 1; j < k; j++) {
                    double d;
                    d = kdist(A[i], centroids[j], vdata);
                    if (d < min) {
                         min = d;
                         clust = j;
                    }
                    if (d > max) {
                         max = d;
                         furthest = A[i];
                    }
               }

               //see if cluster assignment changed -- first get old cluster
               if (!change && old != clust) {
                    change = 1;
               }
               kassign(A[i], clust, vdata);
               kupdate(centroids[clust], A[i], min, vdata);
          }
          if (!change || (maxpasses && passes >= maxpasses)) {
               return passes;
          }

          for (j = 0; j < k; j++) {
               //see if cluster is empty, assign furthest data to empty cluster
               if (!ksettle(centroids[j], vdata) && furthest) {
                    kassign(furthest, j, vdata);
                    kupdate(centroids[j], furthest, 0, vdata);
                    ksettle(centroids[j], vdata);
                    furthest = NULL;
                    dprint("assign empty cluster %d", j);
               }
          }
          //merge clusters
          for (i = 0; (furthest && (i < k)); i++) {
               for (j = (i+1); (furthest && (j < k)); j++) {
                    if (kmerge(centroids[i], centroids[j], vdata)) {
                         kassign(furthest, j, vdata);
                         kupdate(centroids[j], furthest, 0, vdata);
                         ksettle(centroids[j], vdata);
                         furthest = NULL;
                         dprint("merged clusters %d %d", i, j);
                    }
               }

          }
     } while (change);

     return passes;
}

#ifdef __cplusplus
}
#endif

#endif // _KMEANS_H
