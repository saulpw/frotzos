#ifndef FZ_FILEHDR_H_
#define FZ_FILEHDR_H_

typedef unsigned int uint32_t;
typedef unsigned char uint8_t;

typedef struct fz_filehdr {
    uint32_t magic;
    uint32_t reserved1; // for file 'type' or timestamp?
    uint32_t length;    // exact file length if <4GB
    uint8_t msblength;  // length actually += (msblength << 32)
    uint8_t reserved;
    uint8_t status;     // see below
    uint8_t namelength; // in bytes if <= 112 bytes
    char name[];
} fz_filehdr_t;

#define MAGIC 0x46494c45  // 'ELIF' to indicate little-endianness

#define STATUS_EMPTY 0
#define STATUS_EXISTING 1
#define STATUS_APPENDING 2

#endif
