#ifndef LISIBLE_PNG_H
#define LISIBLE_PNG_H

#include <stdio.h>

struct Png;
typedef struct Png Png;

/// Parses the provided PNG stream
///
/// @param stream The PNG stream
/// @returns The parsed PNG as a Png struct pointer or NULL if an error occured.
/// The returned PNG is owned by the caller and must be destroyed with
/// Png_destroy.
Png *lis_Png_parse(FILE *stream);
/// Outputs the provided Png struct as a PPM image to stdout
///
/// @param png The png
void lis_Png_dump_ppm(const Png *png);
/// Destroys a Png instance
void lis_Png_destroy(Png *png);

#endif // LISIBLE_PNG_H
