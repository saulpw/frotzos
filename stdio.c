#include <string.h>
#include <stdio.h>
#include "elifs.h"
#include "kernel.h"

FILE *fopen(const char *path, const char *mode)
{
    const struct fz_filehdr *h = NULL;

    if (mode[0] == 'r') {
        h = elifs_read(path);
        if (h == NULL) {
            DEBUG("fopen(): no existing file '%s'", path);
            return NULL;
        }
    } else {
        assert(mode[0] == 'w');
    }

    FILE * fp = malloc(sizeof(FILE));
    memset(fp, 0, sizeof(*fp));

    if (fp == NULL) {
        // errno = ENOMEM;
        return NULL;
    }

    fp->hdr = (struct fz_filehdr *) h;
    fp->fpos = 0;

    if (h) { // file for reading
        fp->data = elif_data(h);
    } else {
        fp->datalen = 256;
        fp->data = malloc(fp->datalen);
        fp->path = strdup(path);
    }

    return fp;
}

int fclose(FILE *fp)
{
    if (fp->path != NULL) {
        if (elifs_write(fp->path, fp->data, fp->fpos) == NULL) {
            return EOF;
        }
        free(fp->data);
        free(fp->path);
    }
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
    // grossly inefficient but will do for now
    int r = fwrite(&c, 1, 1, fp);
    if (r == 1)
        return c;
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
    DEBUG("fread(void *0x%08X, %d, FILE *0x%08X)", ptr, n, stream->hdr);
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
    if (stream->path == NULL) {
        DEBUG("file not open for writing");
        return 0;
    }

    size_t n = size*nmemb;
    DEBUG("fwrite(void *0x%08X, %u, FILE *0x%08X)\r\n", ptr, n, stream);

    while (stream->fpos + n > stream->datalen) {
        stream->datalen *= 2;
        // possibly multiple reallocs for now
        stream->data = realloc(stream->data, stream->datalen);
    }

    memcpy(&stream->data[stream->fpos], ptr, n);
    stream->fpos += n;

    return n / size;
}

int ferror(FILE *stream)
{
    return 0;
}


