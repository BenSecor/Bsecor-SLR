//  name: 
//
#include "pixel_buffer.h"
#include "grid.h"
#include "map.h"


#include <stdlib.h>
#include <string.h>
#include <stdio.h> 
#include <math.h>


// static const u8 max_rgb_value = 0xffu;
static const float no_data_value = 0.5;
// static const float z = 0.3;
// static const float elevationScale = 0.7;
// static const int isoline_interval =  10; 


// // must convery her SUN_zenith_deg and azimuth as we skipped that step last time 
// static const float sun_zenith_rad = 2.36;
// static const float sun_azimuth_rad = 0.79;

// //for hillshading
// const float SUN_zenith_deg = 45;
// const float SUN_azimuth_deg = 315;

////////////////////////////////////////////////////////////
/*
a b c
d e f
g h i 

return nodata if e is nodata
return (c + 2f + i - a - 2d -g)/8
 */
float dzdx(const Grid* grid, int r, int c){
    return no_data_value;

}




////////////////////////////////////////////////////////////
/*
a b c
d e f
g h i 

return nodata if e is nodata
return (g + 2h + i - a - 2b -c)/8
 */
float dzdy(const Grid* grid, int r, int c){
    return no_data_value;
}
