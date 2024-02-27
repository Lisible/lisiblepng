#include "lisiblepng.h"
#include "assert.h"
#include "deflate.h"
#include "log.h"
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PNG_SIGNATURE_LENGTH 8
const uint8_t PNG_SIGNATURE[PNG_SIGNATURE_LENGTH] = {0x89, 0x50, 0x4E, 0x47,
                                                     0x0D, 0x0A, 0x1A, 0x0A};
#define IHDR_CHUNK_TYPE 0x49484452
#define IEND_CHUNK_TYPE 0x49454e44
#define IDAT_CHUNK_TYPE 0x49444154

const uint32_t CRC32_TABLE[256] = {
    0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA, 0x076DC419, 0x706AF48F,
    0xE963A535, 0x9E6495A3, 0x0EDB8832, 0x79DCB8A4, 0xE0D5E91E, 0x97D2D988,
    0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91, 0x1DB71064, 0x6AB020F2,
    0xF3B97148, 0x84BE41DE, 0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7,
    0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC, 0x14015C4F, 0x63066CD9,
    0xFA0F3D63, 0x8D080DF5, 0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172,
    0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B, 0x35B5A8FA, 0x42B2986C,
    0xDBBBC9D6, 0xACBCF940, 0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59,
    0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116, 0x21B4F4B5, 0x56B3C423,
    0xCFBA9599, 0xB8BDA50F, 0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924,
    0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D, 0x76DC4190, 0x01DB7106,
    0x98D220BC, 0xEFD5102A, 0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433,
    0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818, 0x7F6A0DBB, 0x086D3D2D,
    0x91646C97, 0xE6635C01, 0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E,
    0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457, 0x65B0D9C6, 0x12B7E950,
    0x8BBEB8EA, 0xFCB9887C, 0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65,
    0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2, 0x4ADFA541, 0x3DD895D7,
    0xA4D1C46D, 0xD3D6F4FB, 0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0,
    0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9, 0x5005713C, 0x270241AA,
    0xBE0B1010, 0xC90C2086, 0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F,
    0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4, 0x59B33D17, 0x2EB40D81,
    0xB7BD5C3B, 0xC0BA6CAD, 0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A,
    0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683, 0xE3630B12, 0x94643B84,
    0x0D6D6A3E, 0x7A6A5AA8, 0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1,
    0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE, 0xF762575D, 0x806567CB,
    0x196C3671, 0x6E6B06E7, 0xFED41B76, 0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC,
    0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5, 0xD6D6A3E8, 0xA1D1937E,
    0x38D8C2C4, 0x4FDFF252, 0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B,
    0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60, 0xDF60EFC3, 0xA867DF55,
    0x316E8EEF, 0x4669BE79, 0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236,
    0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F, 0xC5BA3BBE, 0xB2BD0B28,
    0x2BB45A92, 0x5CB36A04, 0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D,
    0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A, 0x9C0906A9, 0xEB0E363F,
    0x72076785, 0x05005713, 0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38,
    0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21, 0x86D3D2D4, 0xF1D4E242,
    0x68DDB3F8, 0x1FDA836E, 0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777,
    0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C, 0x8F659EFF, 0xF862AE69,
    0x616BFFD3, 0x166CCF45, 0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2,
    0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB, 0xAED16A4A, 0xD9D65ADC,
    0x40DF0B66, 0x37D83BF0, 0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9,
    0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6, 0xBAD03605, 0xCDD70693,
    0x54DE5729, 0x23D967BF, 0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94,
    0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D};

struct Png {
  const uint8_t *data;
};

typedef struct {
  FILE *stream;
#ifdef LISIBLE_PNG_COMPUTE_CRC
  uint32_t computed_crc;
#endif // LISIBLE_PNG_COMPUTE_CRC
} DeflateDecompressor;

void DeflateDecompressor_init(DeflateDecompressor *ctx, FILE *stream) {
  ASSERT(ctx != NULL);
  ASSERT(stream != NULL);
  ctx->stream = stream;
#ifdef LISIBLE_PNG_COMPUTE_CRC
  ctx->computed_crc = 0xFFFFFFFFu;
#endif // LISIBLE_PNG_COMPUTE_CRC
}

