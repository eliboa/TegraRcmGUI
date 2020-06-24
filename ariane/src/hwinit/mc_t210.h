/*
* Copyright (c) 2014, NVIDIA Corporation. All rights reserved.
*
* This software is licensed under the terms of the GNU General Public
* License version 2, as published by the Free Software Foundation, and
* may be copied, distributed, and modified under those terms.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*/

#ifndef _MC_T210_H_
#define _MC_T210_H_

#include "types.h"

// Memory Controller registers we need/care about

struct tegra_mc_regs {
	vu32 rsvd_0x0[4];			/* 0x00 */
	vu32 smmu_config;			/* 0x10 */
	vu32 smmu_tlb_config;		/* 0x14 */
	vu32 smmu_ptc_config;		/* 0x18 */
	vu32 smmu_ptb_asid;			/* 0x1c */
	vu32 smmu_ptb_data;			/* 0x20 */
	vu32 rsvd_0x24[3];			/* 0x24 */
	vu32 smmu_tlb_flush;		/* 0x30 */
	vu32 smmu_ptc_flush;		/* 0x34 */
	vu32 rsvd_0x38[6];			/* 0x38 */
	vu32 emem_cfg;			/* 0x50 */
	vu32 emem_adr_cfg;			/* 0x54 */
	vu32 emem_adr_cfg_dev0;		/* 0x58 */
	vu32 emem_adr_cfg_dev1;		/* 0x5c */
	vu32 emem_adr_cfg_channel_mask;	/* 0x60 */
	vu32 emem_adr_cfg_bank_mask_0;	/* 0x64 */
	vu32 emem_adr_cfg_bank_mask_1;	/* 0x68 */
	vu32 emem_adr_cfg_bank_mask_2;	/* 0x6c */
	vu32 security_cfg0;			/* 0x70 */
	vu32 security_cfg1;			/* 0x74 */
	vu32 rsvd_0x78[6];			/* 0x78 */
	vu32 emem_arb_cfg;			/* 0x90 */
	vu32 emem_arb_outstanding_req;	/* 0x94 */
	vu32 emem_arb_timing_rcd;		/* 0x98 */
	vu32 emem_arb_timing_rp;		/* 0x9c */
	vu32 emem_arb_timing_rc;		/* 0xa0 */
	vu32 emem_arb_timing_ras;		/* 0xa4 */
	vu32 emem_arb_timing_faw;		/* 0xa8 */
	vu32 emem_arb_timing_rrd;		/* 0xac */
	vu32 emem_arb_timing_rap2pre;	/* 0xb0 */
	vu32 emem_arb_timing_wap2pre;	/* 0xb4 */
	vu32 emem_arb_timing_r2r;		/* 0xb8 */
	vu32 emem_arb_timing_w2w;		/* 0xbc */
	vu32 emem_arb_timing_r2w;		/* 0xc0 */
	vu32 emem_arb_timing_w2r;		/* 0xc4 */
	vu32 emem_arb_misc2;		/* 0xC8 */
	vu32 rsvd_0xcc[1];			/* 0xCC */
	vu32 emem_arb_da_turns;		/* 0xd0 */
	vu32 emem_arb_da_covers;		/* 0xd4 */
	vu32 emem_arb_misc0;		/* 0xd8 */
	vu32 emem_arb_misc1;		/* 0xdc */
	vu32 emem_arb_ring1_throttle;	/* 0xe0 */
	vu32 emem_arb_ring3_throttle;	/* 0xe4 */
	vu32 emem_arb_override;		/* 0xe8 */
	vu32 emem_arb_rsv;			/* 0xec */
	vu32 rsvd_0xf0[1];			/* 0xf0 */
	vu32 clken_override;		/* 0xf4 */
	vu32 timing_control_dbg;		/* 0xf8 */
	vu32 timing_control;		/* 0xfc */
	vu32 stat_control;			/* 0x100 */
	vu32 rsvd_0x104[65];		/* 0x104 */
	vu32 emem_arb_isochronous_0;	/* 0x208 */
	vu32 emem_arb_isochronous_1;	/* 0x20c */
	vu32 emem_arb_isochronous_2;	/* 0x210 */
	vu32 rsvd_0x214[38];		/* 0x214 */
	vu32 dis_extra_snap_levels;		/* 0x2ac */
	vu32 rsvd_0x2b0[90];		/* 0x2b0 */
	vu32 video_protect_vpr_override;	/* 0x418 */
	vu32 rsvd_0x41c[93];		/* 0x41c */
	vu32 video_protect_vpr_override1;	/* 0x590 */
	vu32 rsvd_0x594[29];		/* 0x594 */
	vu32 display_snap_ring;		/* 0x608 */
	vu32 rsvd_0x60c[15];		/* 0x60c */
	vu32 video_protect_bom;		/* 0x648 */
	vu32 video_protect_size_mb;		/* 0x64c */
	vu32 video_protect_reg_ctrl;	/* 0x650 */
	vu32 rsvd_0x654[2];			/* 0x654 */
	vu32 iram_bom;				/* 0x65C */
	vu32 iram_tom;				/* 0x660 */
	vu32 emem_cfg_access_ctrl;		/* 0x664 */
	vu32 rsvd_0x668[2];			/* 0x668 */
	vu32 sec_carveout_bom;		/* 0x670 */
	vu32 sec_carveout_size_mb;		/* 0x674 */
	vu32 sec_carveout_reg_ctrl;		/* 0x678 */
	vu32 rsvd_0x67c[17];		/* 0x67C-0x6BC */

