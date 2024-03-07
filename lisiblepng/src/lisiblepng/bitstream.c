#include "bitstream.h"
#include "assert.h"

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

void Bitstream_init(Bitstream *bitstream, const uint8_t *data,
                    size_t data_size) {
  ASSERT(bitstream != NULL);
  ASSERT(data != NULL);
  bitstream->data = data;
  bitstream->data_size = data_size;
  bitstream->byte_offset = 0;
  bitstream->bit_index = 0;
}

void bitstream_advance(Bitstream *bitstream, size_t bit_count) {
  ASSERT(bitstream != NULL);
  uint8_t current_bit = bitstream->bit_index;
  bitstream->byte_offset =
      bitstream->byte_offset + (current_bit + bit_count) / 8;
  bitstream->bit_index = (current_bit + bit_count) % 8;
}

void Bitstream_skip(Bitstream *bitstream, size_t bit_count) {
  ASSERT(bitstream != NULL);
  bitstream_advance(bitstream, bit_count);
}
uint16_t Bitstream_next_bits(Bitstream *bitstream, int bit_count) {
  ASSERT(bitstream != NULL);
  ASSERT(bit_count <= 16);
  ASSERT((bitstream->bit_index + bit_count - 1) / 8 + bitstream->byte_offset <
         bitstream->data_size);

  uint16_t val = 0;
  int bits_collected = 0;
  while (bits_collected < bit_count) {
    int bits_in_current_byte = 8 - bitstream->bit_index;
    int bits_to_extract = MIN(bit_count - bits_collected, bits_in_current_byte);
    uint16_t extracted_bits =
        (bitstream->data[bitstream->byte_offset] >> bitstream->bit_index) &
        ((1 << bits_to_extract) - 1);
    val |= extracted_bits << bits_collected;
    bits_collected += bits_to_extract;
    bitstream_advance(bitstream, bits_to_extract);
  }

  return val;
}
