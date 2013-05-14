#include "frotzos.h"
#include "filehdr.h"

FILE *os_path_open(const char *path, const char *mode)
{ 
    return fopen(path, mode);
}

// ----
int disk_highwater = 0;

FILE *fopen(const char *name, const char *mode)
{
    char * file = (char *) 0x8000;
    struct fz_filehdr *h = (struct fz_filehdr *) file;

    while (h->status != STATUS_EMPTY)
    {
        if (strncmp(h->name, name, h->namelength) == 0) {
            break;
        }

        file += sizeof(struct fz_filehdr) + h->namelength + h->length;

        while (*file == 0) ++file; // skip remaining zero padding
        h = (struct fz_filehdr *) file;
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
    fp->data = file + sizeof(struct fz_filehdr) + h->namelength;
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


