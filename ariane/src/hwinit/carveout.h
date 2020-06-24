#ifndef _CARVEOUT_H_
#define _CARVEOUT_H_

#include <stdint.h>
#include <stdlib.h>

#define GPU_CARVEOUT_SIZE_MB		1
#define NVDEC_CARVEOUT_SIZE_MB		1
#define TSEC_CARVEOUT_SIZE_MB		2
#define VPR_CARVEOUT_SIZE_MB		128
#define TRUSTZONE_CARVEOUT_SIZE_MB 0x14

#define TEGRA_DRAM_START 0x80000000

/* Return total size of DRAM memory configured on the platform. */
int sdram_size_mb(void);

/* Find memory below and above 4GiB boundary repsectively. All units 1MiB. */
void memory_in_range_below_4gb(uintptr_t *base_mib, uintptr_t *end_mib);
void memory_in_range_above_4gb(uintptr_t *base_mib, uintptr_t *end_mib);

enum {
	CARVEOUT_TZ,
	CARVEOUT_SEC,
	CARVEOUT_MTS,
	CARVEOUT_VPR,
	CARVEOUT_GPU,
	CARVEOUT_NVDEC,
	CARVEOUT_TSEC,
	CARVEOUT_NUM,
};

/* Provided the careout id, obtain the base and size in 1MiB units. */
void carveout_range(int id, uintptr_t *base_mib, size_t *size_mib);
void print_carveouts(void);

/*
 * There are complications accessing the Trust Zone carveout region. The
 * AVP cannot access these registers and the CPU can't access this register
 * as a non-secure access. When the page tables live in non-secure memory
 * these registers cannot be accessed either. Thus, this function handles
 * both the AVP case and non-secured access case by keeping global state.
 */
void trustzone_region_init(void);
void gpu_region_init(void);
void nvdec_region_init(void);
void tsec_region_init(void);
void vpr_region_init(void);

#endif