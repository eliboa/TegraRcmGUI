/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2009 coresystems GmbH
 * Copyright (C) 2013 Google, Inc.
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

#ifndef _CBMEM_H_
#define _CBMEM_H_

#include <stddef.h>
#include <stdint.h>
#include "hwinit/types.h"

#define CBMEM_FSP_HOB_PTR	0x614

struct cbmem_entry;

/*
 * The dynamic cbmem infrastructure allows for growing cbmem dynamically as
 * things are added. It requires an external function, cbmem_top(), to be
 * implemented by the board or chipset to define the upper address where
 * cbmem lives. This address is required to be a 32-bit address. Additionally,
 * the address needs to be consistent in both romstage and ramstage.  The
 * dynamic cbmem infrastructure allocates new regions below the last allocated
 * region. Regions are defined by a cbmem_entry struct that is opaque. Regions
 * may be removed, but the last one added is the only that can be removed.
 */

#define DYN_CBMEM_ALIGN_SIZE (4096)
#define CBMEM_ROOT_SIZE      DYN_CBMEM_ALIGN_SIZE

/* The root region is at least DYN_CBMEM_ALIGN_SIZE . */
#define CBMEM_ROOT_MIN_SIZE DYN_CBMEM_ALIGN_SIZE
#define CBMEM_LG_ALIGN CBMEM_ROOT_MIN_SIZE

/* Small allocation parameters. */
#define CBMEM_SM_ROOT_SIZE 1024
#define CBMEM_SM_ALIGN 32

/* Determine the size for CBMEM root and the small allocations */
static inline size_t cbmem_overhead_size(void)
{
	return 2 * CBMEM_ROOT_MIN_SIZE;
}

#define CBMEM_ID_IMD_ROOT	0xff4017ff
#define CBMEM_ID_IMD_SMALL	0x53a11439

/* Initialize cbmem to be empty. */
void cbmem_initialize_empty(void);
void cbmem_initialize_empty_id_size(u32 id, u64 size);

/* Return the top address for dynamic cbmem. The address returned needs to
 * be consistent across romstage and ramstage, and it is required to be
 * below 4GiB.
 * x86 boards or chipsets must return NULL before the cbmem backing store has
 * been initialized. */
void *cbmem_top(void);

#endif /* _CBMEM_H_ */
