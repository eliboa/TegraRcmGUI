#include "storage.h"
#include "lib/ffconf.h"

sdmmc_t* get_controller_for_index(u8 pdrv)
{
    static sdmmc_t controllers[FF_VOLUMES] = {0};
    if (pdrv >= FF_VOLUMES)
        return NULL;

    return &controllers[pdrv];
}

sdmmc_storage_t* get_storage_for_index(u8 pdrv)
{
    static sdmmc_storage_t storages[FF_VOLUMES] = {0};
    if (pdrv >= FF_VOLUMES)
        return NULL;

    return &storages[pdrv];
}