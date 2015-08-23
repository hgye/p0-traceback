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
    /* 如果是第一次调用，那么staticebp为0，用traceback caller的%ebp来初始化 */
    cmpl $0x0, staticebp /* check if it's first time called by traceback */
    ja not_first    /* staticebp里存的是栈地址，都是0xffff开头的，如果用符号比较的话永远都是小于0 */
    movl %ebp, %edx /* 这个%ebp就是traceback的%ebp */
    movl %edx, staticebp
not_first:
    movl staticebp, %edx
    movl 4(%edx), %ecx /* get return address of traceback caller */
    pushl %ecx
    call find_caller  /* here should return caller of traceback */
    addl $0x4, %esp   /* IMPORTANT: pop the arguments pushed for find_caller(), or
                         the argument would be the return address of get_next_func() */

    movl (%edx), %ecx /* 更新staticebp成bar()的%ebp */
    movl %ecx, staticebp
    ret

