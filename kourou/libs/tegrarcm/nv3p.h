/*
 * Copyright (c) 2011, NVIDIA CORPORATION
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *  * Neither the name of NVIDIA CORPORATION nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef NV3P_H
#define NV3P_H

#include "usb.h"

// nv3p protocol version
#define NV3P_VERSION   1

// Defines the maximum length of a string, including null terminator.
#define NV3P_STRING_MAX (32)

// commands
#define NV3P_CMD_GET_PLATFORM_INFO       0x01
#define NV3P_CMD_GET_BCT                 0x02
#define NV3P_CMD_DL_BCT                  0x04
#define NV3P_CMD_DL_BL                   0x06
#define NV3P_CMD_STATUS                  0x0a

// nack codes
#define NV3P_NACK_SUCCESS                0x1
#define NV3P_NACK_BAD_CMD                0x2
#define NV3P_NACK_BAD_DATA               0x3

// Holds the handle to the nv3p state.
typedef struct nv3p_state *nv3p_handle_t;

// tegra2 chip sku
#define TEGRA2_CHIP_SKU_AP20     0x01
#define TEGRA2_CHIP_SKU_T20_7    0x07
#define TEGRA2_CHIP_SKU_T20      0x08
#define TEGRA2_CHIP_SKU_T25SE    0x14
#define TEGRA2_CHIP_SKU_AP25     0x17
#define TEGRA2_CHIP_SKU_T25      0x18
#define TEGRA2_CHIP_SKU_AP25E    0x1b
#define TEGRA2_CHIP_SKU_T25E     0x1c

// tegra3 chip sku
#define TEGRA3_CHIP_SKU_AP30     0x87
#define TEGRA3_CHIP_SKU_T30      0x81
#define TEGRA3_CHIP_SKU_T30S     0x83
#define TEGRA3_CHIP_SKU_T33      0X80

// tegra114 chip sku
#define TEGRA114_CHIP_SKU_T114   0x00
#define TEGRA114_CHIP_SKU_T114_1 0x01

// tegra124 chip sku
#define TEGRA124_CHIP_SKU_T124   0x00

// boot device type
#define NV3P_DEV_TYPE_NAND              0x1
#define NV3P_DEV_TYPE_EMMC              0x2
#define NV3P_DEV_TYPE_SPI               0x3
#define NV3P_DEV_TYPE_IDE               0x4
#define NV3P_DEV_TYPE_NAND_X16          0x5
#define NV3P_DEV_TYPE_SNOR              0x6
#define NV3P_DEV_TYPE_MUX_ONE_NAND      0x7
#define NV3P_DEV_TYPE_MOBILE_LBA_NAND   0x8

/*
 * Defines sizes for packet headers in bytes.
 */
#define NV3P_PACKET_SIZE_BASIC      (3 * 4)
#define NV3P_PACKET_SIZE_COMMAND    (2 * 4)
#define NV3P_PACKET_SIZE_DATA       (1 * 4)
#define NV3P_PACKET_SIZE_ENCRYPTED  (1 * 4)
#define NV3P_PACKET_SIZE_FOOTER     (1 * 4)
#define NV3P_PACKET_SIZE_ACK        (0 * 4)
#define NV3P_PACKET_SIZE_NACK       (1 * 4)

// packet type
#define NV3P_PACKET_TYPE_CMD        0x1
#define NV3P_PACKET_TYPE_DATA       0x2
#define NV3P_PACKET_TYPE_ENCRYPTED  0x3
#define NV3P_PACKET_TYPE_ACK        0x4
#define NV3P_PACKET_TYPE_NACK       0x5

/*
 * Holds the chip ID.
 */
typedef struct {
	uint16_t id;
	uint8_t major;
	uint8_t minor;
} nv3p_chip_id_t;

/*
 * board ID
 */
typedef struct {
	uint32_t board_no;
	uint32_t fab;
	uint32_t mem_type;
	uint32_t freq;
} nv3p_board_id_t;

/*
 * Command arguments.
 */

/*
 * nv3p_cmd_status_t: high-level ACK/NACK for commands. This may be used in
 * the event of a mass-storage device failure, etc.
 */
typedef struct {
	char msg[NV3P_STRING_MAX];
	uint32_t code;
	uint32_t flags; // reseved for now
} nv3p_cmd_status_t;

/*
 * nv3p_platform_info_t: retrieves the system information. All parameters
 * are output parameters.
 */
typedef struct {
	uint64_t uid[2];
	nv3p_chip_id_t chip_id;
	uint32_t sku;
	uint32_t version;
	uint32_t boot_device;
	uint32_t op_mode;
	uint32_t dev_conf_strap;
	uint32_t dev_conf_fuse;
	uint32_t sdram_conf_strap;
	uint32_t reserved[2];
	nv3p_board_id_t board_id;
} nv3p_platform_info_t;


/*
 * nv3p_bct_info_t: holds information about BCT size
 */
typedef struct {
	uint32_t length;
} nv3p_bct_info_t;

/*
 * nv3p_cmd_get_bct_t: retrieves the BCT from the device
 */
typedef struct {
	uint32_t length;
} nv3p_cmd_get_bct_t;

/*
 * nv3p_cmd_dl_bct_t: downloads the system's BCT.
 */
typedef struct {
	uint32_t length;
} nv3p_cmd_dl_bct_t;

/*
 * nv3p_cmd_dl_bl_t: downloads the system's bootloader.
 */
typedef struct {
	uint64_t length;
	uint32_t address; // Load address
	uint32_t entry; // Execution entry point
} nv3p_cmd_dl_bl_t;

int nv3p_open(nv3p_handle_t *h3p, usb_device_t *usb);
void nv3p_close(nv3p_handle_t h3p);
int nv3p_cmd_send(nv3p_handle_t h3p, uint32_t command, void *args);
int nv3p_cmd_recv(nv3p_handle_t h3p, uint32_t *command,	void **args);
int nv3p_data_send(nv3p_handle_t h3p, uint8_t *data, uint32_t length);
void nv3p_ack(nv3p_handle_t h3p);
int nv3p_data_recv(nv3p_handle_t h3p, uint8_t *data, uint32_t length);

#endif // NV3P_H
