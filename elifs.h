#ifndef _ELIFS_H_
#define _ELIFS_H_

#include <stdint.h>

typedef struct fz_filehdr {
    uint32_t magic;
    uint32_t length;       // exact file length if <4GB
    uint32_t length_msw;   // unlikely to be used
    uint16_t reserved;
    uint8_t status;     // see below
    uint8_t namelength; // in bytes if <= 128 bytes, 16*(NL-120) otherwise
    char name[];
} fz_filehdr_t;

#define ELIFS_MAGIC 0x46494c45  // 'ELIF' to indicate little-endianness
#define MAX_FILENAME_LEN 2160   // (255 - 120)*16 == 2160

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

DONT_EMIT uint64_t elif_length(const struct fz_filehdr *h)
{
    uint64_t ret = h->length_msw;
    ret <<= 32;
    ret += h->length;
    return ret;
}

DONT_EMIT int elif_fnlen(const struct fz_filehdr *h)
{
    return (h->namelength < 128) ? h->namelength : 16*(h->namelength - 120);
}

DONT_EMIT void *elif_data(const struct fz_filehdr *h)
{
    return ((void *) h) + 16 + elif_fnlen(h);
}

#endif
