/**
 * 根据返回地址在functions数组里找到对应的函数
 * used by get_next_func()
 */

#ifndef FIND_CALLER_H
#define FIND_CALLER_H

#include "traceback_internal.h"

/**
 * Find the function which owns the return address.
 * @param addr a return address
 * @return pointer to a non-modified funcsym_t object
 */
const functsym_t* find_caller(void *addr);

#endif
