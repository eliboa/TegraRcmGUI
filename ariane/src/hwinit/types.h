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

#ifndef _TYPES_H_
#define _TYPES_H_

#define NULL ((void *)0)

#define ALIGN(x, a) (((x) + (a) - 1) & ~((a) - 1))
#define ALIGN_UP(x,a)   ALIGN((x),(a))
#define ALIGN_DOWN(x,a) ((x) & ~((typeof(x))(a)-1UL))
#define IS_ALIGNED(x,a) (((x) & ((typeof(x))(a)-1UL)) == 0)
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define ABS(a) (((a) < 0) ? (-(a)) : (a))
#define CEIL_DIV(a, b)  (((a) + (b) - 1) / (b))
#define IS_POWER_OF_2(x)  (((x) & ((x) - 1)) == 0)
#define DIV_ROUND_UP(x, y)  (((x) + (y) - 1) / (y))

#define OFFSET_OF(t, m) ((u32)&((t *)NULL)->m)
#define CONTAINER_OF(mp, t, mn) ((t *)((u32)mp - OFFSET_OF(t, mn)))

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#endif

typedef char s8;
typedef short s16;
typedef int s32;
typedef long long int s64;

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long int u64;

typedef volatile unsigned char vu8;
typedef volatile unsigned short vu16;
typedef volatile unsigned int vu32;

#endif
