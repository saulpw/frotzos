#ifndef _ELIFS_H_
#define _ELIFS_H_

#include <stdint.h>

#define ELIFS_MAGIC 0x46696c65  // 'eliF' to indicate little-endianness

typedef struct fz_filehdr {
    uint32_t app_use;      // for the application's use
    uint32_t magic;        // should be ELIFS_MAGIC
    uint32_t length;       // exact file length in bytes (max 4GB files)
    uint16_t reserved;     // should be zero
    uint8_t status;        // see below
    uint8_t namelength;    // in bytes
    char name[];
} fz_filehdr_t;

#define MAX_NAME_LEN 255

#define STATUS_SENTINEL 0
#define STATUS_EXISTING 1
#define STATUS_APPENDING 2
#define STATUS_DELETED 4

const struct fz_filehdr *elifs_read(const char *filename);
const struct fz_filehdr *elifs_write(const char *filename,
                                     const char *data, size_t length);
int elifs_sync(struct fz_filehdr *hdr);

// get NULL-terminated array of files from both disks
struct fz_filehdr * const * elifs_enumfiles();

#define DONT_EMIT extern inline __attribute__ ((gnu_inline))

DONT_EMIT void *elif_data(const struct fz_filehdr *h)
{
    return ((void *) h) + 16 + h->namelength;
}

#endif
