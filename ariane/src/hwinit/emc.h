/*
 * Copyright (c) 2013-2015, NVIDIA CORPORATION.  All rights reserved.
 * Copyright (C) 2014 Google Inc.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 */

#ifndef __SOC_NVIDIA_TEGRA210_EMC_H__
#define __SOC_NVIDIA_TEGRA210_EMC_H__

#include <stddef.h>
#include <stdint.h>
#include "types.h"

enum {
	EMC_PIN_RESET_MASK = 1 << 8,
	EMC_PIN_RESET_ACTIVE = 0 << 8,
	EMC_PIN_RESET_INACTIVE = 1 << 8,
	EMC_PIN_DQM_MASK = 1 << 4,
	EMC_PIN_DQM_NORMAL = 0 << 4,
	EMC_PIN_DQM_INACTIVE = 1 << 4,
	EMC_PIN_CKE_MASK = 1 << 0,
	EMC_PIN_CKE_POWERDOWN = 0 << 0,
	EMC_PIN_CKE_NORMAL = 1 << 0,

	EMC_REF_CMD_MASK = 1 << 0,
	EMC_REF_CMD_REFRESH = 1 << 0,
	EMC_REF_NORMAL_MASK = 1 << 1,
	EMC_REF_NORMAL_INIT = 0 << 1,
	EMC_REF_NORMAL_ENABLED = 1 << 1,
	EMC_REF_NUM_SHIFT = 8,
	EMC_REF_NUM_MASK = 0xFF << EMC_REF_NUM_SHIFT,
	EMC_REF_DEV_SELECTN_SHIFT = 30,
	EMC_REF_DEV_SELECTN_MASK = 3 << EMC_REF_DEV_SELECTN_SHIFT,

	EMC_REFCTRL_REF_VALID_MASK = 1 << 31,
	EMC_REFCTRL_REF_VALID_DISABLED = 0 << 31,
	EMC_REFCTRL_REF_VALID_ENABLED = 1 << 31,

	EMC_CFG_EMC2PMACRO_CFG_BYPASS_ADDRPIPE_MASK = 1 << 1,
	EMC_CFG_EMC2PMACRO_CFG_BYPASS_DATAPIPE1_MASK = 1 << 2,
	EMC_CFG_EMC2PMACRO_CFG_BYPASS_DATAPIPE2_MASK = 1 << 3,

	EMC_NOP_CMD_SHIFT = 0,
	EMC_NOP_CMD_MASK = 1 << EMC_NOP_CMD_SHIFT,
	EMC_NOP_DEV_SELECTN_SHIFT = 30,
	EMC_NOP_DEV_SELECTN_MASK = 3 << EMC_NOP_DEV_SELECTN_SHIFT,

	EMC_TIMING_CONTROL_TIMING_UPDATE = 1,

	EMC_PIN_GPIOEN_SHIFT = 16,
	EMC_PIN_GPIO_SHIFT = 12,
	EMC_PMACRO_BRICK_CTRL_RFU1_RESET_VAL = 0x1FFF1FFF,

	AUTOCAL_MEASURE_STALL_ENABLE = 1 << 9,
	WRITE_MUX_ACTIVE = 1 << 1,
	CFG_ADR_EN_LOCKED = 1 << 1,
};

struct tegra_emc_regs {
	vu32 intstatus;		/* 0x0 */
	vu32 intmask;		/* 0x4 */
	vu32 dbg;			/* 0x8 */
	vu32 cfg;			/* 0xc */
	vu32 adr_cfg;		/* 0x10 */
	vu32 rsvd_0x14[3];		/* 0x14-0x1C */

