#include <sys/types.h>
#include "x86.h"
#include "kernel.h"
#include "ata/ata.h"

#define USE_DMA 0

#define CB_DATA       0
#define CB_ERR        1
#define CB_FEATURE    1
#define CB_SECTOR_CNT 2
#define CB_SECTOR_NUM 3
#define CB_CYL_LOW    4
#define CB_CYL_HIGH   5
#define CB_DEV_HEAD   6
#define CB_CMD        7
#define CB_STATUS     7
#define CB_DEV_CTRL   0x206         // ports 0x3f6 and 0x376
#define CB_ALT_STATUS 0x206


#define CB_DC_NIEN          0x02          // no interrupts
#define CB_DC_SOFT_RESET    0x04

#define CB_STATUS_BUSY      0x80
#define CB_STATUS_DEVFAULT  0x20
#define CB_STATUS_DRQ       0x08
#define CB_STATUS_ERR       0x01

#define CB_DEV_HEAD_LBA  0x40

#define CMD_READ_SECTORS                 0x20
#define CMD_IDENTIFY_DEVICE              0xEC
#define CMD_IDENTIFY_PACKET_DEVICE       0xA1

#define DELAY400NS do { \
    ata_in8(d, CB_ALT_STATUS); \
    ata_in8(d, CB_ALT_STATUS); \
    ata_in8(d, CB_ALT_STATUS); \
    ata_in8(d, CB_ALT_STATUS); \
} while (0)

ata_disk disks[4];

inline void ata_out8(const ata_disk *d, u16 reg, u8 val)
{
    out8(d->base_port + reg, val);
}

inline u8 ata_in8(const ata_disk *d, u16 reg)
{
    return in8(d->base_port + reg);
}

inline u16 ata_in16(const ata_disk *d, u16 reg)
{
    return in16(d->base_port + reg);
}

void ata_reset(ata_disk *d)
{
    // reset Bus Master Error bit
    //    ata_out8(d, CB_STATUS, BM_SR_MASK_ERR);
   
    u8 dc = USE_DMA ? 0 : CB_DC_NIEN;
    ata_out8(d, CB_DEV_CTRL, dc | CB_DC_SOFT_RESET);
    DELAY400NS;
    ata_out8(d, CB_DEV_CTRL, dc);
    DELAY400NS;

    int ntries;
    if (d->devnum != 0) {
        for (ntries = 0; ntries < 10; ++ntries) {
            ata_out8(d, CB_DEV_HEAD, 0x10);
            DELAY400NS;
            if (ata_in8(d, CB_SECTOR_CNT) == 0x01 &&
                 ata_in8(d, CB_SECTOR_NUM) == 0x01 ) {
                break;
            }
        }
    }
    
    for (ntries = 0; ntries < 10; ++ntries) {
        if ((ata_in8(d, CB_STATUS) & CB_STATUS_BUSY) == 0) {
            break;
        }
    }

    ata_out8(d, CB_DEV_HEAD, d->devnum ? 0x10 : 0x00);
}

int init_ata_dev(ata_disk *d)
{
    ata_out8(d, CB_DEV_CTRL, USE_DMA ? 0 : CB_DC_NIEN);
    ata_out8(d, CB_DEV_HEAD, d->devnum ? 0x10 : 0x00);
    DELAY400NS;

    // Sector Count and Sector Number will stick on a present device
    ata_out8(d, CB_SECTOR_CNT, 0x55);
    ata_out8(d, CB_SECTOR_NUM, 0xaa);

    if (ata_in8(d, CB_SECTOR_CNT) != 0x55 || 
            ata_in8(d, CB_SECTOR_NUM) != 0xaa) {
        return 0;
    }

    ata_reset(d);

    if (ata_in8(d, CB_SECTOR_CNT) == 0x01 && 
            ata_in8(d, CB_SECTOR_NUM) == 0x01)   // something is there
    {
        u8 cl = ata_in8(d, CB_CYL_LOW);
        u8 ch = ata_in8(d, CB_CYL_HIGH);
        u8 st = ata_in8(d, CB_STATUS);

        if (cl == 0x14 && ch == 0xeb) {          // PATAPI
            d->type = ATAPI;
        } else if (cl == 0x69 && ch == 0x96) {   // SATAPI
            d->type = SATAPI;
        } else if (st != 0) {
            if (cl == 0x00 && ch == 0x00) {      // PATA
                d->type = ATA;
            } else if (cl == 0x3c && ch == 0xc3) { // SATA
                d->type = SATA;
            }
        }
        if (d->type != NONE) return 1;
    }
    return 0;
}

