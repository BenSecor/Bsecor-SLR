/* 
MIT License

Copyright (c) 2023 Caspian Ahlberg

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/
#include "pixel_buffer.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h> 


#pragma clang diagnostic push
// This library spews a bunch of conversion errors, so this ignores them
#pragma clang diagnostic ignored "-Wsign-conversion"
#pragma clang diagnostic ignored "-Wdouble-promotion"

#define STB_IMAGE_WRITE_IMPLEMENTATION // Including the actual function definitions
#include "stb_image_write.h"

#pragma clang diagnostic pop




PixelBuffer init_pixel_buffer(const u16 width, const u16 height) {
  return (PixelBuffer) {(u32*)malloc(width * height * sizeof(u32)), width, height};
}

void deinit_pixel_buffer(const PixelBuffer* const pb) {
  free(pb -> pixels);
}

//////////

void write_pixel_to_buffer(const PixelBuffer* const pb,
			   const u16 x, const u16 y, const u8 r, const u8 g, const u8 b) {
  
  const u16 width = pb -> width;
  
#ifdef DO_BOUNDS_CHECKING_FOR_PIXEL_WRITES
  
  if (x >= width || y >= pb -> height) {
    fprintf(stderr, "X or Y are out of bounds!\n");
    exit(1);
  }
  
#endif
  
  // TODO: will this work on big-endian systems?
  static const u32 alpha_mask = 0xffu << 24u;
  const u32 pixel = alpha_mask | (u32) (b << 16u) | (u32) (g << 8u) | r;
  
  pb -> pixels[y * width + x] = pixel;
}



void save_pixel_buffer_to_file(const PixelBuffer* const pb, const char* const filename) {
  const int jpg_quality = 50;
  
  // No HDR support because pixel channels are in bytes, not floats
  const char* const file_extensions[] = {".png", ".bmp", ".tga", ".jpg"};
  
  //////////
  const u8 num_extension_types = sizeof(file_extensions) / sizeof(*file_extensions);
  
  const size_t filename_length = strlen(filename);
  const char* const filename_end = filename + filename_length;
  
  static const size_t num_channels = sizeof(u32) / sizeof(u8);
  
  //////////
  
  for (u8 i = 0u; i < num_extension_types; i++) {
    const char* const extension = file_extensions[i];
    
    const size_t extension_length = strlen(extension);
    
    if (extension_length > filename_length) {
      printf("Ext too long: %s\n", extension);
      continue;
    }
    
    // TODO: make this code less ugly
    else if (!strcmp(filename_end - extension_length, extension)) {
      if (!strcmp(extension, ".png")) {
	stbi_write_png(filename, pb -> width, pb -> height, num_channels, pb -> pixels, 0); return;
      }
      else if (!strcmp(extension, ".bmp")) {
	stbi_write_bmp(filename, pb -> width, pb -> height, num_channels, pb -> pixels); return;
      }
      else if (!strcmp(extension, ".tga")) {
	stbi_write_tga(filename, pb -> width, pb -> height, num_channels, pb -> pixels); return;
      }
      else if (!strcmp(extension, ".jpg")) {
	stbi_write_jpg(filename, pb -> width, pb -> height, num_channels, pb -> pixels, jpg_quality); return;
      }
    }
  }
  
  fprintf(stderr, "Unsupported file extension for file '%s'\n", filename);
  exit(1);
}

