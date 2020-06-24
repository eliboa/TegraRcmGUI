/*
 * This file is part of the coreboot project.
 *
 * Copyright 2014 Google Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include "t210.h"
#include "mc_t210.h"
#include "clock.h"
#include "i2c.h"
#include "sdram_param_t210.h"
#include "emc.h"
#include "pmc.h"
#include "timer.h"
#include "fuse.h"
#include "max7762x.h"
#include "max77620.h"
#include "sdram_param_t210.h"
#include "lib/printk.h"

static void sdram_patch(uintptr_t addr, uint32_t value)
{
	if (addr != 0)
	{
		volatile uint32_t* volPtr = (void*)addr;
		*volPtr = value;
	}
}

static void writebits(uint32_t value, vu32* addr, uint32_t mask)
{
	volatile uint32_t* volPtr = (void*)addr;
	*volPtr = (*volPtr & (~mask)) | (value & mask);
}

static void sdram_trigger_emc_timing_update(struct tegra_emc_regs *regs)
{
	regs->timing_control = EMC_TIMING_CONTROL_TIMING_UPDATE;
}

/* PMC must be configured before clock-enable and de-reset of MC/EMC. */
static void sdram_configure_pmc(const struct sdram_params *param,
				struct tegra_pmc_regs *regs)
{
	/* VDDP Select */
	regs->vddp_sel = param->PmcVddpSel;
	usleep(param->PmcVddpSelWait);

	/* Set DDR pad voltage */
	writebits(param->PmcDdrPwr, &regs->ddr_pwr, PMC_DDR_PWR_VAL_MASK);

	/* Turn on MEM IO Power */
	writebits(param->PmcNoIoPower, &regs->no_iopower,
		  (PMC_NO_IOPOWER_MEM_MASK | PMC_NO_IOPOWER_MEM_COMP_MASK));

	regs->reg_short = param->PmcRegShort;
	regs->ddr_cntrl = param->PmcDdrCntrl;
}

static void sdram_set_ddr_control(const struct sdram_params *param,
				  struct tegra_pmc_regs *regs)
{
	u32 ddrcntrl = regs->ddr_cntrl;

	/* Deassert HOLD_CKE_LOW */
	ddrcntrl &= ~PMC_CMD_HOLD_LOW_BR00_11_MASK;
	regs->ddr_cntrl = ddrcntrl;
	usleep(param->PmcDdrCntrlWait);
}

/* Start PLLM for SDRAM. */
static void clock_sdram(u32 m, u32 n, u32 p, u32 setup, u32 kvco, u32 kcp,
		 u32 stable_time, u32 emc_source, u32 same_freq)
{
	u32 misc1 = ((setup << PLLM_MISC1_SETUP_SHIFT)),
	    misc2 = ((kvco << PLLM_MISC2_KVCO_SHIFT) |
		     (kcp << PLLM_MISC2_KCP_SHIFT) |
		     PLLM_EN_LCKDET),
	    base;

	if (same_freq)
		emc_source |= CLK_SOURCE_EMC_MC_EMC_SAME_FREQ;
	else
		emc_source &= ~CLK_SOURCE_EMC_MC_EMC_SAME_FREQ;

	/*
	 * Note PLLM_BASE.PLLM_OUT1_RSTN must be in RESET_ENABLE mode, and
	 * PLLM_BASE.ENABLE must be in DISABLE state (both are the default
	 * values after coldboot reset).
	 */

	CLOCK(CLK_RST_CONTROLLER_PLLM_MISC1) = misc1;
	CLOCK(CLK_RST_CONTROLLER_PLLM_MISC2) = misc2;

	/* PLLM.BASE needs BYPASS=0, different from general init_pll */
	base = CLOCK(CLK_RST_CONTROLLER_PLLM_BASE);
	base &= ~(PLLCMX_BASE_DIVN_MASK | PLLCMX_BASE_DIVM_MASK |
		  PLLM_BASE_DIVP_MASK | PLL_BASE_BYPASS);
	base |= ((m << PLL_BASE_DIVM_SHIFT) | (n << PLL_BASE_DIVN_SHIFT) |
		 (p << PLL_BASE_DIVP_SHIFT));
	CLOCK(CLK_RST_CONTROLLER_PLLM_BASE) = base;

	CLOCK(CLK_RST_CONTROLLER_PLLM_BASE) |= PLL_BASE_ENABLE;
	/* stable_time is required, before we can start to check lock. */
	usleep(stable_time);

	while (!(CLOCK(CLK_RST_CONTROLLER_PLLM_BASE) & PLL_BASE_LOCK))
		usleep(1);

	/*
	 * After PLLM reports being locked, we have to delay 10us before
	 * enabling PLLM_OUT.
	 */
	usleep(10);

	/* Enable and start MEM(MC) and EMC. */
	CLOCK(CLK_RST_CONTROLLER_CLK_ENB_H_SET) = CLK_H_MEM | CLK_H_EMC;
	/* Give clocks time to stabilize. */
	usleep(IO_STABILIZATION_DELAY);
	CLOCK(CLK_RST_CONTROLLER_RST_DEV_H_CLR) = CLK_H_MEM | CLK_H_EMC;

	CLOCK(CLK_RST_CONTROLLER_CLK_SOURCE_EMC) = emc_source;
	usleep(IO_STABILIZATION_DELAY);
}

static void sdram_start_clocks(const struct sdram_params *param,
			       struct tegra_emc_regs *regs)
{
	u32 is_same_freq = (param->McEmemArbMisc0 &
			    MC_EMEM_ARB_MISC0_MC_EMC_SAME_FREQ_MASK) ? 1 : 0;
	u32 clk_source_emc = param->EmcClockSource;

	/* Enable the clocks for EMC and MC */
	CLOCK(CLK_RST_CONTROLLER_CLK_ENB_H_SET) |= CLK_H_EMC;	// ENB_EMC
	CLOCK(CLK_RST_CONTROLLER_CLK_ENB_H_SET) |= CLK_H_MEM;	// ENB_MC

	if ((clk_source_emc >> EMC_2X_CLK_SRC_SHIFT) != PLLM_UD)
		CLOCK(CLK_RST_CONTROLLER_CLK_ENB_X_SET) |= CLK_ENB_EMC_DLL;

	/* Remove the EMC and MC controllers from reset */
	CLOCK(CLK_RST_CONTROLLER_RST_DEVICES_H) &= ~(1 << 25);		// SWR_EMC
	CLOCK(CLK_RST_CONTROLLER_RST_DEVICES_H) &= ~(1 << 0);		// SWR_MC

	clk_source_emc |= (is_same_freq << 16);

	CLOCK(CLK_RST_CONTROLLER_CLK_SOURCE_EMC) = clk_source_emc;
	CLOCK(CLK_RST_CONTROLLER_CLK_SOURCE_EMC_DLL) = param->EmcClockSourceDll;

	clock_sdram(param->PllMInputDivider, param->PllMFeedbackDivider,
		    param->PllMPostDivider, param->PllMSetupControl,
		    param->PllMKVCO, param->PllMKCP, param->PllMStableTime,
		    param->EmcClockSource, is_same_freq);

	if (param->ClkRstControllerPllmMisc2OverrideEnable)
		CLOCK(CLK_RST_CONTROLLER_PLLM_MISC2) = param->ClkRstControllerPllmMisc2Override;

	/* Wait for enough time for clk switch to take place */
	usleep(5);

	CLOCK(CLK_RST_CONTROLLER_CLK_ENB_W_CLR) = param->ClearClk2Mc1;
}

static void sdram_set_swizzle(const struct sdram_params *param,
			      struct tegra_emc_regs *regs)
{
	regs->swizzle_rank0_byte0 = param->EmcSwizzleRank0Byte0;
	regs->swizzle_rank0_byte1 = param->EmcSwizzleRank0Byte1;
	regs->swizzle_rank0_byte2 = param->EmcSwizzleRank0Byte2;
	regs->swizzle_rank0_byte3 = param->EmcSwizzleRank0Byte3;

