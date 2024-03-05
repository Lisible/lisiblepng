#ifndef LISIBLE_PNG_LOG_H
#define LISIBLE_PNG_LOG_H

#include <stdio.h>

#define LPNG_LOG_PREFIX "LPNG: "
#define LPNG_DEBUG_LOG_PREFIX "%s:%d " LPNG_LOG_PREFIX
#define LPNG_LOG_ERR0(fmt) fprintf(stderr, LPNG_LOG_PREFIX fmt "\n")
#define LPNG_LOG_ERR(fmt, ...)                                                 \
  fprintf(stderr, LPNG_LOG_PREFIX fmt "\n", __VA_ARGS__)

#ifdef LPNG_DEBUG_LOG
#define LPNG_LOG_DBG0(fmt)                                                     \
  fprintf(stderr, LPNG_DEBUG_LOG_PREFIX fmt "\n", __FILE__, __LINE__)
#define LPNG_LOG_DBG(fmt, ...)                                                 \
  fprintf(stderr, LPNG_DEBUG_LOG_PREFIX fmt "\n", __FILE__, __LINE__,          \
          __VA_ARGS__)
#else
#define LPNG_LOG_DBG0(fmt)
#define LPNG_LOG_DBG(fmt, ...)
#endif // LPNG_DEBUG_LOG

#endif // LISIBLE_PNG_LOG_H
