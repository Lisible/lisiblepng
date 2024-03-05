#ifndef LISIBLE_PNG_DEFLATE_H
#define LISIBLE_PNG_DEFLATE_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

/// Decompresses zlib compressed data as defined by RFC 1950
///
/// @param compressed_data_buffer The compressed data
/// @param compressed_data_length The length in bytes of the compressed data
/// @param output_length The pointer to write the output length to
/// @return A pointer to the decompressed data, the caller is responsible to
/// deallocate it using free(). In case of error, NULL is returned.
char *zlib_decompress(const uint8_t *compressed_data_buffer,
                      const size_t compressed_data_length,
                      size_t *output_length);

#endif // LISIBLE_PNG_DEFLATE_H
