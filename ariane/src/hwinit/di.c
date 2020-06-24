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

#include "di.h"
#include "clock.h"
#include "gpio.h"
#include "pinmux.h"
#include "t210.h"
#include "timer.h"
#include "util.h"
#include "pmc.h"
#include "max7762x.h"
#include "max77620.h"

#include "di.inl"

static u32 _display_ver = 0;

static void _display_dsi_wait(u32 milliseconds, u32 off, u32 mask)
{
	u32 end = get_tmr_ms() + milliseconds;
	while (DSI(off) & mask) { if (get_tmr_ms() >= end) return; }
	usleep(5);
}

void display_init()
{
	//Power on.
	max77620_regulator_set_voltage(REGULATOR_LDO0, 1200000); //1.2V
	max77620_regulator_enable(REGULATOR_LDO0, 1);
	max77620_send_byte(MAX77620_REG_GPIO7, 0x09);

	//Enable MIPI CAL, DSI, DISP1, HOST1X, UART_FST_MIPI_CAL, DSIA LP clocks.
	CLOCK(CLK_RST_CONTROLLER_RST_DEV_H_CLR) = CLK_H_DSI | CLK_H_MIPI_CAL;
	CLOCK(CLK_RST_CONTROLLER_CLK_ENB_H_SET) = CLK_H_DSI | CLK_H_MIPI_CAL;
	CLOCK(CLK_RST_CONTROLLER_RST_DEV_L_CLR) = CLK_L_DISP1 | CLK_L_HOST1X;
	CLOCK(CLK_RST_CONTROLLER_CLK_ENB_L_SET) = CLK_L_DISP1 | CLK_L_HOST1X;
	CLOCK(CLK_RST_CONTROLLER_CLK_ENB_X_SET) = CLK_X_UART_FST_MIPI_CAL;
	CLOCK(CLK_RST_CONTROLLER_CLK_SOURCE_UART_FST_MIPI_CAL) = 0xA;
	CLOCK(CLK_RST_CONTROLLER_CLK_ENB_W_SET) = CLK_W_DSIA_LP;
	CLOCK(CLK_RST_CONTROLLER_CLK_SOURCE_DSIA_LP) = 0xA;

	//DPD idle.
	PMC(APBDEV_PMC_IO_DPD_REQ) = 0x40000000;
	PMC(APBDEV_PMC_IO_DPD2_REQ) = 0x40000000;

	//Config pins.
	pinmux_set_config(PINMUX_GPIO_I0, pinmux_get_config(PINMUX_GPIO_I0) & (~PINMUX_TRISTATE));
	pinmux_set_config(PINMUX_GPIO_I1, pinmux_get_config(PINMUX_GPIO_I1) & (~PINMUX_TRISTATE));
	pinmux_set_config(PINMUX_LCD_BL_PWM_INDEX, pinmux_get_config(PINMUX_LCD_BL_PWM_INDEX) & (~PINMUX_TRISTATE));
	pinmux_set_config(PINMUX_LCD_BL_EN_INDEX, pinmux_get_config(PINMUX_LCD_BL_EN_INDEX) & (~PINMUX_TRISTATE));
	pinmux_set_config(PINMUX_LCD_RST_INDEX, pinmux_get_config(PINMUX_LCD_RST_INDEX) & (~PINMUX_TRISTATE));

	gpio_config(GPIO_PORT_I, GPIO_PIN_0 | GPIO_PIN_1, GPIO_MODE_GPIO); //Backlight +-5V.
	gpio_output_enable(GPIO_PORT_I, GPIO_PIN_0 | GPIO_PIN_1, GPIO_OUTPUT_ENABLE); //Backlight +-5V.
	gpio_write(GPIO_PORT_I, GPIO_PIN_0, GPIO_HIGH); //Backlight +5V enable.

	msleep(10);
	gpio_write(GPIO_PORT_I, GPIO_PIN_1, GPIO_HIGH); //Backlight -5V enable.
	msleep(10);

	gpio_config(GPIO_PORT_V, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2, GPIO_MODE_GPIO); //Backlight PWM, Enable, Reset.
	gpio_output_enable(GPIO_PORT_V, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2, GPIO_OUTPUT_ENABLE);
	gpio_write(GPIO_BY_NAME(LCD_BL_EN), GPIO_HIGH); //Backlight Enable enable.

	//Config display interface and display.
	MIPI_CAL(0x60) = 0;

	exec_cfg((u32 *)CLOCK_BASE, _display_config_1, ARRAY_SIZE(_display_config_1));
	exec_cfg((u32 *)DISPLAY_A_BASE, _display_config_2, ARRAY_SIZE(_display_config_2));
	exec_cfg((u32 *)DSI_BASE, _display_config_3, ARRAY_SIZE(_display_config_3));

	msleep(10);
	gpio_write(GPIO_BY_NAME(LCD_RST), GPIO_HIGH); //Backlight Reset enable.
	msleep(60);

	DSI(_DSIREG(DSI_BTA_TIMING)) = 0x50204;
	DSI(_DSIREG(DSI_WR_DATA)) = 0x337;
	DSI(_DSIREG(DSI_TRIGGER)) = DSI_TRIGGER_HOST;
	_display_dsi_wait(250, _DSIREG(DSI_TRIGGER), DSI_TRIGGER_HOST | DSI_TRIGGER_VIDEO);

	DSI(_DSIREG(DSI_WR_DATA)) = 0x406;
	DSI(_DSIREG(DSI_TRIGGER)) = DSI_TRIGGER_HOST;
	_display_dsi_wait(250, _DSIREG(DSI_TRIGGER), DSI_TRIGGER_HOST | DSI_TRIGGER_VIDEO);

	DSI(_DSIREG(DSI_HOST_CONTROL)) = DSI_HOST_CONTROL_TX_TRIG_HOST | DSI_HOST_CONTROL_IMM_BTA | DSI_HOST_CONTROL_CS | DSI_HOST_CONTROL_ECC;
	_display_dsi_wait(150, _DSIREG(DSI_HOST_CONTROL), DSI_HOST_CONTROL_IMM_BTA);

	msleep(5);

	_display_ver = DSI(_DSIREG(DSI_RD_DATA));
	if (_display_ver == 0x10)
		exec_cfg((u32 *)DSI_BASE, _display_config_4, ARRAY_SIZE(_display_config_4));

	DSI(_DSIREG(DSI_WR_DATA)) = 0x1105;
	DSI(_DSIREG(DSI_TRIGGER)) = DSI_TRIGGER_HOST;

	msleep(180);

	DSI(_DSIREG(DSI_WR_DATA)) = 0x2905;
	DSI(_DSIREG(DSI_TRIGGER)) = DSI_TRIGGER_HOST;

	msleep(20);

	exec_cfg((u32 *)DSI_BASE, _display_config_5, ARRAY_SIZE(_display_config_5));
	exec_cfg((u32 *)CLOCK_BASE, _display_config_6, ARRAY_SIZE(_display_config_6));
	DISPLAY_A(_DIREG(DC_DISP_DISP_CLOCK_CONTROL)) = 4;
	exec_cfg((u32 *)DSI_BASE, _display_config_7, ARRAY_SIZE(_display_config_7));

	msleep(10);

	exec_cfg((u32 *)MIPI_CAL_BASE, _display_config_8, ARRAY_SIZE(_display_config_8));
	exec_cfg((u32 *)DSI_BASE, _display_config_9, ARRAY_SIZE(_display_config_9));
	exec_cfg((u32 *)MIPI_CAL_BASE, _display_config_10, ARRAY_SIZE(_display_config_10));

	msleep(10);

	exec_cfg((u32 *)DISPLAY_A_BASE, _display_config_11, ARRAY_SIZE(_display_config_11));
}

