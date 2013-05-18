#include "frotzos.h"
#include "filehdr.h"

#define MAX_FILES 256

#define NEXT_FILE(HDR) ({ \
    const char * d = (const char *) HDR; \
    d += sizeof(struct fz_filehdr) + HDR->namelength + HDR->length; \
    while (*d == 0) ++d; /* skip remaining zero padding */ \
    (struct fz_filehdr *) d; \
})

const char * const * enumfiles()
{
    static const char **files = NULL;
    
    if (files == NULL) {
        files = (const char **) malloc(sizeof(char *) * 256);   

        int i=0;

        char * data = (char *) 0x8000;
        struct fz_filehdr *h = (struct fz_filehdr *) data;

        while (h->status != STATUS_EMPTY)
        {
            files[i++] = h->name;
            h = NEXT_FILE(h);
        }
        files[i] = (char *) NULL;
    }

    return files;
}

FILE *os_path_open(const char *path, const char *mode)
{ 
    return fopen(path, mode);
}

// ----
int disk_highwater = 0;

FILE *fopen(const char *name, const char *mode)
{
    struct fz_filehdr *h = (struct fz_filehdr *) 0x8000;

    while (h->status != STATUS_EMPTY)
    {
        if (strncmp(h->name, name, h->namelength) == 0) {
            break;
        }

        h = NEXT_FILE(h);
    }

    if (h->status == STATUS_EMPTY)
    {
        size_t namelen = strlen(name);
        if (namelen > 111) {
            os_fatal("filename max length 111 bytes");
        }

        memset(h, 0, sizeof(struct fz_filehdr));
        h->magic = MAGIC;
//        h->length will be set as data is written
        h->status = STATUS_APPENDING;
        h->namelength = namelen;
        strcpy(h->name, name);
    }
    
    FILE * fp = malloc(sizeof(FILE));

    if (fp == NULL) return NULL;

    fp->hdr = h;
    fp->data = ((char *) h) + sizeof(struct fz_filehdr) + h->namelength;
    fp->fpos = 0;

    return fp;
}

int fclose(FILE *fp)
{
    disk_highwater += fp->fpos;
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
        stream->fpos = offset;
    } else if (whence == SEEK_CUR) {
        stream->fpos += offset;
    } else if (whence == SEEK_END) {
        assert((unsigned long) offset <= stream->hdr->length);
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
    memcpy(&stream->data[stream->fpos], ptr, n);
    stream->hdr->length += n;
    stream->fpos += n;

    assert(stream->hdr->length == stream->fpos);
    return n;
}

int ferror(FILE *stream)
{
    return 0;
}


