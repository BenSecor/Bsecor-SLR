//
//  flood.hpp
//  name: 
//

#ifndef flood_hpp
#define flood_hpp


#include "grid.h"
#include <stdio.h>
#include <queue>



typedef struct _point{
  int x,y;
} point;






/*
  simulates SLR flooding with rise = rise_start, until rise = rise_end, with increment; 
 */
void compute_flood_incrementally(const Grid* elev_grid, const float rise_incr,
				 const float rise_end,  Grid* flooded_grid); 



//returns the points on the boundary of the area reached by the flood
std::queue<point> compute_flood_from_queue(const Grid* elevGrid, Grid* floodedGrid,
					   const float rise,
					   std::queue<point>& queue); 


#endif /* flood_hpp */