	vu32 refctrl;		/* 0x20 */
	vu32 pin;			/* 0x24 */
	vu32 timing_control;	/* 0x28 */
	vu32 rc;			/* 0x2c */
	vu32 rfc;			/* 0x30 */
	vu32 ras;			/* 0x34 */
	vu32 rp;			/* 0x38 */
	vu32 r2w;			/* 0x3c */
	vu32 w2r;			/* 0x40 */
	vu32 r2p;			/* 0x44 */
	vu32 w2p;			/* 0x48 */
	vu32 rd_rcd;		/* 0x4c */
	vu32 wr_rcd;		/* 0x50 */
	vu32 rrd;			/* 0x54 */
	vu32 rext;			/* 0x58 */
	vu32 wdv;			/* 0x5c */
	vu32 quse;			/* 0x60 */
	vu32 qrst;			/* 0x64 */
	vu32 qsafe;			/* 0x68 */
	vu32 rdv;			/* 0x6c */
	vu32 refresh;		/* 0x70 */
	vu32 burst_refresh_num;	/* 0x74 */
	vu32 pdex2wr;		/* 0x78 */
	vu32 pdex2rd;		/* 0x7c */
	vu32 pchg2pden;		/* 0x80 */
	vu32 act2pden;		/* 0x84 */
	vu32 ar2pden;		/* 0x88 */
	vu32 rw2pden;		/* 0x8c */
	vu32 txsr;			/* 0x90 */
	vu32 tcke;			/* 0x94 */
	vu32 tfaw;			/* 0x98 */
	vu32 trpab;			/* 0x9c */
	vu32 tclkstable;		/* 0xa0 */
	vu32 tclkstop;		/* 0xa4 */
	vu32 trefbw;		/* 0xa8 */
	vu32 tppd;			/* 0xac */
	vu32 odt_write;		/* 0xb0 */
	vu32 pdex2mrr;		/* 0xb4 */
	vu32 wext;			/* 0xb8 */
	vu32 ctt;			/* 0xbc */
	vu32 rfc_slr;		/* 0xc0 */
	vu32 mrs_wait_cnt2;		/* 0xc4 */
	vu32 mrs_wait_cnt;		/* 0xc8 */
	vu32 mrs;			/* 0xcc */
	vu32 emrs;			/* 0xd0 */
	vu32 ref;			/* 0xd4 */
	vu32 pre;			/* 0xd8 */
	vu32 nop;			/* 0xdc */
	vu32 self_ref;		/* 0xe0 */
	vu32 dpd;			/* 0xe4 */
	vu32 mrw;			/* 0xe8 */
	vu32 mrr;			/* 0xec */
	vu32 cmdq;			/* 0xf0 */
	vu32 mc2emcq;		/* 0xf4 */
	vu32 xm2dqspadctrl3;	/* 0xf8 */
	vu32 rsvd_0xfc[1];		/* 0xfc */
	vu32 fbio_spare;		/* 0x100 */
	vu32 fbio_cfg5;		/* 0x104 */
	vu32 fbio_wrptr_eq_2;	/* 0x108 */
	vu32 rsvd_0x10c[2];		/* 0x10c-0x110 */

	vu32 fbio_cfg6;		/* 0x114 */
	vu32 pdex2cke;		/* 0x118 */
	vu32 cke2pden;		/* 0x11C */
	vu32 cfg_rsv;		/* 0x120 */
	vu32 acpd_control;		/* 0x124 */
	vu32 rsvd_0x128[1];		/* 0x128 */
	vu32 emrs2;			/* 0x12c */
	vu32 emrs3;			/* 0x130 */
	vu32 mrw2;			/* 0x134 */
	vu32 mrw3;			/* 0x138 */
	vu32 mrw4;			/* 0x13c */
	vu32 clken_override;	/* 0x140 */
	vu32 r2r;			/* 0x144 */
	vu32 w2w;			/* 0x148 */
	vu32 einput;		/* 0x14c */
	vu32 einput_duration;	/* 0x150 */
	vu32 puterm_extra;		/* 0x154 */
	vu32 tckesr;		/* 0x158 */
	vu32 tpd;			/* 0x15c */
	vu32 rsvd_0x160[81];	/* 0x160-0x2A0 */