	regs->swizzle_rank1_byte0 = param->EmcSwizzleRank1Byte0;
	regs->swizzle_rank1_byte1 = param->EmcSwizzleRank1Byte1;
	regs->swizzle_rank1_byte2 = param->EmcSwizzleRank1Byte2;
	regs->swizzle_rank1_byte3 = param->EmcSwizzleRank1Byte3;
}

static void sdram_set_pad_controls(const struct sdram_params *param,
				   struct tegra_emc_regs *regs)
{
	/* Program the pad controls */
	regs->xm2comppadctrl = param->EmcXm2CompPadCtrl;
	regs->xm2comppadctrl2 = param->EmcXm2CompPadCtrl2;
	regs->xm2comppadctrl3 = param->EmcXm2CompPadCtrl3;
}

static void sdram_set_pad_macros(const struct sdram_params *param,
				 struct tegra_emc_regs *regs)
{
	u32 rfu_reset, rfu_mask1, rfu_mask2, rfu_step1, rfu_step2;
	u32 cpm_reset_settings, cpm_mask1, cpm_step1;

	regs->pmacro_vttgen_ctrl0 = param->EmcPmacroVttgenCtrl0;
	regs->pmacro_vttgen_ctrl1 = param->EmcPmacroVttgenCtrl1;
	regs->pmacro_vttgen_ctrl2 = param->EmcPmacroVttgenCtrl2;
	/* Trigger timing update so above writes take place */
	sdram_trigger_emc_timing_update(regs);
	/* Add a wait to ensure the regulators settle */
	usleep(10);

	regs->dbg = param->EmcDbg | (param->EmcDbgWriteMux & WRITE_MUX_ACTIVE);

	rfu_reset = EMC_PMACRO_BRICK_CTRL_RFU1_RESET_VAL;
	rfu_mask1 = 0x01120112;
	rfu_mask2 = 0x01BF01BF;

	rfu_step1 = rfu_reset & (param->EmcPmacroBrickCtrlRfu1 | ~rfu_mask1);
	rfu_step2 = rfu_reset & (param->EmcPmacroBrickCtrlRfu1 | ~rfu_mask2);

	/* common pad macro (cpm) */
	cpm_reset_settings = 0x0000000F;
	cpm_mask1 = 0x00000001;
	cpm_step1 = cpm_reset_settings;
	cpm_step1 &= (param->EmcPmacroCommonPadTxCtrl | ~cpm_mask1);

	/* Patch 2 using BCT spare variables */
	sdram_patch(param->EmcBctSpare2, param->EmcBctSpare3);

	/*
	 * Program CMD mapping. Required before brick mapping, else
	 * we can't gaurantee CK will be differential at all times.
	 */
	regs->fbio_cfg7 = param->EmcFbioCfg7;

	regs->cmd_mapping_cmd0_0 = param->EmcCmdMappingCmd0_0;
	regs->cmd_mapping_cmd0_1 = param->EmcCmdMappingCmd0_1;
	regs->cmd_mapping_cmd0_2 = param->EmcCmdMappingCmd0_2;
	regs->cmd_mapping_cmd1_0 = param->EmcCmdMappingCmd1_0;
	regs->cmd_mapping_cmd1_1 = param->EmcCmdMappingCmd1_1;
	regs->cmd_mapping_cmd1_2 = param->EmcCmdMappingCmd1_2;
	regs->cmd_mapping_cmd2_0 = param->EmcCmdMappingCmd2_0;
	regs->cmd_mapping_cmd2_1 = param->EmcCmdMappingCmd2_1;
	regs->cmd_mapping_cmd2_2 = param->EmcCmdMappingCmd2_2;
	regs->cmd_mapping_cmd3_0 = param->EmcCmdMappingCmd3_0;
	regs->cmd_mapping_cmd3_1 = param->EmcCmdMappingCmd3_1;
	regs->cmd_mapping_cmd3_2 = param->EmcCmdMappingCmd3_2;
	regs->cmd_mapping_byte = param->EmcCmdMappingByte;

	/* Program brick mapping. */
	regs->pmacro_brick_mapping0 = param->EmcPmacroBrickMapping0;
	regs->pmacro_brick_mapping1 = param->EmcPmacroBrickMapping1;
	regs->pmacro_brick_mapping2 = param->EmcPmacroBrickMapping2;

	regs->pmacro_brick_ctrl_rfu1 = rfu_step1;

	/* This is required to do any reads from the pad macros */
	regs->config_sample_delay = param->EmcConfigSampleDelay;

	regs->fbio_cfg8 = param->EmcFbioCfg8;

	sdram_set_swizzle(param, regs);

	/* Patch 4 using BCT spare variables */
	sdram_patch(param->EmcBctSpare6, param->EmcBctSpare7);

	sdram_set_pad_controls(param, regs);

	/* Program Autocal controls with shadowed register fields */
	regs->auto_cal_config2 = param->EmcAutoCalConfig2;
	regs->auto_cal_config3 = param->EmcAutoCalConfig3;
	regs->auto_cal_config4 = param->EmcAutoCalConfig4;
	regs->auto_cal_config5 = param->EmcAutoCalConfig5;
	regs->auto_cal_config6 = param->EmcAutoCalConfig6;
	regs->auto_cal_config7 = param->EmcAutoCalConfig7;
	regs->auto_cal_config8 = param->EmcAutoCalConfig8;

	regs->pmacro_rx_term = param->EmcPmacroRxTerm;
	regs->pmacro_dq_tx_drv = param->EmcPmacroDqTxDrv;
	regs->pmacro_ca_tx_drv = param->EmcPmacroCaTxDrv;
	regs->pmacro_cmd_tx_drv = param->EmcPmacroCmdTxDrv;
	regs->pmacro_autocal_cfg_common = param->EmcPmacroAutocalCfgCommon;
	regs->auto_cal_channel = param->EmcAutoCalChannel;
	regs->pmacro_zctrl = param->EmcPmacroZctrl;

	regs->dll_cfg0 = param->EmcDllCfg0;
	regs->dll_cfg1 = param->EmcDllCfg1;
	regs->cfg_dig_dll_1 = param->EmcCfgDigDll_1;

	regs->data_brlshft_0 = param->EmcDataBrlshft0;
	regs->data_brlshft_1 = param->EmcDataBrlshft1;
	regs->dqs_brlshft_0 = param->EmcDqsBrlshft0;
	regs->dqs_brlshft_1 = param->EmcDqsBrlshft1;
	regs->cmd_brlshft_0 = param->EmcCmdBrlshft0;
	regs->cmd_brlshft_1 = param->EmcCmdBrlshft1;
	regs->cmd_brlshft_2 = param->EmcCmdBrlshft2;
	regs->cmd_brlshft_3 = param->EmcCmdBrlshft3;
	regs->quse_brlshft_0 = param->EmcQuseBrlshft0;
	regs->quse_brlshft_1 = param->EmcQuseBrlshft1;
	regs->quse_brlshft_2 = param->EmcQuseBrlshft2;
	regs->quse_brlshft_3 = param->EmcQuseBrlshft3;

	regs->pmacro_brick_ctrl_rfu1 = rfu_step2;
	regs->pmacro_pad_cfg_ctrl = param->EmcPmacroPadCfgCtrl;

