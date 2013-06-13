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
    if (g_files[0] == NULL) {
        int n1, n2;

        n1 = _enumfiles(&g_files[0], (char *) DISK0_MAP_ADDR);
        writablefiles = n1 - 1;
        n2 = _enumfiles(&g_files[n1-1], (char *) DISK1_MAP_ADDR);

        DEBUG("disk0: %d files, disk1: %d files", n1, n2);
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
    DEBUG("elifs_write(%s, 0x%x, %d)\r\n", filename, data, length);

    struct fz_filehdr * const * hdrs = elifs_enumfiles();

    struct fz_filehdr *h = NULL;
    int i=writablefiles;
    for (i=0; hdrs[i]; ++i)
    {
        h = hdrs[i];
        if (h->status == STATUS_APPENDING) {
            kprintf("cannot write %s: %s already appending\r\n",
                    filename, h->name);
            return NULL;
        } else if (h->status == STATUS_EXISTING) {
            // TODO: check for same filename and possibly reuse file
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
  
    int fnlen = strlen(filename) + 1;
    if (fnlen > MAX_NAME_LEN) {
        kprintf("filename length is %d (max length %d)\r\n",
                    fnlen, MAX_NAME_LEN);
        return NULL;
    }

    if (fnlen % 16 != 0)
        fnlen = (fnlen / 16 + 1)*16;

    memset(h, 0, sizeof(struct fz_filehdr) + fnlen);
    h->magic = ELIFS_MAGIC;
    h->length = length;
    h->status = STATUS_EXISTING;
    h->namelength = fnlen;
    strcpy(h->name, filename);

    void *mmap_data = elif_data(h);

    memcpy(mmap_data, data, length);

    if (elifs_sync(h) < 0)
    {
        return NULL;
    }

    return h;
}

int
elifs_sync(struct fz_filehdr *hdr)
{
    const uint8_t *ptr = (const uint8_t *) hdr;
    unsigned int i;
    for (i=0; i < 16 + hdr->namelength + hdr->length; i += 512)
    {
        if (write_sector(&ptr[i]) < 0)
           return -1;
    }

    return 0;
}

void *elif_data(const struct fz_filehdr *h)
{
    return ((void *) h) + 16 + h->namelength;
}


