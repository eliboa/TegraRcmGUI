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

#include "timer.h"
#include "t210.h"

u32 get_tmr_s()
{
	return RTC(RTC_SECONDS);
}

u32 get_tmr_ms()
{
	//reading RTC_MILLI_SECONDS updates RTC_SHADOW_SECONDS value to match
	u32 millis = RTC(RTC_MILLI_SECONDS);
	u32 seconds = RTC(RTC_SHADOW_SECONDS);

	return (millis | (seconds << 10));
}

u32 get_tmr_us()
{
	return TMR(TMR_US_OFFS);
}

void msleep(u32 milliseconds)
{
	u32 start = RTC(RTC_MILLI_SECONDS) | (RTC(RTC_SHADOW_SECONDS) << 10);
	while (((RTC(RTC_MILLI_SECONDS) | (RTC(RTC_SHADOW_SECONDS) << 10)) - start) <= milliseconds) {}
}

void usleep(u32 microseconds)
{
	u32 start = TMR(TMR_US_OFFS);
	while ((TMR(TMR_US_OFFS) - start) <= microseconds) {}
}