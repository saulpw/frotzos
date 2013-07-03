#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <fcntl.h>
#include <sys/mman.h>

#include "../iso9660.h"

#include "zip_crc32.c"

#define ERRNO(X) if ((X) < 0) { perror(#X); }

#define SECTOR_SIZE 2048
#define PACKED __attribute__ ((__packed__))

typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;

typedef struct PACKED {
    u32 signature;
    u16 version_needed;
    u16 bit_flags;
    u16 method;
    u16 time;
    u16 date;
    u32 crc32;
    u32 comp_size;
    u32 uncomp_size;
    u16 filename_len;
    u16 extra_len;
    char filename[];
//    char m_extra[];
} ZipLocalFileHeader;

typedef struct PACKED {
    u32 signature;
    u16 version_made_by;
    u16 version_needed;
    u16 bit_flags;
    u16 method;
    u16 date;
    u16 time;
    u32 crc32;
    u32 comp_size;
    u32 uncomp_size;
    u16 filename_len;
    u16 extra_len;
    u16 comment_len;
    u16 disknum_start;
    u16 internal_attr;
    u32 external_attr;
    u32 local_header_ofs;
    char filename[];
//    char m_extra[];
//    char m_comment[];
} ZipCentralDirFileHeader;

typedef struct PACKED {
    u32 signature;
    u16 disk_num;
    u16 disk_start;
    u16 disk_num_records;
    u16 total_num_records;
    u32 central_dir_bytes;
    u32 central_dir_start;  // relative to start of archive
    u16 comment_len;
    u8 comment[];
} ZipEndCentralDirRecord;

void
create_zip_hdrs(ZipLocalFileHeader **out_lhdr, int *out_lhdr_len, 
                ZipCentralDirFileHeader **out_chdr, int *out_chdr_len, 
                const char *fn, int filesize)
{
    size_t fnlen = strlen(fn);
    size_t extralen = 0; // SECTOR_SIZE - (filesize % SECTOR_SIZE); // XXX: whole extra sector if mod == 0
    size_t lhdrlen = sizeof(ZipLocalFileHeader) + fnlen + extralen;
    ZipLocalFileHeader * lhdr = (ZipLocalFileHeader *) malloc(lhdrlen);

    lhdr->signature = 0x04034b50;
    lhdr->version_needed = 0x00;
    lhdr->bit_flags = 0x00;
    lhdr->method = 0x00;
    lhdr->time = 0x00;
    lhdr->date = 0x00;
    lhdr->crc32 = 0x00;
    lhdr->comp_size = lhdr->uncomp_size = filesize;
    lhdr->filename_len = fnlen;
    lhdr->extra_len = extralen;
    strcpy(lhdr->filename, fn);

    size_t chdrlen = sizeof(ZipCentralDirFileHeader) + fnlen;
    ZipCentralDirFileHeader * chdr = (ZipCentralDirFileHeader *) malloc(chdrlen);
    chdr->signature = 0x02014b50;
    chdr->version_made_by = 0x00;
    chdr->version_needed = 0x00;
    chdr->bit_flags = 0x00;
    chdr->method = 0x00;
    chdr->date = 0x00;
    chdr->time = 0x00;
    chdr->crc32 = 0x00;
    chdr->comp_size = chdr->uncomp_size = filesize;
    chdr->filename_len = fnlen;
    chdr->extra_len = extralen;
    chdr->comment_len = 0;
    chdr->disknum_start = 0;
    chdr->internal_attr = 0x00;
    chdr->external_attr = 0x00;
    chdr->local_header_ofs = 0;
    strcpy(chdr->filename, fn);
//    char m_extra[];
//    char m_comment[];
//
    *out_lhdr = lhdr;
    *out_lhdr_len = lhdrlen;
    *out_chdr = chdr;
    *out_chdr_len = chdrlen;
}

#define SECTOR(N) (isoptr + (N) * SECTOR_SIZE)
#define NEXT_ENTRY(E) \
    ((const DirectoryRecord *) (((const u8 *) E) + E->record_len))

const DirectoryRecord *find_file_at_sector(void *isoptr, int sector_num)
{
    const PrimaryVolumeDescriptor *pvd = SECTOR(16);
    const DirectoryRecord *rootrecord =  &pvd->root_directory_record;
    const DirectoryRecord *entry = SECTOR(rootrecord->data_sector);
    for (; entry->record_len != 0; entry = NEXT_ENTRY(entry))
    {
        int start = entry->data_sector;
        int end = entry->data_sector + entry->data_len / SECTOR_SIZE;

        if (sector_num >= start && sector_num <= end)
        {
            return entry;
        }
    }
    return NULL;
}

