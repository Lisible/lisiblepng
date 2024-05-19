#include "deflate.h"
#include "bitstream.h"
#include <lisiblestd/assert.h>
#include <string.h>

#define CM_LENGTH_BITS 4
#define CINFO_LENGTH_BITS 4
#define FCHECK_LENGTH_BITS 5
#define FDICT_LENGTH_BITS 1
#define FLEVEL_LENGTH_BITS 2

#define BFINAL_LENGTH_BITS 1
#define BTYPE_LENGTH_BITS 2

#define CODE_LENGTH_ALPHABET_MAX_SYMBOL_COUNT 19
#define MAX_HUFFMAN_CODE_LENGTH 15
#define MAX_LENGTH_CODES 288
#define MAX_DISTANCE_CODES 32
#define FIXED_LENGTH_LITERAL_CODE_COUNT 288

typedef enum {
  DeflateBlockType_NoCompression,
  DeflateBlockType_FixedHuffman,
  DeflateBlockType_DynamicHuffman,
  DeflateBlockType_Error
} DeflateBlockType;

typedef struct {
  uint16_t counts[CODE_LENGTH_ALPHABET_MAX_SYMBOL_COUNT];
  uint16_t symbols[CODE_LENGTH_ALPHABET_MAX_SYMBOL_COUNT];
} CodeLengthTable;

typedef struct {
  uint16_t counts[MAX_LENGTH_CODES];
  uint16_t symbols[MAX_LENGTH_CODES];
} LengthLiteralTable;

typedef struct {
  uint16_t counts[MAX_DISTANCE_CODES];
  uint16_t symbols[MAX_DISTANCE_CODES];
} DistanceTable;

#define DEFLATE_OUTPUT_BUFFER_INITIAL_CAPACITY (1024 * 256)

typedef struct {
  uint8_t *buffer;
  size_t len;
  size_t cap;
} OutputBuffer;

void OutputBuffer_init(OutputBuffer *output_buffer) {
  LSTD_ASSERT(output_buffer != NULL);
  output_buffer->buffer = calloc(DEFLATE_OUTPUT_BUFFER_INITIAL_CAPACITY, 1);
  if (!output_buffer->buffer) {
    LOG0_ERROR("Couldn't allocate deflate output buffer (out of memory?)");
    abort();
  }
  output_buffer->cap = DEFLATE_OUTPUT_BUFFER_INITIAL_CAPACITY;
  output_buffer->len = 0;
}

void OutputBuffer_expand(OutputBuffer *output_buffer) {
  LSTD_ASSERT(output_buffer != NULL);
  size_t new_cap = output_buffer->cap * 2;
  output_buffer->buffer = realloc(output_buffer->buffer, new_cap);
  if (!output_buffer->buffer) {
    LOG0_ERROR("Couldn't reallocate deflate output buffer (out of memory?)");
    abort();
  }

  output_buffer->cap = new_cap;
}

void OutputBuffer_push(OutputBuffer *output_buffer, uint8_t byte) {
  LSTD_ASSERT(output_buffer != NULL);
  if (output_buffer->len == output_buffer->cap) {
    OutputBuffer_expand(output_buffer);
  }

  output_buffer->buffer[output_buffer->len++] = byte;
}

size_t OutputBuffer_length(const OutputBuffer *output_buffer) {
  LSTD_ASSERT(output_buffer != NULL);
  return output_buffer->len;
}

int huffman_table_decode(const uint16_t *symbols, const uint16_t *counts,
                         Bitstream *bitstream) {
  LSTD_ASSERT(symbols != NULL);
  LSTD_ASSERT(counts != NULL);
  LSTD_ASSERT(bitstream != NULL);
  int code = 0;
  int index = 0;
  int first = 0;
  for (int i = 1; i <= MAX_HUFFMAN_CODE_LENGTH; i++) {
    code |= Bitstream_next_bits(bitstream, 1);
    int count = counts[i];
    if (code - count < first) {
      return symbols[index + (code - first)];
    }

    index += count;
    first += count;
    first <<= 1;
    code <<= 1;
  }

  return -1;
}

