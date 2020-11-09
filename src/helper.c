#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "helper.h"

extern FILE *output;
extern bool colored;

void nprintf(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    vfprintf(output, format, args);
    va_end(args);
}

void cprintf(BG_COLOR bg, FG_COLOR fg, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    if (colored == true && output == stdout)
        vfprintf(output, _color(bg, fg, format), args);
    else
        vfprintf(output, format, args);
    va_end(args);
}

const char *_color(int bg, int fg, const char *s)
{
    char *str = malloc((strlen(s) + 20) * sizeof(char));
    sprintf(str, "\x1b[%d;%dm%s\x1b[0;0m", bg, fg, s);
    return str;
}

char *cutstr(char *str, int cap)
{
    str[cap] = '\0';
    return str;
}