int
main(int argc, char **argv)
{
    assert(crc32("123456789", 9) == 0xcbf43926);
    assert(sizeof(ZipLocalFileHeader) == 30);
    assert(sizeof(ZipCentralDirFileHeader) == 46);
    assert(sizeof(ZipEndCentralDirRecord) == 22);

    char outfn[256];
    snprintf(outfn, 256, "%s.zip", argv[1]);

    int fdiso = open(argv[1], O_RDONLY);
    ERRNO(fdiso);

    int fdout = open(outfn, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    ERRNO(fdout);

    struct stat st;
    ERRNO(fstat(fdiso, &st));

    size_t isosize = st.st_size;

    void *isoptr = mmap(NULL, isosize, PROT_READ | PROT_WRITE, MAP_PRIVATE, fdiso, 0);

    if (isoptr == MAP_FAILED) {
        perror("mmap");
        exit(-1);
    }

    const PrimaryVolumeDescriptor *pvd = SECTOR(16);
    printf("# sectors=%d\n", pvd->num_sectors);
    const DirectoryRecord *rootrecord =  &pvd->root_directory_record;
    assert(rootrecord->record_len == sizeof(DirectoryRecord) + rootrecord->id_len);

    ZipCentralDirFileHeader *cdir_entries[256] = { NULL };
    int num_files = 0;

    const DirectoryRecord *entry = SECTOR(rootrecord->data_sector);

    for (; entry->record_len > 0; entry = NEXT_ENTRY(entry))
    {
        char fn[256] = { 0 };
        strncpy(fn, entry->id, entry->id_len);

        if (entry->id[0] == 0x0) {
            fn[0] = '.';
        } else if (entry->id[0] == 0x01) {
            fn[0] = '.';
            fn[1] = '.';
        }

        printf("CD file: %s at sector %d (%u bytes)\n",
                fn, entry->data_sector, entry->data_len);

        if (fn[0] == '.') {
            continue;
        }

        const DirectoryRecord *prev_file = find_file_at_sector(isoptr, entry->data_sector - 1);
        int prev_filesize = prev_file->data_len;
        int leftover = SECTOR_SIZE - (prev_filesize % SECTOR_SIZE);

        if (leftover == SECTOR_SIZE) {
            leftover = 0; 
        }

        ZipLocalFileHeader *local_hdr = NULL;
        int localhdr_len = -1;
        int centdirhdr_len = -1;

        create_zip_hdrs(&local_hdr, &localhdr_len,
                        &cdir_entries[num_files], &centdirhdr_len,
                        fn,
                        entry->data_len);

        uint32_t crc = crc32( SECTOR(entry->data_sector), entry->data_len);

        cdir_entries[num_files]->crc32 = local_hdr->crc32 = crc;

        if (leftover >= localhdr_len) {
            void *lhdr = SECTOR(entry->data_sector) - localhdr_len;
            memcpy(lhdr, local_hdr, localhdr_len);
            size_t lhdr_fpos = (entry->data_sector) * SECTOR_SIZE - localhdr_len;
            cdir_entries[num_files]->local_header_ofs = lhdr_fpos;
            num_files++;
        } else {

            // need to insert 
            printf("not putting %s in .zip due to not enough leftover space in previous file (%d/%d)\n", fn, leftover, localhdr_len);
        }

/* 
        // for local file header
        struct added_sector {

        };
 //        note when writing, before the to add a sector
            prepend_sectors_at[num_added_sectors++] = entry->data_sector;

 //        increase by 1 the LBA for anything after this sector (in the mmap)
                      
            actually, if leftover of file ending in previous sector is < this
            
                for each entry in the LSB path table
                    if location of extent >= this file's starting sector,
                        increment

                for each entry in the MSB path table
                    same, but don't forget to BSWAP

                for each entry in the root directory,
                    bump LBA extent in both LSB and MSB if >= as above

                // insert sector, adjust all tables
                //
        }
*/
    }

    assert (isosize % SECTOR_SIZE == 0);

    int i=0;
    while (i < isosize)
    {
        ssize_t r = write(fdout, isoptr + i, SECTOR_SIZE);
        assert (r == SECTOR_SIZE);
        i += r;
    }

    int cdirpos = i;

    // and now to write out the .zip central directory
    
    int cdir_len = 0;
    for (num_files=0; cdir_entries[num_files]; ++num_files)
    {
        size_t entry_len = sizeof(ZipCentralDirFileHeader) + cdir_entries[num_files]->filename_len;
        ssize_t r = write(fdout, cdir_entries[num_files], entry_len);
        assert(r == entry_len);

        cdir_len += r;
    }

    ZipEndCentralDirRecord endcdir;
    endcdir.signature = 0x06054b50;
    endcdir.disk_num = 0;
    endcdir.disk_start = 0;
    endcdir.disk_num_records = endcdir.total_num_records = num_files;
    endcdir.central_dir_bytes = cdir_len;
    endcdir.central_dir_start = cdirpos;
    endcdir.comment_len = 0;

    ssize_t r = write(fdout, &endcdir, sizeof(ZipEndCentralDirRecord));
    assert(r == sizeof(ZipEndCentralDirRecord));

    munmap(isoptr, isosize);
    close(fdiso);
    close(fdout);
}
