#ifndef LISIBLE_PNG_H
#define LISIBLE_PNG_H

#include <lisiblestd/types.h>

typedef enum {
  LisPngColourType_Greyscale = 0,
  LisPngColourType_Truecolour = 2,
  LisPngColourType_IndexedColour = 3,
  LisPngColourType_GreyscaleWithAlpha = 4,
  LisPngColourType_TruecolourWithAlpha = 6,
  LisPngColourType_Unknown
} LisPngColourType;
uint8_t LisPngColourType_sample_count(const LisPngColourType colour_type);

struct LisPng;
typedef struct LisPng LisPng;

/// Parses the provided PNG stream
///
/// @param image_bytes The image data
/// @param image_bytes_length The size of the image data
/// @returns The parsed PNG as a Png struct pointer or NULL if an error occured.
/// The returned PNG is owned by the caller and must be destroyed with
/// Png_destroy.
LisPng *LisPng_decode(const u8 *image_bytes, usize image_bytes_length);

/// Writes the PNG image data as RGBA8 data to a buffer
///
/// Note: The output_data buffer must be allocated with enough memory
/// (width*height*32)
/// @param png The png
/// @param output_data The output buffer
void LisPng_write_RGBA8_data(const LisPng *png, uint8_t *output_data);

/// Outputs the provided Png struct as a PPM image to stdout
///
/// @param png The png
void LisPng_dump_ppm(const LisPng *png);
LisPngColourType LisPng_colour_type(const LisPng *png);
uint8_t LisPng_bits_per_sample(const LisPng *png);
uint8_t *LisPng_data_ptr(const LisPng *png);
uint32_t LisPng_width(const LisPng *png);
uint32_t LisPng_height(const LisPng *png);
/// Destroys a Png instance
void LisPng_destroy(LisPng *png);

#endif // LISIBLE_PNG_H
