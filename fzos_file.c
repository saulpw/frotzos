#include "frotzos.h"
#include "filehdr.h"
#include "io.h"

#define MAX_FILES 256

char * const DISK_MAP_ADDR = (char *) 0x10000000;

struct fz_filehdr *files[MAX_FILES] = { 0 };

struct fz_filehdr * const * enumfiles()
{
    if (files[0] == NULL) {
        int i=0;

        char * data = DISK_MAP_ADDR;

        struct fz_filehdr *h = (struct fz_filehdr *) data;

        while (h->magic == MAGIC && h->status != STATUS_EMPTY)
        {
            files[i++] = h;

            data += sizeof(struct fz_filehdr) + h->namelength + h->length;

            h = (struct fz_filehdr *) data;
        }
        files[i] = NULL;
    }

    return files;
}

FILE *os_path_open(const char *path, const char *mode)
{ 
    return fopen(path, mode);
}

// ----

FILE *fopen(const char *name, const char *mode)
{
    struct fz_filehdr * const * hdrs = enumfiles();

    int i=0;
    const struct fz_filehdr *h = NULL;
    while (hdrs[i])
    {
        h = hdrs[i];
        if (strncmp(h->name, name, h->namelength) == 0) {
            break;
        }
        i++;
    }

    if (h->status == STATUS_EMPTY)
    {
        size_t namelen = strlen(name);
        if (namelen > 111) {
            errmsg = "filename max length 111 bytes";
            return NULL;
        }

        struct fz_filehdr *newh = (struct fz_filehdr *) h; // non-const anymore
        memset((void *) newh, 0, sizeof(struct fz_filehdr));
        newh->magic = MAGIC;
//        newh->length will be set as data is written
        newh->status = STATUS_APPENDING;
        newh->namelength = namelen;
        strcpy(newh->name, name);
        h = newh;
    }
    
    FILE * fp = malloc(sizeof(FILE));

    if (fp == NULL) {
        // errno = ENOMEM;
        return NULL;
    }

    fp->hdr = (struct fz_filehdr *) h;
    fp->data = ((char *) h) + sizeof(struct fz_filehdr) + h->namelength;
    fp->fpos = 0;

    return fp;
}

int fclose(FILE *fp)
{
    // TODO: write out file
    fp->hdr->length = fp->fpos;
    fp->hdr->status = STATUS_EXISTING;
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
    if (fp->hdr->status != 1) return -1;
    fp->data[fp->fpos++] = c;
    return 0;
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
}

int ferror(FILE *stream)
{
    return 0;
}


