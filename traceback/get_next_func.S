/**
 * Get the next caller entry in functions array.
 * This function is non-reentrant
 * @date: 2015-8-23
 */

.global get_next_func
.global find_caller


/* declare staticebp in .bss section */
.local staticebp
.comm  staticebp,4,4

get_next_func:
    /* use traceback's %ebp to initialize staticebp if it's the first time calling get_next_func */
    cmpl $0x0, staticebp /* check if it's first time called by traceback */
    ja not_first    /* here must use unsigned jump, since staticebp contains stack address that is always negative */
    movl %ebp, %edx
    movl %edx, staticebp
not_first:

    movl staticebp, %edx
    movl 4(%esp), %eax  /* get the struct argument */
    movl (%edx), %ecx
    movl %ecx, (%eax)  /* save caller frame pointer to argument */


    movl 4(%edx), %ecx /* get return address of traceback caller */
    pushl %ecx
    call find_caller  /* here should return caller of traceback */
    addl $0x4, %esp   /* IMPORTANT: pop the arguments pushed for find_caller(), or
                         the argument would be the return address of get_next_func() */

    movl 4(%esp), %edx
    movl %eax, 4(%edx) /* save functsym_t* to argument */

    movl staticebp, %edx
    movl (%edx), %ecx /* update staticebp to next caller's %ebp */
    movl %ecx, staticebp
    ret