void ParsingContext_crc_reset(DeflateDecompressor *ctx) {
  ASSERT(ctx != NULL);
#ifdef LISIBLE_PNG_COMPUTE_CRC
  ctx->computed_crc = 0xFFFFFFFFu;
#endif // LISIBLE_PNG_COMPUTE_CRC
}

#ifdef LISIBLE_PNG_COMPUTE_CRC
uint32_t ParsingContext_computed_crc(DeflateDecompressor *ctx) {
  ASSERT(ctx != NULL);
  return ctx->computed_crc ^ 0xFFFFFFFFu;
}
#endif // LISIBLE_PNG_COMPUTE_CRC

long ParsingContext_cursor_position(DeflateDecompressor *ctx) {
  ASSERT(ctx != NULL);
  return ftell(ctx->stream);
}

bool ParsingContext_skip_bytes(DeflateDecompressor *ctx, size_t byte_count) {
  ASSERT(ctx != NULL);
  if (fseek(ctx->stream, byte_count, SEEK_CUR) != 0) {
    LOGN("Couldn't skip bytes: %s", strerror(errno));
    return false;
  }

  return true;
}

bool ParsingContext_parse_bytes(DeflateDecompressor *ctx, size_t byte_count,
                                uint8_t *output_buffer) {
  ASSERT(ctx != NULL);
  ASSERT(output_buffer != NULL);
  if (fread(output_buffer, 1, byte_count, ctx->stream) < byte_count) {
    LOG0("Couldn't parse bytes, EOF reached");
    return false;
  }

#ifdef LISIBLE_PNG_COMPUTE_CRC
  for (size_t i = 0; i < byte_count; i++) {
    const uint32_t index = (ctx->computed_crc ^ output_buffer[i]) & 0xFF;
    ctx->computed_crc = (ctx->computed_crc >> 8) ^ CRC32_TABLE[index];
  }
#endif // LISIBLE_PNG_COMPUTE_CRC

  return true;
}

bool ParsingContext_parse_uint32_t(DeflateDecompressor *ctx,
                                   uint32_t *output_u32) {
  ASSERT(ctx != NULL);
  ASSERT(output_u32 != NULL);
  uint8_t bytes[4];
  if (!ParsingContext_parse_bytes(ctx, 4, bytes)) {
    return false;
  }
  *output_u32 =
      (bytes[0] << 24) + (bytes[1] << 16) + (bytes[2] << 8) + bytes[3];
  return true;
}

bool ParsingContext_parse_uint8_t(DeflateDecompressor *ctx,
                                  uint8_t *output_u8) {
  ASSERT(ctx != NULL);
  ASSERT(output_u8 != NULL);
  if (!ParsingContext_parse_bytes(ctx, 1, output_u8)) {
    return false;
  }
  return true;
}

bool matches_png_signature(uint8_t signature[PNG_SIGNATURE_LENGTH]) {
  return memcmp(signature, PNG_SIGNATURE, PNG_SIGNATURE_LENGTH) == 0;
}

#define PARSE_FIELD(type, field)                                               \
  do {                                                                         \
    if (!ParsingContext_parse_##type(ctx, &field)) {                           \
      return false;                                                            \
    }                                                                          \
  } while (0)

typedef struct {
  uint32_t width;
  uint32_t height;
  uint8_t bit_depth;
  uint8_t colour_type;
  uint8_t compression_method;
  uint8_t filter_method;
  uint8_t interlace_method;
} ImageHeader;

typedef struct {
  size_t length;
  uint8_t *data;
} ImageData;

void ImageHeader_print_image_header(const ImageHeader *image_header) {
  ASSERT(image_header != NULL);
  LOGN("Image header:\n"
       "- dimensions: %dx%d\n"
       "- bit depth: %d\n"
       "- colour type: %d\n"
       "- compression method: %d\n"
       "- filter method: %d\n"
       "- interlace method: %d",
       image_header->width, image_header->height, image_header->bit_depth,
       image_header->colour_type, image_header->compression_method,
       image_header->filter_method, image_header->interlace_method);
}

bool ParsingContext_validate_crc_if_required(DeflateDecompressor *ctx) {
#ifdef LISIBLE_PNG_COMPUTE_CRC
  uint32_t computed_crc = ParsingContext_computed_crc(ctx);
  uint32_t crc;
  PARSE_FIELD(uint32_t, crc);
  if (computed_crc != crc) {
    LOG0("Invalid CRC checksum");
    return false;
  }
#else
  ParsingContext_skip_bytes(ctx, sizeof(uint32_t));
#endif

  return true;
}

