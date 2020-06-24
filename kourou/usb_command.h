#ifndef UCOMMAND_H
#define UCOMMAND_H
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long int u64;
typedef unsigned long DWORD;

#define COMMAND           0x5543
#define BIN_PACKET        0x4249
#define EXEC_COMMAND      0x4558
#define RESPONSE          0x5245
#define DEVINFO           0x4445

#define MAX_FILE_SIZE     0x6400000 //100MB
#define SUCCESS           0
#define USB_BUFFER_LENGTH 0x1000
#define RESPONSE_MAX_SIZE 0x20

#define FS_FAT32	3
#define FS_EXFAT	4

typedef enum _UC_CommandType : u8
{
    NONE,
    COPY,
    READ_SD_FILE,
    WRITE_SD_FILE,
    PUSH_PAYLOAD,
    REBOOT_RCM,
    GET_DEVICE_INFO,
    GET_STATUS

} UC_CommandType;


typedef struct _UC_Header
{
    u16 signature = COMMAND;
    UC_CommandType command;

} UC_Header;

typedef struct _UC_SDIO
{
    u16 signature = COMMAND;
    UC_CommandType command;
    char path[256];
    u32 file_size;
    bool is_lz4_compressed = false;
    bool create_always = false;

} UC_SDIO;

typedef struct _UC_EXEC
{
    u16 signature = EXEC_COMMAND;
    UC_CommandType command;
    u32 bin_size;

} UC_EXEC;

typedef struct _UC_BlockHeader
{
    u16 signature = BIN_PACKET;
    u32 block_size;
    u32 block_full_size; // Full size if LZ4 compressed

} UC_BlockHeader;

typedef struct _UC_DeviceInfo
{
    u16 signature = DEVINFO;  // UC signature
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

/*--------------*/
/* ERROR CODES  */
/*--------------*/
// FS
#define OPEN_FILE_FAILED            -0x0001
#define FILE_TOO_LARGE              -0x0002
#define PATH_TOO_LONG               -0x0003
// COM
#define SEND_COMMAND_FAILED         -0x1001
#define RECEIVE_COMMAND_FAILED      -0x1002
#define USB_WRITE_FAILED            -0x1003

#endif // UCOMMAND_H
