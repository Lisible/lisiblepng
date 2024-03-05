#ifndef LISIBLE_PNG_ASSET_H
#define LISIBLE_PNG_ASSET_H

#include "log.h"

#define ASSERT(predicate)                                                      \
  do {                                                                         \
    if (!(predicate)) {                                                        \
      LPNG_LOG_ERR("Assertion failed in %s:%d:\n\t%s", __FILE__, __LINE__,     \
                   #predicate);                                                \
      exit(1);                                                                 \
    }                                                                          \
  } while (0)

#endif // LISIBLE_PNG_ASSET_H
