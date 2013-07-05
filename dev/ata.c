#include <sys/types.h>
#include <assert.h>
#include "x86.h"
#include "kernel.h"
#include "dev/ata.h"

#define USE_DMA 0

#define CB_DATA       0
#define CB_ERR        1
#define CB_FEATURE    1
#define CB_SECTOR_CNT 2
#define CB_LBA1       3
#define CB_SECTOR_NUM 3
#define CB_LBA2       4
#define CB_CYL_LOW    4
#define CB_LBA3       5
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

#define CMD_PACKET                       0xA0
#define CMD_READ_SECTORS                 0xA0
#define CMD_IDENTIFY_PACKET_DEVICE       0xA1
#define CMD_IDENTIFY_DEVICE              0xEC

#define SCSI_CMD_READ_CAPACITY 0x25
#define SCSI_CMD_READ 0xA8

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

inline void ata_ins16(const ata_disk *d, void *buf, int nbytes)
{
    u16 *wbuf = (u16 *) buf;

    int i;
    for (i=0; i < nbytes/2; i++) {
        *wbuf++ = in16(d->base_port + CB_DATA);
    }
}

// always sends to DATA port
inline void ata_outs16(const ata_disk *d, const void *buf, int nbytes)
{
    const u16 *wbuf = (const u16 *) buf;

    int i;
    for (i=0; i < nbytes/2; i++) {
        out16(d->base_port + CB_DATA, wbuf[i]);
    }
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
    u8 st = ata_in8(disk, CB_ALT_STATUS);

    if (st & 0x01) { // ERR
        u8 err = ata_in8(disk, CB_ERR);
        assert(err);
        if (err & 0xf0) DPRINT(0, "ATAPI Sense Key");
        if (err & 0x80) DPRINT(0, "Bad Block");
        if (err & 0x40) DPRINT(0, "Uncorrected");
        if (err & 0x20) DPRINT(0, "Media Change");
        if (err & 0x10) DPRINT(0, "ID Not Found");
        if (err & 0x08) DPRINT(0, "Media Change Req");
        if (err & 0x04) DPRINT(0, "Command Aborted");
        if (err & 0x02) DPRINT(0, "No Track 0/End Of Media");
        if (err & 0x01) DPRINT(0, "No Address Mark/Illegal Length");
    }

    DPRINT(0, "ata%04x:%d ERROR: %s STATUS= %02X (%s%s%s%s%s%s%s%s)",
            disk->base_port, disk->devnum, context,
            st,
            (st & 0x80) ? "BUSY " : "",
            (st & 0x40) ? "READY " : "",
            (st & 0x20) ? "FAULT" : "",
            (st & 0x10) ? "SERVICE " : "",
            (st & 0x08) ? "DRQ " : "",
            (st & 0x04) ? "CORRECTED " : "",
            (st & 0x02) ? "INDEX " : "",
            (st & 0x01) ? "ERR " : "");

    DPRINT(0, "      SC=%02X  SN=%02X  CL=%02X  CH=%02X  DH=%02X",
                ata_in8(disk, CB_SECTOR_CNT),
                ata_in8(disk, CB_SECTOR_NUM),
                ata_in8(disk, CB_CYL_LOW),
                ata_in8(disk, CB_CYL_HIGH),
                ata_in8(disk, CB_DEV_HEAD));
}

#define ALT_STATUS(D) ata_in8(D, CB_ALT_STATUS)
#define DRQ (ALT_STATUS(d) & CB_STATUS_DRQ)

int ata_status_notbusy(ata_disk *d)
{
    u8 st = ata_in8(d, CB_ALT_STATUS);
    int ntries;
    for (ntries=0; ntries < 1000; ntries++)
    {
        u8 status = ata_in8(d, CB_ALT_STATUS);
        if (status & CB_STATUS_ERR)
            ata_error(d, "inner");

        if ((status & CB_STATUS_BUSY) == 0)
            return 0;
    }
    ata_error(d, "timeout");
    return -1;
}


int
ata_select(ata_disk *d)
{
    do {
        if (ata_status_notbusy(d)) return -1;
    } while (DRQ);

    ata_out8(d, CB_DEV_HEAD, d->devnum ? 0x10 : 0x00);
    DELAY400NS;

    if (ata_status_notbusy(d)) return -1;
    assert(!DRQ);
    return 0;
}