void build_huffman_table_from_codelengths(uint16_t *symbols, uint16_t *counts,
                                          const uint16_t *codelengths,
                                          uint16_t codelength_count) {
  LSTD_ASSERT(symbols != NULL);
  LSTD_ASSERT(counts != NULL);
  LSTD_ASSERT(codelengths != NULL);

  for (int i = 0; i < codelength_count; i++) {
    counts[codelengths[i]]++;
  }
  counts[0] = 0;

  uint16_t next_code[MAX_HUFFMAN_CODE_LENGTH + 1] = {0};
  for (int i = 1; i < MAX_HUFFMAN_CODE_LENGTH; i++) {
    next_code[i + 1] = next_code[i] + counts[i];
  }

  for (int i = 0; i < codelength_count; i++) {
    if (codelengths[i] == 0) {
      continue;
    }

    symbols[next_code[codelengths[i]]++] = i;
  }
}
void CodeLengthTable_build_from_codelengths(
    CodeLengthTable *table,
    uint16_t codelength_codelengths[CODE_LENGTH_ALPHABET_MAX_SYMBOL_COUNT]) {
  LSTD_ASSERT(table != NULL);
  LSTD_ASSERT(codelength_codelengths != NULL);
  static const uint8_t CODELENGTH_MAPPING[] = {
      16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};
  uint16_t mapped_codelengths[CODE_LENGTH_ALPHABET_MAX_SYMBOL_COUNT] = {0};
  for (int i = 0; i < CODE_LENGTH_ALPHABET_MAX_SYMBOL_COUNT; i++) {
    mapped_codelengths[CODELENGTH_MAPPING[i]] = codelength_codelengths[i];
  }

  build_huffman_table_from_codelengths(table->symbols, table->counts,
                                       mapped_codelengths,
                                       CODE_LENGTH_ALPHABET_MAX_SYMBOL_COUNT);
}

void build_codelength_table(CodeLengthTable *codelength_table,
                            Bitstream *bitstream, const uint8_t hclen) {
  LSTD_ASSERT(codelength_table != NULL);
  LSTD_ASSERT(bitstream != NULL);

  uint16_t codelengths_codelengths[CODE_LENGTH_ALPHABET_MAX_SYMBOL_COUNT] = {0};
  for (int i = 0; i < hclen; i++) {
    codelengths_codelengths[i] = Bitstream_next_bits(bitstream, 3);
  }

  CodeLengthTable_build_from_codelengths(codelength_table,
                                         codelengths_codelengths);
}

bool deflate_decompress_(Bitstream *bitstream,
                         const LengthLiteralTable *length_literal_table,
                         const DistanceTable *distance_table,
                         OutputBuffer *output) {
  LSTD_ASSERT(bitstream != NULL);
  LSTD_ASSERT(output != NULL);
  LSTD_ASSERT(length_literal_table != NULL);
  LSTD_ASSERT(distance_table != NULL);
  static const uint16_t length_size_base[29] = {
      3,  4,  5,  6,  7,  8,  9,  10, 11,  13,  15,  17,  19,  23, 27,
      31, 35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258};
  static const uint16_t length_extra_bits[29] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
                                                 1, 1, 2, 2, 2, 2, 3, 3, 3, 3,
                                                 4, 4, 4, 4, 5, 5, 5, 5, 0};
  static const uint16_t distance_offset_base[30] = {
      1,    2,    3,    4,    5,    7,    9,    13,    17,    25,
      33,   49,   65,   97,   129,  193,  257,  385,   513,   769,
      1025, 1537, 2049, 3073, 4097, 6145, 8193, 12289, 16385, 24577};
  static const uint16_t distance_extra_bits[30] = {
      0, 0, 0, 0, 1, 1, 2, 2,  3,  3,  4,  4,  5,  5,  6,
      6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13};

  int symbol = 0;
  while (symbol != 256) {
    symbol = huffman_table_decode(length_literal_table->symbols,
                                  length_literal_table->counts, bitstream);
    if (symbol < 0) {
      LOG0_ERROR("Unknown symbol decoded");
      return false;
    }
    if (symbol < 256) {
      OutputBuffer_push(output, symbol);
    } else if (symbol > 256) {
      symbol -= 257;
      if (symbol >= 29) {
        return false;
      }

      int length = length_size_base[symbol] +
                   Bitstream_next_bits(bitstream, length_extra_bits[symbol]);
      symbol = huffman_table_decode(distance_table->symbols,
                                    distance_table->counts, bitstream);
      if (symbol < 0) {
        return false;
      }

      int distance_increment =
          Bitstream_next_bits(bitstream, distance_extra_bits[symbol]);
      unsigned distance = distance_offset_base[symbol] + distance_increment;
      while (length--) {
        size_t output_buffer_length = OutputBuffer_length(output);
        OutputBuffer_push(output,
                          output->buffer[output_buffer_length - distance]);
      }
    }
  }

  return true;
}