	vu32 auto_cal_config;	/* 0x2a4 */
	vu32 auto_cal_interval;	/* 0x2a8 */
	vu32 auto_cal_status;	/* 0x2ac */
	vu32 req_ctrl;		/* 0x2b0 */
	vu32 status;		/* 0x2b4 */
	vu32 cfg_2;			/* 0x2b8 */
	vu32 cfg_dig_dll;		/* 0x2bc */
	vu32 cfg_dig_dll_period;	/* 0x2c0 */
	vu32 dig_dll_status;	/* 0x2C4 */
	vu32 cfg_dig_dll_1;		/* 0x2C8 */
	vu32 rdv_mask;		/* 0x2cc */
	vu32 wdv_mask;		/* 0x2d0 */
	vu32 rdv_early_mask;	/* 0x2d4 */
	vu32 rdv_early;		/* 0x2d8 */
	vu32 auto_cal_config8;	/* 0x2DC */
	vu32 zcal_interval;		/* 0x2e0 */
	vu32 zcal_wait_cnt;		/* 0x2e4 */
	vu32 zcal_mrw_cmd;		/* 0x2e8 */
	vu32 zq_cal;		/* 0x2ec */
	vu32 xm2cmdpadctrl;		/* 0x2f0 */
	vu32 xm2comppadctrl3;	/* 0x2f4 */
	vu32 auto_cal_vref_sel0;	/* 0x2f8 */
	vu32 xm2dqspadctrl2;	/* 0x2fc */
	vu32 auto_cal_vref_sel1;	/* 0x300 */
	vu32 xm2dqpadctrl2;		/* 0x304 */
	vu32 xm2clkpadctrl;		/* 0x308 */
	vu32 xm2comppadctrl;	/* 0x30c */
	vu32 fdpd_ctrl_dq;		/* 0x310 */
	vu32 fdpd_ctrl_cmd;		/* 0x314 */
	vu32 pmacro_cmd_brick_ctrl_fdpd;	/* 0x318 */
	vu32 pmacro_data_brick_ctrl_fdpd;	/* 0x31c */
	vu32 xm2dqspadctrl4;	/* 0x320 */
	vu32 scratch0;		/* 0x324 */
	vu32 rsvd_0x328[2];		/* 0x328-0x32C */

	vu32 pmacro_brick_ctrl_rfu1; /* 0x330 */
	vu32 pmacro_brick_ctrl_rfu2; /* 0x334 */
	vu32 rsvd_0x338[18];	/* 0x338-0x37C */

	vu32 cmd_mapping_cmd0_0;	/* 0x380 */
	vu32 cmd_mapping_cmd0_1;	/* 0x384 */
	vu32 cmd_mapping_cmd0_2;	/* 0x388 */
	vu32 cmd_mapping_cmd1_0;	/* 0x38c */
	vu32 cmd_mapping_cmd1_1;	/* 0x390 */
	vu32 cmd_mapping_cmd1_2;	/* 0x394 */
	vu32 cmd_mapping_cmd2_0;	/* 0x398 */
	vu32 cmd_mapping_cmd2_1;	/* 0x39C */
	vu32 cmd_mapping_cmd2_2;	/* 0x3A0 */
	vu32 cmd_mapping_cmd3_0;	/* 0x3A4 */
	vu32 cmd_mapping_cmd3_1;	/* 0x3A8 */
	vu32 cmd_mapping_cmd3_2;	/* 0x3AC */
	vu32 cmd_mapping_byte;	/* 0x3B0 */
	vu32 tr_timing_0;		/* 0x3B4 */
	vu32 tr_ctrl_0;		/* 0x3B8 */
	vu32 tr_ctrl_1;		/* 0x3BC */
	vu32 switch_back_ctrl;	/* 0x3C0 */
	vu32 tr_rdv;		/* 0x3C4 */
	vu32 stall_then_exe_before_clkchange;	/* 0x3c8 */
	vu32 stall_then_exe_after_clkchange;	/* 0x3cc */
	vu32 unstall_rw_after_clkchange;		/* 0x3d0 */
	vu32 auto_cal_clk_status;	/* 0x3d4 */
	vu32 sel_dpd_ctrl;		/* 0x3d8 */
	vu32 pre_refresh_req_cnt;	/* 0x3dc */
	vu32 dyn_self_ref_control;	/* 0x3e0 */
	vu32 txsrdll;		/* 0x3e4 */
	vu32 ccfifo_addr;		/* 0x3e8 */
	vu32 ccfifo_data;		/* 0x3ec */
	vu32 ccfifo_status;		/* 0x3f0 */
	vu32 cdb_cntl_1;		/* 0x3f4 */
	vu32 cdb_cntl_2;		/* 0x3f8 */
	vu32 xm2clkpadctrl2;	/* 0x3fc */
	vu32 swizzle_rank0_byte_cfg; /* 0x400 */
	vu32 swizzle_rank0_byte0;	/* 0x404 */
	vu32 swizzle_rank0_byte1;	/* 0x408 */
	vu32 swizzle_rank0_byte2;	/* 0x40c */
	vu32 swizzle_rank0_byte3;	/* 0x410 */
	vu32 swizzle_rank1_byte_cfg; /* 0x414 */
	vu32 swizzle_rank1_byte0;	/* 0x418 */
	vu32 swizzle_rank1_byte1;	/* 0x41c */
	vu32 swizzle_rank1_byte2;	/* 0x420 */
	vu32 swizzle_rank1_byte3;	/* 0x424 */
	vu32 issue_qrst;		/* 0x428 */
	vu32 rsvd_0x42C[5];		/* 0x42C-0x43C */
	vu32 pmc_scratch1;		/* 0x440 */
	vu32 pmc_scratch2;		/* 0x444 */
	vu32 pmc_scratch3;		/* 0x448 */
	vu32 rsvd_0x44C[3];		/* 0x44C-0x454 */
	vu32 auto_cal_config2;	/* 0x458 */
	vu32 auto_cal_config3;	/* 0x45c */
	vu32 auto_cal_status2;	/* 0x460 */
	vu32 auto_cal_channel;	/* 0x464 */
	vu32 ibdly;			/* 0x468 */
	vu32 obdly;			/* 0x46c */
	vu32 rsvd_0x470[3];		/* 0x470-0x478 */

