#ifndef LISIBLE_PNG_LOG_H
#define LISIBLE_PNG_LOG_H

#include <stdio.h>

#define LOG0(fmt) fprintf(stderr, fmt "\n")
#define LOGN(fmt, ...) fprintf(stderr, fmt "\n", __VA_ARGS__)

#endif // LISIBLE_PNG_LOG_H
