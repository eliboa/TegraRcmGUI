/*
 * This file is part of the coreboot project.
 *
 * Copyright 2013 Google Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include "pinmux.h"
#include "t210.h"
#include "gpio.h"

static volatile uint32_t *pinmux_regs = (void *)PINMUX_AUX_BASE;

void pinmux_set_config(int pin_index, uint32_t config)
{
	pinmux_regs[pin_index] = config;
}

uint32_t pinmux_get_config(int pin_index)
{
	return pinmux_regs[pin_index];
}

void pinmux_set_unused(int pin_index, int gpio_index, int has_gpio, int pulltype)
{
	uint32_t reg = pinmux_get_config(pin_index);
	reg &= ~PINMUX_INPUT_ENABLE;
	reg |= PINMUX_TRISTATE;
	reg &= ~PINMUX_PULL_MASK;
	if (pulltype == POR_PU)
		reg |= PINMUX_PULL_UP;
	else if (pulltype == POR_PD)
		reg |= PINMUX_PULL_DOWN;
	pinmux_set_config(pin_index, reg);

	if (has_gpio)
	{
		gpio_config(gpio_index_port(gpio_index), gpio_index_bitmask(gpio_index), GPIO_MODE_GPIO);
		gpio_output_enable(gpio_index_port(gpio_index), gpio_index_bitmask(gpio_index), GPIO_OUTPUT_DISABLE);
	}
}

void pinmux_config_i2c(int i2c_index)
{
	static const int pinmux_i2c[] = { 
		PINMUX_GEN1_I2C_SDA_INDEX, PINMUX_GEN1_I2C_SCL_INDEX, 
		PINMUX_GEN2_I2C_SDA_INDEX, PINMUX_GEN2_I2C_SCL_INDEX,
		PINMUX_GEN3_I2C_SDA_INDEX, PINMUX_GEN3_I2C_SCL_INDEX,
		PINMUX_CAM_I2C_SDA_INDEX, PINMUX_CAM_I2C_SCL_INDEX,
		PINMUX_PWR_I2C_SDA_INDEX, PINMUX_PWR_I2C_SCL_INDEX };

	pinmux_set_config(pinmux_i2c[i2c_index*2+0], PINMUX_INPUT_ENABLE);
	pinmux_set_config(pinmux_i2c[i2c_index*2+1], PINMUX_INPUT_ENABLE);

	static const int gpio_i2c[] = { 
		PAD_TO_GPIO_GEN1_I2C_SDA, PAD_TO_GPIO_GEN1_I2C_SCL, 
		PAD_TO_GPIO_GEN2_I2C_SDA, PAD_TO_GPIO_GEN2_I2C_SCL,
		PAD_TO_GPIO_GEN3_I2C_SDA, PAD_TO_GPIO_GEN3_I2C_SCL,
		PAD_TO_GPIO_CAM_I2C_SDA, PAD_TO_GPIO_CAM_I2C_SCL,
		PAD_TO_GPIO_PWR_I2C_SDA, PAD_TO_GPIO_PWR_I2C_SCL };

	const int gpio_index_sda = gpio_i2c[i2c_index*2+0];
	const int gpio_index_scl = gpio_i2c[i2c_index*2+1];
	gpio_config(gpio_index_port(gpio_index_sda), gpio_index_bitmask(gpio_index_sda), GPIO_MODE_SPIO);
	gpio_config(gpio_index_port(gpio_index_scl), gpio_index_bitmask(gpio_index_scl), GPIO_MODE_SPIO);
}

void pinmux_config_uart(int uart_index)
{
	static const int pinmux_uart[] = { 
		PINMUX_UART1_TX_INDEX, PINMUX_UART1_RX_INDEX, PINMUX_UART1_RTS_INDEX, PINMUX_UART1_CTS_INDEX,
		PINMUX_UART2_TX_INDEX, PINMUX_UART2_RX_INDEX, PINMUX_UART2_RTS_INDEX, PINMUX_UART2_CTS_INDEX,
		PINMUX_UART3_TX_INDEX, PINMUX_UART3_RX_INDEX, PINMUX_UART3_RTS_INDEX, PINMUX_UART3_CTS_INDEX,
		PINMUX_UART4_TX_INDEX, PINMUX_UART4_RX_INDEX, PINMUX_UART4_RTS_INDEX, PINMUX_UART4_CTS_INDEX };

	pinmux_set_config(pinmux_uart[uart_index*4+0], PINMUX_PULL_UP);
	pinmux_set_config(pinmux_uart[uart_index*4+1], PINMUX_INPUT_ENABLE | PINMUX_PULL_UP);
	pinmux_set_config(pinmux_uart[uart_index*4+2], PINMUX_PULL_UP);
	pinmux_set_config(pinmux_uart[uart_index*4+3], PINMUX_INPUT_ENABLE | PINMUX_PULL_UP);

	static const int gpio_uart[] = { 
		PAD_TO_GPIO_UART1_TX, PAD_TO_GPIO_UART1_RX, PAD_TO_GPIO_UART1_RTS, PAD_TO_GPIO_UART1_CTS,
		PAD_TO_GPIO_UART2_TX, PAD_TO_GPIO_UART2_RX, PAD_TO_GPIO_UART2_RTS, PAD_TO_GPIO_UART2_CTS,
		PAD_TO_GPIO_UART3_TX, PAD_TO_GPIO_UART3_RX, PAD_TO_GPIO_UART3_RTS, PAD_TO_GPIO_UART3_CTS,
		PAD_TO_GPIO_UART4_TX, PAD_TO_GPIO_UART4_RX, PAD_TO_GPIO_UART4_RTS, PAD_TO_GPIO_UART4_CTS };

	for (int i=0; i<4; i++)
	{
		const int gpio_index = gpio_uart[uart_index*4+i];
		gpio_config(gpio_index_port(gpio_index), gpio_index_bitmask(gpio_index), GPIO_MODE_SPIO);
	}
}