	vu32 dsr_vttgen_drv;	/* 0x47c */
	vu32 txdsrvttgen;		/* 0x480 */
	vu32 xm2cmdpadctrl4;	/* 0x484 */
	vu32 xm2cmdpadctrl5;	/* 0x488 */
	vu32 we_duration;		/* 0x48C */
	vu32 ws_duration;		/* 0x490 */
	vu32 wev;			/* 0x494 */
	vu32 wsv;			/* 0x498 */
	vu32 cfg_3;			/* 0x49C */
	vu32 mrw5;			/* 0x4A0 */
	vu32 mrw6;			/* 0x4A4 */
	vu32 mrw7;			/* 0x4A8 */
	vu32 mrw8;			/* 0x4AC */
	vu32 mrw9;			/* 0x4B0 */
	vu32 mrw10;			/* 0x4B4 */
	vu32 mrw11;			/* 0x4B8 */
	vu32 mrw12;			/* 0x4BC */
	vu32 mrw13;			/* 0x4C0 */
	vu32 mrw14;			/* 0x4C4 */
	vu32 rsvd_0x4c8[2];		/* 0x4C8-0x4CC */

	vu32 mrw15;			/* 0x4D0 */
	vu32 cfg_sync;		/* 0x4D4 */
	vu32 fdpd_ctrl_cmd_no_ramp;	/* 0x4D8 */
	vu32 rsvd_0x4dc[1];		/* 0x4DC */
	vu32 wdv_chk;		/* 0x4E0 */
	vu32 rsvd_0x4e4[28];	/* 0x4E4-0x550 */

