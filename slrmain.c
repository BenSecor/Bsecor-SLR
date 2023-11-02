//
// Name: Ben Secor and Liam Jachetta
//

#include "flood.hpp"
#include "map.h"
#include "pixel_buffer.h"
#include "grid.h"

#include <stdlib.h>
#include <stdio.h> 
#include <math.h>
#include <getopt.h>
#include <assert.h>


/*
  Reads command line arguments
*/
#include <iostream>
#include <cstdlib>

static const u8 max_rgb_value = 0xffu;
static const float z = ;
static const float alpha = 0.3;
 // must convery correct SUN_zenith_deg and azimuth as we skipped that step last time 
static  float sun_zenith_rad = 2.36;
static  float sun_azimuth_rad = 0.79;

 //for hillshading
 const int SUN_altitude_deg = 45;
 const int SUN_azimuth_deg = 315;



void parseArguments(int argc, char* const* argv, 
                    char** elevName, 
                    char** floodedName, float* rise, float* incr) {

    *elevName = argv[1];
    *floodedName = argv[2];
    try {
        *rise = std::stof(argv[3]);
        *incr = std::stof(argv[4]);
    } catch (const std::invalid_argument& e) {
        std::cerr << "Invalid sea level rise or increment value." << std::endl;
        exit(1);
    }
}

void createGrids(Grid* grid, Grid* slopeGrid, Grid* aspectGrid, Grid* hillshadeGrid) {
  float SUN_azimuth_math;
  float SUN_zenith_deg; 
  SUN_zenith_deg = 90-SUN_altitude_deg;
  sun_zenith_rad = SUN_zenith_deg * PI / 180;
  SUN_azimuth_math = ((90-SUN_azimuth_deg)+360) %360;
  sun_azimuth_rad = SUN_azimuth_math*PI/180;

  for (int y=1; y < (grid->nrows)-1; y++) {
     for (int x=1; x < (grid->ncols)-1; x++) {
        if (grid_is_nodata(grid, y, x)){
          continue;
        }
        else if(grid_is_nodata(grid, y-1,x) || grid_is_nodata(grid, y-1, x-1)|| grid_is_nodata(grid, y-1, x+1) || 
        grid_is_nodata(grid, y+1, x-1) || grid_is_nodata(grid, y+1, x+1) || grid_is_nodata(grid, y+1, x) ||
        grid_is_nodata(grid, y, x-1) || grid_is_nodata(grid, y, x+1) || grid_is_nodata(grid, y, x)){
          grid_set_nodata(slopeGrid, y,x);
        } else {
        //compute slope
        float topLeft = grid_get(grid, y - 1, x - 1);
        float topRight = grid_get(grid, y - 1, x + 1);
        float topMiddle = grid_get(grid, y-1, x);
        float middleLeft = grid_get(grid, y, x-1);
        float middleRight = grid_get(grid, y, x+1);
        float bottomMiddle = grid_get(grid, y+1, x);
        float bottomLeft = grid_get(grid, y + 1, x - 1);
        float bottomRight = grid_get(grid, y + 1, x + 1);
        // Compute the slope using the formula
        float dx = grid->cellsize; // Horizontal distance between diagonal points
        float dy = grid->cellsize; // Vertical distance between diagonal points

        float slopeX = (topRight + 2 * middleRight + bottomRight  - topLeft - 2 * middleLeft - bottomLeft) / (dx*8);
        float slopeY = (topLeft + 2 * topMiddle+ topRight - bottomLeft - 2 * bottomMiddle - bottomRight) / (dy*8);
        float slope = atan(z*sqrt(slopeX*slopeX+slopeY*slopeY));
        grid_set(slopeGrid, y, x, slope);

        //compute the aspect grid
        grid_set(aspectGrid, y, x, atan2(slopeY, slopeX));
        float azimuth_rad;
         if (slopeX != 0){
            azimuth_rad= atan2(slopeY, slopeX);
              if(azimuth_rad < 0){
               azimuth_rad += 2*PI;
              }
         } else {
          if (slopeY > 0) {
            azimuth_rad = PI/2;
          } else if ( slopeY < 0){
            azimuth_rad = 3*PI/2;
          } else{
            azimuth_rad = 0;
          }
        }
        float hillshade = cos(sun_zenith_rad)*cos(slope)+ sin(sun_zenith_rad)*sin(slope)*cos(sun_azimuth_rad - azimuth_rad);
        if (hillshade < 0){
          hillshade = 0;
        }
        grid_set(hillshadeGrid, y, x, hillshade);
      }
    }
  }
}

