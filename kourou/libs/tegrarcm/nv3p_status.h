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
#ifndef NV3P_STATUS_H
#define NV3P_STATUS_H

// nv3p status codes.
typedef enum {
	nv3p_status_ok=                  0x00000000,
	nv3p_status_unknown=             0x00000001,
	nv3p_status_dev_fail=            0x00000002,
	nv3p_status_not_implemented=     0x00000003,
	nv3p_status_invalid_state=       0x00000004,
	nv3p_status_bad_param=           0x00000005,
	nv3p_status_inv_dev=             0x00000006,
	nv3p_status_inv_part=            0x00000007,
	nv3p_status_part_tab_req=        0x00000008,
	nv3p_status_mass_stor_fail=      0x00000009,
	nv3p_status_part_create=         0x0000000A,
	nv3p_status_bct_req=             0x0000000B,
	nv3p_status_inv_bct=             0x0000000C,
	nv3p_status_missing_bl_info=     0x0000000D,
	nv3p_status_inv_part_tab=        0x0000000E,
	nv3p_status_crypto_fail=         0x0000000F,
	nv3p_status_too_many_bl=         0x00000010,
	nv3p_status_not_boot_dev=        0x00000011,
	nv3p_status_not_supp=            0x00000012,
	nv3p_status_inv_part_name=       0x00000013,
	nv3p_status_inv_cmd_after_verif= 0x00000014,
	nv3p_status_bct_not_found=       0x00000015,
	nv3p_status_bct_write_fail=      0x00000016,
	nv3p_status_bct_read_verif=      0x00000017,
	nv3p_status_inv_bct_size=        0x00000018,
	nv3p_status_inv_bct_part_id=     0x00000019,
	nv3p_status_err_bbt=             0x0000001A,
	nv3p_status_no_bl=               0x0000001B,
	nv3p_status_rtc_access_fail=     0x0000001C,
	nv3p_status_unsparse_fail=       0x0000001D,
} nv3p_status_t;

#endif // NV3P_STATUS_H