void display_enable_backlight(u32 on) 
{
	gpio_write(GPIO_BY_NAME(LCD_BL_PWM), on ? GPIO_HIGH : GPIO_LOW); //Backlight PWM.
}

void display_end()
{
	display_enable_backlight(0);
	DSI(_DSIREG(DSI_VIDEO_MODE_CONTROL)) = 1;
	DSI(_DSIREG(DSI_WR_DATA)) = 0x2805;

	u32 end = HOST1X(0x30A4) + 5;
	while (HOST1X(0x30A4) < end) {}

	DISPLAY_A(_DIREG(DC_CMD_STATE_ACCESS)) = READ_MUX | WRITE_MUX;
	DSI(_DSIREG(DSI_VIDEO_MODE_CONTROL)) = 0;

	exec_cfg((u32 *)DISPLAY_A_BASE, _display_config_12, ARRAY_SIZE(_display_config_12));
	exec_cfg((u32 *)DSI_BASE, _display_config_13, ARRAY_SIZE(_display_config_13));

	msleep(10);

	if (_display_ver == 0x10)
		exec_cfg((u32 *)DSI_BASE, _display_config_14, ARRAY_SIZE(_display_config_14));

	DSI(_DSIREG(DSI_WR_DATA)) = 0x1005;
	DSI(_DSIREG(DSI_TRIGGER)) = DSI_TRIGGER_HOST;

	msleep(50);
	gpio_write(GPIO_BY_NAME(LCD_RST), GPIO_LOW); //Backlight Reset disable.
	msleep(10);
	gpio_write(GPIO_PORT_I, GPIO_PIN_1, GPIO_LOW); //Backlight -5V disable.
	msleep(10);
	gpio_write(GPIO_PORT_I, GPIO_PIN_0, GPIO_LOW); //Backlight +5V disable.
	msleep(10);

	//Disable clocks.
	CLOCK(CLK_RST_CONTROLLER_RST_DEV_H_SET) = CLK_H_DSI | CLK_H_MIPI_CAL;
	CLOCK(CLK_RST_CONTROLLER_CLK_ENB_H_CLR) = CLK_H_DSI | CLK_H_MIPI_CAL;
	CLOCK(CLK_RST_CONTROLLER_RST_DEV_L_SET) = CLK_L_DISP1 | CLK_L_HOST1X;
	CLOCK(CLK_RST_CONTROLLER_CLK_ENB_L_CLR) = CLK_L_DISP1 | CLK_L_HOST1X;

	DSI(_DSIREG(DSI_PAD_CONTROL_0)) = DSI_PAD_CONTROL_VS1_PULLDN_CLK | DSI_PAD_CONTROL_VS1_PULLDN(0xF) | DSI_PAD_CONTROL_VS1_PDIO_CLK | DSI_PAD_CONTROL_VS1_PDIO(0xF);
	DSI(_DSIREG(DSI_POWER_CONTROL)) = 0;

	gpio_config(GPIO_BY_NAME(LCD_BL_PWM), GPIO_MODE_SPIO); //Backlight PWM.

	pinmux_set_config(PINMUX_LCD_BL_PWM_INDEX, pinmux_get_config(PINMUX_LCD_BL_PWM_INDEX) | PINMUX_TRISTATE);
	pinmux_set_config(PINMUX_LCD_BL_PWM_INDEX, (pinmux_get_config(PINMUX_LCD_BL_PWM_INDEX) & (~PINMUX_FUNC_MASK)) | PINMUX_LCD_BL_PWM_FUNC_PWM0);
}

