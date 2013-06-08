#include <stdio.h>
#include <libgen.h> // basename
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <getopt.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "elifs.h"

int
main(int argc, char * const argv[])
{
    int opt;
    const char *outfn = "elifs.img";

    assert(sizeof(struct fz_filehdr) == 16);

    while ((opt = getopt(argc, argv, "o:")) != -1)
    {
        switch (opt) {
        case 'o':
            outfn = optarg;
            break;
        default:
            break;
        };
    }

    FILE *fpout = fopen(outfn, "w+b");
    if (fpout == NULL) {
        perror(outfn);
        exit(1);
    }

    while (optind < argc)
    {
        const char *infn = argv[optind++];

        struct stat st;
        if (stat(infn, &st) < 0) {
            perror(infn);
            exit(1);
        }

        int nbytes = st.st_size;
        
        FILE *fpin = fopen(infn, "rb");
        if (fpin == NULL) {
            perror(infn);
            continue;
        }

        fprintf(stderr, "copying %d bytes from %s", nbytes, infn);

        int namelen = strlen(infn)+1;
        if (namelen & 0xf) namelen = (namelen + 16) & ~0x0f; // pad fn
        assert(namelen < MAX_NAME_LEN);

        int hdrlen = sizeof(struct fz_filehdr) + namelen;
        char hdrbuf[hdrlen];
        memset(hdrbuf, 0, hdrlen);

        struct fz_filehdr *hdr = (struct fz_filehdr *) hdrbuf;
        hdr->magic = ELIFS_MAGIC;
        hdr->length = nbytes;
        hdr->status = STATUS_EXISTING;
        hdr->namelength = namelen;
        strcpy(hdr->name, basename(infn));

        size_t n = fwrite(hdr, sizeof(*hdr) + namelen, 1, fpout);
        assert(n == 1);
        n = 0;
        while (n < nbytes) {
            char buf[4096];

            size_t nread = fread(buf, 1, sizeof(buf), fpin);
            fprintf(stderr, ".");
            fflush(stderr);

            if (nread <= 0) break;

            size_t nwritten = fwrite(buf, 1, nread, fpout);
            assert(nwritten == nread);

            n += nwritten;
        }

        fclose(fpin);
        fprintf(stderr, "\n");
    }

    fclose(fpout);

    return 0;
}
