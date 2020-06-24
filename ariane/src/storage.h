#ifndef _STORAGE_H_
#define _STORAGE_H_

#include "hwinit/sdmmc.h"

sdmmc_t* get_controller_for_index(u8 pdrv);
sdmmc_storage_t* get_storage_for_index(u8 pdrv);

#endif