	regs->pmacro_pad_cfg_ctrl = param->EmcPmacroPadCfgCtrl;
	regs->pmacro_cmd_brick_ctrl_fdpd = param->EmcPmacroCmdBrickCtrlFdpd;
	regs->pmacro_brick_ctrl_rfu2 = param->EmcPmacroBrickCtrlRfu2 & 0xFF7FFF7F;
	regs->pmacro_data_brick_ctrl_fdpd = param->EmcPmacroDataBrickCtrlFdpd;
	regs->pmacro_bg_bias_ctrl_0 = param->EmcPmacroBgBiasCtrl0;
	regs->pmacro_data_pad_rx_ctrl = param->EmcPmacroDataPadRxCtrl;
	regs->pmacro_cmd_pad_rx_ctrl = param->EmcPmacroCmdPadRxCtrl;
	regs->pmacro_data_pad_tx_ctrl = param->EmcPmacroDataPadTxCtrl;
	regs->pmacro_data_rx_term_mode = param->EmcPmacroDataRxTermMode;
	regs->pmacro_cmd_rx_term_mode = param->EmcPmacroCmdRxTermMode;
	regs->pmacro_cmd_pad_tx_ctrl = param->EmcPmacroCmdPadTxCtrl;

	regs->cfg_3 = param->EmcCfg3;
	regs->pmacro_tx_pwrd_0 = param->EmcPmacroTxPwrd0;
	regs->pmacro_tx_pwrd_1 = param->EmcPmacroTxPwrd1;
	regs->pmacro_tx_pwrd_2 = param->EmcPmacroTxPwrd2;
	regs->pmacro_tx_pwrd_3 = param->EmcPmacroTxPwrd3;
	regs->pmacro_tx_pwrd_4 = param->EmcPmacroTxPwrd4;
	regs->pmacro_tx_pwrd_5 = param->EmcPmacroTxPwrd5;
	regs->pmacro_tx_sel_clk_src_0 = param->EmcPmacroTxSelClkSrc0;
	regs->pmacro_tx_sel_clk_src_1 = param->EmcPmacroTxSelClkSrc1;
	regs->pmacro_tx_sel_clk_src_2 = param->EmcPmacroTxSelClkSrc2;
	regs->pmacro_tx_sel_clk_src_3 = param->EmcPmacroTxSelClkSrc3;
	regs->pmacro_tx_sel_clk_src_4 = param->EmcPmacroTxSelClkSrc4;
	regs->pmacro_tx_sel_clk_src_5 = param->EmcPmacroTxSelClkSrc5;
	regs->pmacro_ddll_bypass = param->EmcPmacroDdllBypass;
	regs->pmacro_ddll_pwrd_0 = param->EmcPmacroDdllPwrd0;
	regs->pmacro_ddll_pwrd_1 = param->EmcPmacroDdllPwrd1;
	regs->pmacro_ddll_pwrd_2 = param->EmcPmacroDdllPwrd2;
	regs->pmacro_cmd_ctrl_0 = param->EmcPmacroCmdCtrl0;
	regs->pmacro_cmd_ctrl_1 = param->EmcPmacroCmdCtrl1;
	regs->pmacro_cmd_ctrl_2 = param->EmcPmacroCmdCtrl2;
	regs->pmacro_ib_vref_dq_0 = param->EmcPmacroIbVrefDq_0;
	regs->pmacro_ib_vref_dq_1 = param->EmcPmacroIbVrefDq_1;
	regs->pmacro_ib_vref_dqs_0 = param->EmcPmacroIbVrefDqs_0;
	regs->pmacro_ib_vref_dqs_1 = param->EmcPmacroIbVrefDqs_1;
	regs->pmacro_ib_rxrt = param->EmcPmacroIbRxrt;
	regs->pmacro_quse_ddll_rank0_0 = param->EmcPmacroQuseDdllRank0_0;
	regs->pmacro_quse_ddll_rank0_1 = param->EmcPmacroQuseDdllRank0_1;
	regs->pmacro_quse_ddll_rank0_2 = param->EmcPmacroQuseDdllRank0_2;
	regs->pmacro_quse_ddll_rank0_3 = param->EmcPmacroQuseDdllRank0_3;
	regs->pmacro_quse_ddll_rank0_4 = param->EmcPmacroQuseDdllRank0_4;
	regs->pmacro_quse_ddll_rank0_5 = param->EmcPmacroQuseDdllRank0_5;
	regs->pmacro_quse_ddll_rank1_0 = param->EmcPmacroQuseDdllRank1_0;
	regs->pmacro_quse_ddll_rank1_1 = param->EmcPmacroQuseDdllRank1_1;
	regs->pmacro_quse_ddll_rank1_2 = param->EmcPmacroQuseDdllRank1_2;
	regs->pmacro_quse_ddll_rank1_3 = param->EmcPmacroQuseDdllRank1_3;
	regs->pmacro_quse_ddll_rank1_4 = param->EmcPmacroQuseDdllRank1_4;
	regs->pmacro_quse_ddll_rank1_5 = param->EmcPmacroQuseDdllRank1_5;
	regs->pmacro_brick_ctrl_rfu1 = param->EmcPmacroBrickCtrlRfu1;
	regs->pmacro_ob_ddll_long_dq_rank0_0 = param->EmcPmacroObDdllLongDqRank0_0;
	regs->pmacro_ob_ddll_long_dq_rank0_1 = param->EmcPmacroObDdllLongDqRank0_1;
	regs->pmacro_ob_ddll_long_dq_rank0_2 = param->EmcPmacroObDdllLongDqRank0_2;
	regs->pmacro_ob_ddll_long_dq_rank0_3 = param->EmcPmacroObDdllLongDqRank0_3;
	regs->pmacro_ob_ddll_long_dq_rank0_4 = param->EmcPmacroObDdllLongDqRank0_4;
	regs->pmacro_ob_ddll_long_dq_rank0_5 = param->EmcPmacroObDdllLongDqRank0_5;
	regs->pmacro_ob_ddll_long_dq_rank1_0 = param->EmcPmacroObDdllLongDqRank1_0;
	regs->pmacro_ob_ddll_long_dq_rank1_1 = param->EmcPmacroObDdllLongDqRank1_1;
	regs->pmacro_ob_ddll_long_dq_rank1_2 = param->EmcPmacroObDdllLongDqRank1_2;
	regs->pmacro_ob_ddll_long_dq_rank1_3 = param->EmcPmacroObDdllLongDqRank1_3;
	regs->pmacro_ob_ddll_long_dq_rank1_4 = param->EmcPmacroObDdllLongDqRank1_4;
	regs->pmacro_ob_ddll_long_dq_rank1_5 = param->EmcPmacroObDdllLongDqRank1_5;

