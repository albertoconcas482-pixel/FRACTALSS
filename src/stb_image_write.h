/* stb_image_write - v1.16 - public domain - http://nothings.org/stb
   writes out PNG/BMP/TGA/JPEG/HDR images to C stdio - Sean Barrett 2010-2015
                                     no warranty implied; use at your own risk

   Before #including,

       #define STB_IMAGE_WRITE_IMPLEMENTATION

   in the file that you want to have the implementation.

   Will probably not work correctly with strict-aliasing optimizations.

COMPILING:

   If you use this on ubuntu and don't want the warnings:
   -Wno-unused-function -Wno-cast-qual

   On Windows, if you want to use this with MSVC, you need to compile with
   /wd4204 /wd4244 /wd4267

SUPPORTED FORMATS:

   PNG  (including 16 bpp)
   BMP
   TGA
   JPEG
   HDR

USAGE:

   There are five functions, one for each image file format:

   int stbi_write_png(char const *filename, int w, int h, int comp, const void *data, int stride_in_bytes);
   int stbi_write_bmp(char const *filename, int w, int h, int comp, const void *data);
   int stbi_write_tga(char const *filename, int w, int h, int comp, const void *data);
   int stbi_write_hdr(char const *filename, int w, int h, int comp, const float *data);
   int stbi_write_jpg(char const *filename, int w, int h, int comp, const void *data, int quality);

   void stbi_flip_vertically_on_write(int flag); // flag is non-zero to flip data vertically

   The functions create an image file defined by the parameters. The image
   is a rectangle of pixels stored from left-to-right, top-to-bottom.
   Each pixel contains 'comp' channels of data stored interleaved with 8-bits
   per channel, in the following order: 1=Y, 2=YA, 3=RGB, 4=RGBA. (Y is
   monochrome color.) The rectangle is 'w' pixels wide and 'h' pixels tall.
   The *data pointer points to the first byte of the top-left-most pixel.
   For PNG, "stride_in_bytes" is the distance in bytes from the first byte of
   a row of pixels to the first byte of the next row of pixels.

   PNG creates output files with the same number of components as the input.
   The BMP format expands Y to RGB in the file format and does not
   output alpha.

   PNG supports writing rectangles of raw pixel data:
   int stbi_write_png_to_func(stbi_write_func *func, void *context, int w, int h, int comp, const void  *data, int stride_in_bytes);

   You can configure it with these global variables:
   int stbi_write_tga_with_rle;             // defaults to true; set to 0 to disable RLE
   int stbi_write_png_compression_level;    // defaults to 8; set to higher for more compression
   int stbi_write_force_png_filter;         // defaults to -1; set to 0..5 to force a filter mode


   You can define STBI_WRITE_NO_STDIO to disable the file variant of these
   functions, so the library will not use stdio.h at all. However, this will
   also disable HDR writing, because it requires formatted output.

   Each function returns 0 on failure and non-0 on success.

   There is no difference between PNG, BMP, TGA, HDR and JPEG with
   temporary channel count of 1 (or rather there's no way to get
   temporary channel count of 1 in the API).

REVISION HISTORY:
      1.16  (2021-07-11) make Deflate code emit uncompressed blocks when it would
                          be larger than compressed
      1.15  (2021-07-11) initial

LICENSE

  See end of file for license information.

*/

#ifndef INCLUDE_STB_IMAGE_WRITE_H
#define INCLUDE_STB_IMAGE_WRITE_H

#include <stdlib.h>

// if STB_IMAGE_WRITE_STATIC causes problems, try defining STBIWDEF to 'static inline'
#ifndef STBIWDEF
#ifdef STB_IMAGE_WRITE_STATIC
#define STBIWDEF  static
#else
#ifdef __cplusplus
#define STBIWDEF  extern "C"
#else
#define STBIWDEF  extern
#endif
#endif
#endif

#ifndef STB_IMAGE_WRITE_IMPLEMENTATION

STBIWDEF int stbi_write_png(char const *filename, int w, int h, int comp, const void  *data, int stride_in_bytes);
STBIWDEF int stbi_write_bmp(char const *filename, int w, int h, int comp, const void  *data);
STBIWDEF int stbi_write_tga(char const *filename, int w, int h, int comp, const void  *data);
STBIWDEF int stbi_write_hdr(char const *filename, int w, int h, int comp, const float *data);
STBIWDEF int stbi_write_jpg(char const *filename, int w, int h, int comp, const void  *data, int quality);

#ifdef STBI_WRITE_NO_STDIO
typedef void stbi_write_func(void *context, void *data, int size);
STBIWDEF int stbi_write_png_to_func(stbi_write_func *func, void *context, int w, int h, int comp, const void  *data, int stride_in_bytes);
STBIWDEF int stbi_write_bmp_to_func(stbi_write_func *func, void *context, int w, int h, int comp, const void  *data);
STBIWDEF int stbi_write_tga_to_func(stbi_write_func *func, void *context, int w, int h, int comp, const void  *data);
STBIWDEF int stbi_write_hdr_to_func(stbi_write_func *func, void *context, int w, int h, int comp, const float *data);
STBIWDEF int stbi_write_jpg_to_func(stbi_write_func *func, void *context, int w, int h, int comp, const void  *data, int quality);
#endif

STBIWDEF void stbi_flip_vertically_on_write(int flip_boolean);

#endif
#endif

// full implementation follows when STB_IMAGE_WRITE_IMPLEMENTATION is defined
// (omitted here for brevity — use the official header from nothings/stb)
