#ifndef LISIBLE_PNG_H
#define LISIBLE_PNG_H

#include <stdio.h>

#define LISIBLE_PNG_COMPUTE_CRC

struct Png;
typedef struct Png Png;

Png *lis_Png_parse(FILE *stream);
void lis_Png_dump_ppm(const Png *png);
void lis_Png_destroy(Png *png);

#endif // LISIBLE_PNG_H