	vu32 emem_arb_timing_rfcpb;		/* 0x6C0 */
	vu32 emem_arb_timing_ccdmw;		/* 0x6C4 */
	vu32 rsvd_0x6c8[10];		/* 0x6C8-0x6EC */

	vu32 emem_arb_refpb_hp_ctrl;	/* 0x6F0 */
	vu32 emem_arb_refpb_bank_ctrl;	/* 0x6F4 */
	vu32 rsvd_0x6f8[155];		/* 0x6F8-0x960 */
	vu32 iram_reg_ctrl;			/* 0x964 */
	vu32 emem_arb_override_1;		/* 0x968 */
	vu32 rsvd_0x96c[3];			/* 0x96c */
	vu32 video_protect_bom_adr_hi;	/* 0x978 */
	vu32 rsvd_0x97c[2];			/* 0x97c */
	vu32 video_protect_gpu_override_0;	/* 0x984 */
	vu32 video_protect_gpu_override_1;	/* 0x988 */
	vu32 rsvd_0x98c[5];			/* 0x98c */
	vu32 mts_carveout_bom;		/* 0x9a0 */
	vu32 mts_carveout_size_mb;		/* 0x9a4 */
	vu32 mts_carveout_adr_hi;		/* 0x9a8 */
	vu32 mts_carveout_reg_ctrl;		/* 0x9ac */
	vu32 rsvd_0x9b0[4];			/* 0x9b0 */
	vu32 emem_bank_swizzle_cfg0;	/* 0x9c0 */
	vu32 emem_bank_swizzle_cfg1;	/* 0x9c4 */
	vu32 emem_bank_swizzle_cfg2;	/* 0x9c8 */
	vu32 emem_bank_swizzle_cfg3;	/* 0x9cc */
	vu32 rsvd_0x9d0[1];			/* 0x9d0 */
	vu32 sec_carveout_adr_hi;		/* 0x9d4 */
	vu32 rsvd_0x9d8;			/* 0x9D8 */
	vu32 da_config0;			/* 0x9DC */
	vu32 rsvd_0x9c0[138];		/* 0x9E0-0xc04 */

	vu32 security_carveout1_cfg0;	/* 0xc08 */
	vu32 security_carveout1_bom;	/* 0xc0c */
	vu32 security_carveout1_bom_hi;	/* 0xc10 */
	vu32 security_carveout1_size_128kb;	/* 0xc14 */
	vu32 security_carveout1_ca0;	/* 0xc18 */
	vu32 security_carveout1_ca1;	/* 0xc1c */
	vu32 security_carveout1_ca2;	/* 0xc20 */
	vu32 security_carveout1_ca3;	/* 0xc24 */
	vu32 security_carveout1_ca4;	/* 0xc28 */
	vu32 security_carveout1_cfia0;	/* 0xc2c */
	vu32 security_carveout1_cfia1;	/* 0xc30 */
	vu32 security_carveout1_cfia2;	/* 0xc34 */
	vu32 security_carveout1_cfia3;	/* 0xc38 */
	vu32 security_carveout1_cfia4;	/* 0xc3c */
	vu32 rsvd_0xc40[6];			/* 0xc40-0xc54 */

