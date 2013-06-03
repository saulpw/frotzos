#include <stdlib.h>
#include <string.h>
#include "kernel.h"
#include "elifs.h"

#define MAX_FILES 256

struct fz_filehdr *files[MAX_FILES] = { 0 };

struct fz_filehdr * const * elifs_enumfiles()
{
    if (files[0] == NULL) {
        int i=0;

        char * data = (char *) DISK_MAP_ADDR;

        struct fz_filehdr *h = (struct fz_filehdr *) data;

        while (h->magic == MAGIC && h->status != STATUS_SENTINEL)
        {
            files[i++] = h;

            data += sizeof(struct fz_filehdr) + h->namelength + h->length;

            h = (struct fz_filehdr *) data;
        }
        files[i] = NULL;
    }

    return files;
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
                strncmp(h->name, filename, elif_fnlen(h)) == 0)
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
#if 0
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

    return h;
#endif
    return NULL;
}