int
ata_setup_command(ata_disk *d, u8 fr, u8 sc, u8 sn, u8 cl, u8 ch)
{
    if (ata_select(d)) return -1;

    ata_out8(d, CB_DEV_CTRL, USE_DMA ? 0 : CB_DC_NIEN);
    ata_out8(d, CB_FEATURE, fr);
    ata_out8(d, CB_SECTOR_CNT, sc);
    ata_out8(d, CB_SECTOR_NUM, sn);
    ata_out8(d, CB_CYL_LOW, cl);
    ata_out8(d, CB_CYL_HIGH, ch);
//    ata_out8(d, CB_DEV_HEAD, CB_DEV_HEAD_LBA | (d->devnum ? 0x10 : 0x00));
//       (should be handled by ata_select)

    DELAY400NS;

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
ata_identify_device(ata_disk *d, IDENTIFY_DEVICE_DATA *id)
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

    if (ata_status_notbusy(d)) {

        return -1; 
    }

    u8 status = ata_in8(d, CB_STATUS);

    if ((status & (CB_STATUS_BUSY | CB_STATUS_DRQ)) == CB_STATUS_DRQ)
    {
        assert(sizeof(*id) == 512);
        ata_ins16(d, id, sizeof(*id));
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

    d->max_lba = id->UserAddressableSectors; // via LBA28
    d->sector_size = 512;

    if (d->type == ATAPI) {
        if (atapi_get_capacity(d)) return -1;
    }

    DPRINT(0, "%s %x:%d maxLba=%u blocksize=%u",
             ata_types[d->type], d->base_port, d->devnum,
             d->max_lba, d->sector_size);
    DPRINT(1, "    %s   #%s  Rev %s ",
             id->ModelNumber, id->SerialNumber, id->FirmwareRevision);

    assert(!DRQ);
    return 0;
}

int atapi_packet(ata_disk *d,
                 const u8 *inpkt, int inpktsize,
                 u8 *outbuf, int outbufsize)
{
    if (ata_setup_command(d, 0, 0, 0, outbufsize & 0xff, outbufsize >> 8)) return -1;

    ata_out8(d, CB_CMD, CMD_PACKET);

    DELAY400NS;
    if (ata_status_notbusy(d)) return -1; 

    u8 status = ata_in8(d, CB_STATUS);
    if (status & CB_STATUS_ERR) {
        ata_error(d, "before sending atapi packet");
        return -1;
    }
    assert(DRQ);  // it wants us to send the inpkt

    ata_outs16(d, inpkt, inpktsize);

    DELAY400NS;
    if (ata_status_notbusy(d)) return -1; 

    status = ata_in8(d, CB_STATUS);
    assert (!(status & CB_STATUS_BUSY));

    if (status & CB_STATUS_ERR) {
        ata_error(d, "after sending atapi packet");
        return -1;
    }

    assert (status & CB_STATUS_DRQ);
    while (status & CB_STATUS_DRQ) {
        u16 nbytes = (ata_in8(d, CB_LBA3) << 8) | ata_in8(d, CB_LBA2);
        assert(nbytes <= outbufsize);
        assert(nbytes > 0);
        DPRINT(2, "atapi recving %d bytes", nbytes);
        ata_ins16(d, outbuf, nbytes);

        // BUG: receives into the same buffer until !DRQ
        //   so buffer better be big enough

        status = ata_in8(d, CB_STATUS);
    }

    DELAY400NS;
    assert(!DRQ);
    return 0;
}

int
atapi_get_capacity(ata_disk *d)
{
    const u8 atapi_cmd_pkt[18] = { SCSI_CMD_READ_CAPACITY, 0 };
    u32 buf[2];

    if (atapi_packet(d, (void *) atapi_cmd_pkt, sizeof(atapi_cmd_pkt), (void *) buf, sizeof(buf)))
    {
        ata_error(d, "get capacity");
        return -1;
    }

    d->max_lba = NTOHL(buf[0]);
    d->sector_size = NTOHL(buf[1]);

    return 0;
}

int
ata_read_lba28(ata_disk *d, u8 *buf, u16 buflen, u32 lba)
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

    if (ata_status_notbusy(d)) {
        ata_error(d, "preread");
        return -1; 
    }

    u8 status = ata_in8(d, CB_STATUS);

    if ((status & (CB_STATUS_BUSY | CB_STATUS_DRQ)) == CB_STATUS_DRQ)
    {
        ata_ins16(d, buf, buflen);

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

int
atapi_read_lba(ata_disk *d, u8 *buf, u16 buflen, u32 lba)
{
    const u8 atapi_cmd_pkt[12] = { SCSI_CMD_READ, 0, lba >> 24, lba >> 16, lba >> 8, lba,
                                  0, 0, 0, 1 /* sectors */, 0, 0 };

    if (atapi_packet(d, (void *) atapi_cmd_pkt, sizeof(atapi_cmd_pkt), (void *) buf, buflen))
    {
        ata_error(d, "atapi_read_lba");
        return -1;
    }

    return 0;
}
