/*
 * Ariane, payload based USB backend
 *
 * Copyright (c) 2020 eliboa
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _ARIANE_H_
#define _ARIANE_H_

#include <utils/util.h>
#include "rcm_usb.h"
#include "usb_command.h"

#define USB_EP_BULK_IN_BUF_ADDR   0xFF000000
#define USB_EP_BULK_OUT_BUF_ADDR  0xFF800000

#define ALLOW_SEPT_REBOOT  1
#define FORBID_SEPT_REBOOT 0

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

// This is a safe and unused DRAM region for our payloads.
#define RELOC_META_OFF      0x7C
#define PATCHED_RELOC_SZ    0x94
#define PATCHED_RELOC_STACK 0x40007000
#define PATCHED_RELOC_ENTRY 0x40010000
#define EXT_PAYLOAD_ADDR    0xC0000000
#define RCM_PAYLOAD_ADDR    (EXT_PAYLOAD_ADDR + ALIGN(PATCHED_RELOC_SZ, 0x10))
#define COREBOOT_END_ADDR   0xD0000000
#define CBFS_DRAM_EN_ADDR   0x4003e000
#define  CBFS_DRAM_MAGIC    0x4452414D // "DRAM"
#endif

typedef struct _DispatchEntry {
    UC_CommandType command;
    void (*f_ptr)();
} DispatchEntry;

typedef struct _SystemTitle {
    const char fw_version[10];
    const char nca_filename[40];
    u32 kb;
    const char nca_filename_exfat[40];
} SystemTitle;

bool command_dispatcher(UC_CommandType command);
void send_response(const void* in_buffer, u32 size);
bool recv_bin_packet(u32 *bytesReceived);
void hos_set_keys(bool allow_sept_reboot);
bool get_autorcm_state(bool *state);
bool set_autorcm_state(bool autoRCM);
void set_deviceinfo();
void reboot_to_rcm();

// Command functions
UC_CommandType usb_command_read();
void get_deviceinfo_command();
void sdmmc_filesize_command();
void sdmmc_isdir_command();
void sdmmc_mkdir_command();
void sdmmc_mkpath_command();
void sdmmc_readfile_command();
void sdmmc_writefile_command();
void rcm_reboot_command();
void set_autorcm_on_command();
void set_autorcm_off_command();
void get_keys_command();
void launch_payload_command();

#endif
