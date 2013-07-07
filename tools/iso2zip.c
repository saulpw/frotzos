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

const DirectoryRecord *find_file_at_sector(void *isoptr, int sector_num)
{
    const PrimaryVolumeDescriptor *pvd = SECTOR(16);
    const DirectoryRecord *rootrecord =  &pvd->root_directory_record;
    const DirectoryRecord *entry = SECTOR(rootrecord->data_sector);
    for (; entry->record_len != 0; entry = NEXT_DIR_ENTRY(entry))
    {
        int start = entry->data_sector;
        int end = entry->data_sector + entry->data_len / SECTOR_SIZE;

        if (entry->data_len % SECTOR_SIZE == 0)
            end -= 1;

//        printf("[looking for %d] %s (%d bytes) at sectors %d-%d\n", sector_num, entry->id, entry->data_len, start, end);

        if (sector_num >= start && sector_num <= end)
        {
            return entry;
        }
    }
    return NULL;
}

// for local file header
struct added_sector
{
    u8 data[SECTOR_SIZE];
    int sector_lba; // inserted just before this LBA in the original ISO
};

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

    PrimaryVolumeDescriptor *pvd = SECTOR(16);
    printf("# sectors=%d\n", pvd->num_sectors);
    const DirectoryRecord *rootrecord =  &pvd->root_directory_record;
    assert(rootrecord->record_len == sizeof(DirectoryRecord) + rootrecord->id_len);

    ZipCentralDirFileHeader *cdir_entries[256] = { NULL };
    struct added_sector *add_sectors[256] = { NULL };
    int ninserts = 0;
    int num_files = 0;

    DirectoryRecord *entry = NULL;

    for (entry = SECTOR(rootrecord->data_sector); entry->record_len > 0; entry = NEXT_DIR_ENTRY(entry))
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
        if (prev_file == NULL) {
            printf("skipping file, no previous file to put zip header\n");
            continue;
        }         
                
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
        void *iso_ziplhdr = NULL;
        size_t lhdr_fpos = 0;

        if (leftover >= localhdr_len) {
            iso_ziplhdr = SECTOR(entry->data_sector) - localhdr_len;
            lhdr_fpos = (entry->data_sector + ninserts) * SECTOR_SIZE - localhdr_len;
        } else {
#if 0
            struct added_sector *newsect = (struct added_sector *) malloc(sizeof(struct added_sector));
            // bzero(newsect->data, SECTOR_SIZE)?
            newsect->sector_lba = entry->data_sector;

            iso_ziplhdr = newsect->data + SECTOR_SIZE - localhdr_len;
            lhdr_fpos = (entry->data_sector + ninserts + 1) * SECTOR_SIZE - localhdr_len;

            printf("inserting zip local file header for %s at sector %d\n",
                        fn, newsect->sector_lba);

            add_sectors[ninserts++] = newsect;

#else
            printf("not putting %s in .zip due to not enough leftover space in previous file %s (%d bytes) (%d/%d)\n", fn, prev_file->id, prev_filesize, leftover, localhdr_len);
            continue;
#endif
        }

        memcpy(iso_ziplhdr, local_hdr, localhdr_len);
        cdir_entries[num_files]->local_header_ofs = lhdr_fpos;
        num_files++;
/* 
                for each entry in the LSB path table
                    if location of extent >= this file's starting sector,
                        increment

                for each entry in the MSB path table
                    same, but don't forget to BSWAP
*/
    }

    assert (isosize % SECTOR_SIZE == 0);

    // for each inserted sector, bump the LBAs in directories and path tables.
    int i;
    for (i=0; add_sectors[i]; ++i)
    {
        // for each entry in the root directory,
        //     bump LBA extent +1 in both LSB and MSB if >= as above

        for (entry = SECTOR(rootrecord->data_sector);
                entry->record_len > 0; entry = NEXT_DIR_ENTRY(entry))
        {
            if (entry->data_sector >= add_sectors[i]->sector_lba)
            {
                entry->data_sector += 1;
                // XXX: also bump entry->msb_data_sector
            }
        }
        
        PathTableEntry *lsb_path_table = SECTOR(pvd->lsb_path_table_sector);
        int j;
        for (j=0; j < pvd->path_table_size; ++j) {
            if (lsb_path_table[j].dir_sector >= add_sectors[i]->sector_lba) {
                lsb_path_table[j].dir_sector += 1;
            }
            lsb_path_table = NEXT_PATH_TABLE_ENTRY(lsb_path_table);
        }
//     PathTableEntry *msb_path_table = SECTOR(ntohl(pvd->msb_path_table_sector));

    }

    pvd->num_sectors += ninserts;
    // XXX: also msb_num_sectors;

    int sector;
    for (sector=0; sector * SECTOR_SIZE < isosize; ++sector)
    {
        ssize_t r;

        int i;
        for (i=0; add_sectors[i]; ++i) {
            if (add_sectors[i]->sector_lba == sector)
            {
                r = write(fdout, add_sectors[i]->data, SECTOR_SIZE);
                printf("inserted sector at %d\n", sector);
                assert (r == SECTOR_SIZE);
                break;
            }
        }            

        r = write(fdout, isoptr + sector*SECTOR_SIZE, SECTOR_SIZE);
        assert (r == SECTOR_SIZE);
    }

    int cdirpos = (sector + ninserts) * SECTOR_SIZE;

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