void pixelbuffer_overlay(const PixelBuffer* pbout, const PixelBuffer* pb1, const PixelBuffer* pb2, float alpha) {
       // Iterate over each pixel in the buffers and perform the overlay operation
      for (u16 y = 0; y < pb1->height; y++) {
        for (u16 x = 0; x < pb1->width; x++) {
             u32 pixel1 = pb1->pixels[y * pb1->width + x];
            u32 pixel2 = pb2->pixels[y * pb2->width + x];

            // Extract the R, G, and B components of pixel1 and pixel2
            u8 b1 = (u8)((pixel1 >> 16) & 0xFF);
            u8 g1 = (u8)((pixel1 >> 8) & 0xFF);
            u8 r1 = (u8)(pixel1 & 0xFF);

            u8 b2 = (u8)((pixel2 >> 16) & 0xFF);
            u8 g2 = (u8)((pixel2 >> 8) & 0xFF);
            u8 r2 = (u8)(pixel2 & 0xFF);

            // Multiply the pixel colors by alpha
            u8 result_r = (u8)((alpha * r1) + ((1.0f - alpha) * r2));
            u8 result_g = (u8)((alpha * g1) + ((1.0f - alpha) * g2));
            u8 result_b = (u8)((alpha * b1) + ((1.0f - alpha) * b2));

            // Write the resulting R, G, and B values to pbout
            write_pixel_to_buffer(pbout, x, y, result_r, result_g, result_b);
        }
    }
}

void computeFloodBuffer(const PixelBuffer* pb, const Grid* grid, const Grid* floodGrid, int rise_inc){
  float color[3];
  for (u16 y = 0; y < pb->height; y++) {
        for (u16 x = 0; x < pb->width; x++) {
      if (grid_is_nodata(grid, y, x)) {
            color[0] = 0; color[1] = 0; color[2] = 0.5; //blue if nodata 
          } else if (grid_is_nodata(floodGrid, y, x)){
            color[0] = color[1] = color[2] = 1;
          } else {
        int c = grid_get(floodGrid, y, x)/rise_inc;
        // Assuming that 'c' is a float representing a specific value
        // 'color' is an array to store the RGB values
        switch (c) {
        case 1:
            color[0] = 0.68;
            color[1] = 0.85;
            color[2] = 0.9;
            break;
        case 2:
            color[0] = 0.31;
            color[1] = 0.53;
            color[2] = 0.97;
            break;
        case 3:
            color[0] = 0.15;
            color[1] = 0.23;
            color[2] = 0.89;
            break;
        case 4:
            color[0] = 0.64;
            color[1] = 0.76;
            color[2] = 0.68;
            break;
        case 5:
            color[0] = 0.6;
            color[1] = 0.1;
            color[2] = 0.1;
            break;
        case 6:
            color[0] = 0.7;
            color[1] = 0.5;
            color[2] = 0.0;
            break;
        case 7:
            color[0] = 0.2;
            color[1] = 0.0;
            color[2] = 0.16;
            break;
        case 8:
            color[0] = 0.1;
            color[1] = 0.8;
            color[2] = 0.4;
            break;
        case 9:
            color[0] = 0.9;
            color[1] = 0.6;
            color[2] = 0.1;
            break;
        case 10:
            color[0] = 0.5;
            color[1] = 0.2;
            color[2] = 0.7;
            break;
        case 11:
            color[0] = 0.8;
            color[1] = 0.4;
            color[2] = 0.2;
            break;
        case 12:
            color[0] = 0.3;
            color[1] = 0.7;
            color[2] = 0.6;
            break;
        case 13:
            color[0] = 0.2;
            color[1] = 0.5;
            color[2] = 0.8;
            break;
        case 14:
            color[0] = 0.6;
            color[1] = 0.4;
            color[2] = 0.9;
            break;
        case 15:
            color[0] = 0.8;
            color[1] = 0.2;
            color[2] = 0.7;
            break;
        case 16:
            color[0] = 0.4;
            color[1] = 0.8;
            color[2] = 0.1;
            break;
        case 17:
            color[0] = 0.7;
            color[1] = 0.9;
            color[2] = 0.2;
            break;
        case 18:
            color[0] = 0.2;
            color[1] = 0.6;
            color[2] = 0.5;
            break;
        case 19:
            color[0] = 0.5;
            color[1] = 0.3;
            color[2] = 0.4;
            break;
        case 20:
            color[0] = 0.9;
            color[1] = 0.7;
            color[2] = 0.3;
            break;
        default:
            color[0] = 1;
            color[1] = 1;
            color[2] = 1;
            break;
          }
          //bring it from [0,1] to [0, 255]
          u16 rgb_color[3] = {
          (u16) (color[0] * max_rgb_value),
          (u16) (color[1] * max_rgb_value),
          (u16) (color[2] * max_rgb_value)}; 
          
          write_pixel_to_buffer(pb, x, y, rgb_color[0], rgb_color[1], rgb_color[2]);
      }
    }
  }
}

  void computeHillshadeBuffer(const PixelBuffer* pb, const Grid* grid, const Grid* hillshadeGrid){
    float color[3];
    for (u16 y=0; y < pb->height; y++) {
      for (u16 x=0; x< pb->width; x++) {
        if (grid_is_nodata(grid, y, x)) {
          color[0] = 0; color[1] = 0; color[2] = 0; //black if nodata 
        } else {
          // get aspect ratio and slope value of the current pixel
          float h = grid_get(hillshadeGrid, y, x); //aspect ration of this pixel 
          
            // set color value to the sum of that interpolated color based on slope/aspect 
            color[0] = color[1] = color[2] = h;
        }
        //bring it from [0,1] to [0, 255]
        u16 rgb_color[3] = {
        (u16) (color[0] * max_rgb_value),
        (u16) (color[1] * max_rgb_value),
        (u16) (color[2] * max_rgb_value)}; 
        
        write_pixel_to_buffer(pb, x, y, rgb_color[0], rgb_color[1], rgb_color[2]);
      }
    }
  }

 void computeElevGradBuffer(const PixelBuffer* pb, Grid* grid){
    float color[3];
    for (u16 y = 0; y < pb->height; y++) {
        for (u16 x = 0; x < pb->width; x++) {
          float h = grid_get(grid, y, x); //height of this pixel 
      if (grid_is_nodata(grid, y, x)) {
	      color[0] = 0; color[1] = 0; color[2] = 0; //blue if nodata 
      } else {

        //interpolate the color 
        float c = (h - grid->min_value)/(grid->max_value - grid->min_value);

        // determine the color of each pixel based on a specified height interval
        if (c < 0.05) {
          color[0] = 0;
          color[1] = 0;
          color[2] = 0.25;
        } 
        // Triggered when elevation (c) is between 0.05 and 0.2, creating a gradual increase in grayscale
        else if (c < 0.2) {
          color[2] = 0.2;
          color[0] = color[1] = 0.4;
        } 
        // Triggered when elevation (c) is between 0.2 and 0.45, creating a gradual increase in green and blue
        else if (c < 0.45) {
          color[1] = color[2] = 0.45;
          color[0] = 0;
        } 
        // Triggered when elevation (c) is between 0.45 and 0.7, creating a gradual increase in green and blue, decrease in red
        else if (c < 0.7) {
          color[1] = color[2] = 0.3;
          color[0] = 0.7;
        } 
        // Triggered when elevation (c) is greater than or equal to 0.7, creating a gradual increase in green and blue
        else {
          color[1] = color[2] = 0.95;
          color[0] = 0.95;
        }
      }

      //bring it from [0,1] to [0, 255]
      u16 rgb_color[3] = {
	    (u16) (color[0] * max_rgb_value),
	    (u16) (color[1] * max_rgb_value),
	    (u16) (color[2] * max_rgb_value)}; 
      
      write_pixel_to_buffer(pb, x, y, rgb_color[0], rgb_color[1], rgb_color[2]);
    }
  } 
 }

