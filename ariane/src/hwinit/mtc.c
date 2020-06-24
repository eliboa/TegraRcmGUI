#include "timer.h"
#include "sdram.h"
#include "lib/decomp.h"
#include "lib/crc32.h"
#include "lib/printk.h"
#include "lib/heap.h"
#include <string.h>

typedef int bool;
#define true  1
#define false 0
#include "minerva_tc/mtc/mtc.h"
#include "minerva_tc/mtc/mtc_mc_emc_regs.h"

extern size_t mtc_sdram_lzma_size;
extern const char mtc_sdram_lzma[];

static mtc_config_t _current_config = {0};
static u32 _current_table_crc = 0;

static bool _initialize_mtc_table()
{
    const size_t mtc_tables_size = ulzman(mtc_sdram_lzma, mtc_sdram_lzma_size, NULL, 0);
    if (mtc_tables_size > 0)
    {
#ifdef MTC_DEBUGGING
        printk("[MTC_LOAD] Decompressing %u -> %u bytes of MTC tables...\n", mtc_sdram_lzma_size, mtc_tables_size);
#endif
        u8* decomp_buffer = malloc(mtc_tables_size);
        
#ifdef MTC_DEBUGGING
        const u32 decomp_time_start = get_tmr_us();
#endif
        const size_t mtc_decomp_size = ulzman(mtc_sdram_lzma, mtc_sdram_lzma_size, decomp_buffer, mtc_tables_size);
#ifdef MTC_DEBUGGING
        const u32 decomp_time_end = get_tmr_us();
#endif
        if (mtc_decomp_size != mtc_tables_size)
        {
            printk("[MTC_LOAD] Error during lzma decompression, got %u instead of %u bytes out!\n", mtc_decomp_size, mtc_tables_size);
            free(decomp_buffer);
        }
        else
        {
#ifdef MTC_DEBUGGING
            printk("[MTC_LOAD] Decompression took %u us.\n", decomp_time_end - decomp_time_start);
#endif
            
            u8 matchBuffer[0x40];
            memcpy(matchBuffer, decomp_buffer, sizeof(matchBuffer));

            const u8* matchStart = &decomp_buffer[sizeof(matchBuffer)];
            const u8* matchEnd = &decomp_buffer[mtc_tables_size];

            typedef struct
            {
                u32 start;
                u32 len;
            } ramEntry_t;

            ramEntry_t entries[8];
            memset(entries, 0, sizeof(entries));

#ifdef MTC_DEBUGGING
            const u32 search_time_start = get_tmr_us();
#endif
            u32 numEntries = 0;
            while (matchStart < matchEnd)
            {
                if (memcmp(matchStart, matchBuffer, sizeof(matchBuffer)) == 0)
                {
                    entries[numEntries].len = (matchStart - decomp_buffer) - entries[numEntries].start;
                    entries[++numEntries].start = (matchStart - decomp_buffer);
                }
                matchStart += sizeof(matchBuffer);
            }
            entries[numEntries].len = (matchEnd - decomp_buffer) - entries[numEntries].start;
            numEntries++;
#ifdef MTC_DEBUGGING
            const u32 search_time_end = get_tmr_us();
#endif

            _current_config.sdram_id = get_sdram_id();
            for (u32 i=0; i<numEntries; i++)
            {
#ifdef MTC_DEBUGGING
                printk("[MTC_LOAD] \tentry(%u): %08x %08x", i, entries[i].start, entries[i].len);
#endif
                if (i == _current_config.sdram_id)
                {
#ifdef MTC_DEBUGGING
                    printk(" <- CHOSEN");
#endif

                    _current_config.mtc_table = (void*)(0x80000000);
                    _current_config.table_entries = entries[i].len / sizeof(emc_table_t);
                    _current_config.emc_2X_clk_src_is_pllmb = false;
	                _current_config.fsp_for_src_freq = false;
                    _current_config.train_ram_patterns = true;

                    const size_t total_table_size = _current_config.table_entries * sizeof(emc_table_t);
                    memcpy(_current_config.mtc_table, &decomp_buffer[entries[i].start], total_table_size);
                    _current_table_crc = crc32b((unsigned char*)_current_config.mtc_table, total_table_size);
                }
                
#ifdef MTC_DEBUGGING
                printk("\n");
#endif
            }

            free(decomp_buffer);
#ifdef MTC_DEBUGGING
            printk("[MTC_LOAD] Finding entries took %u us.\n", search_time_end - search_time_start);
#endif
            return true;
        }
    }

    return false;
}