bool deflate_decompress(Bitstream *bitstream, OutputBuffer *output) {
  LSTD_ASSERT(bitstream != NULL);
  LSTD_ASSERT(output != NULL);

  uint8_t b_final = 0;
  while (!b_final) {
    LOG0_DEBUG("Parse deflate block");
    b_final = Bitstream_next_bits(bitstream, BFINAL_LENGTH_BITS);
    LOG_DEBUG("Final block: %d", b_final);
    const uint8_t b_type = Bitstream_next_bits(bitstream, BTYPE_LENGTH_BITS);
    if (b_type == DeflateBlockType_NoCompression) {
      LOG0_ERROR("Uncompressed deflate blocks aren't supported");
      abort();
    } else {
      if (b_type == DeflateBlockType_FixedHuffman) {
        LOG0_DEBUG("Static huffman table");
        static bool are_tables_blank = true;
        static LengthLiteralTable length_literal_table = {0};
        static DistanceTable distance_table = {0};

        uint16_t lenlit_codelengths[FIXED_LENGTH_LITERAL_CODE_COUNT];

        if (are_tables_blank) {
          LOG0_DEBUG("Computing static huffman table");
          for (int symbol = 0; symbol < 144; symbol++) {
            lenlit_codelengths[symbol] = 8;
          }

          for (int symbol = 144; symbol < 256; symbol++) {
            lenlit_codelengths[symbol] = 9;
          }

          for (int symbol = 256; symbol < 280; symbol++) {
            lenlit_codelengths[symbol] = 7;
          }

          for (int symbol = 280; symbol < FIXED_LENGTH_LITERAL_CODE_COUNT;
               symbol++) {
            lenlit_codelengths[symbol] = 8;
          }

          build_huffman_table_from_codelengths(
              length_literal_table.symbols, length_literal_table.counts,
              lenlit_codelengths, FIXED_LENGTH_LITERAL_CODE_COUNT);

          uint16_t distance_codelengths[MAX_DISTANCE_CODES] = {0};
          for (int symbol = 0; symbol < MAX_DISTANCE_CODES; symbol++) {
            distance_codelengths[symbol] = 5;
          }

          build_huffman_table_from_codelengths(
              distance_table.symbols, distance_table.counts,
              distance_codelengths, MAX_DISTANCE_CODES);
          are_tables_blank = false;
        }
        deflate_decompress_(bitstream, &length_literal_table, &distance_table,
                            output);
      } else {
        LengthLiteralTable length_literal_table = {0};
        DistanceTable distance_table = {0};
        const uint16_t hlit = Bitstream_next_bits(bitstream, 5) + 257;
        const uint8_t hdist = Bitstream_next_bits(bitstream, 5) + 1;
        const uint8_t hclen = Bitstream_next_bits(bitstream, 4) + 4;
        LOG_DEBUG("Dynamic table:\n"
                  "hlit: %d\n"
                  "hdist: %d\n"
                  "hclen: %d",
                  hlit, hdist, hclen);
        CodeLengthTable codelength_table = {0};
        build_codelength_table(&codelength_table, bitstream, hclen);

        uint16_t lenlit_dist_codelengths[MAX_LENGTH_CODES +
                                         MAX_DISTANCE_CODES] = {0};
        int index = 0;
        while (index < hlit + hdist) {
          int symbol = huffman_table_decode(codelength_table.symbols,
                                            codelength_table.counts, bitstream);
          if (symbol < 0) {
            LOG0_ERROR("Unknown symbol decoded using huffman table");
            return false;
          } else if (symbol < 16) {
            lenlit_dist_codelengths[index++] = symbol;
          } else {
            int repeat = 0;
            int codelength = 0;
            if (symbol == 16) {
              repeat = 3 + Bitstream_next_bits(bitstream, 2);
              codelength = lenlit_dist_codelengths[index - 1];
            } else if (symbol == 17) {
              repeat = 3 + Bitstream_next_bits(bitstream, 3);
            } else {
              repeat = 11 + Bitstream_next_bits(bitstream, 7);
            }

            for (int i = 0; i < repeat; i++) {
              lenlit_dist_codelengths[index++] = codelength;
            }
          }
        }

        // The block must have an end
        LSTD_ASSERT(lenlit_dist_codelengths[256] > 0);

        build_huffman_table_from_codelengths(length_literal_table.symbols,
                                             length_literal_table.counts,
                                             lenlit_dist_codelengths, hlit);
        build_huffman_table_from_codelengths(
            distance_table.symbols, distance_table.counts,
            lenlit_dist_codelengths + hlit, hdist);

        deflate_decompress_(bitstream, &length_literal_table, &distance_table,
                            output);
      }
    }
  }
  return true;
}

