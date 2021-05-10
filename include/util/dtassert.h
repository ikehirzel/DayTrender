#ifndef DTASSERT_H
#define DTASSERT_H

#include <assert.h>

#define TEST_MODE 0

#if TEST_MODE
#define dt_assert(x) assert(x)
#else
#define dt_assert(x) x 
#endif

#endif
