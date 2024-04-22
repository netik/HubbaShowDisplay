#include <stdio.h>
#include <string.h>
#include <stdlib.h>

size_t safe_strlen(const char *str, size_t max_len)
{
    const char * end = (const char *)memchr(str, '\0', max_len);
    if (end == NULL)
        return max_len;
    else
        return end - str;
}