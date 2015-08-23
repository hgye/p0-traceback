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

void print_func(FILE *fp, funcframe_t *funcframe);

/**
 * traceback一般是放在SIGSEGV的signal handler里调用的，
 * 所以traceback里必须使用async-signal-safe的function
 */
void traceback(FILE *fp)
{

	/* the following just makes a sample access to "functions" array.
	 * note if "functions" is not referenced somewhere outside the
	 * file that it's declared in, symtabgen won't be able to find
	 * the symbol. So be sure to always do something with functions */

    funcframe_t funcframe;
    funcframe_t *pframe = &funcframe;
    do {
        get_next_func(pframe);
        print_func(fp, pframe);

    } while (strncmp("main", pframe->pfunc->name, 4) != 0); 
}


void print_func(FILE *fp, funcframe_t *pframe)
{
    fprintf(fp, "name: %s, addr: %p, ebp: %p\n", pframe->pfunc->name, pframe->pfunc->addr, pframe->ebp);
}


