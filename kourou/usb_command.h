#pragma once
#include "windows.h"
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long int u64;

#define COMMAND      0x5543
#define BIN_PACKET   0x4249
#define EXEC_COMMAND 0x4558

typedef enum _UC_CommandType : u8
{
    NONE,
    COPY,
    READ_SD_FILE,
    WRITE_SD_FILE,
    PUSH_PAYLOAD,
    REBOOT_RCM

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
    int block_size;
    int block_full_size; // Full size if LZ4 compressed

} UC_BlockHeader;