	regs->pmacro_ob_ddll_long_dqs_rank0_0 = param->EmcPmacroObDdllLongDqsRank0_0;
	regs->pmacro_ob_ddll_long_dqs_rank0_1 = param->EmcPmacroObDdllLongDqsRank0_1;
	regs->pmacro_ob_ddll_long_dqs_rank0_2 = param->EmcPmacroObDdllLongDqsRank0_2;
	regs->pmacro_ob_ddll_long_dqs_rank0_3 = param->EmcPmacroObDdllLongDqsRank0_3;
	regs->pmacro_ob_ddll_long_dqs_rank0_4 = param->EmcPmacroObDdllLongDqsRank0_4;
	regs->pmacro_ob_ddll_long_dqs_rank0_5 = param->EmcPmacroObDdllLongDqsRank0_5;
	regs->pmacro_ob_ddll_long_dqs_rank1_0 = param->EmcPmacroObDdllLongDqsRank1_0;
	regs->pmacro_ob_ddll_long_dqs_rank1_1 = param->EmcPmacroObDdllLongDqsRank1_1;
	regs->pmacro_ob_ddll_long_dqs_rank1_2 = param->EmcPmacroObDdllLongDqsRank1_2;
	regs->pmacro_ob_ddll_long_dqs_rank1_3 = param->EmcPmacroObDdllLongDqsRank1_3;
	regs->pmacro_ob_ddll_long_dqs_rank1_4 = param->EmcPmacroObDdllLongDqsRank1_4;
	regs->pmacro_ob_ddll_long_dqs_rank1_5 = param->EmcPmacroObDdllLongDqsRank1_5;
	regs->pmacro_ib_ddll_long_dqs_rank0_0 = param->EmcPmacroIbDdllLongDqsRank0_0;
	regs->pmacro_ib_ddll_long_dqs_rank0_1 = param->EmcPmacroIbDdllLongDqsRank0_1;
	regs->pmacro_ib_ddll_long_dqs_rank0_2 = param->EmcPmacroIbDdllLongDqsRank0_2;
	regs->pmacro_ib_ddll_long_dqs_rank0_3 = param->EmcPmacroIbDdllLongDqsRank0_3;
	regs->pmacro_ib_ddll_long_dqs_rank1_0 = param->EmcPmacroIbDdllLongDqsRank1_0;
	regs->pmacro_ib_ddll_long_dqs_rank1_1 = param->EmcPmacroIbDdllLongDqsRank1_1;
	regs->pmacro_ib_ddll_long_dqs_rank1_2 = param->EmcPmacroIbDdllLongDqsRank1_2;
	regs->pmacro_ib_ddll_long_dqs_rank1_3 = param->EmcPmacroIbDdllLongDqsRank1_3;
	regs->pmacro_ddll_long_cmd_0 = param->EmcPmacroDdllLongCmd_0;
	regs->pmacro_ddll_long_cmd_1 = param->EmcPmacroDdllLongCmd_1;
	regs->pmacro_ddll_long_cmd_2 = param->EmcPmacroDdllLongCmd_2;
	regs->pmacro_ddll_long_cmd_3 = param->EmcPmacroDdllLongCmd_3;
	regs->pmacro_ddll_long_cmd_4 = param->EmcPmacroDdllLongCmd_4;
	regs->pmacro_ddll_short_cmd_0 = param->EmcPmacroDdllShortCmd_0;
	regs->pmacro_ddll_short_cmd_1 = param->EmcPmacroDdllShortCmd_1;
	regs->pmacro_ddll_short_cmd_2 = param->EmcPmacroDdllShortCmd_2;
	regs->pmacro_common_pad_tx_ctrl = cpm_step1;
}

static void sdram_setup_wpr_carveouts(const struct sdram_params *param,
				      struct tegra_mc_regs *regs)
{
	/* Program the 5 WPR carveouts with initial BCT settings. */
	regs->security_carveout1_bom = param->McGeneralizedCarveout1Bom;
	regs->security_carveout1_bom_hi = param->McGeneralizedCarveout1BomHi;
	regs->security_carveout1_size_128kb = param->McGeneralizedCarveout1Size128kb;
	regs->security_carveout1_ca0 = param->McGeneralizedCarveout1Access0;
	regs->security_carveout1_ca1 = param->McGeneralizedCarveout1Access1;
	regs->security_carveout1_ca2 = param->McGeneralizedCarveout1Access2;
	regs->security_carveout1_ca3 = param->McGeneralizedCarveout1Access3;
	regs->security_carveout1_ca4 = param->McGeneralizedCarveout1Access4;
	regs->security_carveout1_cfia0 = param->McGeneralizedCarveout1ForceInternalAccess0;
	regs->security_carveout1_cfia1 = param->McGeneralizedCarveout1ForceInternalAccess1;
	regs->security_carveout1_cfia2 = param->McGeneralizedCarveout1ForceInternalAccess2;
	regs->security_carveout1_cfia3 = param->McGeneralizedCarveout1ForceInternalAccess3;
	regs->security_carveout1_cfia4 = param->McGeneralizedCarveout1ForceInternalAccess4;
	regs->security_carveout1_cfg0 = param->McGeneralizedCarveout1Cfg0;

	regs->security_carveout2_bom = param->McGeneralizedCarveout2Bom;
	regs->security_carveout2_bom_hi = param->McGeneralizedCarveout2BomHi;
	regs->security_carveout2_size_128kb = param->McGeneralizedCarveout2Size128kb;
	regs->security_carveout2_ca0 = param->McGeneralizedCarveout2Access0;
	regs->security_carveout2_ca1 = param->McGeneralizedCarveout2Access1;
	regs->security_carveout2_ca2 = param->McGeneralizedCarveout2Access2;
	regs->security_carveout2_ca3 = param->McGeneralizedCarveout2Access3;
	regs->security_carveout2_ca4 = param->McGeneralizedCarveout2Access4;
	regs->security_carveout2_cfia0 = param->McGeneralizedCarveout2ForceInternalAccess0;
	regs->security_carveout2_cfia1 = param->McGeneralizedCarveout2ForceInternalAccess1;
	regs->security_carveout2_cfia2 = param->McGeneralizedCarveout2ForceInternalAccess2;
	regs->security_carveout2_cfia3 = param->McGeneralizedCarveout2ForceInternalAccess3;
	regs->security_carveout2_cfia4 = param->McGeneralizedCarveout2ForceInternalAccess4;
	regs->security_carveout2_cfg0 = param->McGeneralizedCarveout2Cfg0;

	regs->security_carveout3_bom = param->McGeneralizedCarveout3Bom;
	regs->security_carveout3_bom_hi = param->McGeneralizedCarveout3BomHi;
	regs->security_carveout3_size_128kb = param->McGeneralizedCarveout3Size128kb;
	regs->security_carveout3_ca0 = param->McGeneralizedCarveout3Access0;
	regs->security_carveout3_ca1 = param->McGeneralizedCarveout3Access1;
	regs->security_carveout3_ca2 = param->McGeneralizedCarveout3Access2;
	regs->security_carveout3_ca3 = param->McGeneralizedCarveout3Access3;
	regs->security_carveout3_ca4 = param->McGeneralizedCarveout3Access4;
	regs->security_carveout3_cfia0 = param->McGeneralizedCarveout3ForceInternalAccess0;
	regs->security_carveout3_cfia1 = param->McGeneralizedCarveout3ForceInternalAccess1;
	regs->security_carveout3_cfia2 = param->McGeneralizedCarveout3ForceInternalAccess2;
	regs->security_carveout3_cfia3 = param->McGeneralizedCarveout3ForceInternalAccess3;
	regs->security_carveout3_cfia4 = param->McGeneralizedCarveout3ForceInternalAccess4;
	regs->security_carveout3_cfg0 = param->McGeneralizedCarveout3Cfg0;

	regs->security_carveout4_bom = param->McGeneralizedCarveout4Bom;
	regs->security_carveout4_bom_hi = param->McGeneralizedCarveout4BomHi;
	regs->security_carveout4_size_128kb = param->McGeneralizedCarveout4Size128kb;
	regs->security_carveout4_ca0 = param->McGeneralizedCarveout4Access0;
	regs->security_carveout4_ca1 = param->McGeneralizedCarveout4Access1;
	regs->security_carveout4_ca2 = param->McGeneralizedCarveout4Access2;
	regs->security_carveout4_ca3 = param->McGeneralizedCarveout4Access3;
	regs->security_carveout4_ca4 = param->McGeneralizedCarveout4Access4;
	regs->security_carveout4_cfia0 = param->McGeneralizedCarveout4ForceInternalAccess0;
	regs->security_carveout4_cfia1 = param->McGeneralizedCarveout4ForceInternalAccess1;
	regs->security_carveout4_cfia2 = param->McGeneralizedCarveout4ForceInternalAccess2;
	regs->security_carveout4_cfia3 = param->McGeneralizedCarveout4ForceInternalAccess3;
	regs->security_carveout4_cfia4 = param->McGeneralizedCarveout4ForceInternalAccess4;
	regs->security_carveout4_cfg0 = param->McGeneralizedCarveout4Cfg0;

