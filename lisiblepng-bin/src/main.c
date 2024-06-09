#include <errno.h>
#include <fcntl.h>
#include <lisiblepng.h>
#include <lisiblestd/log.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>

#define LOG0(msg) fprintf(stderr, msg "\n")
#define LOGN(fmt, ...) fprintf(stderr, fmt "\n", __VA_ARGS__)

int main(int argc, char **argv) {
  lstd_log_init();
  if (argc != 2) {
    LOG0("Usage: lisiblepng <png filepath>");
    return 1;
  }

  const char *png_filepath = argv[1];
  int fd = open(png_filepath, O_RDONLY);
  struct stat st;
  fstat(fd, &st);
  u8 *png_file_data = mmap(NULL, st.st_size, PROT_READ, MAP_SHARED, fd, 0);
  LOGN("File size: %ld bytes", st.st_size);

  if (!png_file_data) {
    const char *error_message = strerror(errno);
    LOGN("Couldn't map PNG file: %s", error_message);
    return 1;
  }

  LisPng *png = LisPng_decode(png_file_data, st.st_size);
  if (!png) {
    LOG0("Couldn't decode PNG");
    return 1;
  }

  LisPng_dump_ppm(png);
  LisPng_destroy(png);

  if (munmap(png_file_data, st.st_size) != 0) {
    const char *error_message = strerror(errno);
    LOGN("Couldn't unmap PNG file: %s", error_message);
    return 1;
  }

  return 0;
}
