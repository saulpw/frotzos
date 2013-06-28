#ifndef DISKFILE_H_
#define DISKFILE_H_

#include <stdint.h>

typedef struct DiskFile {
    char filename[64];
    void *data;
    uint64_t length;
} DiskFile;

#endif