	regs->security_carveout5_bom = param->McGeneralizedCarveout5Bom;
	regs->security_carveout5_bom_hi = param->McGeneralizedCarveout5BomHi;
	regs->security_carveout5_size_128kb = param->McGeneralizedCarveout5Size128kb;
	regs->security_carveout5_ca0 = param->McGeneralizedCarveout5Access0;
	regs->security_carveout5_ca1 = param->McGeneralizedCarveout5Access1;
	regs->security_carveout5_ca2 = param->McGeneralizedCarveout5Access2;
	regs->security_carveout5_ca3 = param->McGeneralizedCarveout5Access3;
	regs->security_carveout5_ca4 = param->McGeneralizedCarveout5Access4;
	regs->security_carveout5_cfia0 = param->McGeneralizedCarveout5ForceInternalAccess0;
	regs->security_carveout5_cfia1 = param->McGeneralizedCarveout5ForceInternalAccess1;
	regs->security_carveout5_cfia2 = param->McGeneralizedCarveout5ForceInternalAccess2;
	regs->security_carveout5_cfia3 = param->McGeneralizedCarveout5ForceInternalAccess3;
	regs->security_carveout5_cfia4 = param->McGeneralizedCarveout5ForceInternalAccess4;
	regs->security_carveout5_cfg0 = param->McGeneralizedCarveout5Cfg0;
}

static void sdram_init_mc(const struct sdram_params *param,
			  struct tegra_mc_regs *regs)
{
	/* Initialize MC VPR settings */
	regs->video_protect_bom = param->McVideoProtectBom;
	regs->video_protect_bom_adr_hi = param->McVideoProtectBomAdrHi;
	regs->video_protect_size_mb = param->McVideoProtectSizeMb;
	regs->video_protect_vpr_override = param->McVideoProtectVprOverride;
	regs->video_protect_vpr_override1 = param->McVideoProtectVprOverride1;
	regs->video_protect_gpu_override_0 = param->McVideoProtectGpuOverride0;
	regs->video_protect_gpu_override_1 = param->McVideoProtectGpuOverride1;

	/* Program SDRAM geometry paarameters */
	regs->emem_adr_cfg = param->McEmemAdrCfg;
	regs->emem_adr_cfg_dev0 = param->McEmemAdrCfgDev0;
	regs->emem_adr_cfg_dev1 = param->McEmemAdrCfgDev1;
	regs->emem_adr_cfg_channel_mask = param->McEmemAdrCfgChannelMask;

	/* Program bank swizzling */
	regs->emem_adr_cfg_bank_mask_0 = param->McEmemAdrCfgBankMask0;
	regs->emem_adr_cfg_bank_mask_1 = param->McEmemAdrCfgBankMask1;
	regs->emem_adr_cfg_bank_mask_2 = param->McEmemAdrCfgBankMask2;

	/* Program external memory aperature (base and size) */
	regs->emem_cfg = param->McEmemCfg;

	/* Program SEC carveout (base and size) */
	regs->sec_carveout_bom = param->McSecCarveoutBom;
	regs->sec_carveout_adr_hi = param->McSecCarveoutAdrHi;
	regs->sec_carveout_size_mb = param->McSecCarveoutSizeMb;

	/* Program MTS carveout (base and size) */
	regs->mts_carveout_bom = param->McMtsCarveoutBom;
	regs->mts_carveout_adr_hi = param->McMtsCarveoutAdrHi;
	regs->mts_carveout_size_mb = param->McMtsCarveoutSizeMb;

	/* Initialize the WPR carveouts */
	sdram_setup_wpr_carveouts(param, regs);

	/* Program the memory arbiter */
	regs->emem_arb_cfg = param->McEmemArbCfg;
	regs->emem_arb_outstanding_req = param->McEmemArbOutstandingReq;
	regs->emem_arb_refpb_hp_ctrl = param->McEmemArbRefpbHpCtrl;
	regs->emem_arb_refpb_bank_ctrl = param->McEmemArbRefpbBankCtrl;
	regs->emem_arb_timing_rcd = param->McEmemArbTimingRcd;
	regs->emem_arb_timing_rp = param->McEmemArbTimingRp;
	regs->emem_arb_timing_rc = param->McEmemArbTimingRc;
	regs->emem_arb_timing_ras = param->McEmemArbTimingRas;
	regs->emem_arb_timing_faw = param->McEmemArbTimingFaw;
	regs->emem_arb_timing_rrd = param->McEmemArbTimingRrd;
	regs->emem_arb_timing_rap2pre = param->McEmemArbTimingRap2Pre;
	regs->emem_arb_timing_wap2pre = param->McEmemArbTimingWap2Pre;
	regs->emem_arb_timing_r2r = param->McEmemArbTimingR2R;
	regs->emem_arb_timing_w2w = param->McEmemArbTimingW2W;
	regs->emem_arb_timing_ccdmw = param->McEmemArbTimingCcdmw;
	regs->emem_arb_timing_r2w = param->McEmemArbTimingR2W;
	regs->emem_arb_timing_w2r = param->McEmemArbTimingW2R;
	regs->emem_arb_timing_rfcpb = param->McEmemArbTimingRFCPB;
	regs->emem_arb_da_turns = param->McEmemArbDaTurns;
	regs->emem_arb_da_covers = param->McEmemArbDaCovers;
	regs->emem_arb_misc0 = param->McEmemArbMisc0;
	regs->emem_arb_misc1 = param->McEmemArbMisc1;
	regs->emem_arb_misc2 = param->McEmemArbMisc2;
	regs->emem_arb_ring1_throttle = param->McEmemArbRing1Throttle;
	regs->emem_arb_override = param->McEmemArbOverride;
	regs->emem_arb_override_1 = param->McEmemArbOverride1;
	regs->emem_arb_rsv = param->McEmemArbRsv;
	regs->da_config0 = param->McDaCfg0;

	/* Trigger MC timing update */
	regs->timing_control = EMC_TIMING_CONTROL_TIMING_UPDATE;

	/* Program second-level clock enable overrides */
	regs->clken_override = param->McClkenOverride;

	/* Program statistics gathering */
	regs->stat_control = param->McStatControl;
}

static void sdram_init_emc(const struct sdram_params *param,
			   struct tegra_emc_regs *regs)
{
	/* Program SDRAM geometry parameters */
	regs->adr_cfg = param->EmcAdrCfg;

	/* Program second-level clock enable overrides */
	regs->clken_override = param->EmcClkenOverride;

	/* Program EMC pad auto calibration */
	regs->pmacro_autocal_cfg0 = param->EmcPmacroAutocalCfg0;
	regs->pmacro_autocal_cfg1 = param->EmcPmacroAutocalCfg1;
	regs->pmacro_autocal_cfg2 = param->EmcPmacroAutocalCfg2;

	regs->auto_cal_vref_sel0 = param->EmcAutoCalVrefSel0;
	regs->auto_cal_vref_sel1 = param->EmcAutoCalVrefSel1;

	regs->auto_cal_interval = param->EmcAutoCalInterval;
	regs->auto_cal_config = param->EmcAutoCalConfig;
	usleep(param->EmcAutoCalWait);
}

