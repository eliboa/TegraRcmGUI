#pragma once
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long int u64;

#define COMMAND      0x5543
#define BIN_PACKET   0x4249
#define EXEC_COMMAND 0x4558
#define RESPONSE     0x5245
#define DEVINFO      0x4445

#define MAX_FILE_SIZE     0x6400000 //100MB
#define SUCCESS           0
#define USB_BUFFER_LENGTH 0x1000
#define RESPONSE_MAX_SIZE 0x20

typedef enum _UC_CommandType
{
    NONE,
    COPY,
    READ_SD_FILE,
    WRITE_SD_FILE,
    PUSH_PAYLOAD,
    REBOOT_RCM,
    GET_DEVICE_INFO,
    GET_STATUS,
    SET_AUTORCM_ON,
    SET_AUTORCM_OFF

} UC_CommandType;

typedef struct _UC_Header
{
    u16 signature;
    UC_CommandType command;

} UC_Header;

typedef struct _UC_SDIO
{
    u16 signature;
    UC_CommandType command;
    char path[256];
    u32 file_size;
    bool is_lz4_compressed;
    bool create_always;

} UC_SDIO;

typedef struct _UC_EXEC
{
    u16 signature;
    UC_CommandType command;
    u32 bin_size;

} UC_EXEC;

typedef struct _UC_BlockHeader
{
    u16 signature;
    u32 block_size;
    u32 block_full_size; // Full size if LZ4 compressed

} UC_BlockHeader;


typedef struct _UC_DeviceInfo
{
    u16 signature;            // UC signature
    u32 battery_capacity;     // Fuel gauge
    bool autoRCM;             // autoRCM state
    u32 burnt_fuses;          // Number of burnt fuses
    bool sdmmc_initialized;   // MMC FS initialized
    u8 emmc_fs_type;          // 3 is FAT32, 4 is exFAT
    u16 emmc_fs_cl_size;      // Cluster size in sectors (always 512B per sectors)
    DWORD emmc_fs_last_cl;    // Last allocated cluster
    DWORD emmc_fs_free_cl;    // Number of free cluster
    bool cfw_sxos;            // SX OS bootloader
    bool cfw_ams;             // AMS fusee
    bool cbl_hekate;          // Hekate

} UC_DeviceInfo;