	vu32 cfg_pipe2;		/* 0x554 */
	vu32 cfg_pipe_clk;		/* 0x558 */
	vu32 cfg_pipe1;		/* 0x55C */
	vu32 cfg_pipe;		/* 0x560 */
	vu32 qpop;			/* 0x564 */
	vu32 quse_width;		/* 0x568 */
	vu32 puterm_width;		/* 0x56c */
	vu32 bgbias_ctl0;		/* 0x570 */
	vu32 auto_cal_config7;	/* 0x574 */
	vu32 xm2comppadctrl2;	/* 0x578 */
	vu32 comppadswctrl;		/* 0x57C */
	vu32 refctrl2;		/* 0x580 */
	vu32 fbio_cfg7;		/* 0x584 */
	vu32 data_brlshft_0;	/* 0x588 */
	vu32 data_brlshft_1;	/* 0x58C */
	vu32 rfcpb;			/* 0x590 */
	vu32 dqs_brlshft_0;		/* 0x594 */
	vu32 dqs_brlshft_1;		/* 0x598 */
	vu32 cmd_brlshft_0;		/* 0x59C */
	vu32 cmd_brlshft_1;		/* 0x5A0 */
	vu32 cmd_brlshft_2;		/* 0x5A4 */
	vu32 cmd_brlshft_3;		/* 0x5A8 */
	vu32 quse_brlshft_0;	/* 0x5AC */
	vu32 auto_cal_config4;	/* 0x5B0 */
	vu32 auto_cal_config5;	/* 0x5B4 */
	vu32 quse_brlshft_1;	/* 0x5B8 */
	vu32 quse_brlshft_2;	/* 0x5BC */
	vu32 ccdmw;			/* 0x5C0 */
	vu32 quse_brlshft_3;	/* 0x5C4 */
	vu32 fbio_cfg8;		/* 0x5C8 */
	vu32 auto_cal_config6;	/* 0x5CC */
	vu32 protobist_config_addr_1; /* 0x5D0 */
	vu32 protobist_config_addr_2; /* 0x5D4 */
	vu32 protobist_misc;	/* 0x5D8 */
	vu32 protobist_wdata_lower;	/* 0x5DC */
	vu32 protobist_wdata_upper;	/* 0x5E0 */
	vu32 dll_cfg0;		/* 0x5E4 */
	vu32 dll_cfg1;		/* 0x5E8 */
	vu32 protobist_rdata;	/* 0x5EC */
	vu32 config_sample_delay;	/* 0x5F0 */
	vu32 cfg_update;		/* 0x5F4 */
	vu32 rsvd_0x5f8[2];		/* 0x5F8-0x5FC */

	vu32 pmacro_quse_ddll_rank0_0;	/* 0x600 */
	vu32 pmacro_quse_ddll_rank0_1;	/* 0x604 */
	vu32 pmacro_quse_ddll_rank0_2;	/* 0x608 */
	vu32 pmacro_quse_ddll_rank0_3;	/* 0x60C */
	vu32 pmacro_quse_ddll_rank0_4;	/* 0x610 */
	vu32 pmacro_quse_ddll_rank0_5;	/* 0x614 */
	vu32 rsvd_0x618[2];		/* 0x618-0x61C */

	vu32 pmacro_quse_ddll_rank1_0;	/* 0x620 */
	vu32 pmacro_quse_ddll_rank1_1;	/* 0x624 */
	vu32 pmacro_quse_ddll_rank1_2;	/* 0x628 */
	vu32 pmacro_quse_ddll_rank1_3;	/* 0x62C */
	vu32 pmacro_quse_ddll_rank1_4;	/* 0x630 */
	vu32 pmacro_quse_ddll_rank1_5;	/* 0x634 */
	vu32 rsvd_0x638[2];		/* 0x638-0x63C */

	vu32 pmacro_ob_ddll_long_dq_rank0_0;	/* 0x640 */
	vu32 pmacro_ob_ddll_long_dq_rank0_1;	/* 0x644 */
	vu32 pmacro_ob_ddll_long_dq_rank0_2;	/* 0x648 */
	vu32 pmacro_ob_ddll_long_dq_rank0_3;	/* 0x64C */
	vu32 pmacro_ob_ddll_long_dq_rank0_4;	/* 0x650 */
	vu32 pmacro_ob_ddll_long_dq_rank0_5;	/* 0x654 */
	vu32 rsvd_0x658[2];		/* 0x658-0x65C */

	vu32 pmacro_ob_ddll_long_dq_rank1_0;	/* 0x660 */
	vu32 pmacro_ob_ddll_long_dq_rank1_1;	/* 0x664 */
	vu32 pmacro_ob_ddll_long_dq_rank1_2;	/* 0x668 */
	vu32 pmacro_ob_ddll_long_dq_rank1_3;	/* 0x66C */
	vu32 pmacro_ob_ddll_long_dq_rank1_4;	/* 0x670 */
	vu32 pmacro_ob_ddll_long_dq_rank1_5;	/* 0x674 */
	vu32 rsvd_0x678[2];		/* 0x678-0x67C */

