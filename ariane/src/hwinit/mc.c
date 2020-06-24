#include "mc.h"
#include "t210.h"
#include "mc_t210.h"
#include "clock.h"
#include "timer.h"

void mc_config_tsec_carveout(u32 bom, u32 size1mb, int lock)
{
	struct tegra_mc_regs* const mc = (void *)MC_BASE;

	mc->sec_carveout_bom = bom;
	mc->sec_carveout_size_mb = size1mb;
	if (lock)
		mc->sec_carveout_reg_ctrl |= 1;
}

void mc_config_carveout()
{
	struct tegra_mc_regs * const mc = (void *)MC_BASE;

	*(vu32 *)0x8005FFFC = 0xC0EDBBCC;
	mc->video_protect_gpu_override_0 = 1;
	mc->video_protect_gpu_override_1 = 0;
	mc->video_protect_bom = 0;
	mc->video_protect_size_mb = 0;
	mc->video_protect_reg_ctrl = 1;

	//Configure TSEC carveout @ 0x90000000, 1MB.
	//mc_config_tsec_carveout(0x90000000, 1, 0);
	mc_config_tsec_carveout(0, 0, 1);

	mc->mts_carveout_bom = 0;
	mc->mts_carveout_size_mb = 0;
	mc->mts_carveout_adr_hi = 0;
	mc->mts_carveout_reg_ctrl = 1;
	mc->security_carveout1_bom = 0;
	mc->security_carveout1_bom_hi = 0;
	mc->security_carveout1_size_128kb = 0;
	mc->security_carveout1_ca0 = 0;
	mc->security_carveout1_ca1 = 0;
	mc->security_carveout1_ca2 = 0;
	mc->security_carveout1_ca3 = 0;
	mc->security_carveout1_ca4 = 0;
	mc->security_carveout1_cfia0 = 0;
	mc->security_carveout1_cfia1 = 0;
	mc->security_carveout1_cfia2 = 0;
	mc->security_carveout1_cfia3 = 0;
	mc->security_carveout1_cfia4 = 0;
	mc->security_carveout1_cfg0 = 0x4000006;
	mc->security_carveout2_bom = 0x80020000;
	mc->security_carveout2_bom_hi = 0;
	mc->security_carveout2_size_128kb = 2;
	mc->security_carveout2_ca0 = 0;
	mc->security_carveout2_ca1 = 0;
	mc->security_carveout2_ca2 = 0x3000000;
	mc->security_carveout2_ca3 = 0;
	mc->security_carveout2_ca4 = 0x300;
	mc->security_carveout2_cfia0 = 0;
	mc->security_carveout2_cfia1 = 0;
	mc->security_carveout2_cfia2 = 0;
	mc->security_carveout2_cfia3 = 0;
	mc->security_carveout2_cfia4 = 0;
	mc->security_carveout2_cfg0 = 0x440167e;
	mc->security_carveout3_bom = 0;
	mc->security_carveout3_bom_hi = 0;
	mc->security_carveout3_size_128kb = 0;
	mc->security_carveout3_ca0 = 0;
	mc->security_carveout3_ca1 = 0;
	mc->security_carveout3_ca2 = 0x3000000;
	mc->security_carveout3_ca3 = 0;
	mc->security_carveout3_ca4 = 0x300;
	mc->security_carveout3_cfia0 = 0;
	mc->security_carveout3_cfia1 = 0;
	mc->security_carveout3_cfia2 = 0;
	mc->security_carveout3_cfia3 = 0;
	mc->security_carveout3_cfia4 = 0;
	mc->security_carveout3_cfg0 = 0x4401e7e;
	mc->security_carveout4_bom = 0;
	mc->security_carveout4_bom_hi = 0;
	mc->security_carveout4_size_128kb = 0;
	mc->security_carveout4_ca0 = 0;
	mc->security_carveout4_ca1 = 0;
	mc->security_carveout4_ca2 = 0;
	mc->security_carveout4_ca3 = 0;
	mc->security_carveout4_ca4 = 0;
	mc->security_carveout4_cfia0 = 0;
	mc->security_carveout4_cfia1 = 0;
	mc->security_carveout4_cfia2 = 0;
	mc->security_carveout4_cfia3 = 0;
	mc->security_carveout4_cfia4 = 0;
	mc->security_carveout4_cfg0 = 0x8f;
	mc->security_carveout5_bom = 0;
	mc->security_carveout5_bom_hi = 0;
	mc->security_carveout5_size_128kb = 0;
	mc->security_carveout5_ca0 = 0;
	mc->security_carveout5_ca1 = 0;
	mc->security_carveout5_ca2 = 0;
	mc->security_carveout5_ca3 = 0;
	mc->security_carveout5_ca4 = 0;
	mc->security_carveout5_cfia0 = 0;
	mc->security_carveout5_cfia1 = 0;
	mc->security_carveout5_cfia2 = 0;
	mc->security_carveout5_cfia3 = 0;
	mc->security_carveout5_cfia4 = 0;
	mc->security_carveout5_cfg0 = 0x8f;
}

static const u32 ARC_CLK_OVR_ON = 1u << 19;

void mc_enable_ahb_redirect()
{
	struct tegra_mc_regs* const mc = (void *)MC_BASE;

	CLOCK(CLK_RST_CONTROLLER_LVL2_CLK_GATE_OVRD) |= ARC_CLK_OVR_ON;
	
	//mc->iram_reg_ctrl &= ~(1);
	mc->iram_bom = 0x40000000;
	mc->iram_tom = 0x4003F000;
}

void mc_disable_ahb_redirect()
{
	struct tegra_mc_regs* const mc = (void *)MC_BASE;

	mc->iram_bom = 0xFFFFF000;
	mc->iram_tom = 0;
	//mc->iram_reg_ctrl |= 1; //Disable IRAM_CFG_WRITE_ACCESS (sticky).
	
	CLOCK(CLK_RST_CONTROLLER_LVL2_CLK_GATE_OVRD) &= ~ARC_CLK_OVR_ON;
}

void mc_enable()
{
	CLOCK(CLK_RST_CONTROLLER_CLK_SOURCE_EMC) = (CLOCK(CLK_RST_CONTROLLER_CLK_SOURCE_EMC) & 0x1FFFFFFF) | 0x40000000;
	//Enable EMC clock.
	CLOCK(CLK_RST_CONTROLLER_CLK_ENB_H_SET) |= CLK_H_EMC;
	//Enable MEM clock.
	CLOCK(CLK_RST_CONTROLLER_CLK_ENB_H_SET) |= CLK_H_MEM;
	//Enable EMC DLL clock.
	CLOCK(CLK_RST_CONTROLLER_CLK_ENB_X_SET) |= CLK_X_EMC_DLL;
	//Clear EMC and MEM reset.
	CLOCK(CLK_RST_CONTROLLER_RST_DEV_H_SET) = CLK_H_MEM | CLK_H_EMC; 
	usleep(5);

	mc_disable_ahb_redirect();
	#ifdef CONFIG_ENABLE_AHB_REDIRECT
	mc_enable_ahb_redirect();
	#endif
}
