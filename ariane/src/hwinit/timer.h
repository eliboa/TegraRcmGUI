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

#ifndef _TIMER_H_
#define _TIMER_H_

#include "types.h"

#define TMR_US_OFFS     0x10
#define TMR_US_CFG_OFFS 0x14

#define RTC_SECONDS		0x8
#define RTC_SHADOW_SECONDS	0xC
#define RTC_MILLI_SECONDS	0x10

u32 get_tmr_s();
u32 get_tmr_ms();
u32 get_tmr_us();

void msleep(u32 milliseconds);
void usleep(u32 microseconds);

#endif
