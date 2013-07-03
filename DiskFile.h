#ifndef DISKFILE_H_
#define DISKFILE_H_

#include <stdint.h>

typedef struct DiskFile {
    char filename[64];
    void *data;
    uint64_t length;
} DiskFile;

DiskFile * iso9660_enumfiles();
DiskFile * iso9660_fopen_r(const char *filename);
DiskFile * iso9660_fopen_w(const char *filename);

#endif
