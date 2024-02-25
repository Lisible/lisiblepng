#ifndef LISIBLE_PNG_DEFLATE_H
#define LISIBLE_PNG_DEFLATE_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

bool zlib_decompress(const uint8_t *compressed_data_buffer,
                     const size_t compressed_data_length);

#endif // LISIBLE_PNG_DEFLATE_H
