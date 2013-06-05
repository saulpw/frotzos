#include <string.h>
#include <stdio.h>
#include "elifs.h"

FILE *fopen(const char *path, const char *mode)
{
    if (mode[0] != 'r')
        return NULL;

    const struct fz_filehdr *h = elifs_read(path);

    FILE * fp = malloc(sizeof(FILE));

    if (fp == NULL) {
        // errno = ENOMEM;
        return NULL;
    }

    fp->hdr = (struct fz_filehdr *) h;
    fp->data = elif_data(h);
    fp->fpos = 0;

    return fp;
}

int fclose(FILE *fp)
{
#if 0
    // TODO: write out file
    fp->hdr->length = fp->fpos;
    fp->hdr->status = STATUS_EXISTING;
#endif
    free(fp);
    return 0;
}

int fgetc(FILE *fp)
{
    if (fp->fpos > fp->hdr->length) return EOF;
    else return fp->data[fp->fpos++];
}

int ungetc(int c, FILE *stream)
{
    const char *p = &stream->data[stream->fpos - 1];
    if (*p != c) {
        return EOF; 
    }
    stream->fpos -= 1;
    return c;
}

int fputc(int c, FILE *fp)
{
#if 0
    if (fp->hdr->status != 1) return -1;
    fp->data[fp->fpos++] = c;
    return 0;
#endif
    return EOF;
}

int fseek(FILE *stream, long offset, int whence)
{
    if (whence == SEEK_SET) {
        if (offset > (long) stream->hdr->length) {
            return -1;
        }
        stream->fpos = offset;
    } else if (whence == SEEK_CUR) {
        if (stream->fpos + offset > stream->hdr->length) {
            return -1;
        }
        stream->fpos += offset;
    } else if (whence == SEEK_END) {
        if (offset > (long) stream->hdr->length) {
            return -1;
        }

        stream->fpos = stream->hdr->length - offset;
    }

    return 0;
}

int ftell(FILE *fp)
{
    return fp->fpos;
}

size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    int n = size*nmemb;
    if (stream->fpos + n > stream->hdr->length)
    {
        n = stream->hdr->length - stream->fpos;
    }
    memcpy(ptr, &stream->data[stream->fpos], n);
    stream->fpos += n;
    return n / size;
}

size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream)
{
#if 0
    size_t n = size*nmemb;

    if (stream->fpos + n > stream->hdr->length) {
        if (stream->hdr->status != STATUS_APPENDING) {
            return 0;
        }

        stream->data = realloc(stream->data, stream->fpos + n);
        stream->hdr->length = stream->fpos + n;
    }

    memcpy(&stream->data[stream->fpos], ptr, n);
    stream->fpos += n;

    return n / size;
#endif
    return 0;
}

int ferror(FILE *stream)
{
    return 0;
}


