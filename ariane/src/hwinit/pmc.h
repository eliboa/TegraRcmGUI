/*
 * Copyright (c) 2010-2015, NVIDIA CORPORATION.  All rights reserved.
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

#ifndef _TEGRA210_PMC_H_
#define _TEGRA210_PMC_H_

#include "types.h"

enum {
	POWER_PARTID_CRAIL = 0,
	POWER_PARTID_TD = 1,
	POWER_PARTID_VE = 2,
	POWER_PARTID_PCX = 3,
	POWER_PARTID_C0L2 = 5,
	POWER_PARTID_MPE = 6,
	POWER_PARTID_HEG = 7,
	POWER_PARTID_SAX = 8,
	POWER_PARTID_CE1 = 9,
	POWER_PARTID_CE2 = 10,
	POWER_PARTID_CE3 = 11,
	POWER_PARTID_CELP = 12,
	POWER_PARTID_CE0 = 14,
	POWER_PARTID_C0NC = 15,
	POWER_PARTID_C1NC = 16,
	POWER_PARTID_SOR = 17,
	POWER_PARTID_DIS = 18,
	POWER_PARTID_DISB = 19,
	POWER_PARTID_XUSBA = 20,
	POWER_PARTID_XUSBB = 21,
	POWER_PARTID_XUSBC = 22,
	POWER_PARTID_VIC = 23,
	POWER_PARTID_IRAM = 24,
	POWER_PARTID_NVDEC = 25,
	POWER_PARTID_NVJPG = 26,
	POWER_PARTID_APE = 27,
	POWER_PARTID_DFD = 28,
	POWER_PARTID_VE2 = 29,
};

struct tegra_pmc_regs {
	vu32 cntrl;
	vu32 sec_disable;
	vu32 pmc_swrst;
	vu32 wake_mask;
	vu32 wake_lvl;
	vu32 wake_status;
	vu32 sw_wake_status;
	vu32 dpd_pads_oride;
	vu32 dpd_sample;
	vu32 dpd_enable;
	vu32 pwrgate_timer_off;
	vu32 clamp_status;
	vu32 pwrgate_toggle;
	vu32 remove_clamping_cmd;
	vu32 pwrgate_status;
	vu32 pwrgood_timer;
	vu32 blink_timer;
	vu32 no_iopower;
	vu32 pwr_det;
	vu32 pwr_det_latch;
	vu32 scratch0;
	vu32 scratch1;
	vu32 scratch2;
	vu32 scratch3;
	vu32 scratch4;
	vu32 scratch5;
	vu32 scratch6;
	vu32 scratch7;
	vu32 scratch8;
	vu32 scratch9;
	vu32 scratch10;
	vu32 scratch11;
	vu32 scratch12;
	vu32 scratch13;
	vu32 scratch14;
	vu32 scratch15;
	vu32 scratch16;
	vu32 scratch17;
	vu32 scratch18;
	vu32 scratch19;
	vu32 odmdata;
	vu32 scratch21;
	vu32 scratch22;
	vu32 scratch23;
	vu32 secure_scratch0;
	vu32 secure_scratch1;
	vu32 secure_scratch2;
	vu32 secure_scratch3;
	vu32 secure_scratch4;
	vu32 secure_scratch5;
	vu32 cpupwrgood_timer;
	vu32 cpupwroff_timer;
	vu32 pg_mask;
	vu32 pg_mask_1;
	vu32 auto_wake_lvl;
	vu32 auto_wake_lvl_mask;
	vu32 wake_delay;
	vu32 pwr_det_val;
	vu32 ddr_pwr;
	vu32 usb_debounce_del;
	vu32 usb_a0;
	vu32 crypto_op;
	vu32 pllp_wb0_override;
	vu32 scratch24;
	vu32 scratch25;
	vu32 scratch26;
	vu32 scratch27;
	vu32 scratch28;
	vu32 scratch29;
	vu32 scratch30;
	vu32 scratch31;
	vu32 scratch32;
	vu32 scratch33;
	vu32 scratch34;
	vu32 scratch35;
	vu32 scratch36;
	vu32 scratch37;
	vu32 scratch38;
	vu32 scratch39;
	vu32 scratch40;
	vu32 scratch41;
	vu32 scratch42;
	vu32 bondout_mirror[3];
	vu32 sys_33v_en;
	vu32 bondout_mirror_access;
	vu32 gate;
	vu32 wake2_mask;
	vu32 wake2_lvl;
	vu32 wake2_status;
	vu32 sw_wake2_status;
	vu32 auto_wake2_lvl_mask;
	vu32 pg_mask_2;
	vu32 pg_mask_ce1;
	vu32 pg_mask_ce2;
	vu32 pg_mask_ce3;
	vu32 pwrgate_timer_ce[7];
	vu32 pcx_edpd_cntrl;
	vu32 osc_edpd_over;
	vu32 clk_out_cntrl;
	vu32 sata_pwrgt;
	vu32 sensor_ctrl;
	vu32 rst_status;
	vu32 io_dpd_req;
	vu32 io_dpd_status;
	vu32 io_dpd2_req;
	vu32 io_dpd2_status;
	vu32 sel_dpd_tim;
	vu32 vddp_sel;
	vu32 ddr_cfg;
	vu32 e_no_vttgen;
	u8 _rsv0[4];
	vu32 pllm_wb0_override_freq;
	vu32 test_pwrgate;
	vu32 pwrgate_timer_mult;
	vu32 dis_sel_dpd;
	vu32 utmip_uhsic_triggers;
	vu32 utmip_uhsic_saved_state;
	vu32 utmip_pad_cfg;
	vu32 utmip_term_pad_cfg;
	vu32 utmip_uhsic_sleep_cfg;
	vu32 utmip_uhsic_sleepwalk_cfg;
	vu32 utmip_sleepwalk_p[3];
	vu32 uhsic_sleepwalk_p0;
	vu32 utmip_uhsic_status;
	vu32 utmip_uhsic_fake;
	vu32 bondout_mirror3[5 - 3];
	vu32 secure_scratch6;
	vu32 secure_scratch7;
	vu32 scratch43;
	vu32 scratch44;
	vu32 scratch45;
	vu32 scratch46;
	vu32 scratch47;
	vu32 scratch48;
	vu32 scratch49;
	vu32 scratch50;
	vu32 scratch51;
	vu32 scratch52;
	vu32 scratch53;
	vu32 scratch54;
	vu32 scratch55;
	vu32 scratch0_eco;
	vu32 por_dpd_ctrl;
	vu32 scratch2_eco;
	vu32 utmip_uhsic_line_wakeup;
	vu32 utmip_bias_master_cntrl;
	vu32 utmip_master_config;
	vu32 td_pwrgate_inter_part_timer;
	vu32 utmip_uhsic2_triggers;
	vu32 utmip_uhsic2_saved_state;
	vu32 utmip_uhsic2_sleep_cfg;
	vu32 utmip_uhsic2_sleepwalk_cfg;
	vu32 uhsic2_sleepwalk_p1;
	vu32 utmip_uhsic2_status;
	vu32 utmip_uhsic2_fake;
	vu32 utmip_uhsic2_line_wakeup;
	vu32 utmip_master2_config;
	vu32 utmip_uhsic_rpd_cfg;
	vu32 pg_mask_ce0;
	vu32 pg_mask3[5 - 3];
	vu32 pllm_wb0_override2;
	vu32 tsc_mult;
	vu32 cpu_vsense_override;
	vu32 glb_amap_cfg;
	vu32 sticky_bits;
	vu32 sec_disable2;
	vu32 weak_bias;
	vu32 reg_short;
	vu32 pg_mask_andor;
	u8 _rsv1[0x2c];
	vu32 secure_scratch8;	/* offset 0x300 */
	vu32 secure_scratch9;
	vu32 secure_scratch10;
	vu32 secure_scratch11;
	vu32 secure_scratch12;
	vu32 secure_scratch13;
	vu32 secure_scratch14;
	vu32 secure_scratch15;
	vu32 secure_scratch16;
	vu32 secure_scratch17;
	vu32 secure_scratch18;
	vu32 secure_scratch19;
	vu32 secure_scratch20;
	vu32 secure_scratch21;
	vu32 secure_scratch22;
	vu32 secure_scratch23;
	vu32 secure_scratch24;
	vu32 secure_scratch25;
	vu32 secure_scratch26;
	vu32 secure_scratch27;
	vu32 secure_scratch28;
	vu32 secure_scratch29;
	vu32 secure_scratch30;
	vu32 secure_scratch31;
	vu32 secure_scratch32;
	vu32 secure_scratch33;
	vu32 secure_scratch34;
	vu32 secure_scratch35;
	vu32 secure_scratch36;
	vu32 secure_scratch37;
	vu32 secure_scratch38;
	vu32 secure_scratch39;
	vu32 secure_scratch40;
	vu32 secure_scratch41;
	vu32 secure_scratch42;
	vu32 secure_scratch43;
	vu32 secure_scratch44;
	vu32 secure_scratch45;
	vu32 secure_scratch46;
	vu32 secure_scratch47;
	vu32 secure_scratch48;
	vu32 secure_scratch49;
	vu32 secure_scratch50;
	vu32 secure_scratch51;
	vu32 secure_scratch52;
	vu32 secure_scratch53;
	vu32 secure_scratch54;
	vu32 secure_scratch55;
	vu32 secure_scratch56;
	vu32 secure_scratch57;
	vu32 secure_scratch58;
	vu32 secure_scratch59;
	vu32 secure_scratch60;
	vu32 secure_scratch61;
	vu32 secure_scratch62;
	vu32 secure_scratch63;
	vu32 secure_scratch64;
	vu32 secure_scratch65;
	vu32 secure_scratch66;
	vu32 secure_scratch67;
	vu32 secure_scratch68;
	vu32 secure_scratch69;
	vu32 secure_scratch70;
	vu32 secure_scratch71;
	vu32 secure_scratch72;
	vu32 secure_scratch73;
	vu32 secure_scratch74;
	vu32 secure_scratch75;
	vu32 secure_scratch76;
	vu32 secure_scratch77;
	vu32 secure_scratch78;
	vu32 secure_scratch79;
	vu32 _rsv0x420[8];
	vu32 cntrl2;			/* 0x440 */
	vu32 _rsv0x444[2];
	vu32 event_counter;	/* 0x44C */
	vu32 fuse_control;
	vu32 scratch1_eco;
	vu32 _rsv0x458[1];
	vu32 io_dpd3_req;	/* 0x45C */
	vu32 io_dpd3_status;
	vu32 io_dpd4_req;
	vu32 io_dpd4_status;
	vu32 _rsv0x46C[30];
	vu32 ddr_cntrl;		/* 0x4E4 */
	vu32 _rsv0x4E8[70];
	vu32 scratch56;		/* 0x600 */
	vu32 scratch57;
	vu32 scratch58;
	vu32 scratch59;
	vu32 scratch60;
	vu32 scratch61;
	vu32 scratch62;
	vu32 scratch63;
	vu32 scratch64;
	vu32 scratch65;
	vu32 scratch66;
	vu32 scratch67;
	vu32 scratch68;
	vu32 scratch69;
	vu32 scratch70;
	vu32 scratch71;
	vu32 scratch72;
	vu32 scratch73;
	vu32 scratch74;
	vu32 scratch75;
	vu32 scratch76;
	vu32 scratch77;
	vu32 scratch78;
	vu32 scratch79;
	vu32 scratch80;
	vu32 scratch81;
	vu32 scratch82;
	vu32 scratch83;
	vu32 scratch84;
	vu32 scratch85;
	vu32 scratch86;
	vu32 scratch87;
	vu32 scratch88;
	vu32 scratch89;
	vu32 scratch90;
	vu32 scratch91;
	vu32 scratch92;
	vu32 scratch93;
	vu32 scratch94;
	vu32 scratch95;
	vu32 scratch96;
	vu32 scratch97;
	vu32 scratch98;
	vu32 scratch99;
	vu32 scratch100;
	vu32 scratch101;
	vu32 scratch102;
	vu32 scratch103;
	vu32 scratch104;
	vu32 scratch105;
	vu32 scratch106;
	vu32 scratch107;
	vu32 scratch108;
	vu32 scratch109;
	vu32 scratch110;
	vu32 scratch111;
	vu32 scratch112;
	vu32 scratch113;
	vu32 scratch114;
	vu32 scratch115;
	vu32 scratch116;
	vu32 scratch117;
	vu32 scratch118;
	vu32 scratch119;
	vu32 scratch120;		/* 0x700 */
	vu32 scratch121;
	vu32 scratch122;
	vu32 scratch123;
	vu32 scratch124;
	vu32 scratch125;
	vu32 scratch126;
	vu32 scratch127;
	vu32 scratch128;
	vu32 scratch129;
	vu32 scratch130;
	vu32 scratch131;
	vu32 scratch132;
	vu32 scratch133;
	vu32 scratch134;
	vu32 scratch135;
	vu32 scratch136;
	vu32 scratch137;
	vu32 scratch138;
	vu32 scratch139;
	vu32 scratch140;
	vu32 scratch141;
	vu32 scratch142;
	vu32 scratch143;
	vu32 scratch144;
	vu32 scratch145;
	vu32 scratch146;
	vu32 scratch147;
	vu32 scratch148;
	vu32 scratch149;
	vu32 scratch150;
	vu32 scratch151;
	vu32 scratch152;
	vu32 scratch153;
	vu32 scratch154;
	vu32 scratch155;
	vu32 scratch156;
	vu32 scratch157;
	vu32 scratch158;
	vu32 scratch159;
	vu32 scratch160;
	vu32 scratch161;
	vu32 scratch162;
	vu32 scratch163;
	vu32 scratch164;
	vu32 scratch165;
	vu32 scratch166;
	vu32 scratch167;
	vu32 scratch168;
	vu32 scratch169;
	vu32 scratch170;
	vu32 scratch171;
	vu32 scratch172;
	vu32 scratch173;
	vu32 scratch174;
	vu32 scratch175;
	vu32 scratch176;
	vu32 scratch177;
	vu32 scratch178;
	vu32 scratch179;
	vu32 scratch180;
	vu32 scratch181;
	vu32 scratch182;
	vu32 scratch183;
	vu32 scratch184;
	vu32 scratch185;
	vu32 scratch186;
	vu32 scratch187;
	vu32 scratch188;
	vu32 scratch189;
	vu32 scratch190;
	vu32 scratch191;
	vu32 scratch192;
	vu32 scratch193;
	vu32 scratch194;
	vu32 scratch195;
	vu32 scratch196;
	vu32 scratch197;
	vu32 scratch198;
	vu32 scratch199;
	vu32 scratch200;
	vu32 scratch201;
	vu32 scratch202;
	vu32 scratch203;
	vu32 scratch204;
	vu32 scratch205;
	vu32 scratch206;
	vu32 scratch207;
	vu32 scratch208;
	vu32 scratch209;
	vu32 scratch210;
	vu32 scratch211;
	vu32 scratch212;
	vu32 scratch213;
	vu32 scratch214;
	vu32 scratch215;
	vu32 scratch216;
	vu32 scratch217;
	vu32 scratch218;
	vu32 scratch219;
	vu32 scratch220;
	vu32 scratch221;
	vu32 scratch222;
	vu32 scratch223;
	vu32 scratch224;
	vu32 scratch225;
	vu32 scratch226;
	vu32 scratch227;
	vu32 scratch228;
	vu32 scratch229;
	vu32 scratch230;
	vu32 scratch231;
	vu32 scratch232;
	vu32 scratch233;
	vu32 scratch234;
	vu32 scratch235;
	vu32 scratch236;
	vu32 scratch237;
	vu32 scratch238;
	vu32 scratch239;
	vu32 scratch240;
	vu32 scratch241;
	vu32 scratch242;
	vu32 scratch243;
	vu32 scratch244;
	vu32 scratch245;
	vu32 scratch246;
	vu32 scratch247;
	vu32 scratch248;
	vu32 scratch249;
	vu32 scratch250;
	vu32 scratch251;
	vu32 scratch252;
	vu32 scratch253;
	vu32 scratch254;
	vu32 scratch255;
	vu32 scratch256;
	vu32 scratch257;
	vu32 scratch258;
	vu32 scratch259;
	vu32 scratch260;
	vu32 scratch261;
	vu32 scratch262;
	vu32 scratch263;
	vu32 scratch264;
	vu32 scratch265;
	vu32 scratch266;
	vu32 scratch267;
	vu32 scratch268;
	vu32 scratch269;
	vu32 scratch270;
	vu32 scratch271;
	vu32 scratch272;
	vu32 scratch273;
	vu32 scratch274;
	vu32 scratch275;
	vu32 scratch276;
	vu32 scratch277;
	vu32 scratch278;
	vu32 scratch279;
	vu32 scratch280;
	vu32 scratch281;
	vu32 scratch282;
	vu32 scratch283;
	vu32 scratch284;
	vu32 scratch285;
	vu32 scratch286;
	vu32 scratch287;
	vu32 scratch288;
	vu32 scratch289;
	vu32 scratch290;
	vu32 scratch291;
	vu32 scratch292;
	vu32 scratch293;
	vu32 scratch294;
	vu32 scratch295;
	vu32 scratch296;
	vu32 scratch297;
	vu32 scratch298;
	vu32 scratch299;			/* 0x9CC */
	vu32 _rsv0x9D0[50];
	vu32 secure_scratch80;	/* 0xa98 */
	vu32 secure_scratch81;
	vu32 secure_scratch82;
	vu32 secure_scratch83;
	vu32 secure_scratch84;
	vu32 secure_scratch85;
	vu32 secure_scratch86;
	vu32 secure_scratch87;
	vu32 secure_scratch88;
	vu32 secure_scratch89;
	vu32 secure_scratch90;
	vu32 secure_scratch91;
	vu32 secure_scratch92;
	vu32 secure_scratch93;
	vu32 secure_scratch94;
	vu32 secure_scratch95;
	vu32 secure_scratch96;
	vu32 secure_scratch97;
	vu32 secure_scratch98;
	vu32 secure_scratch99;
	vu32 secure_scratch100;
	vu32 secure_scratch101;
	vu32 secure_scratch102;
	vu32 secure_scratch103;
	vu32 secure_scratch104;
	vu32 secure_scratch105;
	vu32 secure_scratch106;
	vu32 secure_scratch107;
	vu32 secure_scratch108;
	vu32 secure_scratch109;
	vu32 secure_scratch110;
	vu32 secure_scratch111;
	vu32 secure_scratch112;
	vu32 secure_scratch113;
	vu32 secure_scratch114;
	vu32 secure_scratch115;
	vu32 secure_scratch116;
	vu32 secure_scratch117;
	vu32 secure_scratch118;
	vu32 secure_scratch119;
};