// detect controllers and disks
int
init_ata()
{
    int i=0;

    disks[i].base_port = 0x1f0;
    disks[i].devnum = 0;
    if (init_ata_dev(&disks[i])) ++i;

    disks[i].base_port = 0x1f0;
    disks[i].devnum = 1;
    if (init_ata_dev(&disks[i])) ++i;

    disks[i].base_port = 0x170;
    disks[i].devnum = 0;
    if (init_ata_dev(&disks[i])) ++i;

    disks[i].base_port = 0x170;
    disks[i].devnum = 1;
    if (init_ata_dev(&disks[i])) ++i;

    return i;
}

void
ata_error(const ata_disk *disk, const char *context)
{
    u8 err = ata_in8(disk, CB_ERR);
    u8 st = ata_in8(disk, CB_ALT_STATUS);

    DPRINT(0, "ata%04x:%d ERROR: %s"
              "  ERR= %02X (%s%s%s%s%s%s%s%s)"
              "  STATUS= %02X (%s%s%s%s%s%s%s%s)",
            disk->base_port, disk->devnum, context,
            err,
            (err & 0x80) ? "BBK " : "",
            (err & 0x40) ? "UNC " : "",
            (err & 0x20) ? "MC " : "",
            (err & 0x10) ? "IDNF " : "",
            (err & 0x08) ? "MCR " : "",
            (err & 0x04) ? "ABRT " : "",
            (err & 0x02) ? "NTK0 " : "",
            (err & 0x01) ? "NDAM " : "",
            st,
            (st & 0x80) ? "BSY " : "",
            (st & 0x40) ? "RDY " : "",
            (st & 0x20) ? "DF " : "",
            (st & 0x10) ? "SERV " : "",
            (st & 0x08) ? "DRQ " : "",
            (st & 0x04) ? "CORR " : "",
            (st & 0x02) ? "IDX " : "",
            (st & 0x01) ? "ERR " : "");

    DPRINT(0, "SC=%02X  SN=%02X  CL=%02X  CH=%02X  DH=%02X",
                ata_in8(disk, CB_SECTOR_CNT),
                ata_in8(disk, CB_SECTOR_NUM),
                ata_in8(disk, CB_CYL_LOW),
                ata_in8(disk, CB_CYL_HIGH),
                ata_in8(disk, CB_DEV_HEAD));
}

// waits for status_bits to NOT be set
// returns 0 on success
int ata_wait_status(const ata_disk *disk, u8 status_bits)
{
    int ntries;
    for (ntries=0; ntries < 1000; ntries++)
    {
        u8 status = ata_in8(disk, CB_ALT_STATUS);
        if ((status & status_bits) == 0)
            return 0;
    }

    ata_error(disk, "ata_wait_status");
    return -1; 
}

int
ata_select(const ata_disk *d)
{
    if (ata_wait_status(d, CB_STATUS_BUSY | CB_STATUS_DRQ)) return -1;
    ata_out8(d, CB_DEV_HEAD, d->devnum ? 0x10 : 0x00);
    DELAY400NS;
    if (ata_wait_status(d, CB_STATUS_BUSY | CB_STATUS_DRQ)) return -1;
    return 0;
}

#define WORDSWAP(S) swapwords(S, sizeof(S))

void swapwords(u8 *s, int slen)
{
    int i;
    for (i=0; i < slen; i += 2)
    {
        u8 tmp = s[i];
        s[i] = s[i+1];
        s[i+1] = tmp;
    }
    s[slen-1] = 0;
}

