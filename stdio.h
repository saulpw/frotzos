#ifndef FZ_STDIO_H_
#define FZ_STDIO_H_

#include "stdlib.h"

#define printf(args...)
#define fprintf(args...)
#define sprintf(args...)
#define fputs(args...)

typedef struct
{
    struct fz_filehdr *hdr;

    unsigned char *data;
    unsigned long fpos;

    // if NULL, fwrites fail
    // otherwise, fwrites realloc data and fclose flushes
    char *path;
    unsigned long datalen;    // current maximum buffer size for writes
    unsigned long actualsize; // current length of file to be written
} FILE;

extern FILE *fopen(const char *path, const char *mode);
extern int fclose(FILE *fp);
extern int fgetc(FILE *fp);
extern int fputc(int c, FILE *fp);
extern int fseek(FILE *stream, long offset, int whence);

extern int ftell(FILE *fp);
extern size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream);
extern size_t fwrite(const void *ptr, size_t size, size_t nmemb,
                     FILE *stream);
extern int ferror(FILE *stream);
extern int ungetc(int c, FILE *stream);

#ifndef SEEK_SET
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2
#endif

#define EOF -1

extern int getchar(void);
extern int putchar(int c);

#endif
