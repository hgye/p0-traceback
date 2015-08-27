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
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <errno.h>
#include <time.h>

#define ARG_BUF_LEN 256
#define LINE_BUF_LEN 2048
#define STR_BUF_LEN 35

#define STR_LEN_LIMIT 25
#define STR_ARRAY_LIMIT 4

void print_func(int fd, funcframe_t *pframe);
int is_printable(const char *s, int* toolong);
void proc_arg(char *buf, const int len, const argsym_t *arg, void *ebp);
void proc_str(char *buf, const int len, const char *name, char *str);
void proc_str_arr(char *buf, const int len, const char *name, char **ss);

int check_pointer(const void *p);
static int g_tmpfd = -1;    /* fd used by check_pointer */

void fatal(int fd, char *msg, int err)
{
    char linebuf[1024] = {0};
    snprintf(linebuf, 1024, "FATAL: %s:%s\n", msg, strerror(err));
    write(fd, linebuf, strlen(linebuf));
}

/**
 * traceback一般是放在SIGSEGV的signal handler里调用的，
 * 所以traceback里必须使用async-signal-safe的function
 */
void traceback(FILE *fp)
{
    char tmpfile[40] = {0};
    int fd;

    if ((fd = fileno(fp)) < 0) {
        fprintf(fp, "FATAL: fp to descriptor fail: %s\n", strerror(errno));
        return;
    }

    snprintf(tmpfile, 50, "/tmp/traceback-tmpfile-%d", getuid());
    g_tmpfd = open(tmpfile, O_CREAT | O_TRUNC | O_RDWR, 0644);
    if (g_tmpfd < 0) {
        fatal(fd, "cannot open temp file", errno);
        return;
    }

    funcframe_t funcframe;
    funcframe_t *pframe = &funcframe;

    do {
        get_next_func(pframe);
        print_func(fd, pframe);

    } while (pframe->pfunc == NULL || strncmp("main", pframe->pfunc->name, 4) != 0);
}

/**
 * print the function name and its arguments
 */
void print_func(int fd, funcframe_t *pframe)
{
    char linebuf[LINE_BUF_LEN] = {0};
    char argbuf[ARG_BUF_LEN] = {0};
    const argsym_t *arg;
    int i, buflen;


    if (NULL == pframe->pfunc) {
        snprintf(linebuf, LINE_BUF_LEN, "Function %p(...), in\n", pframe->retaddr);
        write(fd, linebuf, strlen(linebuf));
        return;
    }

    snprintf(linebuf, LINE_BUF_LEN, "Function %s(", pframe->pfunc->name);
    for (i = 0; i < ARGS_MAX_NUM; i++) {
        arg = pframe->pfunc->args + i;
        if (strlen(arg->name) == 0)
            break;

        proc_arg(argbuf, ARG_BUF_LEN, arg, pframe->ebp);
        buflen = strlen(linebuf);
        snprintf(linebuf + buflen, LINE_BUF_LEN - buflen, (i > 0 ? ", %s" : "%s"), argbuf);
    }
    snprintf(linebuf + strlen(linebuf), LINE_BUF_LEN - strlen(linebuf), i == 0 ? "void), in\n" : "), in\n");

    write(fd, linebuf, strlen(linebuf));
}