static void sdram_set_emc_timing(const struct sdram_params *param,
				 struct tegra_emc_regs *regs)
{
	/* Program EMC timing configuration */
	regs->cfg_2 = param->EmcCfg2;
	regs->cfg_pipe = param->EmcCfgPipe;
	regs->cfg_pipe1 = param->EmcCfgPipe1;
	regs->cfg_pipe2 = param->EmcCfgPipe2;
	regs->cmdq = param->EmcCmdQ;
	regs->mc2emcq = param->EmcMc2EmcQ;
	regs->mrs_wait_cnt = param->EmcMrsWaitCnt;
	regs->mrs_wait_cnt2 = param->EmcMrsWaitCnt2;
	regs->fbio_cfg5 = param->EmcFbioCfg5;
	regs->rc = param->EmcRc;
	regs->rfc = param->EmcRfc;
	regs->rfcpb = param->EmcRfcPb;
	regs->refctrl2 = param->EmcRefctrl2;
	regs->rfc_slr = param->EmcRfcSlr;
	regs->ras = param->EmcRas;
	regs->rp = param->EmcRp;
	regs->tppd = param->EmcTppd;
	regs->r2r = param->EmcR2r;
	regs->w2w = param->EmcW2w;
	regs->r2w = param->EmcR2w;
	regs->w2r = param->EmcW2r;
	regs->r2p = param->EmcR2p;
	regs->w2p = param->EmcW2p;
	regs->ccdmw = param->EmcCcdmw;
	regs->rd_rcd = param->EmcRdRcd;
	regs->wr_rcd = param->EmcWrRcd;
	regs->rrd = param->EmcRrd;
	regs->rext = param->EmcRext;
	regs->wext = param->EmcWext;
	regs->wdv = param->EmcWdv;
	regs->wdv_chk = param->EmcWdvChk;
	regs->wsv = param->EmcWsv;
	regs->wev = param->EmcWev;
	regs->wdv_mask = param->EmcWdvMask;
	regs->ws_duration = param->EmcWsDuration;
	regs->we_duration = param->EmcWeDuration;
	regs->quse = param->EmcQUse;
	regs->quse_width = param->EmcQuseWidth;
	regs->ibdly = param->EmcIbdly;
	regs->obdly = param->EmcObdly;
	regs->einput = param->EmcEInput;
	regs->einput_duration = param->EmcEInputDuration;
	regs->puterm_extra = param->EmcPutermExtra;
	regs->puterm_width = param->EmcPutermWidth;

	regs->pmacro_common_pad_tx_ctrl = param->EmcPmacroCommonPadTxCtrl;
	regs->dbg = param->EmcDbg;
	regs->qrst = param->EmcQRst;
	regs->issue_qrst = 1;
	regs->issue_qrst = 0;
	regs->qsafe = param->EmcQSafe;
	regs->rdv = param->EmcRdv;
	regs->rdv_mask = param->EmcRdvMask;
	regs->rdv_early = param->EmcRdvEarly;
	regs->rdv_early_mask = param->EmcRdvEarlyMask;
	regs->qpop = param->EmcQpop;
	regs->refresh = param->EmcRefresh;
	regs->burst_refresh_num = param->EmcBurstRefreshNum;
	regs->pre_refresh_req_cnt = param->EmcPreRefreshReqCnt;
	regs->pdex2wr = param->EmcPdEx2Wr;
	regs->pdex2rd = param->EmcPdEx2Rd;
	regs->pchg2pden = param->EmcPChg2Pden;
	regs->act2pden = param->EmcAct2Pden;
	regs->ar2pden = param->EmcAr2Pden;
	regs->rw2pden = param->EmcRw2Pden;
	regs->cke2pden = param->EmcCke2Pden;
	regs->pdex2cke = param->EmcPdex2Cke;
	regs->pdex2mrr = param->EmcPdex2Mrr;
	regs->txsr = param->EmcTxsr;
	regs->txsrdll = param->EmcTxsrDll;
	regs->tcke = param->EmcTcke;
	regs->tckesr = param->EmcTckesr;
	regs->tpd = param->EmcTpd;
	regs->tfaw = param->EmcTfaw;
	regs->trpab = param->EmcTrpab;
	regs->tclkstable = param->EmcTClkStable;
	regs->tclkstop = param->EmcTClkStop;
	regs->trefbw = param->EmcTRefBw;
	regs->odt_write = param->EmcOdtWrite;
	regs->cfg_dig_dll = param->EmcCfgDigDll;
	regs->cfg_dig_dll_period = param->EmcCfgDigDllPeriod;

	/* Don't write CFG_ADR_EN (bit 1) here - lock bit written later */
	regs->fbio_spare = param->EmcFbioSpare & ~CFG_ADR_EN_LOCKED;
	regs->cfg_rsv = param->EmcCfgRsv;
	regs->pmc_scratch1 = param->EmcPmcScratch1;
	regs->pmc_scratch2 = param->EmcPmcScratch2;
	regs->pmc_scratch3 = param->EmcPmcScratch3;
	regs->acpd_control = param->EmcAcpdControl;
	regs->txdsrvttgen = param->EmcTxdsrvttgen;

	/*
	 * Set pipe bypass enable bits before sending any DRAM commands.
	 * Note other bits in EMC_CFG must be set AFTER REFCTRL is configured.
	 */
	writebits(param->EmcCfg, &regs->cfg,
		  (EMC_CFG_EMC2PMACRO_CFG_BYPASS_ADDRPIPE_MASK |
		   EMC_CFG_EMC2PMACRO_CFG_BYPASS_DATAPIPE1_MASK |
		   EMC_CFG_EMC2PMACRO_CFG_BYPASS_DATAPIPE2_MASK));
}

static void sdram_patch_bootrom(const struct sdram_params *param,
				struct tegra_mc_regs *regs)
{
	if (param->BootRomPatchControl & BOOT_ROM_PATCH_CONTROL_ENABLE_MASK) {
		uintptr_t addr = ((param->BootRomPatchControl &
				  BOOT_ROM_PATCH_CONTROL_OFFSET_MASK) >>
				  BOOT_ROM_PATCH_CONTROL_OFFSET_SHIFT);
		addr = BOOT_ROM_PATCH_CONTROL_BASE_ADDRESS + (addr << 2);
		*(volatile uint32_t *)(addr) = param->BootRomPatchData;
		regs->timing_control = EMC_TIMING_CONTROL_TIMING_UPDATE;
	}
}

static void sdram_rel_dpd(const struct sdram_params *param,
			  struct tegra_pmc_regs *regs)
{
	u32 dpd3_val, dpd3_val_sel_dpd;

	/* Release SEL_DPD_CMD */
	dpd3_val = (param->EmcPmcScratch1 & 0x3FFFFFFF) | DPD_OFF;
	dpd3_val_sel_dpd = dpd3_val & 0xCFFF0000;
	regs->io_dpd3_req = dpd3_val_sel_dpd;
	usleep(param->PmcIoDpd3ReqWait);
}

/* Program DPD3/DPD4 regs (coldboot path) */
static void sdram_set_dpd(const struct sdram_params *param,
			  struct tegra_pmc_regs *regs)
{
	u32 dpd3_val, dpd3_val_sel_dpd;
	u32 dpd4_val, dpd4_val_e_dpd, dpd4_val_e_dpd_vttgen;

	/* Enable sel_dpd on unused pins */
	dpd3_val = (param->EmcPmcScratch1 & 0x3FFFFFFF) | DPD_ON;
	dpd3_val_sel_dpd = (dpd3_val ^ 0x0000FFFF) & 0xC000FFFF;
	regs->io_dpd3_req = dpd3_val_sel_dpd;
	usleep(param->PmcIoDpd3ReqWait);

	dpd4_val = dpd3_val;
	/* Disable e_dpd_vttgen */
	dpd4_val_e_dpd_vttgen = (dpd4_val ^ 0x3FFF0000) & 0xFFFF0000;
	regs->io_dpd4_req = dpd4_val_e_dpd_vttgen;
	usleep(param->PmcIoDpd4ReqWait);

	/* Disable e_dpd_bg */
	dpd4_val_e_dpd = (dpd4_val ^ 0x0000FFFF) & 0xC000FFFF;
	regs->io_dpd4_req = dpd4_val_e_dpd;
	usleep(param->PmcIoDpd4ReqWait);

	regs->weak_bias = 0;
	/* Add a wait to make sure clock switch takes place */
	usleep(1);
}

static void sdram_set_clock_enable_signal(const struct sdram_params *param,
					  struct tegra_emc_regs *regs)
{
	volatile uint32_t dummy = 0;
	uint32_t val = 0;

