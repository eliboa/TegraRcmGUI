/*
 * This file is part of the coreboot project.
 *
 * Copyright 2014 Google Inc.
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

#include "cbmem.h"
#include "hwinit/carveout.h"
#include "lib/printk.h"
#include <string.h>
#include <stdbool.h>

#if defined(__INT_MAX__) && __INT_MAX__ == 2147483647
typedef int ssize_t;
#else
typedef long ssize_t;
#endif

void *cbmem_top(void)
{
	static uintptr_t addr;

	if (addr == 0) {
		uintptr_t begin_mib;
		uintptr_t end_mib;

		memory_in_range_below_4gb(&begin_mib, &end_mib);
		/* Make sure we consume everything up to 4GIB. */
		if (end_mib == 4096)
			addr = ~(uint32_t)0;
		else
			addr = end_mib << 20;
	}

	return (void *)addr;
}

void cbmem_initialize_empty(void)
{
	cbmem_initialize_empty_id_size(0, 0);
}

/*
 * The struct imd is a handle for working with an in-memory directory.
 *
 * NOTE: Do not directly touch any fields within this structure. An imd pointer
 * is meant to be opaque, but the fields are exposed for stack allocation.
 */
struct imdr {
	uintptr_t limit;
	void *r;
};
struct imd {
	struct imdr lg;
	struct imdr sm;
};

static const uint32_t IMD_ROOT_PTR_MAGIC = 0xc0389481;
static const uint32_t IMD_ENTRY_MAGIC = ~0xc0389481;
static const uint32_t SMALL_REGION_ID = CBMEM_ID_IMD_SMALL;
static const size_t LIMIT_ALIGN = 4096;

static void imdr_init(struct imdr *ir, void *upper_limit)
{
	uintptr_t limit = (uintptr_t)upper_limit;
	/* Upper limit is aligned down to 4KiB */
	ir->limit = ALIGN_DOWN(limit, LIMIT_ALIGN);
	ir->r = NULL;
}

/* Initialize imd handle. */
static void imd_handle_init(struct imd *imd, void *upper_limit)
{
	imdr_init(&imd->lg, upper_limit);
	imdr_init(&imd->sm, NULL);
}

/* In-memory data structures. */
struct imd_root_pointer {
	uint32_t magic;
	/* Relative to upper limit/offset. */
	int32_t root_offset;
} __packed;

struct imd_entry {
	uint32_t magic;
	/* start is located relative to imd_root */
	int32_t start_offset;
	uint32_t size;
	uint32_t id;
} __packed;

struct imd_root {
	uint32_t max_entries;
	uint32_t num_entries;
	uint32_t flags;
	uint32_t entry_align;
	/* Used for fixing the size of an imd. Relative to the root. */
	int32_t max_offset;
	struct imd_entry entries[0];
} __packed;

#define IMD_FLAG_LOCKED 1

static void imd_entry_assign(struct imd_entry *e, uint32_t id,
				ssize_t offset, size_t size)
{
	e->magic = IMD_ENTRY_MAGIC;
	e->start_offset = offset;
	e->size = size;
	e->id = id;
}

static void *relative_pointer(void *base, ssize_t offset)
{
	intptr_t b = (intptr_t)base;
	b += offset;
	return (void *)b;
}

static struct imd_root *imdr_root(const struct imdr *imdr)
{
	return imdr->r;
}

/*
 * The root pointer is relative to the upper limit of the imd. i.e. It sits
 * just below the upper limit.
 */
static struct imd_root_pointer *imdr_get_root_pointer(const struct imdr *imdr)
{
	struct imd_root_pointer *rp;

	rp = relative_pointer((void *)imdr->limit, -sizeof(*rp));

	return rp;
}

static void imd_link_root(struct imd_root_pointer *rp, struct imd_root *r)
{
	rp->magic = IMD_ROOT_PTR_MAGIC;
	rp->root_offset = (int32_t)((intptr_t)r - (intptr_t)rp);
}

static struct imd_entry *root_last_entry(struct imd_root *r)
{
	return &r->entries[r->num_entries - 1];
}

static size_t root_num_entries(size_t root_size)
{
	size_t entries_size;

	entries_size = root_size;
	entries_size -= sizeof(struct imd_root_pointer);
	entries_size -= sizeof(struct imd_root);

	return entries_size / sizeof(struct imd_entry);
}

static size_t imd_root_data_left(struct imd_root *r)
{
	struct imd_entry *last_entry;

	last_entry = root_last_entry(r);

	if (r->max_offset != 0)
		return last_entry->start_offset - r->max_offset;

	return ~(size_t)0;
}

static bool root_is_locked(const struct imd_root *r)
{
	return !!(r->flags & IMD_FLAG_LOCKED);
}