enum {
	PMC_RST_STATUS_SOURCE_MASK = 0x7,
	PMC_RST_STATUS_SOURCE_POR = 0x0,
	PMC_RST_STATUS_SOURCE_WATCHDOG = 0x1,
	PMC_RST_STATUS_SOURCE_SENSOR = 0x2,
	PMC_RST_STATUS_SOURCE_SW_MAIN = 0x3,
	PMC_RST_STATUS_SOURCE_LP0 = 0x4,
	PMC_RST_STATUS_NUM_SOURCES = 0x5,
};

enum {
	PMC_PWRGATE_TOGGLE_PARTID_MASK = 0x1f,
	PMC_PWRGATE_TOGGLE_PARTID_SHIFT = 0,
	PMC_PWRGATE_TOGGLE_START = 0x1 << 8
};

enum {
	PMC_CNTRL_KBC_CLK_DIS = 0x1 << 0,
	PMC_CNTRL_RTC_CLK_DIS = 0x1 << 1,
	PMC_CNTRL_RTC_RST = 0x1 << 2,
	PMC_CNTRL_KBC_RST = 0x1 << 3,
	PMC_CNTRL_MAIN_RST = 0x1 << 4,
	PMC_CNTRL_LATCHWAKE_EN = 0x1 << 5,
	PMC_CNTRL_GLITCHDET_DIS = 0x1 << 6,
	PMC_CNTRL_BLINK_EN = 0x1 << 7,
	PMC_CNTRL_PWRREQ_POLARITY = 0x1 << 8,
	PMC_CNTRL_PWRREQ_OE = 0x1 << 9,
	PMC_CNTRL_SYSCLK_POLARITY = 0x1 << 10,
	PMC_CNTRL_SYSCLK_OE = 0x1 << 11,
	PMC_CNTRL_PWRGATE_DIS = 0x1 << 12,
	PMC_CNTRL_AOINIT = 0x1 << 13,
	PMC_CNTRL_SIDE_EFFECT_LP0 = 0x1 << 14,
	PMC_CNTRL_CPUPWRREQ_POLARITY = 0x1 << 15,
	PMC_CNTRL_CPUPWRREQ_OE = 0x1 << 16,
	PMC_CNTRL_INTR_POLARITY = 0x1 << 17,
	PMC_CNTRL_FUSE_OVERRIDE = 0x1 << 18,
	PMC_CNTRL_CPUPWRGOOD_EN = 0x1 << 19,
	PMC_CNTRL_CPUPWRGOOD_SEL_SHIFT = 20,
	PMC_CNTRL_CPUPWRGOOD_SEL_MASK =
		0x3 << PMC_CNTRL_CPUPWRGOOD_SEL_SHIFT
};

