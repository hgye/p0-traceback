/**
 * Get the frame pointer and the entry in functions table of caller of traceback
 */

#ifndef GET_NEXT_FUNC_H
#define GET_NEXT_FUNC_H

#include "traceback_internal.h"

typedef struct funcframe_t {
    void *ebp;  /* frame pointer */
    const functsym_t* pfunc;
} funcframe_t;

void get_next_func(funcframe_t *funcframe);

#endif

