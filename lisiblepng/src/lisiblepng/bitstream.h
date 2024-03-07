#ifndef CUTTERENG_BITSTREAM_H
#define CUTTERENG_BITSTREAM_H

#include <stdint.h>
#include <stdlib.h>

typedef struct {
  const uint8_t *data;
  size_t data_size;

  size_t byte_offset;
  uint8_t bit_index;
} Bitstream;

void Bitstream_init(Bitstream *bitstream, const uint8_t *data,
                    size_t data_size);
void Bitstream_skip(Bitstream *bitstream, size_t bit_count);
uint16_t Bitstream_next_bits(Bitstream *bitstream, int bit_count);

#endif // CUTTERENG_BITSTREAM_H
