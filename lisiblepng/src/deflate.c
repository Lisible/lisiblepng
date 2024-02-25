#include "deflate.h"
#include "assert.h"
#include "bitstream.h"

#define CM_LENGTH_BITS 4
#define CINFO_LENGTH_BITS 4
#define FCHECK_LENGTH_BITS 5
#define FDICT_LENGTH_BITS 1
#define FLEVEL_LENGTH_BITS 2

bool deflate_decompress(Bitstream *bitstream);

bool zlib_decompress(const uint8_t *compressed_data_buffer,
                     const size_t compressed_data_length) {
  ASSERT(compressed_data_buffer != NULL);

  Bitstream bitstream;
  Bitstream_init(&bitstream, compressed_data_buffer, compressed_data_length);

  uint16_t cm = Bitstream_next_bits(&bitstream, CM_LENGTH_BITS);
  uint16_t cinfo = Bitstream_next_bits(&bitstream, CINFO_LENGTH_BITS);
  uint16_t fcheck = Bitstream_next_bits(&bitstream, FCHECK_LENGTH_BITS);
  uint16_t fdict = Bitstream_next_bits(&bitstream, FDICT_LENGTH_BITS);
  uint16_t flevel = Bitstream_next_bits(&bitstream, FLEVEL_LENGTH_BITS);

  LOGN("zlib informations:\n"
       "- CM (compression method): %d\n"
       "- CINFO (compression info): %d\n"
       "- FCHECK (check bits for CMF and FLG): %d\n"
       "- FDICT (preset dictionary): %d\n"
       "- FLEVEL (compression level): %d",
       cm, cinfo, fcheck, fdict, flevel);
  uint16_t cmf = bitstream.data[0];
  uint16_t flg = bitstream.data[1];
  if ((cmf * 256 + flg) % 31 != 0) {
    LOG0("fcheck validation failed");
    return false;
  }

  uint32_t dictionary_identifier = 0;
  if (fdict != 0) {
    uint16_t dictionary_identifier_0 = Bitstream_next_bits(&bitstream, 16);
    uint16_t dictionary_identifier_1 = Bitstream_next_bits(&bitstream, 16);
    dictionary_identifier =
        (dictionary_identifier_0 << 0) + (dictionary_identifier_1 << 16);
    LOGN("Dictionary identifier: %d", dictionary_identifier);
  }

  if (!deflate_decompress(&bitstream)) {
    return false;
  }

  // TODO adler32 checksum

  return true;
}
