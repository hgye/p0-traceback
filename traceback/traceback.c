/** @file traceback.c
 *  @brief The traceback function
 *
 *  This file contains the traceback function for the traceback library
 *
 *  @author Harry Q. Bovik (hqbovik)
 *  @bug Unimplemented
 */

#include "traceback_internal.h"
#include "get_next_func.h"
#include <string.h>

void traceback(FILE *fp)
{

	/* the following just makes a sample access to "functions" array.
	 * note if "functions" is not referenced somewhere outside the
	 * file that it's declared in, symtabgen won't be able to find
	 * the symbol. So be sure to always do something with functions */

    const functsym_t *pfunc = NULL;
    do {
        pfunc = get_next_func();
        if (NULL == pfunc)
            break;

        fprintf(fp, "name: %s, addr: %p\n", pfunc->name, pfunc->addr);

    } while (strncmp("main", pfunc->name, 4) != 0);
}