void display_color_screen(u32 color)
{
	exec_cfg((u32 *)DISPLAY_A_BASE, cfg_display_one_color, 8);

	//Configure display to show single color.
	DISPLAY_A(_DIREG(DC_WIN_AD_WIN_OPTIONS)) = 0;
	DISPLAY_A(_DIREG(DC_WIN_BD_WIN_OPTIONS)) = 0;
	DISPLAY_A(_DIREG(DC_WIN_CD_WIN_OPTIONS)) = 0;
	DISPLAY_A(_DIREG(DC_DISP_BLEND_BACKGROUND_COLOR)) = color;
	DISPLAY_A(_DIREG(DC_CMD_STATE_CONTROL)) = (DISPLAY_A(_DIREG(DC_CMD_STATE_CONTROL)) & 0xFFFFFFFE) | GENERAL_ACT_REQ;

	msleep(35);

	display_enable_backlight(1);
}

u32 *display_init_framebuffer(u32 *fb)
{
	//This configures the framebuffer @ 0xC0000000 with a resolution of 1280x720 (line stride 768).
	exec_cfg((u32 *)DISPLAY_A_BASE, cfg_display_framebuffer, ARRAY_SIZE(cfg_display_framebuffer));

	msleep(35);

	return (u32 *)0xC0000000;
}