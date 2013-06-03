#include "frotzos.h"
#include <stdio.h>

FILE *os_path_open(const char *path, const char *mode)
{
    return fopen(path, mode);
}

