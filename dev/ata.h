#ifndef SPOS_ATA_H_
#define SPOS_ATA_H_
// 512 bytes returned by IDENTIFY_DEVICE packet
#define bits unsigned short
#define bool unsigned short

typedef struct PACKED _IDENTIFY_DEVICE_DATA {
  struct {
    bool                        __Reserved1        : 1;
    bool                        __Retired3         : 1;
    bool ResponseIncomplete                        : 1;
    bits                        __Retired2         : 3;
    bool FixedDevice                               : 1;
    bool RemovableMedia                            : 1;

    bits                        __Retired1         : 7;
    bool DeviceType                                : 1;
  } GeneralConfiguration;

  u16    NumCylinders;
  u16                           __ReservedWord2;

  u16    NumHeads;
  u16                           __Retired1[2];

  u16    NumSectorsPerTrack;

  u16    VendorUnique1[3];
  char   SerialNumber[20];
  u16                           __Retired2[2];
  u16                           __Obsolete1;
  char   FirmwareRevision[8];
  char   ModelNumber[40];

  u8     MaximumBlockTransfer;

  u8     VendorUnique2;
  u16                           __ReservedWord48;

  struct {
    bits                        __ReservedByte49   : 8;
    bool DmaSupported                              : 1;
    bool LbaSupported                              : 1;
    bool IordyDisable                              : 1;
    bool IordySupported                            : 1;
    bool                        __Reserved1        : 1;
    bool StandybyTimerSupport                      : 1;
    bits                        __Reserved2        : 2;
    bits                        __ReservedWord50   : 16;
  } Capabilities;

  u16                           __ObsoleteWords51[2];

  bits   TranslationFieldsValid                    : 3;
  bits                          __Reserved3        : 13;

  u16    NumberOfCurrentCylinders;
  u16    NumberOfCurrentHeads;
  u16    CurrentSectorsPerTrack;
  u32    CurrentSectorCapacity;
  u8     CurrentMultiSectorSetting;

  bool   MultiSectorSettingValid                   : 1;
  bits                          __ReservedByte59   : 7;

  u32    UserAddressableSectors;
  u16                           __ObsoleteWord62;

  bits   MultiWordDMASupport                       : 8;
  bits   MultiWordDMAActive                        : 8;
  bits   AdvancedPIOModes                          : 8;
  bits                          __ReservedByte64   : 8;

  u16    MinimumMWXferCycleTime;
  u16    RecommendedMWXferCycleTime;
  u16    MinimumPIOCycleTime;
  u16    MinimumPIOCycleTimeIORDY;
  u16                           __ReservedWords69[6];
  bits   QueueDepth                                : 5;
  bits                          __ReservedWord75   : 11;
  u16                           __ReservedWords76[4];
  u16    MajorRevision;
  u16    MinorRevision;
  struct {
    bool SmartCommands                             : 1;
    bool SecurityMode                              : 1;
    bool RemovableMediaFeature                     : 1;
    bool PowerManagement                           : 1;
    bool                        __Reserved1        : 1;
    bool WriteCache                                : 1;
    bool LookAhead                                 : 1;
    bool ReleaseInterrupt                          : 1;

    bool ServiceInterrupt                          : 1;
    bool DeviceReset                               : 1;
    bool HostProtectedArea                         : 1;
    bool                        __Obsolete1        : 1;
    bool WriteBuffer                               : 1;
    bool ReadBuffer                                : 1;
    bool Nop                                       : 1;
    bool                        __Obsolete2        : 1;

    bool DownloadMicrocode                         : 1;
    bool DmaQueued                                 : 1;
    bool Cfa                                       : 1;
    bool AdvancedPm                                : 1;
    bool Msn                                       : 1;
    bool PowerUpInStandby                          : 1;
    bool ManualPowerUp                             : 1;
    bool                        __Reserved2        : 1;

    bool SetMax                                    : 1;
    bool Acoustics                                 : 1;
    bool BigLba                                    : 1;
    bool DeviceConfigOverlay                       : 1;
    bool FlushCache                                : 1;
    bool FlushCacheExt                             : 1;
    bits                        __Reserved3        : 2;

    bool SmartErrorLog                             : 1;
    bool SmartSelfTest                             : 1;
    bool MediaSerialNumber                         : 1;
    bool MediaCardPassThrough                      : 1;
    bool StreamingFeature                          : 1;
    bool GpLogging                                 : 1;
    bool WriteFua                                  : 1;
    bool WriteQueuedFua                            : 1;

    bool WWN64Bit                                  : 1;
    bool URGReadStream                             : 1;
    bool URGWriteStream                            : 1;
    bits ReservedForTechReport                     : 2;
    bool IdleWithUnloadFeature                     : 1;
    bits                        __Reserved4        : 2;
  } CommandSetSupport, CommandSetActive;

  bits   UltraDMASupport                           : 8;
  bits   UltraDMAActive                            : 8;

  u16                           __ReservedWord89[4];
  u16    HardwareResetResult;

  bits   CurrentAcousticValue                      : 8;
  bits   RecommendedAcousticValue                  : 8;

  u16                           __ReservedWord95[5];
  u32    Max48BitLBA[2];
  u16    StreamingTransferTime;
  u16                           __ReservedWord105;

  struct {
    bits LogicalSectorsPerPhysicalSector           : 4;
    bits                        __Reserved0        : 8;
    bool LogicalSectorLongerThan256Words           : 1;
    bool MultipleLogicalSectorsPerPhysicalSector   : 1;
    bits                        __Reserved1        : 2;
  } PhysicalLogicalSectorSize;

  u16    InterSeekDelay;
  u16    WorldWideName[4];
  u16    ReservedForWorldWideName128[4];
  u16    ReservedForTlcTechnicalReport;
  u16    WordsPerLogicalSector[2];

  struct {
    bool ReservedForDrqTechnicalReport             : 1;
    bool WriteReadVerifySupported                  : 1;
    bits                        __Reserved01       : 11;
    bits                        __Reserved1        : 2;
  } CommandSetSupportExt, CommandSetActiveExt;

  u16    ReservedForExpandedSupportandActive[6];

  u16    MsnSupport                                : 2;
  u16                           __ReservedWord1274 : 14;

  struct {
    bool SecuritySupported                         : 1;
    bool SecurityEnabled                           : 1;
    bool SecurityLocked                            : 1;
    bool SecurityFrozen                            : 1;
    bool SecurityCountExpired                      : 1;
    bool EnhancedSecurityEraseSupported            : 1;
    bits                        __Reserved0        : 2;

    bool SecurityLevel                             : 1;
    bits                        __Reserved1        : 7;
  } SecurityStatus;

  u16                           __ReservedWord129[31];

  struct {
    bits MaximumCurrentInMA2                       : 12;
    bool CfaPowerMode1Disabled                     : 1;
    bool CfaPowerMode1Required                     : 1;
    bool                        __Reserved0        : 1;
    bool Word160Supported                          : 1;
  } CfaPowerModel;

  u16 ReservedForCfaWord161[8];

  struct {
    bool SupportsTrim                              : 1;
    bits                        __Reserved0        : 15;
  } DataSetManagementFeature;

  u16    ReservedForCfaWord170[6];

  u16    CurrentMediaSerialNumber[30];
  u16                           __ReservedWord206;
  u16                           __ReservedWord207[2];

  struct {
    bits AlignmentOfLogicalWithinPhysical          : 14;
    bool Word209Supported                          : 1;
    bits                        __Reserved0        : 1;
  } BlockAlignment;

  u16    WriteReadVerifySectorCountMode3Only[2];
  u16    WriteReadVerifySectorCountMode2Only[2];

  struct {
    bool NVCachePowerModeEnabled                   : 1;
    bits                        __Reserved0        : 3;
    bool NVCacheFeatureSetEnabled                  : 1;
    bits                        __Reserved1        : 3;

    bits NVCachePowerModeVersion                   : 4;
    bits NVCacheFeatureSetVersion                  : 4;
  } NVCacheCapabilities;

  u16    NVCacheSizeLSW;
  u16    NVCacheSizeMSW;
  u16    NominalMediaRotationRate;
  u16                           __ReservedWord218;

  struct {
    u8   NVCacheEstimatedTimeToSpinUpInSeconds;
    u8                          __Reserved;
  } NVCacheOptions;

  u16                           __ReservedWord220[35];
  bits   Signature  : 8;
  bits   CheckSum   : 8;
} IDENTIFY_DEVICE_DATA;

typedef enum { NONE=0, ATA, SATA, ATAPI, SATAPI, UNKNOWN } ata_type_t;

typedef struct
{
    u16         base_port;
    u8          devnum;             // local to this controller, 0 or 1

    u32         max_lba;
    u32         sector_size;

    ata_type_t  type;

    IDENTIFY_DEVICE_DATA id;

} ata_disk;

static const char *ata_types[] = { 
    "NONE", "ATA  ", "SATA ", "ATAPI", "SATAPI", "UNKNOWN"
};

extern ata_disk disks[];

int init_ata();
int ata_identify_device(ata_disk *disk, IDENTIFY_DEVICE_DATA *id);
int ata_read_lba28(ata_disk *disk, u8 *buf, u16 buflen, u32 lba);
int atapi_read_lba(ata_disk *d, u8 *buf, u16 buflen, u32 lba);
int atapi_get_capacity(ata_disk *d);

#endif