	vu32 security_carveout2_cfg0;	/* 0xc58 */
	vu32 security_carveout2_bom;	/* 0xc5c */
	vu32 security_carveout2_bom_hi;	/* 0xc60 */
	vu32 security_carveout2_size_128kb;	/* 0xc64 */
	vu32 security_carveout2_ca0;	/* 0xc68 */
	vu32 security_carveout2_ca1;	/* 0xc6c */
	vu32 security_carveout2_ca2;	/* 0xc70 */
	vu32 security_carveout2_ca3;	/* 0xc74 */
	vu32 security_carveout2_ca4;	/* 0xc78 */
	vu32 security_carveout2_cfia0;	/* 0xc7c */
	vu32 security_carveout2_cfia1;	/* 0xc80 */
	vu32 security_carveout2_cfia2;	/* 0xc84 */
	vu32 security_carveout2_cfia3;	/* 0xc88 */
	vu32 security_carveout2_cfia4;	/* 0xc8c */
	vu32 rsvd_0xc90[6];			/* 0xc90-0xca4 */

	vu32 security_carveout3_cfg0;	/* 0xca8 */
	vu32 security_carveout3_bom;	/* 0xcac */
	vu32 security_carveout3_bom_hi;	/* 0xcb0 */
	vu32 security_carveout3_size_128kb;	/* 0xcb4 */
	vu32 security_carveout3_ca0;	/* 0xcb8 */
	vu32 security_carveout3_ca1;	/* 0xcbc */
	vu32 security_carveout3_ca2;	/* 0xcc0 */
	vu32 security_carveout3_ca3;	/* 0xcc4 */
	vu32 security_carveout3_ca4;	/* 0xcc8 */
	vu32 security_carveout3_cfia0;	/* 0xccc */
	vu32 security_carveout3_cfia1;	/* 0xcd0 */
	vu32 security_carveout3_cfia2;	/* 0xcd4 */
	vu32 security_carveout3_cfia3;	/* 0xcd8 */
	vu32 security_carveout3_cfia4;	/* 0xcdc */
	vu32 rsvd_0xce0[6];			/* 0xce0-0xcf4 */

	vu32 security_carveout4_cfg0;	/* 0xcf8 */
	vu32 security_carveout4_bom;	/* 0xcfc */
	vu32 security_carveout4_bom_hi;	/* 0xd00 */
	vu32 security_carveout4_size_128kb;	/* 0xd04 */
	vu32 security_carveout4_ca0;	/* 0xd08 */
	vu32 security_carveout4_ca1;	/* 0xd0c */
	vu32 security_carveout4_ca2;	/* 0xd10 */
	vu32 security_carveout4_ca3;	/* 0xd14 */
	vu32 security_carveout4_ca4;	/* 0xd18 */
	vu32 security_carveout4_cfia0;	/* 0xd1c */
	vu32 security_carveout4_cfia1;	/* 0xd20 */
	vu32 security_carveout4_cfia2;	/* 0xd24 */
	vu32 security_carveout4_cfia3;	/* 0xd28 */
	vu32 security_carveout4_cfia4;	/* 0xd2c */
	vu32 rsvd_0xd30[6];			/* 0xd30-0xd44 */

	vu32 security_carveout5_cfg0;	/* 0xd48 */
	vu32 security_carveout5_bom;	/* 0xd4c */
	vu32 security_carveout5_bom_hi;	/* 0xd50 */
	vu32 security_carveout5_size_128kb;	/* 0xd54 */
	vu32 security_carveout5_ca0;	/* 0xd58 */
	vu32 security_carveout5_ca1;	/* 0xd5c */
	vu32 security_carveout5_ca2;	/* 0xd60 */
	vu32 security_carveout5_ca3;	/* 0xd64 */
	vu32 security_carveout5_ca4;	/* 0xd68 */
	vu32 security_carveout5_cfia0;	/* 0xd6c */
	vu32 security_carveout5_cfia1;	/* 0xd70 */
	vu32 security_carveout5_cfia2;	/* 0xd74 */
	vu32 security_carveout5_cfia3;	/* 0xd78 */
	vu32 security_carveout5_cfia4;	/* 0xd7c */
};

enum {
	MC_SMMU_CONFIG_ENABLE = 1,

	MC_EMEM_CFG_SIZE_MB_SHIFT = 0,
	MC_EMEM_CFG_SIZE_MB_MASK = 0x3fff,

	MC_EMEM_ARB_MISC0_MC_EMC_SAME_FREQ_SHIFT = 27,
	MC_EMEM_ARB_MISC0_MC_EMC_SAME_FREQ_MASK = 1 << 27,

	MC_EMEM_CFG_ACCESS_CTRL_WRITE_ACCESS_DISABLED = 1,

	MC_TIMING_CONTROL_TIMING_UPDATE = 1,
};

#define MC_SECURITY_CARVEOUT_LOCKED			(1 << 1)
#define MC_VPR_WR_ACCESS_DISABLE			(1 << 0)
#define MC_VPR_ALLOW_TZ_WR_ACCESS_ENABLE	(1 << 1)

#endif
