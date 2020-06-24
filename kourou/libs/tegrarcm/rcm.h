/*
 * Copyright (c) 2011-2016, NVIDIA CORPORATION
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
#ifndef _RCM_H
#define _RCM_H

#include <stdint.h>

#define RCM_MIN_MSG_LENGTH 1024 // In bytes

#define NVBOOT_VERSION(a,b) ((((a)&0xffff) << 16) | ((b)&0xffff))
#define RCM_VERSION_1 (NVBOOT_VERSION(1, 0))
#define RCM_VERSION_35 (NVBOOT_VERSION(0x35, 1))
#define RCM_VERSION_40 (NVBOOT_VERSION(0x40, 1))
#define RCM_VERSION_MAJOR(ver) ((ver) >> 16)
#define RCM_VERSION_MINOR(ver) ((ver) & 0xffff)

// recovery mode commands
#define RCM_CMD_NONE               0x0
#define RCM_CMD_SYNC               0x1
#define RCM_CMD_DL_MINILOADER      0x4
#define RCM_CMD_QUERY_BR_VERSION   0x5
#define RCM_CMD_QUERY_RCM_VERSION  0x6
#define RCM_CMD_QUERY_BD_VERSION   0x7

// AES block size in bytes
#define RCM_AES_BLOCK_SIZE      (128 / 8)
#define RCM_RSA_MODULUS_SIZE	(2048 / 8)
#define RCM_RSA_SIG_SIZE	RCM_RSA_MODULUS_SIZE

/*
 * Defines the header for RCM messages from the host.
 * Messages from the host have the format:
 *     rcm_msg_t
 *     payload
 *     padding
 */
typedef struct {
	uint32_t len_insecure;		// 000-003
	uint8_t cmac_hash[RCM_AES_BLOCK_SIZE];	// 004-013
	uint8_t reserved[16];		// 014-023
	uint32_t opcode;		// 024-027
	uint32_t len_secure;		// 028-02b
	uint32_t payload_len;		// 02c-02f
	uint32_t rcm_version;		// 030-033
	uint8_t args[48];		// 034-063
	uint8_t padding[16];		// 064-073
} rcm1_msg_t;

typedef struct {
	uint32_t len_insecure;		// 000-003
	uint8_t modulus[RCM_RSA_MODULUS_SIZE];	// 004-103
	union {
		uint8_t cmac_hash[RCM_AES_BLOCK_SIZE];
		uint8_t rsa_pss_sig[RCM_RSA_SIG_SIZE];
	} object_sig;			// 104-203
	uint8_t reserved[16];		// 204-213
	uint32_t ecid[4];		// 214-223
	uint32_t opcode;		// 224-227
	uint32_t len_secure;		// 228-22b
	uint32_t payload_len;		// 22c-22f
	uint32_t rcm_version;		// 230-233
	uint8_t args[48];		// 234-263
	uint8_t padding[16];		// 264-273
} rcm35_msg_t;

typedef struct {
	uint32_t len_insecure;		// 000-003
	uint8_t modulus[RCM_RSA_MODULUS_SIZE];	// 004-103
	struct {
		uint8_t cmac_hash[RCM_AES_BLOCK_SIZE];
		uint8_t rsa_pss_sig[RCM_RSA_SIG_SIZE];
	} object_sig;			// 104-213
	uint8_t reserved[16];		// 214-223
	uint32_t ecid[4];		// 224-233
	uint32_t opcode;		// 234-237
	uint32_t len_secure;		// 238-23b
	uint32_t payload_len;		// 23c-23f
	uint32_t rcm_version;		// 240-243
	uint8_t args[48];		// 244-273
	uint8_t padding[16];		// 274-283
} rcm40_msg_t;

// security operating modes
#define RCM_OP_MODE_PRE_PRODUCTION  0x1
#define RCM_OP_MODE_DEVEL           0x3
#define RCM_OP_MODE_ODM_SECURE      0x4
#define RCM_OP_MODE_ODM_OPEN        0x5
#define RCM_OP_MODE_ODM_SECURE_PKC  0x6

int rcm_init(uint32_t version, const char *keyfile);
uint32_t rcm_get_msg_len(uint8_t *msg);
int rcm_create_msg(
	uint32_t opcode,
	uint8_t *args,
	uint32_t args_len,
	uint8_t *payload,
	uint32_t payload_len,
	uint8_t **msg);

#endif // _RCM_H