int
ata_identify_device(const ata_disk *d, IDENTIFY_DEVICE_DATA *id)
{
    if (ata_select(d)) return -1;

    // sub_setup_command()
    ata_out8(d, CB_DEV_CTRL, USE_DMA ? 0 : CB_DC_NIEN);
    ata_out8(d, CB_FEATURE, 0);
    ata_out8(d, CB_SECTOR_CNT, 0);
    ata_out8(d, CB_SECTOR_NUM, 0);
    ata_out8(d, CB_CYL_LOW, 0);
    ata_out8(d, CB_CYL_HIGH, 0);
    ata_out8(d, CB_DEV_HEAD, CB_DEV_HEAD_LBA | (d->devnum ? 0x10 : 0x00));

    ata_out8(d, CB_CMD, d->type == ATAPI ? CMD_IDENTIFY_PACKET_DEVICE
                                         : CMD_IDENTIFY_DEVICE);
    DELAY400NS;

    if (ata_wait_status(d, CB_STATUS_BUSY)) {
        return -1; 
    }

    u8 status = ata_in8(d, CB_STATUS);

    if ((status & (CB_STATUS_BUSY | CB_STATUS_DRQ)) == CB_STATUS_DRQ)
    {
        u16 *buf = (u16 *) id;
        // pio_drq_block_in(CB_DATA, buf, 256);
        int i;
        for (i=0; i < 512; i += 2) {
            *buf++ = ata_in16(d, CB_DATA);
        }

        DELAY400NS;
    }

    if (status & (CB_STATUS_BUSY | CB_STATUS_DEVFAULT | CB_STATUS_ERR))
    {
        ata_error(d, "read error");
        return -1;
    }

    if ((status & CB_STATUS_DRQ) == 0)
    {
        ata_error(d, "DRQ");
        return -1;
    }

    WORDSWAP(id->ModelNumber);
    WORDSWAP(id->SerialNumber);
    WORDSWAP(id->FirmwareRevision);

    return 0;
}

int
ata_read_lba28(const ata_disk *d, u8 *buf, u32 lba)
{
    if (ata_select(d)) return -1;

    // sub_setup_command()
    ata_out8(d, CB_DEV_CTRL, USE_DMA ? 0 : CB_DC_NIEN);
    ata_out8(d, CB_FEATURE, 0);
    ata_out8(d, CB_SECTOR_CNT, 1);
    ata_out8(d, CB_SECTOR_NUM, (u8) lba);
    ata_out8(d, CB_CYL_LOW, (u8) (lba >> 8));
    ata_out8(d, CB_CYL_HIGH, (u8) (lba >> 16));

    u8 dh = CB_DEV_HEAD_LBA | (d->devnum ? 0x10 : 0x00);
    ata_out8(d, CB_DEV_HEAD, (u8) ((dh & 0xf0) | ((lba >> 24) & 0x0f)));

    ata_out8(d, CB_CMD, CMD_READ_SECTORS);
    DELAY400NS;

    if (ata_wait_status(d, CB_STATUS_BUSY)) {
        return -1; 
    }

    u8 status = ata_in8(d, CB_STATUS);

    if ((status & (CB_STATUS_BUSY | CB_STATUS_DRQ)) == CB_STATUS_DRQ)
    {
        u16 *wbuf = (u16 *) buf;
        // pio_drq_block_in(CB_DATA, buf, 256);
        int i;
        for (i=0; i < 512; i += 2) {
            *wbuf++ = ata_in16(d, CB_DATA);
        }

        DELAY400NS;
    }

    if (status & (CB_STATUS_BUSY | CB_STATUS_DEVFAULT | CB_STATUS_ERR))
    {
        ata_error(d, "read error");
        return -1;
    }

    if ((status & CB_STATUS_DRQ) == 0)
    {
        ata_error(d, "DRQ");
        return -1;
    }

    return 0;
}