static int imdr_create_empty(struct imdr *imdr, size_t root_size,
				size_t entry_align)
{
	struct imd_root_pointer *rp;
	struct imd_root *r;
	struct imd_entry *e;
	ssize_t root_offset;

	if (!imdr->limit)
		return -1;

	/* root_size and entry_align should be a power of 2. */
	if (!IS_POWER_OF_2(root_size))
    {
        dbg_print("IMDR root_size not a power of 2, given %u\n", root_size);
        while (1) {}
    }
	if (!IS_POWER_OF_2(entry_align))
    {
        dbg_print("IMDR entry_align not a power of 2, given %u\n", entry_align);
        while (1) {}
    }

	if (!imdr->limit)
		return -1;

	/*
	 * root_size needs to be large enough to accommodate root pointer and
	 * root book keeping structure. The caller needs to ensure there's
	 * enough room for tracking individual allocations.
	 */
	if (root_size < (sizeof(*rp) + sizeof(*r)))
		return -1;

	/* For simplicity don't allow sizes or alignments to exceed LIMIT_ALIGN.
	 */
	if (root_size > LIMIT_ALIGN || entry_align > LIMIT_ALIGN)
		return -1;

	/* Additionally, don't handle an entry alignment > root_size. */
	if (entry_align > root_size)
		return -1;

	rp = imdr_get_root_pointer(imdr);

	root_offset = -(ssize_t)root_size;
	/* Set root pointer. */
	imdr->r = relative_pointer((void *)imdr->limit, root_offset);
	r = imdr_root(imdr);
	imd_link_root(rp, r);

	memset(r, 0, sizeof(*r));
	r->entry_align = entry_align;

	/* Calculate size left for entries. */
	r->max_entries = root_num_entries(root_size);

	/* Fill in first entry covering the root region. */
	r->num_entries = 1;
	e = &r->entries[0];
	imd_entry_assign(e, CBMEM_ID_IMD_ROOT, 0, root_size);

	dbg_print("IMD: root @ %p %u entries.\n", r, r->max_entries);
	return 0;
}

static int imdr_limit_size(struct imdr *imdr, size_t max_size)
{
	struct imd_root *r;
	ssize_t smax_size;
	size_t root_size;

	r = imdr_root(imdr);
	if (r == NULL)
		return -1;

	root_size = imdr->limit - (uintptr_t)r;

	if (max_size < root_size)
		return -1;

	/* Take into account the root size. */
	smax_size = max_size - root_size;
	smax_size = -smax_size;

	r->max_offset = smax_size;

	return 0;
}

static void *imdr_entry_at(const struct imdr *imdr, const struct imd_entry *e)
{
	return relative_pointer(imdr_root(imdr), e->start_offset);
}

static struct imd_entry *imd_entry_add_to_root(struct imd_root *r, uint32_t id,
						size_t size)
{
	struct imd_entry *entry;
	struct imd_entry *last_entry;
	ssize_t e_offset;
	size_t used_size;

	if (r->num_entries == r->max_entries)
		return NULL;

	/* Determine total size taken up by entry. */
	used_size = ALIGN_UP(size, r->entry_align);

	/* See if size overflows imd total size. */
	if (used_size > imd_root_data_left(r))
		return NULL;

	/*
	 * Determine if offset field overflows. All offsets should be lower
	 * than the previous one.
	 */
	last_entry = root_last_entry(r);
	e_offset = last_entry->start_offset;
	e_offset -= (ssize_t)used_size;
	if (e_offset > last_entry->start_offset)
		return NULL;

	entry = root_last_entry(r) + 1;
	r->num_entries++;

	imd_entry_assign(entry, id, e_offset, size);

	return entry;
}

static const struct imd_entry *imdr_entry_add(const struct imdr *imdr,
						uint32_t id, size_t size)
{
	struct imd_root *r;

	r = imdr_root(imdr);

	if (r == NULL)
		return NULL;

	if (root_is_locked(r))
		return NULL;

	return imd_entry_add_to_root(r, id, size);
}

static int imd_create_tiered_empty(struct imd *imd,
				size_t lg_root_size, size_t lg_entry_align,
				size_t sm_root_size, size_t sm_entry_align)
{
	size_t sm_region_size;
	const struct imd_entry *e;
	struct imdr *imdr;

	imdr = &imd->lg;

	if (imdr_create_empty(imdr, lg_root_size, lg_entry_align) != 0)
		return -1;

	/* Calculate the size of the small region to request. */
	sm_region_size = root_num_entries(sm_root_size) * sm_entry_align;
	sm_region_size += sm_root_size;
	sm_region_size = ALIGN_UP(sm_region_size, lg_entry_align);

	/* Add a new entry to the large region to cover the root and entries. */
	e = imdr_entry_add(imdr, SMALL_REGION_ID, sm_region_size);

	if (e == NULL)
		goto fail;

	imd->sm.limit = (uintptr_t)imdr_entry_at(imdr, e);
	imd->sm.limit += sm_region_size;

	if (imdr_create_empty(&imd->sm, sm_root_size, sm_entry_align) != 0 ||
		imdr_limit_size(&imd->sm, sm_region_size))
		goto fail;

	return 0;
fail:
	imd_handle_init(imd, (void *)imdr->limit);
	return -1;
}

void cbmem_initialize_empty_id_size(u32 id, u64 size)
{
	struct imd *imd;
	struct imd imd_backing;

	imd = &imd_backing;
	imd_handle_init(imd, cbmem_top());

	dbg_print("CBMEM:\n");

	if (imd_create_tiered_empty(imd, CBMEM_ROOT_MIN_SIZE, CBMEM_LG_ALIGN,
					CBMEM_SM_ROOT_SIZE, CBMEM_SM_ALIGN)) {
		dbg_print("failed.\n");
		return;
	}

	/* Add the specified range first */
	if (size)
		dbg_print("CBMEM region can only be empty, while size of %llu specified!\n", size);
}
