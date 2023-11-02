// Laura Toma
//
#include "grid.h"

#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <limits.h>



/* ************************************************************ */
void grid_print_stats(const Grid* g, char* name) {

  printf("grid %s: \n\tn=%ld [rows=%d,cols=%d], range=[%.2f, %.2f], "
	 "NODATA values=%d (%.2f percent)\n",
	 name, 
	 (long int)g->nrows*(long int)g->ncols, g->nrows, g->ncols, 
	 g->min_value, g->max_value, 
	 g->nb_nodata_values,   
	 100.0*((double)g->nb_nodata_values)/((double)g->nrows*g->ncols));
}


// Returns an empty grid object
Grid* grid_init() {
  Grid* grid;
  grid = (Grid*) malloc(sizeof(Grid));
  assert(grid);
  grid->data = NULL;
  grid->min_value = INT_MAX;
  grid->max_value = -INT_MAX;
  grid->nb_nodata_values = 0; 
  return grid;
}

//grid must be allocated 
void grid_read_header(FILE* in_file, Grid* grid) {

  assert(grid); 
  assert(fscanf(in_file, "ncols %d\n",        &grid->ncols));
  assert(fscanf(in_file, "nrows %d\n",        &grid->nrows));
  assert(fscanf(in_file, "xllcorner %f\n",    &grid->xllcorner));
  assert(fscanf(in_file, "yllcorner %f\n",    &grid->yllcorner));
  assert(fscanf(in_file, "cellsize %f\n",     &grid->cellsize));
  assert(fscanf(in_file, "NODATA_value %f\n", &grid->nodata_value));
}


// allocate space for the grid data.
void grid_malloc_data(Grid* grid) {
  if (!grid->data) {
    grid->data = (float*)malloc(grid->nrows * grid->ncols * sizeof(float*));
    assert(grid->data);
  }
}

void grid_read_data(FILE* in_file, Grid* grid) {
  float val;
  for (int r = 0; r < grid->nrows; r++) {
    for (int c = 0; c < grid->ncols; c++) {
      assert(fscanf(in_file, "%f ", &val));
      grid_set(grid, r, c, val);
    }//for c
  }//for r
}


// Read grid from file and return it 
Grid* grid_read(FILE* in_file) {
  Grid* grid = grid_init();
  grid_read_header(in_file, grid);
  grid_malloc_data(grid);
  grid_read_data(in_file, grid);
  return grid;
}


//read from file 
Grid* grid_read_from_file(char* filename) {

  FILE* infile = fopen(filename, "r");
  if (!infile) {
    printf("cannot open file %s\n", filename);
    exit(1); 
  } 
  Grid* grid = grid_read(infile);
  fclose(infile);
  return grid; 
} 

  

void grid_write_header(FILE* out_file, Grid* grid) {
  fprintf(out_file, "ncols %d\n",        grid->ncols);
  fprintf(out_file, "nrows %d\n",        grid->nrows);
  fprintf(out_file, "xllcorner %f\n",    grid->xllcorner);
  fprintf(out_file, "yllcorner %f\n",    grid->yllcorner);
  fprintf(out_file, "cellsize %f\n",     grid->cellsize);
  fprintf(out_file, "NODATA_value %f\n", grid->nodata_value);
}


// Write the complete asc file for a grid.
void grid_write(FILE* out_file, Grid* grid) {

  grid_write_header(out_file, grid);
  for (int r = 0; r < grid->nrows; r++) {
    for (int c = 0; c < grid->ncols; c++) {
      fprintf(out_file, "%f ", grid_get(grid, r, c));
    }
    fprintf(out_file, "\n");
  }
}

 

// Write the complete asc file for a grid.
void grid_write_to_file(char* filename, Grid* grid) {

  FILE* outfile = fopen(filename, "w+");
  if (!outfile) {
    printf("cannot open file %s\n", filename);
    exit(1); 
  } 
  grid_write(outfile, grid);
  fclose(outfile); 
} 

 
void grid_copy_header(Grid* grid, Grid* new_grid) {
  assert (grid && new_grid); 
  new_grid->ncols =        grid->ncols;
  new_grid->nrows =        grid->nrows;
  new_grid->xllcorner =    grid->xllcorner;
  new_grid->yllcorner =    grid->yllcorner;
  new_grid->cellsize =     grid->cellsize;
  new_grid->nodata_value = grid->nodata_value;
}


// Initialize a grid based on an existing grid, copying e.g. its dimensions.
Grid* grid_init_from(Grid* grid) {
  Grid* new_grid = grid_init();
  grid_copy_header(grid, new_grid);
  grid_malloc_data(new_grid);
  return new_grid;
}


// Free a grid and its associated malloced data;
void grid_free(Grid* grid) {
  free(grid->data);
  free(grid);
}