enum {
	PMC_DDR_PWR_EMMC_MASK = 1 << 1,
	PMC_DDR_PWR_VAL_MASK = 1 << 0,
};

enum {
	PMC_DDR_CFG_PKG_MASK = 1 << 0,
	PMC_DDR_CFG_IF_MASK = 1 << 1,
	PMC_DDR_CFG_XM0_RESET_TRI_MASK = 1 << 12,
	PMC_DDR_CFG_XM0_RESET_DPDIO_MASK = 1 << 13,
};

enum {
	PMC_NO_IOPOWER_MEM_MASK = 1 << 7,
	PMC_NO_IOPOWER_MEM_COMP_MASK = 1 << 16,
};

enum {
	PMC_POR_DPD_CTRL_MEM0_ADDR0_CLK_SEL_DPD_MASK = 1 << 0,
	PMC_POR_DPD_CTRL_MEM0_ADDR1_CLK_SEL_DPD_MASK = 1 << 1,
	PMC_POR_DPD_CTRL_MEM0_HOLD_CKE_LOW_OVR_MASK = 1 << 31,
};

enum {
	PMC_CNTRL2_HOLD_CKE_LOW_EN = 0x1 << 12
};

enum {
	PMC_OSC_EDPD_OVER_XOFS_SHIFT = 1,
	PMC_OSC_EDPD_OVER_XOFS_MASK =
		0x3f << PMC_OSC_EDPD_OVER_XOFS_SHIFT
};

