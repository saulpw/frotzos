#include <stdlib.h>
#include <string.h>
#include "kernel.h"
#include "elifs.h"

#define MAX_FILES 256

static struct fz_filehdr *g_files[MAX_FILES] = { 0 };
static int writablefiles = 0;

static 
int
_enumfiles(struct fz_filehdr **files, const char *diskstart)
{
    const char *data = diskstart;
    struct fz_filehdr *h = (struct fz_filehdr *) data;

    int i=0;
    while (h->magic == ELIFS_MAGIC && h->status != STATUS_SENTINEL)
    {
        files[i++] = h;

        data += sizeof(struct fz_filehdr) + h->namelength + h->length;

        h = (struct fz_filehdr *) data;
    }
    files[i++] = h; // include sentinel
    files[i] = NULL;
    return i;
}

struct fz_filehdr * const * elifs_enumfiles()
{
    // always recompute since files may have changed and it's fast once 
    //    the pages are cached
//    if (g_files[0] == NULL)
    {
        int n1, n2;

        n1 = _enumfiles(&g_files[0], (char *) DISK0_MAP_ADDR);
        writablefiles = n1 - 1;
        n2 = _enumfiles(&g_files[n1-1], (char *) DISK1_MAP_ADDR);

        DEBUG("disk0: %d files, disk1: %d files\r\n", n1, n2);
        g_files[writablefiles+n2] = NULL;
    }

    return g_files;
}

const struct fz_filehdr *
elifs_read(const char *filename)
{
    struct fz_filehdr * const * hdrs = elifs_enumfiles();

    int i=0;
    while (hdrs[i])
    {
        const struct fz_filehdr *h = hdrs[i];
        if (h->status == STATUS_EXISTING && 
                strncmp(h->name, filename, h->namelength) == 0)
        {
            return h;
        }
        i++;
    }
    return NULL;
}

const struct fz_filehdr *
elifs_write(const char *filename, const char *data, size_t length)
{
    int fnlen = strlen(filename) + 1;

    if (fnlen % 16 != 0)
        fnlen = (fnlen / 16 + 1)*16;

    if (fnlen > MAX_NAME_LEN) {
        kprintf("filename length is %d (max length %d)\r\n",
                    fnlen, MAX_NAME_LEN);
        return NULL;
    }

    DEBUG("elifs_write(%s, 0x%x, %d)\r\n", filename, data, length);

    struct fz_filehdr * const * hdrs = elifs_enumfiles();

    int i=0;
    for (i=0; i < writablefiles; ++i) 
    {
        const struct fz_filehdr *h = hdrs[i];
        if (h->status == STATUS_EXISTING && 
                strncmp(h->name, filename, h->namelength) == 0)
        {
            kprintf("can't overwrite file on read-only disk\r\n");
            return NULL;
        }
    }

    struct fz_filehdr *h = NULL;
    struct fz_filehdr *existinghdr = NULL;
    // i = writablefiles;
    for (; hdrs[i]; ++i)
    {
        h = hdrs[i];
        if (h->status == STATUS_APPENDING) {
            kprintf("cannot write %s: %s already appending\r\n",
                    filename, h->name);
            return NULL;
        } else if (h->status == STATUS_EXISTING) {
            if (strncmp(h->name, filename, h->namelength) == 0) {
                // try to reuse existing file
                if (h->length >= length) {
                    break;
                }
                // otherwise delete it and skip to end
                if (existinghdr) {
                    kprintf("multiple existing files named '%s'\r\n", filename);

                    // delete the previous one to clean it up
                    existinghdr->status = STATUS_DELETED;
                }
                existinghdr = h;
            }
        } else if (h->status == STATUS_DELETED) {
            // maybe we can use it
            if (h->length >= length) {
                break;
            }
        } else if (h->status == STATUS_SENTINEL) {
            break;
        }
    }

    if (hdrs[i] == NULL)
    {
        kprintf("no available header to write file to (%d files)\r\n", i);
        return NULL;
    }

    // by getting here, h either points to a deleted file with enough
    // room, or to the end of the elifs
  
    // mark previously existing file as deleted
    if (existinghdr) {
        existinghdr->status = STATUS_DELETED;
    }

    memset(h, 0, sizeof(struct fz_filehdr) + fnlen);
    h->magic = ELIFS_MAGIC;
    h->length = length;
    h->status = STATUS_EXISTING;
    h->namelength = fnlen;
    strcpy(h->name, filename);

    void *mmap_data = elif_data(h);

    memcpy(mmap_data, data, length);

    if (ksync() < 0)
    {
        return NULL;
    }

    return h;
}

void *elif_data(const struct fz_filehdr *h)
{
    return ((void *) h) + sizeof(struct fz_filehdr) + h->namelength;
}


