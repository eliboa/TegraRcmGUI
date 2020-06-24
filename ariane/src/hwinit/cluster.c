/*
* Copyright (c) 2018 naehrwert
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

#include "cluster.h"
#include "clock.h"
#include "timer.h"
#include "pmc.h"
#include "flow.h"
#include "t210.h"
#include "i2c.h"
#include "max7762x.h"
#include "max77620.h"

void _cluster_enable_power()
{
	u8 tmp = max77620_recv_byte(MAX77620_REG_AME_GPIO);
	max77620_send_byte(MAX77620_REG_AME_GPIO, tmp & 0xDF);
	max77620_send_byte(MAX77620_REG_GPIO5, MAX77620_CNFG_GPIO_DRV_PUSHPULL | MAX77620_CNFG_GPIO_OUTPUT_VAL_HIGH);

	// Enable cores power.
	max7762x_send_byte(MAX77621_CPU_I2C_ADDR, MAX77621_CONTROL1_REG, 
						MAX77621_AD_ENABLE | MAX77621_NFSR_ENABLE | MAX77621_SNS_ENABLE); // 1-3.x: MAX77621_NFSR_ENABLE
	max7762x_send_byte(MAX77621_CPU_I2C_ADDR, MAX77621_CONTROL2_REG, 
						MAX77621_T_JUNCTION_120 | MAX77621_WDTMR_ENABLE | MAX77621_CKKADV_TRIP_75mV_PER_US| MAX77621_INDUCTOR_NOMINAL); 
						// 1-3.x: MAX77621_T_JUNCTION_120 | MAX77621_CKKADV_TRIP_DISABLE | MAX77621_INDUCTOR_NOMINAL
	max7762x_send_byte(MAX77621_CPU_I2C_ADDR, MAX77621_VOUT_REG, MAX77621_VOUT_ENABLE | 0x37);
	max7762x_send_byte(MAX77621_CPU_I2C_ADDR, MAX77621_VOUT_DVC_REG, MAX77621_VOUT_ENABLE | 0x37);
}

int _cluster_pmc_enable_partition(u32 part, u32 toggle, u32 enable)
{
	//Check if the partition has already been turned on.
	if (enable && (PMC(APBDEV_PMC_PWRGATE_STATUS) & part))
		return 1;

	u32 startTime = get_tmr_ms();
	while (PMC(APBDEV_PMC_PWRGATE_TOGGLE) & 0x100)
	{
		if (get_tmr_ms() - startTime >= 5) //only wait for 5ms
			return 0;
	}

	PMC(APBDEV_PMC_PWRGATE_TOGGLE) = toggle | (enable ? 0x100 : 0);

	startTime = get_tmr_ms();
	while ((PMC(APBDEV_PMC_PWRGATE_STATUS) & part) == 0)
	{
		if (get_tmr_ms() - startTime >= 5) //only wait for 5ms
			return 0;
	}

	return 1;
}

#define PLLX_VALUE_FROM_DIVS(DIVM, DIVN, DIVP) (((DIVP & 0x1F) << 20) | ((DIVN & 0xFF) << 8) | (DIVM & 0xFF))

void cluster_boot_cpu0(u32 entry)
{
	struct flow_ctlr* const flow = (void *)FLOW_CTLR_BASE;	
	flow->bpmp_cluster_control &= ~(1u << 0); //Set ACTIVE_CLUSER to FAST.

	_cluster_enable_power();

	//final_freq = ((38.4MHz / DIVM) * DIVN) / (2^DIVP)
	typedef struct freqEntry_s { u32 clkFreqHz; u32 pllxDividers; } freqEntry_t;
	static const freqEntry_t frequencies[] =
	{
		{ 93600000,		PLLX_VALUE_FROM_DIVS(2, 78, 4) 	},	//93.6MHz	(bench: 6677351 us)
		{ 187200000,	PLLX_VALUE_FROM_DIVS(2, 78, 3) 	},	//187.2MHz	(bench: 5341881 us)
		{ 249600000,	PLLX_VALUE_FROM_DIVS(3, 156, 3)	},	//249.6MHz	(bench: 4006410 us)
		{ 499200000,	PLLX_VALUE_FROM_DIVS(3, 156, 2)	},	//499.2MHz	(bench: 3004808 us)
		{ 748800000,	PLLX_VALUE_FROM_DIVS(2, 78, 1) 	},	//748.8MHz	(bench: 2670940 us)
		{ 998400000,	PLLX_VALUE_FROM_DIVS(3, 156, 1)	}	//998.4MHz	(bench: 2003206 us)
	};

	static const u32 pllxDividers = frequencies[0].pllxDividers;
	if (!(CLOCK(CLK_RST_CONTROLLER_PLLX_BASE) & (1u << 30)))
	{
		CLOCK(CLK_RST_CONTROLLER_PLLX_MISC_3) &= ~(1u << 3);
		usleep(2);
		CLOCK(CLK_RST_CONTROLLER_PLLX_BASE) = (1u << 31) | pllxDividers;
		CLOCK(CLK_RST_CONTROLLER_PLLX_BASE) = 0x00000000 | pllxDividers;
		CLOCK(CLK_RST_CONTROLLER_PLLX_MISC) |= (1u << 18);
		CLOCK(CLK_RST_CONTROLLER_PLLX_BASE) = (1u << 30) | pllxDividers;
	}
	while (!(CLOCK(CLK_RST_CONTROLLER_PLLX_BASE) & 0x8000000)) {}

	//Configure MSELECT source and enable clock.
	CLOCK(CLK_RST_CONTROLLER_CLK_SOURCE_MSELECT) = (CLOCK(CLK_RST_CONTROLLER_CLK_SOURCE_MSELECT) & 0x1FFFFF00) | 6;
	CLOCK(CLK_RST_CONTROLLER_CLK_OUT_ENB_V) |= CLK_V_MSELECT;

	//Configure initial CPU clock frequency and enable clock.
	CLOCK(CLK_RST_CONTROLLER_CCLK_BURST_POLICY) = 0x20008888;
	CLOCK(CLK_RST_CONTROLLER_SUPER_CCLK_DIVIDER) = 0x80000000;
	CLOCK(CLK_RST_CONTROLLER_CLK_ENB_V_SET) = CLK_V_CPUG;

	clock_enable_coresight();

	//CAR2PMC_CPU_ACK_WIDTH should be set to 0.
	CLOCK(CLK_RST_CONTROLLER_CPU_SOFTRST_CTRL2) &= 0xFFFFF000;

	//Enable CPU rail.
	_cluster_pmc_enable_partition(1, 0, 1);
	//Enable cluster 0 non-CPU.
	_cluster_pmc_enable_partition(0x8000, 15, 1);
	//Enable CE0.
	_cluster_pmc_enable_partition(0x4000, 14, 1);

	//Request and wait for RAM repair.
	flow->ram_repair = 1;
	while (!(flow->ram_repair & 2)) {}

	EXCP_VEC(0x100) = 0;
	
	//Keep bootrom accessible after cluster boot
	SB(SB_PIROM_START) = 96*1024;	
	//Set reset vector.
	SB(SB_AA64_RESET_LOW) = entry | 1;
	SB(SB_AA64_RESET_HIGH) = 0;
	//Non-secure reset vector write disable.
	SB(SB_CSR) = 2;
	(void)SB(SB_CSR);

	//Clear MSELECT reset.
	CLOCK(CLK_RST_CONTROLLER_RST_DEVICES_V) &= ~CLK_V_MSELECT;
	//Clear NONCPU reset.
	CLOCK(CLK_RST_CONTROLLER_RST_CPUG_CMPLX_CLR) = 0x20000000;
	//Clear CPU{0,1,2,3} POR and CORE, CX0, L2, and DBG reset.
	CLOCK(CLK_RST_CONTROLLER_RST_CPUG_CMPLX_CLR) = 0x411F000F;
}
