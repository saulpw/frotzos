#ifndef ISO9660_H_
#define ISO9660_H_

#include <stdint.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;

typedef struct {
    char year[4];
    char month[2];
    char day[2];
    char hour[2];
    char minute[2];
    char second[2];
    char centisecond[2];   // 100ths of a second
    int8_t gmt_offset;     // 15-min intervals
} DecimalDateTime;

struct DirectoryRecord
{
    u8   record_len;
    u8   ear_sectors;
    u32  data_sector;
    u32  msb_data_sector;
    u32  data_len;
    u32  msb_data_len;
    u8   years_since_1900, 
         month, 
         day, 
         hour, 
         minute, 
         second, 
         gmt_offset;
    u8   flags;
    u8   interleaved_file_unit_size;
    u8   interleaved_gap_size;
    u16  volume_seq_num;
    u16  msb_volume_seq_num;
    u8   id_len;
    char id[];

} __attribute__ ((packed));

typedef struct DirectoryRecord DirectoryRecord;

typedef struct PrimaryVolumeDescriptor {
	u8   type;
    char id[5];
    u8   version;
    u8   __unused1;
    char system_id[32];
    char volume_id[32];
    char __unused2[8];
    u32  num_sectors;          // volume_space_size
    u32  msb_num_sectors;
	char escape_sequences[32];
    u16  volume_set_size;
    u16  msb_volume_set_size;
	u16  volume_sequence_number;
	u16  msb_volume_sequence_number;
	u16  logical_block_size;
	u16  logical_block_size_msb;
	u32  path_table_size;
	u32  msb_path_table_size;
	u32  lsb_path_table_sector;
	u32  lsb_alt_path_table_sector;
	u32  msb_path_table_sector;
	u32  msb_alt_path_table_sector;
    DirectoryRecord root_directory_record;
    char volume_set_id[128];
    char publisher_id[128];
    char preparer_id[128];
	char application_id[128];
	char copyright_file_id[37];
	char abstract_file_id[37];
	char bibliographical_file_id[37];
    DecimalDateTime creation_date;
    DecimalDateTime modification_date;
    DecimalDateTime expiration_date;
    DecimalDateTime effective_date;
    u8 file_structure_version;
	u8 __unused4;
	u8 application_data[512];
	u8 __unused5;
} PrimaryVolumeDescriptor;

typedef struct PathTableEntry {
    u8 id_len;
    u8 ear_length;
    u32 dir_sector;
    u16 parent_dir_num;
    char id[];
    // and then a padding byte if id_len is odd
} PathTableEntry;

#define NEXT_PATH_TABLE_ENTRY(E) \
    ((PathTableEntry *) (((u8 *) E) + sizeof(PathTableEntry) + E->id_len + (E->id_len & 0x1)))

#define NEXT_DIR_ENTRY(E) \
    ((DirectoryRecord *) (((u8 *) E) + E->record_len))

#endif
