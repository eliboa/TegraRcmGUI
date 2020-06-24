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

#include "clock.h"
#include "uart.h"
#include "i2c.h"
#include "sdram.h"
#include "di.h"
#include "mc.h"
#include "t210.h"
#include "pmc.h"
#include "gpio.h"
#include "pinmux.h"
#include "max77620.h"
#include "max7762x.h"
#include "fuse.h"
#include "timer.h"
#include "carveout.h"
#include <string.h>

void config_oscillators()
{
	CLOCK(CLK_RST_CONTROLLER_SPARE_REG0) = (CLOCK(CLK_RST_CONTROLLER_SPARE_REG0) & 0xFFFFFFF3) | 4;
	SYSCTR0(SYSCTR0_CNTFID0) = 19200000;
	TMR(TMR_US_CFG_OFFS) = 0x45F;
	CLOCK(CLK_RST_CONTROLLER_OSC_CTRL) = 0x50000071;
	PMC(APBDEV_PMC_OSC_EDPD_OVER) = (PMC(APBDEV_PMC_OSC_EDPD_OVER) & 0xFFFFFF81) | 0xE;
	PMC(APBDEV_PMC_OSC_EDPD_OVER) |= 0x400000;
	PMC(APBDEV_PMC_CNTRL2) |= 0x1000;
	PMC(APBDEV_PMC_SCRATCH188) = (PMC(APBDEV_PMC_SCRATCH188) & 0xFCFFFFFF) | 0x2000000;
	CLOCK(CLK_RST_CONTROLLER_CLK_SYSTEM_RATE) = 0x10;
	CLOCK(CLK_RST_CONTROLLER_PLLMB_BASE) &= 0xBFFFFFFF;
	PMC(APBDEV_PMC_TSC_MULT) = (PMC(APBDEV_PMC_TSC_MULT) & 0xFFFF0000) | 0x249F; //0x249F = 19200000 * (16 / 32.768 kHz)
	CLOCK(CLK_RST_CONTROLLER_SCLK_BURST_POLICY) = 0x20004444;
	CLOCK(CLK_RST_CONTROLLER_SUPER_SCLK_DIVIDER) = 0x80000000;
	CLOCK(CLK_RST_CONTROLLER_CLK_SYSTEM_RATE) = 2;
}

void config_gpios()
{
	pinmux_set_config(PINMUX_UART2_TX_INDEX, 0);
	pinmux_set_config(PINMUX_UART3_TX_INDEX, 0);
	pinmux_set_config(PINMUX_GPIO_PE6_INDEX, PINMUX_INPUT_ENABLE);
	pinmux_set_config(PINMUX_GPIO_PH6_INDEX, PINMUX_INPUT_ENABLE);

	gpio_config(GPIO_BY_NAME(UART2_TX), GPIO_MODE_GPIO);
	gpio_config(GPIO_BY_NAME(UART3_TX), GPIO_MODE_GPIO);
	gpio_config(GPIO_DECOMPOSE(GPIO_E6_INDEX), GPIO_MODE_GPIO);
	gpio_config(GPIO_DECOMPOSE(GPIO_H6_INDEX), GPIO_MODE_GPIO);
	gpio_output_enable(GPIO_BY_NAME(UART2_TX), GPIO_OUTPUT_DISABLE);
	gpio_output_enable(GPIO_BY_NAME(UART3_TX), GPIO_OUTPUT_DISABLE);
	gpio_output_enable(GPIO_DECOMPOSE(GPIO_E6_INDEX), GPIO_OUTPUT_DISABLE);
	gpio_output_enable(GPIO_DECOMPOSE(GPIO_H6_INDEX), GPIO_OUTPUT_DISABLE);

	pinmux_config_i2c(I2C_1);
	pinmux_config_i2c(I2C_5);

	//unused UART_A
	PINMUX_SET_UNUSED_BY_NAME(UART1_RX);
	PINMUX_SET_UNUSED_BY_NAME(UART1_TX);
	PINMUX_SET_UNUSED_BY_NAME(UART1_CTS);
	PINMUX_SET_UNUSED_BY_NAME(UART1_RTS);

	//Configure volume up/down as inputs.
	gpio_config(GPIO_BY_NAME(BUTTON_VOL_UP), GPIO_MODE_GPIO);
	gpio_config(GPIO_BY_NAME(BUTTON_VOL_DOWN), GPIO_MODE_GPIO);
	gpio_output_enable(GPIO_BY_NAME(BUTTON_VOL_UP), GPIO_OUTPUT_DISABLE);
	gpio_output_enable(GPIO_BY_NAME(BUTTON_VOL_DOWN), GPIO_OUTPUT_DISABLE);

	//Configure SD card detect pin
	pinmux_set_config(PINMUX_GPIO_Z1, PINMUX_INPUT_ENABLE | PINMUX_PULL_UP | PINMUX_GPIO_PZ1_FUNC_SDMMC1);
	gpio_config(GPIO_DECOMPOSE(GPIO_Z1_INDEX), GPIO_MODE_GPIO);
	gpio_output_enable(GPIO_DECOMPOSE(GPIO_Z1_INDEX), GPIO_OUTPUT_DISABLE);
	APB_MISC(APB_MISC_GP_VGPIO_GPIO_MUX_SEL) = 0; //use GPIO for all SDMMC 

	//Configure SD power enable pin (powered off by default)
	pinmux_set_config(PINMUX_DMIC3_CLK_INDEX, PINMUX_INPUT_ENABLE | PINMUX_PULL_DOWN | PINMUX_DMIC3_CLK_FUNC_I2S5A); //not sure about the altfunc here
	gpio_config(GPIO_BY_NAME(DMIC3_CLK), GPIO_MODE_GPIO);
	gpio_write(GPIO_BY_NAME(DMIC3_CLK), GPIO_LOW);
	gpio_output_enable(GPIO_BY_NAME(DMIC3_CLK), GPIO_OUTPUT_ENABLE);
}

