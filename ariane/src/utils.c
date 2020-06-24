#include <stdbool.h>
#include "utils.h"
#include "fuse.h"
#include "hwinit/t210.h"
#include "hwinit/pmc.h"
#include "panic_color.h"

#include "hwinit/btn.h"

__attribute__ ((noreturn)) void panic(uint32_t code) {
    /* Set Panic Code for NX_BOOTLOADER. */
    if (PMC(APBDEV_PMC_SCRATCH200) == 0) {
        PMC(APBDEV_PMC_SCRATCH200) = code;
    }

    /* TODO: Custom Panic Driver, which displays to screen without rebooting. */
    /* For now, just use NX BOOTLOADER's panic. */
    fuse_disable_programming();
    PMC(APBDEV_PMC_CRYPTO_OP) = 1; /* Disable all SE operations. */

    /* FIXME: clean up and use this instead of replacing things */
    while(btn_read() != BTN_POWER);
    
    /* Ensure we boot back into RCM, for development. */
    PMC(APBDEV_PMC_SCRATCH0) = (1 << 1);

    /* Reset the processor. */
    PMC(0) = (1 << 4);
    while(1);
}

__attribute__ ((noreturn)) void generic_panic(void) {
    panic(0xFF000006);
}

__attribute__ ((noreturn)) void panic_predefined(uint32_t which) {
    static const uint32_t codes[0x10] = {COLOR_0, COLOR_1, COLOR_2, COLOR_3, COLOR_4, COLOR_5, COLOR_6, COLOR_7, COLOR_8, COLOR_9, COLOR_A, COLOR_B, COLOR_C, COLOR_D, COLOR_E, COLOR_F};
    panic(codes[which & 0xF]);
}

__attribute__((noinline)) bool overlaps(uint64_t as, uint64_t ae, uint64_t bs, uint64_t be)
{
    if(as <= bs && bs <= ae)
        return true;
    if(bs <= as && as <= be)
        return true;
    return false;
}
