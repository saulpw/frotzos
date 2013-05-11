#include "frotzos.h"

FILE *os_path_open(const char *path, const char *mode) { return fopen(path, mode); }
// ----
extern char _binary_zcode_z5_start[];
extern int _binary_zcode_z5_size[];
int disk_highwater = 0;

FILE *fopen(const char *path, const char *mode)
{
    FILE * fp = malloc(sizeof(FILE));

    if (fp == NULL) return fp;

    fp->hdr = NULL;
    fp->data = NULL;
    fp->fpos = 0;

    char * file = &_binary_zcode_z5_start[0];
    struct header *h;

    h = (struct header *) malloc(sizeof(struct header));
    strncpy(h->path, path, sizeof(h->path));
    h->status = 1;
    h->length = (int) _binary_zcode_z5_size;

#ifdef WITH_HEADERS // and WRITABLE
    h = (struct header *) file;
    while (file[0])
    {
        h = (struct header *) file;

        if (strncmp(h->path, path, sizeof(h->path)) == 0) {
            break;
        }

        file += sizeof(struct header) + h->length;
    }

    if (file != &_KERNEL_END[disk_highwater])
    {
        strncpy(h->path, path, sizeof(h->path));
        h->status = 1;
    }
#endif
    
    fp->hdr = h;                
    fp->data = file; //  + sizeof(*h);

    return fp;
}

int fclose(FILE *fp)
{
    disk_highwater += fp->fpos;
    fp->hdr->length = fp->fpos;
    fp->hdr->status = 2;
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
        assert(offset <= stream->hdr->length);
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
    stream->fpos += n;
    return n;
}

int ferror(FILE *stream)
{
    return 0;
}