	if (param->MemoryType == NvBootMemoryType_LpDdr4) {

		val = (param->EmcPinGpioEn << EMC_PIN_GPIOEN_SHIFT) |
		      (param->EmcPinGpio << EMC_PIN_GPIO_SHIFT);
		regs->pin = val;

		regs->pin &= ~(EMC_PIN_RESET_MASK | EMC_PIN_DQM_MASK | EMC_PIN_CKE_MASK);
		/*
		 * Assert dummy read of PIN register to ensure above write goes
		 * through. Wait an additional 200us here as per NVIDIA.
		 */
		dummy |= regs->pin;
		usleep(param->EmcPinExtraWait + 200);

		/* Deassert reset */
		regs->pin |= EMC_PIN_RESET_INACTIVE;

		/*
		 * Assert dummy read of PIN register to ensure above write goes
		 * through. Wait an additional 2000us here as per NVIDIA.
		 */
		dummy |= regs->pin;
		usleep(param->EmcPinExtraWait + 2000);
	}

	/* Enable clock enable signal */
	regs->pin |= EMC_PIN_CKE_NORMAL;

	/* Dummy read of PIN register to ensure final write goes through */
	dummy |= regs->pin;
	usleep(param->EmcPinProgramWait);

	if (!dummy)
	{
		dbg_print("Failed to program EMC pin.\n!!!!");
		while (1) {}
	}

	if (param->MemoryType != NvBootMemoryType_LpDdr4) {

		/* Send NOP (trigger just needs to be non-zero) */
		writebits(((1 << EMC_NOP_CMD_SHIFT) |
			  (param->EmcDevSelect << EMC_NOP_DEV_SELECTN_SHIFT)),
			  &regs->nop,
			  EMC_NOP_CMD_MASK | EMC_NOP_DEV_SELECTN_MASK);
	}

	/* On coldboot w/LPDDR2/3, wait 200 uSec after asserting CKE high */
	if (param->MemoryType == NvBootMemoryType_LpDdr2)
		usleep(param->EmcPinExtraWait + 200);
}

static void sdram_init_lpddr3(const struct sdram_params *param,
			      struct tegra_emc_regs *regs)
{
	/* Precharge all banks. DEV_SELECTN = 0 => Select all devices */
	regs->pre = (param->EmcDevSelect << EMC_REF_DEV_SELECTN_SHIFT) | 1;

	/* Send Reset MRW command */
	regs->mrw = param->EmcMrwResetCommand;
	usleep(param->EmcMrwResetNInitWait);

	regs->mrw = param->EmcZcalInitDev0;
	usleep(param->EmcZcalInitWait);

	if ((param->EmcDevSelect & 2) == 0) {
		regs->mrw = param->EmcZcalInitDev1;
		usleep(param->EmcZcalInitWait);
	}

	/* Write mode registers */
	regs->mrw2 = param->EmcMrw2;
	regs->mrw = param->EmcMrw1;
	regs->mrw3 = param->EmcMrw3;
	regs->mrw4 = param->EmcMrw4;

	/* Patch 6 using BCT spare variables */
	sdram_patch(param->EmcBctSpare10, param->EmcBctSpare11);

	if (param->EmcExtraModeRegWriteEnable)
		regs->mrw =param->EmcMrwExtra;
}

static void sdram_init_lpddr4(const struct sdram_params *param,
			      struct tegra_emc_regs *regs)
{
	/* Patch 6 using BCT spare variables */
	sdram_patch(param->EmcBctSpare10, param->EmcBctSpare11);

	/* Write mode registers */
	regs->mrw2 = param->EmcMrw2;
	regs->mrw = param->EmcMrw1;
	regs->mrw3 = param->EmcMrw3;
	regs->mrw4 = param->EmcMrw4;
	regs->mrw6 = param->EmcMrw6;
	regs->mrw14 = param->EmcMrw14;

	regs->mrw8 = param->EmcMrw8;
	regs->mrw12 = param->EmcMrw12;
	regs->mrw9 = param->EmcMrw9;
	regs->mrw13 = param->EmcMrw13;

	/* Issue ZQCAL start, device 0 */
	regs->zq_cal = param->EmcZcalInitDev0;
	usleep(param->EmcZcalInitWait);
	/* Issue ZQCAL latch */
	regs->zq_cal = param->EmcZcalInitDev0 ^ 0x3;

	if ((param->EmcDevSelect & 2) == 0) {
		/* Same for device 1 */
		regs->zq_cal = param->EmcZcalInitDev1;
		usleep(param->EmcZcalInitWait);
		regs->zq_cal = param->EmcZcalInitDev1 ^ 0x3;
	}
}

static void sdram_init_zq_calibration(const struct sdram_params *param,
				      struct tegra_emc_regs *regs)
{
	if (param->MemoryType == NvBootMemoryType_LpDdr2)
		sdram_init_lpddr3(param, regs);
	else if (param->MemoryType == NvBootMemoryType_LpDdr4)
		sdram_init_lpddr4(param, regs);
}

static void sdram_set_zq_calibration(const struct sdram_params *param,
				     struct tegra_emc_regs *regs)
{
	if (param->EmcAutoCalInterval == 0)
		regs->auto_cal_config = param->EmcAutoCalConfig | AUTOCAL_MEASURE_STALL_ENABLE;

	regs->pmacro_brick_ctrl_rfu2 = param->EmcPmacroBrickCtrlRfu2;

	/* ZQ CAL setup (not actually issuing ZQ CAL now) */
	if (param->MemoryType == NvBootMemoryType_LpDdr4) {
		regs->zcal_wait_cnt = param->EmcZcalWaitCnt;
		regs->zcal_mrw_cmd = param->EmcZcalMrwCmd;
	}

	sdram_trigger_emc_timing_update(regs);
	usleep(param->EmcTimingControlWait);
}

static void sdram_set_refresh(const struct sdram_params *param,
			      struct tegra_emc_regs *regs)
{
	/* Insert burst refresh */
	if (param->EmcExtraRefreshNum > 0) {
		uint32_t refresh_num = (1 << param->EmcExtraRefreshNum) - 1;

		writebits((EMC_REF_CMD_REFRESH | EMC_REF_NORMAL_ENABLED |
			   (refresh_num << EMC_REF_NUM_SHIFT) |
			   (param->EmcDevSelect << EMC_REF_DEV_SELECTN_SHIFT)),
			  &regs->ref, (EMC_REF_CMD_MASK | EMC_REF_NORMAL_MASK |
				       EMC_REF_NUM_MASK |
				       EMC_REF_DEV_SELECTN_MASK));
	}

	/* Enable refresh */
	regs->refctrl = param->EmcDevSelect | EMC_REFCTRL_REF_VALID_ENABLED;

	/*
	 * NOTE: Programming CFG must happen after REFCTRL to delay
	 * active power-down to after init (DDR2 constraint).
	 */
	regs->dyn_self_ref_control = param->EmcDynSelfRefControl;
	regs->cfg_update = param->EmcCfgUpdate;
	regs->cfg = param->EmcCfg;
	regs->fdpd_ctrl_dq = param->EmcFdpdCtrlDq;
	regs->fdpd_ctrl_cmd = param->EmcFdpdCtrlCmd;
	regs->sel_dpd_ctrl = param->EmcSelDpdCtrl;

	/* Write addr swizzle lock bit */
	regs->fbio_spare = param->EmcFbioSpare | CFG_ADR_EN_LOCKED;

	/* Re-trigger timing to latch power saving functions */
	sdram_trigger_emc_timing_update(regs);

	/* Enable EMC pipe clock gating */
	regs->cfg_pipe_clk = param->EmcCfgPipeClk;
	/* Depending on freqency, enable CMD/CLK fdpd */
	regs->fdpd_ctrl_cmd_no_ramp = param->EmcFdpdCtrlCmdNoRamp;
}

