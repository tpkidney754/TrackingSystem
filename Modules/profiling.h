#ifndef __PROFILING__
#define __PROFILING__

#include "includeall.h"

#define NSEC_PER_SEC        1000000000
#define TEST_ITERATIONS     1000
#define NUM_TESTS           4
typedef struct timespec timespec_t;

void PRO_delta(timespec_t *start, timespec_t *stop, timespec_t *delta_t);
void PRO_RunProfilingSuite(void);
#endif //__PROFILING__