int main(int argc, char** argv) {
  char *elev_name, *flooded_name;
  float rise, rise_incr;
  parseArguments(argc, argv, &elev_name,  &flooded_name, &rise, &rise_incr);
  printf("running %s with arguments:\n\telev_name: %s \n\tflooded_name: %s \n\tslr=%.1f, incr=%.1f\n",
	 argv[0], elev_name, flooded_name, rise, rise_incr);

  printf("reading raster %s\n", argv[1]);
  FILE* infile = fopen(argv[1], "r");
  if (!infile) {
    printf("cannot open file %s\n", argv[1]);
    exit(1); 
  } 
  Grid* grid = grid_read(infile); 
  Grid* slopeGrid = grid_init_from(grid);
  Grid* aspectGrid = grid_init_from(grid);
  Grid* hillshadeGrid = grid_init_from(grid);
  Grid* floodedGrid = grid_init_from(grid);

  printf("compute sea lever rise with %d rows and %d colums \n", grid->nrows, grid->ncols);
  
  compute_flood_incrementally(grid, rise_incr, rise, floodedGrid);

  createGrids(grid, slopeGrid, aspectGrid, hillshadeGrid);

  //create a bmp of same size as the raster
  const PixelBuffer floodPB = init_pixel_buffer(grid->ncols, grid->nrows);
  const PixelBuffer elevPB = init_pixel_buffer(grid->ncols, grid->nrows);
  const PixelBuffer hillshadePB = init_pixel_buffer(grid->ncols, grid->nrows);
  const PixelBuffer outPB = init_pixel_buffer(grid->ncols, grid->nrows);
 


  computeHillshadeBuffer(&hillshadePB, grid, hillshadeGrid);
  computeFloodBuffer(&floodPB, grid, floodedGrid, rise_incr);
  computeElevGradBuffer(&elevPB, grid);

  printf("writing map.hillshade\n"); 
  save_pixel_buffer_to_file(&hillshadePB, "map.hillshade.bmp");

  pixelbuffer_overlay(&outPB, &hillshadePB, &elevPB, alpha);
  //flooded grid, colors intervals discrete
  printf("writing map.flooded.colordiscrete\n"); 
  save_pixel_buffer_to_file(&outPB, "map..elev-over-hillshade.bmp");

  printf("writing map.flooded.colordiscrete\n"); 
  save_pixel_buffer_to_file(&floodPB, "map.flooded.colordiscrete.bmp");
  pixelbuffer_overlay(&outPB, &floodPB, &hillshadePB, alpha);
  printf("writing map.flooded-over-hillshade.bmp\n"); 
  save_pixel_buffer_to_file(&outPB, "map.flooded-over-hillshade.bmp");
  
}