int mtc_perform_memory_training(int targetFreq)
{
    if (_current_config.mtc_table == NULL)
        _initialize_mtc_table();

    const emc_table_t* mtcTable = _current_config.mtc_table;
    const u32 numEntries = _current_config.table_entries;    
    const size_t mtcTableSize = numEntries * sizeof(emc_table_t);

    if (mtcTable == NULL || numEntries == 0)
        return 0;

    if (crc32b((unsigned char*)mtcTable, mtcTableSize) != _current_table_crc)
        return 0;

    const emc_table_t* currentTable = _current_config.current_emc_table;
    if (currentTable == NULL)
    {
        u32 runningEntry;
        const u32 currEmcClkSrc = CLOCK(CLK_RST_CONTROLLER_CLK_SOURCE_EMC);
        for (runningEntry=0; runningEntry<numEntries; runningEntry++) 
        {
            if (mtcTable[runningEntry].clk_src_emc == currEmcClkSrc)
                break;
        }

        if (runningEntry >= numEntries) 
        {
            printk("[MTC] failed to find currently running entry\n");
            return 0;
        }
        currentTable = &mtcTable[runningEntry];
    }
    
#ifdef MTC_DEBUGGING
    printk("[MTC] currently using entry for %d kHz: %s\n", currentTable->rate_khz, currentTable->dvfs_ver);
#endif

#ifdef MTC_SEPARATE_TRAINING
    for (u32 i=0; i<numEntries; i++) 
    {
		if (&mtcTable[i] == currentTable) 
            continue;

        _current_config.rate_from = currentTable->rate_khz;
        _current_config.rate_to = mtcTable[i].rate_khz;
        _current_config.train_mode = OP_TRAIN;
		printk("[MTC] Training %d kHz -> %d kHz\n", _current_config.rate_from, _current_config.rate_to);
        minerva_main(&_current_config);
	}
#endif

	const u32 wantedRateKhz = (targetFreq < 0) ? (-targetFreq) : targetFreq;
    for (u32 nextTableIdx=(currentTable-mtcTable)+1; nextTableIdx<numEntries; nextTableIdx++)
    {
        const emc_table_t* nextTable = &mtcTable[nextTableIdx];

        if (nextTable->rate_khz > wantedRateKhz)
            break;

		if (nextTable->periodic_training && targetFreq > 0)
			break;

        _current_config.rate_from = currentTable->rate_khz;
        _current_config.rate_to = nextTable->rate_khz;
#ifdef MTC_SEPARATE_TRAINING
        _current_config.train_mode = OP_SWITCH;
#else
        _current_config.train_mode = OP_TRAIN_SWITCH;
#endif
		printk("[MTC] Switching %d kHz -> %d kHz\n", _current_config.rate_from, _current_config.rate_to);
        minerva_main(&_current_config);
        if (_current_config.current_emc_table != NULL)
            currentTable = _current_config.current_emc_table;

        if (_current_config.current_emc_table != nextTable)
        {
            printk("[MTC] Failed to switch! Remaining on %d kHz\n", currentTable->rate_khz);
            break;
        }
	}

	int newClock = currentTable->rate_khz;
    if (currentTable->periodic_training)
        newClock = -newClock;

    return newClock;
}

u32 mtc_redo_periodic_training(u32 lastPeriodicMs)
{
    const emc_table_t* mtcTable = _current_config.mtc_table;
    const u32 numEntries = _current_config.table_entries;    
    const size_t mtcTableSize = numEntries * sizeof(emc_table_t);

    if (mtcTable == NULL || numEntries == 0)
        return 0;

    if (crc32b((unsigned char*)mtcTable, mtcTableSize) != _current_table_crc)
        return 0;

    const emc_table_t* currentTable = _current_config.current_emc_table;
    if (currentTable == NULL)
        return 0;

    const u32 currTimeMs = get_tmr_ms();
    if (currTimeMs < (lastPeriodicMs + EMC_PERIODIC_TRAIN_MS))
        return lastPeriodicMs;

    _current_config.rate_from = currentTable->rate_khz;
    _current_config.rate_to = currentTable->rate_khz;
    _current_config.train_mode = OP_PERIODIC_TRAIN;
    minerva_main(&_current_config);
    return currTimeMs;
}