#define AHB_ARB_XBAR_CTRL	0x6000C0E0
static void sdram_enable_arbiter(const struct sdram_params *param)
{
	
	/* TODO(hungte) Move values here to standalone header file. */
	volatile uint32_t *ahb_arbitration_xbar_ctrl = (void*)(AHB_ARB_XBAR_CTRL);

	*ahb_arbitration_xbar_ctrl |= param->AhbArbitrationXbarCtrlMemInitDone << 16;

}

static void sdram_lock_carveouts(const struct sdram_params *param,
				 struct tegra_mc_regs *regs)
{
	/* Lock carveouts, and emem_cfg registers */
	regs->video_protect_reg_ctrl = param->McVideoProtectWriteAccess;
	regs->sec_carveout_reg_ctrl = param->McSecCarveoutProtectWriteAccess;
	regs->mts_carveout_reg_ctrl = param->McMtsCarveoutRegCtrl;

	/* Write this last, locks access */
	regs->emem_cfg_access_ctrl = MC_EMEM_CFG_ACCESS_CTRL_WRITE_ACCESS_DISABLED;
}


//static void _sdram_config(const struct sdram_params *param)
void _sdram_config(const struct sdram_params *param)
{
	struct tegra_pmc_regs *pmc = (struct tegra_pmc_regs *)PMC_BASE;
	struct tegra_mc_regs *mc = (struct tegra_mc_regs *)MC_BASE;
	struct tegra_emc_regs *emc = (struct tegra_emc_regs *)EMC_BASE;

	if (param->MemoryType != NvBootMemoryType_LpDdr4 &&
	    param->MemoryType != NvBootMemoryType_LpDdr2)
	{
		dbg_print("Invalid memory type!!!\n");
		while (1) {}
	}

	sdram_configure_pmc(param, pmc);
	sdram_patch(param->EmcBctSpare0, param->EmcBctSpare1);

	sdram_set_dpd(param, pmc);
	sdram_start_clocks(param, emc);
	sdram_set_pad_macros(param, emc);
	sdram_patch(param->EmcBctSpare4, param->EmcBctSpare5);

	sdram_trigger_emc_timing_update(emc);
	sdram_init_mc(param, mc);
	sdram_init_emc(param, emc);
	sdram_patch(param->EmcBctSpare8, param->EmcBctSpare9);

	sdram_set_emc_timing(param, emc);
	sdram_patch_bootrom(param, mc);
	sdram_rel_dpd(param, pmc);
	sdram_set_zq_calibration(param, emc);
	sdram_set_ddr_control(param, pmc);
	sdram_set_clock_enable_signal(param, emc);

	sdram_init_zq_calibration(param, emc);

	/* Set package and DPD pad control */
	pmc->ddr_cfg = param->PmcDdrCfg;

	/* Start periodic ZQ calibration (LPDDRx only) */
	if (param->MemoryType == NvBootMemoryType_LpDdr4 ||
	    param->MemoryType == NvBootMemoryType_LpDdr2) {
		emc->zcal_interval = param->EmcZcalInterval;
		emc->zcal_wait_cnt = param->EmcZcalWaitCnt;
		emc->zcal_mrw_cmd = param->EmcZcalMrwCmd;
	}
	sdram_patch(param->EmcBctSpare12, param->EmcBctSpare13);	

	sdram_trigger_emc_timing_update(emc);
	sdram_set_refresh(param, emc);

	sdram_enable_arbiter(param);	
	sdram_lock_carveouts(param, mc);
}

u32 get_sdram_id()
{
	return (fuse_read_odm(4) & 0x38) >> 3;
}

#ifdef GET_SDRAM_FROM_BOOT0
#include "mc.h"
#include "sdmmc.h"
#include "sdmmc_driver.h"
#include "lib/crc32.h"
#else
#include "lib/decomp.h"
#include "sdram_lz4.inl"
#endif

const struct sdram_params* sdram_get_params()
{
	void* retVal = NULL;
	u32 board_sdram_id = get_sdram_id(); //TODO: sdram_id should be in [0,4].
	if (board_sdram_id >= 4)
	{
		dbg_print("ERROR: fuse-read board_sdram_id is %u (valid values 0..4)\n", board_sdram_id);
		return retVal;
	}
	u8* IRAM_BCT_LOCATION = (void*)0x40030000;
	u8* iram_sdram_params_ptr = &IRAM_BCT_LOCATION[0x58C];

#ifdef GET_SDRAM_FROM_BOOT0
	sdmmc_storage_t mmcPart;
    sdmmc_t mmcDev;

#ifndef CONFIG_ENABLE_AHB_REDIRECT //otherwise already enabled
    mc_enable_ahb_redirect();
#endif
    if (!sdmmc_storage_init_mmc(&mmcPart, &mmcDev, SDMMC_4, SDMMC_BUS_WIDTH_8, 4))
	{
        dbg_print("ERROR initializing eMMC for BCT read!\n");
		goto ending_redirect;
	}
	if (!sdmmc_storage_set_mmc_partition(&mmcPart, 1))
	{
		dbg_print("ERROR switching eMMC to BOOT0 for BCT read!\n");
		goto ending_sdmmc;
	}

	if (!sdmmc_storage_read(&mmcPart, 0, 0x2800/512, IRAM_BCT_LOCATION))
	{
		dbg_print("ERROR reading BCT from eMMC BOOT0!\n");
		goto ending_sdmmc;
	}

	static const u32 BOOTDATA_VERSION_T210 = 0x00210001;
	u32* boot_data_version_ptr = &IRAM_BCT_LOCATION[0x530];
	if (*boot_data_version_ptr != BOOTDATA_VERSION_T210)
	{
		dbg_print("ERROR: boot_data_version in BCT not correct! (expecting 0x%08X, read 0x%08X)\n", BOOTDATA_VERSION_T210, *boot_data_version_ptr);
		goto ending_sdmmc;
	}
	
	retVal = &iram_sdram_params_ptr[board_sdram_id*0x768];
	dbg_print("GOT sdram params for device %u (crc32: 0x%08x)\n", board_sdram_id, crc32b((u8*)retVal, 0x768));
ending_sdmmc:
	sdmmc_storage_end(&mmcPart);
ending_redirect:
#ifndef CONFIG_ENABLE_AHB_REDIRECT //otherwise already enabled
    mc_disable_ahb_redirect();
#endif
#else
	size_t compResult = ulz4fn(_dram_cfg_lz4, sizeof(_dram_cfg_lz4), iram_sdram_params_ptr, 0x774*4);
	if (compResult != 0)
		retVal = &iram_sdram_params_ptr[board_sdram_id*0x774];
#endif
	return retVal;
}

void sdram_init(const struct sdram_params* param)
{
	const sdram_params_t *params = (const sdram_params_t *)param;
	/*
	max77620_send_byte(MAX77620_REG_SD_CFG2, 0x05);
	max77620_regulator_set_voltage(REGULATOR_SD1, 1100000); //1.1V

	
	*/
	// Set DRAM voltage.
	i2c_send_byte(I2C_5, MAX77620_I2C_ADDR, MAX77620_REG_SD_CFG2, 0x05);
	max77620_regulator_set_voltage(REGULATOR_SD1, 1100000);

	// VDDP Select.
	PMC(APBDEV_PMC_VDDP_SEL) = params->PmcVddpSel;
	usleep(params->PmcVddpSelWait);

	// Set DDR pad voltage.
	PMC(APBDEV_PMC_DDR_PWR) = PMC(APBDEV_PMC_DDR_PWR);

	// Turn on MEM IO Power.
	PMC(APBDEV_PMC_NO_IOPOWER) = params->PmcRegShort;
	PMC(APBDEV_PMC_REG_SHORT) = params->PmcNoIoPower;

	PMC(APBDEV_PMC_DDR_CNTRL) = params->PmcDdrCntrlWait;

	// Patch 1 using BCT spare variables
	if (params->EmcBctSpare0)
		*(vu32 *)params->EmcBctSpare0 = params->EmcBctSpare1;

	_sdram_config(params);	
}
