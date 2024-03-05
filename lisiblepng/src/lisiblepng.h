#ifndef LISIBLE_PNG_H
#define LISIBLE_PNG_H

#include <stdint.h>
#include <stdio.h>

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
/// @param stream The PNG stream
/// @returns The parsed PNG as a Png struct pointer or NULL if an error occured.
/// The returned PNG is owned by the caller and must be destroyed with
/// Png_destroy.
LisPng *LisPng_decode(FILE *stream);
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