void config_pmc_scratch()
{
	PMC(APBDEV_PMC_SCRATCH20) &= 0xFFF3FFFF;
	PMC(APBDEV_PMC_SCRATCH190) &= 0xFFFFFFFE;
	PMC(APBDEV_PMC_SECURE_SCRATCH21) |= 0x10;
}

void mbist_workaround()
{
	CLOCK(CLK_RST_CONTROLLER_CLK_SOURCE_SOR1) = (CLOCK(CLK_RST_CONTROLLER_CLK_SOURCE_SOR1) | 0x8000) & 0xFFFFBFFF;
	CLOCK(CLK_RST_CONTROLLER_PLLD_BASE) |= 0x40800000u;
	CLOCK(CLK_RST_CONTROLLER_RST_DEV_Y_CLR) = CLK_Y_APE;
	CLOCK(CLK_RST_CONTROLLER_RST_DEV_X_CLR) = CLK_X_VIC;
	CLOCK(CLK_RST_CONTROLLER_RST_DEV_L_CLR) = CLK_L_DISP1 | CLK_L_HOST1X;
	usleep(2);

	I2S(0x0A0) |= 0x400;
	I2S(0x088) &= 0xFFFFFFFE;
	I2S(0x1A0) |= 0x400;
	I2S(0x188) &= 0xFFFFFFFE;
	I2S(0x2A0) |= 0x400;
	I2S(0x288) &= 0xFFFFFFFE;
	I2S(0x3A0) |= 0x400;
	I2S(0x388) &= 0xFFFFFFFE;
	I2S(0x4A0) |= 0x400;
	I2S(0x488) &= 0xFFFFFFFE;
	DISPLAY_A(_DIREG(DC_COM_DSC_TOP_CTL)) |= 4;
	VIC(0x8C) = 0xFFFFFFFF;
	usleep(2);

	CLOCK(CLK_RST_CONTROLLER_RST_DEV_Y_SET) = CLK_Y_APE;
	CLOCK(CLK_RST_CONTROLLER_RST_DEV_L_SET) = CLK_L_DISP1 | CLK_L_HOST1X;
	CLOCK(CLK_RST_CONTROLLER_RST_DEV_X_SET) = CLK_X_VIC;
	CLOCK(CLK_RST_CONTROLLER_CLK_OUT_ENB_H) = CLK_H_PMC | CLK_H_FUSE;
	CLOCK(CLK_RST_CONTROLLER_CLK_OUT_ENB_L) = CLK_L_RTC | CLK_L_TMR | CLK_L_GPIO | CLK_L_USBD | CLK_L_CACHE2;
	CLOCK(CLK_RST_CONTROLLER_CLK_OUT_ENB_U) = CLK_U_CSITE | CLK_U_IRAMA | CLK_U_IRAMB | CLK_U_IRAMC | CLK_U_IRAMD | CLK_U_CRAM2;
	CLOCK(CLK_RST_CONTROLLER_CLK_OUT_ENB_V) = CLK_V_MSELECT | CLK_V_APB2APE | CLK_V_SPDIF_DOUBLER | 0x80000000;
	CLOCK(CLK_RST_CONTROLLER_CLK_OUT_ENB_W) = CLK_W_PCIERX0 | CLK_W_PCIERX1 | CLK_W_PCIERX2 | CLK_W_PCIERX3 | CLK_W_PCIERX4 | CLK_W_PCIERX5 | CLK_W_ENTROPY | CLK_W_MC1;
	CLOCK(CLK_RST_CONTROLLER_CLK_OUT_ENB_X) = CLK_X_MC_CAPA | CLK_X_MC_CBPA | CLK_X_MC_CPU | CLK_X_MC_BBC | CLK_X_GPU | CLK_X_DPGAPB | CLK_X_PLLG_REF;
	CLOCK(CLK_RST_CONTROLLER_CLK_OUT_ENB_Y) = CLK_Y_MC_CDPA | CLK_Y_MC_CCPA;
	CLOCK(CLK_RST_CONTROLLER_LVL2_CLK_GATE_OVRA) = 0;
	CLOCK(CLK_RST_CONTROLLER_LVL2_CLK_GATE_OVRB) = 0;
	CLOCK(CLK_RST_CONTROLLER_LVL2_CLK_GATE_OVRC) = 0;
	CLOCK(CLK_RST_CONTROLLER_LVL2_CLK_GATE_OVRD) = 0;
	CLOCK(CLK_RST_CONTROLLER_LVL2_CLK_GATE_OVRE) = 0;
	CLOCK(CLK_RST_CONTROLLER_PLLD_BASE) &= 0x1F7FFFFF;
	CLOCK(CLK_RST_CONTROLLER_CLK_SOURCE_SOR1) &= 0xFFFF3FFF;
	CLOCK(CLK_RST_CONTROLLER_CLK_SOURCE_VI) = (CLOCK(CLK_RST_CONTROLLER_CLK_SOURCE_VI) & 0x1FFFFFFF) | 0x80000000;
	CLOCK(CLK_RST_CONTROLLER_CLK_SOURCE_HOST1X) = (CLOCK(CLK_RST_CONTROLLER_CLK_SOURCE_HOST1X) & 0x1FFFFFFF) | 0x80000000;
	CLOCK(CLK_RST_CONTROLLER_CLK_SOURCE_NVENC) = (CLOCK(CLK_RST_CONTROLLER_CLK_SOURCE_NVENC) & 0x1FFFFFFF) | 0x80000000;
}

