#include <string.h>

char *strdup(const char *src)
{
    char *dest = (char*) malloc(strlen(src) + 1);
    strcpy(dest, src);
    return dest;
}
