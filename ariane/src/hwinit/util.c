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

#include "util.h"
#include "t210.h"
#include "max7762x.h"
#include "max77620.h"

void exec_cfg(u32 *base, const cfg_op_t *ops, u32 num_ops)
{
	for(u32 i = 0; i < num_ops; i++)
		base[ops[i].off] = ops[i].val;
}

int running_on_bpmp(void)
{
	static const u32 avp_id = 0xaaaaaaaa;
	static volatile u32* uptag_ptr = (void *)PG_UP_BASE;
	
	return (*uptag_ptr) == avp_id;
}

void shutdown_using_pmic()
{
    u8 regVal = max77620_recv_byte(MAX77620_REG_ONOFFCNFG1);
    regVal |= MAX77620_ONOFFCNFG1_PWR_OFF;
    max77620_send_byte(MAX77620_REG_ONOFFCNFG1, regVal);
}
