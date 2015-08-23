#include "find_caller.h"
#include <string.h>
#include <stdint.h>
extern const functsym_t functions[FUNCTS_MAX_NUM];


/**
 * find the owner function of the return address
 */
const functsym_t* find_caller(void *addr)
{
    int i;
    /* 编译的时候指定了-m32，所以是32位地址，用32位整数足以 */
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

