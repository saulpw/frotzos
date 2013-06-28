#include "kernel.h"
#include "dev/ata.h"

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

void
setup_hdd()
{
    int i=0;
    for (i=0; i < 2; ++i)
    {
#if 0
        IDENTIFY_DEVICE_DATA devid;
        int rc = reg_pio_data_in_lba28(i, CMD_IDENTIFY_DEVICE, 0, 0, 0,
                                        (u8 *) &devid, 1, 0);
        if (rc == 0) {
            WORDSWAP(devid.ModelNumber);
            WORDSWAP(devid.SerialNumber);
            WORDSWAP(devid.FirmwareRevision);

            kprintf("ide%d: %s (%d sectors, C/H/S %d/%d/%d)\r\n",
                    i, devid.ModelNumber,
                    devid.UserAddressableSectors,
                    devid.NumberOfCurrentCylinders,
                    devid.NumberOfCurrentHeads,
                    devid.CurrentSectorsPerTrack);
        }
#endif
    }
}
