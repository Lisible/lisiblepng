#include <errno.h>
#include <lisiblepng.h>
#include <stdio.h>
#include <string.h>

#define LOG0(msg) fprintf(stderr, msg)
#define LOGN(fmt, ...) fprintf(stderr, fmt, __VA_ARGS__)

int main(int argc, char **argv) {
  if (argc != 2) {
    LOG0("Usage: lisiblepng <png filepath>");
    return 1;
  }

  const char *png_filepath = argv[1];
  FILE *png_file = fopen(png_filepath, "r");
  if (!png_file) {
    const char *error_message = strerror(errno);
    LOGN("Couldn't open PNG file: %s", error_message);
    goto err;
  }

  Png *png = lis_Png_decode(png_file);
  lis_Png_dump_ppm(png);
  lis_Png_destroy(png);

  if (fclose(png_file) != 0) {
    const char *error_message = strerror(errno);
    LOGN("Couldn't close PNG file: %s", error_message);
    goto err;
  }

  return 0;
err:
  return 1;
}