	vu32 pmacro_ob_ddll_long_dqs_rank0_0;	/* 0x680 */
	vu32 pmacro_ob_ddll_long_dqs_rank0_1;	/* 0x684 */
	vu32 pmacro_ob_ddll_long_dqs_rank0_2;	/* 0x688 */
	vu32 pmacro_ob_ddll_long_dqs_rank0_3;	/* 0x68C */
	vu32 pmacro_ob_ddll_long_dqs_rank0_4;	/* 0x690 */
	vu32 pmacro_ob_ddll_long_dqs_rank0_5;	/* 0x694 */
	vu32 rsvd_0x698[2];		/* 0x698-0x69C */

	vu32 pmacro_ob_ddll_long_dqs_rank1_0;	/* 0x6A0 */
	vu32 pmacro_ob_ddll_long_dqs_rank1_1;	/* 0x6A4 */
	vu32 pmacro_ob_ddll_long_dqs_rank1_2;	/* 0x6A8 */
	vu32 pmacro_ob_ddll_long_dqs_rank1_3;	/* 0x6AC */
	vu32 pmacro_ob_ddll_long_dqs_rank1_4;	/* 0x6B0 */
	vu32 pmacro_ob_ddll_long_dqs_rank1_5;	/* 0x6B4 */
	vu32 rsvd_0x6B8[2];		/* 0x6B8-0x6BC */

	vu32 pmacro_ib_ddll_long_dqs_rank0_0;	/* 0x6C0 */
	vu32 pmacro_ib_ddll_long_dqs_rank0_1;	/* 0x6C4 */
	vu32 pmacro_ib_ddll_long_dqs_rank0_2;	/* 0x6C8 */
	vu32 pmacro_ib_ddll_long_dqs_rank0_3;	/* 0x6CC */
	vu32 pmacro_ib_ddll_long_dqs_rank0_4;	/* 0x6D0 */
	vu32 pmacro_ib_ddll_long_dqs_rank0_5;	/* 0x6D4 */
	vu32 rsvd_0x6D8[2];		/* 0x6D8-0x6DC */

	vu32 pmacro_ib_ddll_long_dqs_rank1_0;	/* 0x6E0 */
	vu32 pmacro_ib_ddll_long_dqs_rank1_1;	/* 0x6E4 */
	vu32 pmacro_ib_ddll_long_dqs_rank1_2;	/* 0x6E8 */
	vu32 pmacro_ib_ddll_long_dqs_rank1_3;	/* 0x6EC */
	vu32 pmacro_ib_ddll_long_dqs_rank1_4;	/* 0x6F0 */
	vu32 pmacro_ib_ddll_long_dqs_rank1_5;	/* 0x6F4 */
	vu32 rsvd_0x6F8[2];		/* 0x6F8-0x6FC */

	vu32 pmacro_autocal_cfg0;	/* 0x700 */
	vu32 pmacro_autocal_cfg1;	/* 0x704 */
	vu32 pmacro_autocal_cfg2;	/* 0x708 */
	vu32 rsvd_0x70C[5];		/* 0x70C-0x71C */

	vu32 pmacro_tx_pwrd_0;	/* 0x720 */
	vu32 pmacro_tx_pwrd_1;	/* 0x724 */
	vu32 pmacro_tx_pwrd_2;	/* 0x728 */
	vu32 pmacro_tx_pwrd_3;	/* 0x72C */
	vu32 pmacro_tx_pwrd_4;	/* 0x730 */
	vu32 pmacro_tx_pwrd_5;	/* 0x734 */
	vu32 rsvd_0x738[2];		/* 0x738-0x73C */

	vu32 pmacro_tx_sel_clk_src_0;	/* 0x740 */
	vu32 pmacro_tx_sel_clk_src_1;	/* 0x744 */
	vu32 pmacro_tx_sel_clk_src_2;	/* 0x748 */
	vu32 pmacro_tx_sel_clk_src_3;	/* 0x74C */
	vu32 pmacro_tx_sel_clk_src_4;	/* 0x750 */
	vu32 pmacro_tx_sel_clk_src_5;	/* 0x754 */
	vu32 rsvd_0x758[2];		/* 0x758-0x75C */

	vu32 pmacro_ddll_bypass;	/* 0x760 */
	vu32 rsvd_0x764[3];		/* 0x764-0x76C */