enum {
	PMC_CMD_HOLD_LOW_BR00_11_MASK = 0x0007FF80,
	DPD_OFF = 1 << 30,
	DPD_ON = 2 << 30,
};

enum {
	PMC_GPIO_RAIL_AO_SHIFT = 21,
	PMC_GPIO_RAIL_AO_MASK = (1 << PMC_GPIO_RAIL_AO_SHIFT),
	PMC_GPIO_RAIL_AO_DISABLE = (0 << PMC_GPIO_RAIL_AO_SHIFT),
	PMC_GPIO_RAIL_AO_ENABLE = (1 << PMC_GPIO_RAIL_AO_SHIFT),

	PMC_AUDIO_RAIL_AO_SHIFT = 18,
	PMC_AUDIO_RAIL_AO_MASK = (1 << PMC_AUDIO_RAIL_AO_SHIFT),
	PMC_AUDIO_RAIL_AO_DISABLE = (0 << PMC_AUDIO_RAIL_AO_SHIFT),
	PMC_AUDIO_RAIL_AO_ENABLE = (1 << PMC_AUDIO_RAIL_AO_SHIFT),

	PMC_SDMMC3_RAIL_AO_SHIFT = 13,
	PMC_SDMMC3_RAIL_AO_MASK = (1 << PMC_SDMMC3_RAIL_AO_SHIFT),
	PMC_SDMMC3_RAIL_AO_DISABLE = (0 << PMC_SDMMC3_RAIL_AO_SHIFT),
	PMC_SDMMC3_RAIL_AO_ENABLE = (1 << PMC_SDMMC3_RAIL_AO_SHIFT),
};