void proc_arg(char *buf, const int len, const argsym_t *arg, void *ebp)
{
    char c;
    int i;
    float f;
    double d;
    char *str, **ss;

    switch (arg->type) {
        case TYPE_CHAR:
            c = *(char*)(ebp + arg->offset);
            if (isprint(c))
                snprintf(buf, len, "char %s='%c'", arg->name, c);
            else
                snprintf(buf, len, "char %s='\\%o'", arg->name, c);
            break;
        case TYPE_INT:
            i = *(int*)(ebp + arg->offset);
            snprintf(buf, len, "int %s=%d", arg->name, i);
            break;
        case TYPE_FLOAT:
            f = *(float*)(ebp + arg->offset);
            snprintf(buf, len, "float %s=%f", arg->name, f);
            break;
        case TYPE_DOUBLE:
            d = *(double*)(ebp + arg->offset);
            snprintf(buf, len, "double %s=%f", arg->name, d);
            break;
        case TYPE_STRING:
            str = *(char**)(ebp + arg->offset);
            proc_str(buf, len, arg->name, str);
            break;
        case TYPE_STRING_ARRAY:
            ss = *(char***)(ebp + arg->offset);
            proc_str_arr(buf, len, arg->name, ss);
            break;
        case TYPE_VOIDSTAR:
            snprintf(buf, len, "void *%s=0v%x", arg->name, (uint32_t)(ebp + arg->offset));
            break;
        case TYPE_UNKNOWN:
        default:
            snprintf(buf, len, "UNKNOWN %s=%p", arg->name, (ebp + arg->offset));
            break;
    }

}


void proc_str(char *buf, const int len, const char *name, char *str)
{
    int tmp, too_long = 0;

    if (NULL != name) { /* for string */
        if (!is_printable(str, &too_long))
            snprintf(buf, len, "char *%s=%p", name, str);
        else {
            if (!too_long) {
                snprintf(buf, len, "char *%s=\"%s\"", name, str);
            } else {
                tmp = strlen("char *") + strlen(name) + 3; /* ="\0 */
                snprintf(buf, tmp, "char *%s=\"", name);
                snprintf(buf + strlen(buf), STR_LEN_LIMIT + 1, "%s", str);
                snprintf(buf + strlen(buf), len - strlen(buf), "...\"");
            }
        }
    } else {    /* for string array */
        if (!is_printable(str, &too_long))
            snprintf(buf, len, "\"%p\"", str);
        else {
            if (!too_long) {
                snprintf(buf, len, "\"%s\"", str);
            } else {
                snprintf(buf + strlen(buf), STR_LEN_LIMIT + 2, "\"%s", str);
                snprintf(buf + strlen(buf), len - strlen(buf), "...\"");
            }
        }
    }
}

void proc_str_arr(char *buf, const int len, const char *name, char **ss)
{
    int i;
    char *str, *next;
    char strbuf[STR_BUF_LEN] = {0};

    if (NULL == ss || !check_pointer(ss)) {
        snprintf(buf, len, "char **%s=%p", name, ss);
        return;
    }

    snprintf(buf, len, "char **%s={", name);
    /* only 3 elements needed to print */
    for (i = 0; i < 3; i++) {
        str = ss[i];
        next = ss[i + 1];
        proc_str(strbuf, STR_BUF_LEN, NULL, str);
        snprintf(buf + strlen(buf), len - strlen(buf), (i > 0 ? ", %s" : "%s"), strbuf);
        if (NULL == next)
            break;
    }

    snprintf(buf + strlen(buf), len - strlen(buf), NULL != next ? ", ...}" : "}");
}

/**
 * check if the string has unprintable character
 * @return 1 for printable, 0 for unprintable
 */
int is_printable(const char *s, int* too_long)
{
    int has_null = 0;
    int printable = 1;
    int i;
    for (i = 0; i < STR_LEN_LIMIT; i++) {
        if (!check_pointer(s + i))
            return 0;

        if ('\0' == s[i]) {
            has_null = 1;
            break;  /* reached end of string */
        }
        else if (!isprint(s[i]) && printable != 0)  /* only record first unprintable */
            printable = 0;
    }

    if (!has_null)  /* length greater than 25 or is not a valid C string */
        *too_long = 1;

    return printable;
}

/**
 * check if the pointer is point to a valid address
 * @return 1 for valid, 0 for invalid
 */
int check_pointer(const void *p)
{
    int rc;
    rc = write(g_tmpfd, p, 1);
    /* write return -1 and set errno to EFAULT if p is a invalid address */
    if (rc == -1 && errno == EFAULT)
        return 0;

    return 1;
}