uint8_t *zlib_decompress(const uint8_t *compressed_data_buffer,
                         const size_t compressed_data_length,
                         size_t *output_length) {
  LSTD_ASSERT(compressed_data_buffer != NULL);

  Bitstream bitstream;
  Bitstream_init(&bitstream, compressed_data_buffer, compressed_data_length);

  uint16_t cm = Bitstream_next_bits(&bitstream, CM_LENGTH_BITS);
  uint16_t cinfo = Bitstream_next_bits(&bitstream, CINFO_LENGTH_BITS);
  uint16_t fcheck = Bitstream_next_bits(&bitstream, FCHECK_LENGTH_BITS);
  uint16_t fdict = Bitstream_next_bits(&bitstream, FDICT_LENGTH_BITS);
  uint16_t flevel = Bitstream_next_bits(&bitstream, FLEVEL_LENGTH_BITS);

#ifdef LPNG_DEBUG_LOG
  LOG_DEBUG("zlib informations:\n"
            "- CM (compression method): %d\n"
            "- CINFO (compression info): %d\n"
            "- FCHECK (check bits for CMF and FLG): %d\n"
            "- FDICT (preset dictionary): %d\n"
            "- FLEVEL (compression level): %d",
            cm, cinfo, fcheck, fdict, flevel);
#else
  (void)cm;
  (void)cinfo;
  (void)fcheck;
#endif // LPNG_DEBUG_LOG

  uint16_t cmf = bitstream.data[0];
  uint16_t flg = bitstream.data[1];
  if ((cmf * 256 + flg) % 31 != 0) {
    LOG0_ERROR("fcheck validation failed");
    return false;
  }

  if (flevel > 9) {
    LOG0_ERROR("Invalid compression level");
    return NULL;
  }

  if (fdict != 0) {
    LOG0_ERROR("preset dictionnaries are unsupported");
    return NULL;
  }

  OutputBuffer output;
  OutputBuffer_init(&output);
  if (!deflate_decompress(&bitstream, &output)) {
    LOG0_ERROR("deflate decompression failed");
    return NULL;
  }

  size_t adler32_offset = bitstream.byte_offset;
  if (bitstream.bit_index % 8 != 0) {
    adler32_offset++;
  }

  uint32_t adler32 = bitstream.data[adler32_offset + 3] +
                     (bitstream.data[adler32_offset + 2] << 8) +
                     (bitstream.data[adler32_offset + 1] << 16) +
                     (bitstream.data[adler32_offset] << 24);
  LOG_DEBUG("Adler32 checksum: %u", adler32);
  uint32_t a = 1;
  uint32_t b = 0;
  for (size_t i = 0; i < output.len; i++) {
    a = (a + (uint8_t)output.buffer[i]) % 65521;
    b = (b + a) % 65521;
  }
  uint32_t computed_adler32 = (b << 16) | a;
  LOG_DEBUG("Computed Adler32 checksum: %u", computed_adler32);
  if (adler32 != computed_adler32) {
    LOG0_ERROR("Invalid checksum");
    exit(1);
  }

  *output_length = output.len;
  return output.buffer;
}