bool parse_IHDR_chunk(DeflateDecompressor *ctx, ImageHeader *image_header) {
  ASSERT(ctx != NULL);
  ASSERT(image_header != NULL);

  uint32_t length;
  PARSE_FIELD(uint32_t, length);

  ParsingContext_crc_reset(ctx);

  uint32_t type;
  PARSE_FIELD(uint32_t, type);
  if (type != IHDR_CHUNK_TYPE) {
    LOG0("Expected IHDR chunk");
    return false;
  }

  long data_start = ParsingContext_cursor_position(ctx);
  PARSE_FIELD(uint32_t, image_header->width);
  PARSE_FIELD(uint32_t, image_header->height);
  PARSE_FIELD(uint8_t, image_header->bit_depth);
  PARSE_FIELD(uint8_t, image_header->colour_type);
  PARSE_FIELD(uint8_t, image_header->compression_method);
  PARSE_FIELD(uint8_t, image_header->filter_method);
  PARSE_FIELD(uint8_t, image_header->interlace_method);
  long read_data_length = ParsingContext_cursor_position(ctx) - data_start;
  ASSERT(read_data_length == length);

  if (!ParsingContext_validate_crc_if_required(ctx)) {
    return false;
  }

  return true;
}

bool parse_IDAT_chunk(DeflateDecompressor *ctx, uint32_t data_length,
                      ImageData *image_data) {
  ASSERT(ctx != NULL);
  ASSERT(image_data != NULL);

  image_data->data = realloc(image_data->data, data_length);
  ParsingContext_parse_bytes(ctx, data_length,
                             &image_data->data[image_data->length]);
  image_data->length = data_length;

  if (!ParsingContext_validate_crc_if_required(ctx)) {
    return false;
  }

  return true;
}

uint32_t uint32_t_to_le(uint32_t value) {
  char *value_bytes = (char *)&value;
  return (value_bytes[0] << 24) + (value_bytes[1] << 16) +
         (value_bytes[2] << 8) + value_bytes[3];
}

Png *lis_Png_parse(FILE *stream) {
  Png *png = malloc(sizeof(Png));
  DeflateDecompressor ctx;

  DeflateDecompressor_init(&ctx, stream);
  uint8_t parsed_png_signature[PNG_SIGNATURE_LENGTH];
  if (!ParsingContext_parse_bytes(&ctx, PNG_SIGNATURE_LENGTH,
                                  parsed_png_signature)) {
    LOG0("Couldn't parse signature");
    goto err;
  }

  if (!matches_png_signature(parsed_png_signature)) {
    LOG0("Invalid signature");
    goto err;
  }

  ImageHeader header;
  if (!parse_IHDR_chunk(&ctx, &header)) {
    goto err;
  }
  ImageHeader_print_image_header(&header);

  ImageData image_data = {0};
  bool end_reached = false;
  while (!end_reached) {
    uint32_t length;
    if (!ParsingContext_parse_uint32_t(&ctx, &length)) {
      LOG0("Couldn't parse chunk length");
      goto cleanup_data;
    }

    ParsingContext_crc_reset(&ctx);
    uint32_t type;
    if (!ParsingContext_parse_uint32_t(&ctx, &type)) {
      LOG0("Couldn't parse chunk type");
      goto cleanup_data;
    }

    uint32_t type_le = uint32_t_to_le(type);
    LOGN("Parsing %.4s chunk", (char *)&type_le);
    switch (type) {
    case IDAT_CHUNK_TYPE:
      parse_IDAT_chunk(&ctx, length, &image_data);
      break;
    case IEND_CHUNK_TYPE:
      end_reached = true;
      ParsingContext_skip_bytes(&ctx, sizeof(uint32_t));
      break;
    default:
      LOG0("Unknown chunk type, skipping chunk...");
      ParsingContext_skip_bytes(&ctx, length + sizeof(uint32_t));
      break;
    }
  }

  LOGN("Data length: %zul", image_data.length);
  zlib_decompress(image_data.data, image_data.length);

  return png;

cleanup_data:
  free(image_data.data);
err:
  return NULL;
}

#undef PARSE_FIELD
void lis_Png_destroy(Png *png) { free(png); }
void lis_Png_dump_ppm(const Png *png) { ASSERT(png != NULL); }