	vu32 pmacro_ddll_pwrd_0;	/* 0x770 */
	vu32 pmacro_ddll_pwrd_1;	/* 0x774 */
	vu32 pmacro_ddll_pwrd_2;	/* 0x778 */
	vu32 rsvd_0x77C[1];		/* 0x77C */
	vu32 pmacro_cmd_ctrl_0;	/* 0x780 */
	vu32 pmacro_cmd_ctrl_1;	/* 0x784 */
	vu32 pmacro_cmd_ctrl_2;	/* 0x788 */
	vu32 rsvd_0x78C[277];	/* 0x78C-0xBDC */

	vu32 pmacro_ib_vref_dq_0;		/* 0xBE0 */
	vu32 pmacro_ib_vref_dq_1;		/* 0xBE4 */
	vu32 pmacro_ib_vref_dq_2;		/* 0xBE8 */
	vu32 rsvd_0xBEC[1];			/* 0xBEC */
	vu32 pmacro_ib_vref_dqs_0;		/* 0xBF0 */
	vu32 pmacro_ib_vref_dqs_1;		/* 0xBF4 */
	vu32 pmacro_ib_vref_dqs_2;		/* 0xBF8 */
	vu32 rsvd_0xBFC[1];			/* 0xBFC */
	vu32 pmacro_ddll_long_cmd_0;	/* 0xC00 */
	vu32 pmacro_ddll_long_cmd_1;	/* 0xC04 */
	vu32 pmacro_ddll_long_cmd_2;	/* 0xC08 */
	vu32 pmacro_ddll_long_cmd_3;	/* 0xC0C */
	vu32 pmacro_ddll_long_cmd_4;	/* 0xC10 */
	vu32 pmacro_ddll_long_cmd_5;	/* 0xC14 */
	vu32 rsvd_0xC18[2];			/* 0xC18-0xC1C */

	vu32 pmacro_ddll_short_cmd_0;	/* 0xC20 */
	vu32 pmacro_ddll_short_cmd_1;	/* 0xC24 */
	vu32 pmacro_ddll_short_cmd_2;	/* 0xC28 */
	vu32 rsvd_0xC2C[2];			/* 0xC2C-0xC30 */

	vu32 pmacro_vttgen_ctrl0;		/* 0xC34 */
	vu32 pmacro_vttgen_ctrl1;		/* 0xC38 */
	vu32 pmacro_bg_bias_ctrl_0;		/* 0xC3C */
	vu32 pmacro_pad_cfg_ctrl;		/* 0xC40 */
	vu32 pmacro_zctrl;			/* 0xC44 */
	vu32 pmacro_rx_term;		/* 0xC48 */
	vu32 pmacro_cmd_tx_drv;		/* 0xC4C */
	vu32 pmacro_cmd_pad_rx_ctrl;	/* 0xC50 */
	vu32 pmacro_data_pad_rx_ctrl;	/* 0xC54 */
	vu32 pmacro_cmd_rx_term_mode;	/* 0xC58 */
	vu32 pmacro_data_rx_term_mode;	/* 0xC5C */
	vu32 pmacro_cmd_pad_tx_ctrl;	/* 0xC60 */
	vu32 pmacro_data_pad_tx_ctrl;	/* 0xC64 */
	vu32 pmacro_common_pad_tx_ctrl;	/* 0xC68 */
	vu32 rsvd_0xC6C[1];			/* 0xC6C */
	vu32 pmacro_dq_tx_drv;		/* 0xC70 */
	vu32 pmacro_ca_tx_drv;		/* 0xC74 */
	vu32 pmacro_autocal_cfg_common;	/* 0xC78 */
	vu32 rsvd_0xC7C[1];			/* 0xC7C */
	vu32 pmacro_brick_mapping0;		/* 0xC80 */
	vu32 pmacro_brick_mapping1;		/* 0xC84 */
	vu32 pmacro_brick_mapping2;		/* 0xC88 */
	vu32 rsvd_0xC8C[25];		/* 0xC8C-0xCEC */

	vu32 pmacro_vttgen_ctrl2;		/* 0xCF0 */
	vu32 pmacro_ib_rxrt;		/* 0xCF4 */
	vu32 pmacro_training_ctrl0;		/* 0xCF8 */
	vu32 pmacro_training_ctrl1;		/* 0xCFC */
};

#endif /* __SOC_NVIDIA_TEGRA210_EMC_H__ */
