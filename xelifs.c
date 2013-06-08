#include <assert.h>
#include <stdlib.h>
#include <getopt.h>
#include <stdio.h>
#include "elifs.h"

#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))

// extract files from elifs archive

int
main(int argc, char * const argv[])
{
    int opt;
    int skip = 0;
    const char *outdir = NULL;

    while ((opt = getopt(argc, argv, "k:C:")) != -1)
    {
        switch (opt) {
        case 'k': // skip bytes at beginning
            skip = atoi(optarg);
            break;
        case 'C':
            outdir = optarg;
            break;
        default:
            break;
        };
    }

    if (optind < 1) {
        fprintf(stderr, "Usage: %s [-C <extract-dir>] [-k <skip-bytes>] <elifs-image>\n", argv[0]);
        exit(1);
    }

    FILE *fpin = fopen(argv[optind], "r+b");

    if (fpin == NULL) {
        perror(argv[optind]);
        exit(1);
    }

    if (outdir) {
        if (chdir(outdir) < 0) {
            perror(outdir);
            exit(1);
        }
    }
    size_t curpos = 0;

    if (skip > 0) {
        fprintf(stderr, "skipping %d bytes\n", skip);
        fseek(fpin, skip, SEEK_SET);
        curpos += skip;
    }

    while (1) {
        struct fz_filehdr hdr;

        int r = fread(&hdr, sizeof(hdr), 1, fpin);
        assert (r == 1);

        if (hdr.magic == 0) {
            break;
        }

        if (hdr.magic != ELIFS_MAGIC) {
            fprintf(stderr, "bad magic at pos %lu!\n", ftell(fpin));
            exit(1);
        }

        int fnlen = elif_fnlen(&hdr);
        uint64_t nbytes = elif_length(&hdr);

        if (fnlen == 0) {
            fprintf(stderr, "no filename! (length %lu)", nbytes);
            continue;
        }

        char fn[fnlen+1];

        r = fread(fn, 1, fnlen, fpin);
        assert (r == fnlen);

        uint64_t n = 0;
        fprintf(stderr, "extracting '%s' (length %lu)", fn, nbytes);

        curpos += sizeof(hdr) + fnlen;
        assert(ftell(fpin) == curpos);

        FILE *fpout = fopen(fn, "w+b");
        if (fpout == NULL) {
            perror(fn);
            fseek(fpin, curpos, SEEK_SET);
        } else {
            while (n < nbytes) {
                char buf[4096];

                size_t nread = fread(buf, 1, MIN(sizeof(buf), nbytes - n), fpin);
                fprintf(stderr, ".");
                fflush(stderr);

                if (nread <= 0) break;

                size_t nwritten = fwrite(buf, 1, nread, fpout);
                assert(nwritten == nread);

                n += nwritten;
            }
            fclose(fpout);
        }

        curpos += nbytes;

        fprintf(stderr, "\n");
    }

    return 0;
}


