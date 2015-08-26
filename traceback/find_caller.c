#include "find_caller.h"
#include <string.h>
#include <stdint.h>
extern const functsym_t functions[FUNCTS_MAX_NUM];


/**
 * Find the owner function of the return address
 * TODO: need to check if *addr* is a valid return address
 */
const functsym_t* find_caller(void *addr)
{
    int i;
    /* since target architecture is 32bit, use unsigned int to hold address is ok */
    uint32_t funcaddr;
    uint32_t maxaddr = 0;

    for (i = 0; i < FUNCTS_MAX_NUM; ++i) {
        if (strlen(functions[i].name) == 0)
            break;

        funcaddr = (uint32_t)(functions[i].addr);
        if (funcaddr < (uint32_t)addr)
            maxaddr = funcaddr;
        else
            return (i > 0 && maxaddr != 0) ? &functions[i - 1] : NULL;
    }

    return NULL;
}