void config_hw()
{
	clock_enable_fuse(1);


	// This memset needs to happen here, else TZRAM will behave weirdly later on.
	memset((void *)TZRAM_BASE, 0, 0x10000);
	PMC(APBDEV_PMC_CRYPTO_OP) = 0;
	SE(0x010) = 0x1F;

	// Clear the boot reason to avoid problems later
	PMC(APBDEV_PMC_SCRATCH200) = 0x0;
	PMC(0x1B4) = 0x0;
	APB_MISC(0x08) = (APB_MISC(0x08) & 0xF0) | (7 << 10);

	SYSREG(AHB_AHB_SPARE_REG) &= 0xFFFFFF9F; // Unset APB2JTAG_OVERRIDE_EN and OBS_OVERRIDE_EN.
	PMC(APBDEV_PMC_SCRATCH49) = PMC(APBDEV_PMC_SCRATCH49) & 0xFFFFFFFC;
	

	mbist_workaround();
	clock_enable_se();

	//Enable fuse clock.
	clock_enable_fuse(1);
	//Disable fuse programming.
	fuse_disable_program();

	mc_enable();

	config_oscillators();
	APB_MISC(APB_MISC_PP_PINMUX_GLOBAL) = 0;
	config_gpios();
	

#ifdef DEBUG_UART_PORT
	clock_enable_uart(DEBUG_UART_PORT);
	uart_init(DEBUG_UART_PORT, 115200);
	pinmux_config_uart(DEBUG_UART_PORT);
#endif

	clock_enable_cl_dvfs();

	clock_enable_i2c(I2C_1);
	clock_enable_i2c(I2C_5);

	// Clock TZRAM
	clock_enable_se();
	static const clock_t clock_unk = { CLK_RST_CONTROLLER_RST_DEVICES_V, CLK_RST_CONTROLLER_CLK_OUT_ENB_V, 0, 30, 0, 0 };
	clock_enable(&clock_unk);

	i2c_init(I2C_1);
	i2c_init(I2C_5);

	max77620_send_byte(MAX77620_REG_CNFGBBC, 0x40);
	max77620_send_byte(MAX77620_REG_ONOFFCNFG1, 0x78);

	max77620_send_byte(MAX77620_REG_FPS_CFG0, 0x38);
	max77620_send_byte(MAX77620_REG_FPS_CFG1, 0x3A);
	max77620_send_byte(MAX77620_REG_FPS_CFG2, 0x38);
	max77620_regulator_config_fps(REGULATOR_LDO4);
	max77620_regulator_config_fps(REGULATOR_LDO8);
	max77620_regulator_config_fps(REGULATOR_SD0);
	max77620_regulator_config_fps(REGULATOR_SD1);
	max77620_regulator_config_fps(REGULATOR_SD3);
	max77620_regulator_set_voltage(REGULATOR_SD0, 1125000); //1.125V

	//Set SDMMC1 IO clamps to default value before changing voltage
	PMC(APBDEV_PMC_PWR_DET_VAL) |= (1 << 12);

	//Start up the SDMMC1 IO voltage regulator
	max77620_regulator_set_voltage(REGULATOR_LDO2, 3300000);
	max77620_regulator_enable(REGULATOR_LDO2, 1);

	//Remove isolation from SDMMC1 and core domain
	PMC(APBDEV_PMC_NO_IOPOWER) &= ~(1 << 12);



	// Fix CPU/GPU after a L4T warmboot.
	i2c_send_byte(I2C_5, MAX77620_I2C_ADDR, MAX77620_REG_GPIO5, 2);
	i2c_send_byte(I2C_5, MAX77620_I2C_ADDR, MAX77620_REG_GPIO6, 2);

	i2c_send_byte(I2C_5, MAX77621_CPU_I2C_ADDR, MAX77621_VOUT_REG, MAX77621_VOUT_0_95V); // Disable power.
	i2c_send_byte(I2C_5, MAX77621_CPU_I2C_ADDR, MAX77621_VOUT_DVC_REG, MAX77621_VOUT_ENABLE | MAX77621_VOUT_1_09V); // Enable DVS power.
	i2c_send_byte(I2C_5, MAX77621_CPU_I2C_ADDR, MAX77621_CONTROL1_REG, MAX77621_RAMP_50mV_PER_US);
	i2c_send_byte(I2C_5, MAX77621_CPU_I2C_ADDR, MAX77621_CONTROL2_REG,
		MAX77621_T_JUNCTION_120 | MAX77621_FT_ENABLE | MAX77621_CKKADV_TRIP_75mV_PER_US_HIST_DIS |
		MAX77621_CKKADV_TRIP_150mV_PER_US | MAX77621_INDUCTOR_NOMINAL);

	i2c_send_byte(I2C_5, MAX77621_GPU_I2C_ADDR, MAX77621_VOUT_REG, MAX77621_VOUT_0_95V); // Disable power.
	i2c_send_byte(I2C_5, MAX77621_GPU_I2C_ADDR, MAX77621_VOUT_DVC_REG, MAX77621_VOUT_ENABLE | MAX77621_VOUT_1_09V); // Enable DVS power.
	i2c_send_byte(I2C_5, MAX77621_GPU_I2C_ADDR, MAX77621_CONTROL1_REG, MAX77621_RAMP_50mV_PER_US);
	i2c_send_byte(I2C_5, MAX77621_GPU_I2C_ADDR, MAX77621_CONTROL2_REG,
		MAX77621_T_JUNCTION_120 | MAX77621_FT_ENABLE | MAX77621_CKKADV_TRIP_75mV_PER_US_HIST_DIS |
		MAX77621_CKKADV_TRIP_150mV_PER_US | MAX77621_INDUCTOR_NOMINAL);



	config_pmc_scratch();
	
	//CLOCK(CLK_RST_CONTROLLER_SCLK_BURST_POLICY) = (CLOCK(CLK_RST_CONTROLLER_SCLK_BURST_POLICY) & 0xFFFF8888) | 0x3333;
	CLOCK(CLK_RST_CONTROLLER_SCLK_BURST_POLICY) = 0x20003333; // Set SCLK to PLLP_OUT (408MHz).


	//mc_config_carveout();
	{
		const struct sdram_params* sdram_params = sdram_get_params();
		sdram_init(sdram_params);
		//TODO: test this with LP0 wakeup.
		//sdram_lp0_save_params(sdram_params);
	}	

	/*
	 * IMPORTANT:
	 * DO NOT INITIALIZE ANY CARVEOUT BEFORE TZ.
	 *
	 * Trust Zone needs to be initialized after the DRAM initialization
	 * because carveout registers are programmed during DRAM init.
	 * cbmem_initialize() is dependent on the Trust Zone region
	 * initialization because CBMEM lives right below the Trust Zone which
	 * needs to be properly identified.
	 */
	trustzone_region_init();

	// Now do various other carveouts
	gpu_region_init();
	nvdec_region_init();
	tsec_region_init();
	vpr_region_init();
	print_carveouts();
}
