//
//  flood.cpp
//  name: 
//
//
//  assume that in the initial elevation grid the sea points are
//  represented as NODATA;


#include "flood.hpp"


#include <queue>
#include <limits>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>


/*  Simulates the SLR flooding with a sea level of <rise> starting
    from the points in the queue, which are all assumed to represent
    sea.
    
*/ 
std::queue<point> compute_flood_from_queue(const Grid* elev_grid, Grid* flooded_grid,
					    const float rise, 
					    std::queue<point>& queue) {

  assert(elev_grid && flooded_grid);
  std::queue<point> next_queue; 
  point currPoint;
  while  (!queue.empty()) {
    //get the next point in the queue 
    currPoint = queue.front();
    if (grid_is_nodata(elev_grid, currPoint.y, currPoint.x)){
      //mark as visited ocean
      grid_set(flooded_grid, currPoint.y, currPoint.x, 900);
    }
    queue.pop();
    //check neighbors
    for(int i = -1 ; i <= 1 ; i++){
      for( int j = -1 ; j <= 1 ; j++){
        point nextPoint;
        nextPoint.x = currPoint.x + i;
        nextPoint.y = currPoint.y + j;
        if(nextPoint.x < elev_grid->ncols && nextPoint.x >= 0 && nextPoint.y < elev_grid->nrows && nextPoint.y >= 0 && (grid_get(flooded_grid, nextPoint.y, nextPoint.x) == 0.000000 || grid_get(flooded_grid, nextPoint.y, nextPoint.x) == 901)){
          if(!grid_is_nodata(elev_grid, nextPoint.y, nextPoint.x) && grid_get(elev_grid, nextPoint.y, nextPoint.x) <= rise){
            //point is land and needs to flooded 
            grid_set(flooded_grid, nextPoint.y, nextPoint.x, rise);
            queue.push(nextPoint);
          } else if (!grid_is_nodata(elev_grid, nextPoint.y, nextPoint.x) && grid_get(flooded_grid, nextPoint.y, nextPoint.x)!=901){
            //set as going to check in next queue
            grid_set(flooded_grid, nextPoint.y, nextPoint.x, 901);
            next_queue.push(nextPoint);
          }
        }
      }
    }
  }
  std::queue<point> bnd_queue; 
  return next_queue;
}

/*
  simulates SLR flooding with rise = rise_incr, 2*rise_incr, ..., until rise > rise_end
 */
void compute_flood_incrementally(const Grid* elev_grid, const float rise_incr,
				 const float rise_end,  Grid* flooded_grid) {
  //must exist 
  assert(flooded_grid && elev_grid); 
  float rise;
  printf("compute SLR flooding up to slr=%.1f (incr=%.1f)\n", rise_end, rise_incr);
  std::queue<point> bnd_queue;
  point currPoint;
  //add top and bottom no data points
  for (int y=0; y < (elev_grid->nrows); y++) {
    if (grid_is_nodata(elev_grid, y, 0)){
      currPoint.x = 0;
      currPoint.y = y;           
      bnd_queue.push(currPoint);
    }
    if (grid_is_nodata(elev_grid, y, (elev_grid->ncols)-1)){
      currPoint.x = elev_grid->ncols-1;
      currPoint.y = y;           
      bnd_queue.push(currPoint);
    }
  }
  //side boundary no data points
  for (int x=1; x < (elev_grid->ncols)-1; x++) {
    if (grid_is_nodata(elev_grid, (elev_grid->nrows)-1, x)){
      currPoint.x = x;
      currPoint.y = (elev_grid->nrows)-1;           
      bnd_queue.push(currPoint);
    }
    if (grid_is_nodata(elev_grid, 0, x)){
      currPoint.x = x;
      currPoint.y = 0;           
      bnd_queue.push(currPoint);
    }
  }
  printf("Number of items in the queue: %zu\n\n", bnd_queue.size());
  int count;
  int total = 0;
  int most = 0;
  float most_rise = 0;
  for (rise = rise_incr; rise <= rise_end; rise += rise_incr) {
    count = 0;
    bnd_queue = compute_flood_from_queue(elev_grid, flooded_grid, rise, bnd_queue);
    for (int y=0; y < (elev_grid->nrows); y++) {
      for (int x=0; x < (elev_grid->ncols); x++) {
        if (grid_get(flooded_grid, y, x) == rise){
          count += 1;
        }
      }
    }
    if (count > most) {
      most = count; 
      most_rise = rise;
    }
    total += count;
    printf("\tAt slr =%.1f:  flooding    %d new cells \n", rise, count);
  }
  unsigned int percentage = ((float)total / (elev_grid->nrows * elev_grid ->ncols)) * 100.0;
  printf("\nTotal nb. cells flooded: %d (%.2f percent)\n", count, (float)percentage);
  printf("largest flood is from slr=%.1f to slr=%.1f, total %d cells\n\n", most_rise - 1, most_rise, total);
} 