/*! PMC registers. */
#define APBDEV_PMC_PWRGATE_TOGGLE 0x30
#define APBDEV_PMC_PWRGATE_STATUS 0x38
#define APBDEV_PMC_NO_IOPOWER 0x44
#define APBDEV_PMC_SCRATCH0 0x50
#define APBDEV_PMC_SCRATCH1 0x54
#define APBDEV_PMC_SCRATCH20 0xA0
#define APBDEV_PMC_PWR_DET_VAL 0xE4
#define APBDEV_PMC_DDR_PWR 0xE8
#define APBDEV_PMC_CRYPTO_OP 0xF4
#define APBDEV_PMC_OSC_EDPD_OVER 0x1A4
#define APBDEV_PMC_IO_DPD_REQ 0x1B8
#define APBDEV_PMC_IO_DPD2_REQ 0x1C0
#define APBDEV_PMC_VDDP_SEL 0x1CC
#define APBDEV_PMC_TSC_MULT 0x2B4
#define APBDEV_PMC_REG_SHORT 0x2CC
#define APBDEV_PMC_WEAK_BIAS 0x2C8
#define APBDEV_PMC_SECURE_SCRATCH21 0x334
#define APBDEV_PMC_SECURE_SCRATCH32 0x360
#define APBDEV_PMC_SECURE_SCRATCH49 0x3A4
#define APBDEV_PMC_CNTRL2 0x440
#define APBDEV_PMC_IO_DPD4_REQ 0x464
#define APBDEV_PMC_DDR_CNTRL 0x4E4
#define APBDEV_PMC_SCRATCH188 0x810
#define APBDEV_PMC_SCRATCH190 0x818
#define APBDEV_PMC_SCRATCH200 0x840
#define APBDEV_PMC_SCRATCH49 0x244
#endif
