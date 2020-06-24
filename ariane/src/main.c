#include "utils.h"
#include "lib/printk.h"
#include "lib/heap.h"
#include "display/video_fb.h"

#include "hwinit/btn.h"
#include "hwinit/hwinit.h"
#include "hwinit/di.h"
#include "hwinit/mc.h"
#include "hwinit/mtc.h"
#include "hwinit/t210.h"
#include "hwinit/sdmmc.h"
#include "hwinit/timer.h"
#include "hwinit/cluster.h"
#include "hwinit/clock.h"
#include "hwinit/max77620.h"
#include "hwinit/max7762x.h"
#include "hwinit/util.h"
#include "hwinit/i2c.h"
#include "hwinit/pmc.h"
#include "hwinit/uart.h"
#include "hwinit/fuse.h"
#include "hwinit/pinmux.h"
#include "hwinit/sdram.h"
#include "hwinit/carveout.h"
#include "rcm_usb.h"
#include "usb_output.h"
#include "storage.h"
#include "lib/ff.h"
#include "lib/decomp.h"
#include "cbmem.h"
#include <alloca.h>
#include <strings.h>

#include "usb_command.h"

#define XVERSION 3
#define IPL_HEAP_START 0x84000000
#define IPL_HEAP_SZ    0x20000000 // 512MB.
#define IPL_STACK_TOP  0x83100000
#define IPL_LOAD_ADDR  0x40008000

extern void pivot_stack(u32 stack_top);

static int initialize_mount(FATFS* outFS, u8 devNum)
{
	sdmmc_t* currCont = get_controller_for_index(devNum);
    sdmmc_storage_t* currStor = get_storage_for_index(devNum);

    if (currCont == NULL || currStor == NULL)
    {
        printk("get_controller_for_index(%d) OR get_storage_for_index(%d) failed\n", devNum, devNum);
        return 0;
    }
    if (currStor->sdmmc != NULL)
        return 1; //already initialized

    if (devNum == 0) //maybe support more ?
    {
        if (sdmmc_storage_init_sd(currStor, currCont, SDMMC_1, SDMMC_BUS_WIDTH_4, 11) && f_mount(outFS, "", 1) == FR_OK)
            return 1;
        else
        {
            if (currStor->sdmmc != NULL)
                sdmmc_storage_end(currStor, 0);

            memset(currCont, 0, sizeof(sdmmc_t));
            memset(currStor, 0, sizeof(sdmmc_storage_t));
        }
    }

    if (devNum == 1)
    {
        if (sdmmc_storage_init_mmc(currStor, currCont, SDMMC_4, SDMMC_BUS_WIDTH_8, 4) == 1)
        {
            printk("sdmmc_storage_init_mmc succeeded\n");
            if (sdmmc_storage_set_mmc_partition(currStor, 1)) // BOOT0
                return 1;

            printk("sdmmc_storage_set_mmc_partition failed\n");
        }

        printk("sdmmc_storage_init_mmc 2 failed\n");
        if (currStor->sdmmc != NULL)
            sdmmc_storage_end(currStor, 0);

        memset(currCont, 0, sizeof(sdmmc_t));
        memset(currStor, 0, sizeof(sdmmc_storage_t));
    }

	return 0;
}

static void deinitialize_storage(u32 devNum)
{
    if (devNum == 0)
        f_unmount("");

    sdmmc_storage_t* currStor = get_storage_for_index((u8)devNum);
    if (currStor != NULL && currStor->sdmmc != NULL)
    {
        if (!sdmmc_storage_end(currStor, 1))
            dbg_print("sdmmc_storage_end for storage idx %u FAILED!\n", devNum);
        else
            memset(currStor, 0, sizeof(sdmmc_storage_t));
    }

}
static void deinitialize_storages()
{
    for (u32 i=0; i<FF_VOLUMES; i++)
    {
        deinitialize_storage(i);
    }
}


#define USB_EP_BULK_IN_BUF_ADDR   0xFF000000
#define USB_EP_BULK_OUT_BUF_ADDR  0xFF800000
u8* usb_write_buff = (u8*)USB_EP_BULK_IN_BUF_ADDR;
u8* usb_read_buff  = (u8*)USB_EP_BULK_OUT_BUF_ADDR;
static UC_CommandType usb_command_read()
{

    memset(usb_read_buff, 0, 4096);  
    unsigned int bytesTransferred = 0;
    if (!rcm_usb_device_read_ep1_out_sync(usb_read_buff, USB_BUFFER_LENGTH, &bytesTransferred))
    {        

        if (bytesTransferred > 2)
        {
            UC_Header* uc = (UC_Header*)usb_read_buff;
            if (uc->signature == COMMAND || uc->signature == EXEC_COMMAND)
            {
                printk("Received command from USB host\n");
                return uc->command;
            }
        }
    }
    return NONE;
}

// This is a safe and unused DRAM region for our payloads.
#define RELOC_META_OFF      0x7C
#define PATCHED_RELOC_SZ    0x94
#define PATCHED_RELOC_STACK 0x40007000
#define PATCHED_RELOC_ENTRY 0x40010000
#define EXT_PAYLOAD_ADDR    0xC0000000
#define RCM_PAYLOAD_ADDR    (EXT_PAYLOAD_ADDR + ALIGN(PATCHED_RELOC_SZ, 0x10))
#define COREBOOT_END_ADDR   0xD0000000
#define CBFS_DRAM_EN_ADDR   0x4003e000
#define CBFS_DRAM_MAGIC     0x4452414D // "DRAM"
#define EMC_SCRATCH0        0x324
#define  EMC_HEKA_UPD (1 << 30)

static void *coreboot_addr;

typedef struct __attribute__((__packed__)) _reloc_meta_t
{
	u32 start;
	u32 stack;
	u32 end;
	u32 ep;
} reloc_meta_t;

void reloc_patcher(u32 payload_dst, u32 payload_src, u32 payload_size)
{
	memcpy((u8 *)payload_src, (u8 *)IPL_LOAD_ADDR, PATCHED_RELOC_SZ);

	volatile reloc_meta_t *relocator = (reloc_meta_t *)(payload_src + RELOC_META_OFF);

	relocator->start = payload_dst - ALIGN(PATCHED_RELOC_SZ, 0x10);
	relocator->stack = PATCHED_RELOC_STACK;
	relocator->end   = payload_dst + payload_size;
	relocator->ep    = payload_dst;
}

FRESULT f_write_buffer(FIL* fp, const void* buffer, u32 length, u32 *bytesWritten)
{
    int block_size = 4096;
    int btw = 0;
    *bytesWritten = 0;
    FRESULT res;
    while(btw < length)
    {
        char b[block_size]; 
        memcpy(b, buffer + btw, block_size);                                                
        unsigned int bytesWrite = 0;
        int bs = btw + block_size > length ? length - btw : block_size;
        printk("Prepare to write %db to file\n", bs);
        res = f_write(fp, b, bs, &bytesWrite);
        if (res != FR_OK) 
        {
            printk("Res = %d\n", res);
            *bytesWritten = btw;
            return res;
        }
        if (!(bytesWrite > 0))
        {
            printk("ERROR 2 bytesWrite = 0\n");
            return 20;
        }
        btw += bytesWrite;
        printk("%d bytesWriten\n", bytesWrite);
    }      
    *bytesWritten = btw;
    return res;
}

void send_response(const void* in_buffer, u32 size)
{
    u32 new_size = size + sizeof(u16), bytes = 0;
    if (new_size > RESPONSE_MAX_SIZE)
        return;

    u16 signature = RESPONSE;
    memcpy(&usb_write_buff[0], (u8*)&signature, sizeof(u16));
    memcpy(&usb_write_buff[sizeof(u16)], in_buffer, size);
	rcm_usb_device_write_ep1_in_sync(&usb_write_buff[0], new_size, &bytes);    
}

bool recv_bin_packet(u32 *bytesReceived)
{
    //printk("Preparing to receive packet\n");
    memset(&usb_read_buff[0], 0, sizeof(UC_BlockHeader));
    *bytesReceived = 0;
    u32 bytesTransferred = 0;
    if(rcm_usb_device_read_ep1_out_sync(usb_read_buff, USB_BUFFER_LENGTH, &bytesTransferred) && !bytesTransferred)
    {
        printk("Failed to receive packet\n");
        return false;
    }

    UC_BlockHeader *bh = (UC_BlockHeader*)&usb_read_buff[0];

    if (bh->signature != BIN_PACKET)
    {
        printk("Error, did not receive a bin packet from usb pipe\n");
        return false;
    }

    //memcpy(&[buffer[0], &usb_read_buff[sizeof(UC_BlockHeader)]], bh->block_size);

    // Send bloc size in response
    send_response((const void*)&bh->block_size, sizeof(bh->block_size));
    
    *bytesReceived = bh->block_size;
    //printk("Received a packet %db\n", bh->block_size);
    return true;
}

void on_WriteSDFileCommand()
{
    UC_SDIO* uc = (UC_SDIO*)usb_read_buff;
               
    printk("Received a WRITE_SD_FILE for path : %s (%uMB)\n", uc->path, uc->file_size / 1024 / 1024);
    printk("file_size = %u, bytescount = %u\n", uc->file_size, 0); 
    //printk("file_size = %u, bytescount = %u\n", uc->file_size, 0); 
    //printk("Size of UC_SDIO = %ld, sign=%ld, command=%ld, path=%ld, filesize=%ld\n", sizeof(*uc), sizeof(uc->signature), sizeof(uc->command), sizeof(uc->path), sizeof(uc->file_size));
    
    bool lz4compress = uc->is_lz4_compressed;
    FIL fp;
    FRESULT res;
    f_unlink(uc->path);
    res = f_open (&fp, uc->path, FA_WRITE | FA_CREATE_ALWAYS);
    if (res == FR_OK)
    {               
        u32* bytesCount = malloc(sizeof(u32));
        u32* bytesTotal = malloc(sizeof(u32));
        u32* begin_time = malloc(sizeof(u32));
        *bytesCount = 0;
        *bytesTotal = uc->file_size;
        *begin_time = get_tmr_s();
        int numTries = 0;
        numTries = 0;
        char *path = malloc(256);
        memcpy(path, uc->path, 256);              
        u8* buf32 = malloc(64);
        int *buf32_s = malloc(sizeof(int));
        *buf32_s = 0;
        int *pre_pct = malloc(sizeof(int));
        *pre_pct = -1;
        while (*bytesCount < *bytesTotal)
        {
            u32 bytesTransferred = 0;
            if (!recv_bin_packet(&bytesTransferred))
                break;

            //printk("Preparing to write file block\n");
            // Write (uncompressed bin)
            u32 bytesWrite = 0;
            if (f_write(&fp, &usb_read_buff[32], bytesTransferred, &bytesWrite) != FR_OK)
            {
                printk("Failed to write tmp_buffer\n");
                break;
            }
            if (!(bytesWrite > 0))
            {
                printk("ERROR bytesWrite = 0\n");
                break;
            }
            *bytesCount += bytesWrite; 

            //printk("File block written (total %u\n", *bytesCount);

            int pct = (u64)(*bytesCount) * 100 / *bytesTotal;
            if (pct != *pre_pct || *bytesCount >= *bytesTotal) 
            {
                printk("\rDownloading %s - %d%% (%d MB)              ", path, pct, *bytesCount / 1024 / 1024 );
                *pre_pct = pct;
            }

            /*
            //printk("Prepare to receive file\n");
            if(rcm_usb_device_read_ep1_out_sync(usb_read_buff, USB_BUFFER_LENGTH, &bytesTransferred))
            {
                printk("Failed to receive file at off. %u \n", *bytesCount);
                numTries++;
                continue;
            }    
            if (bytesTransferred) 
            {
                unsigned int bytesWrite = 0;
                UC_BlockHeader *bh = (UC_BlockHeader*)&usb_read_buff[0];

                if (bh->signature != BIN_PACKET)
                {
                    printk("Error, did not receive a bin packet from usb pipe");
                    break;
                }

                if (lz4compress) 
                {
                    int bfs = bh->block_full_size;
                    int bs = bh->block_size;          
                    //printk("Received a compress packet. block_size %d, block_full_size %d, max_size %d\n\n", bh->block_size, bh->block_full_size, 1024*1024);
                    
                    u8 *lz4_buffer = malloc(1024*1024);
                    memset(lz4_buffer, 0, 1024*1024);

                    int len = LZ4_decompress_safe((char*)&usb_read_buff[32], (char*)lz4_buffer, bs, 1024*1024);
                    if (len <= 0) 
                    {
                        printk("Error decomp, len is %d", len);
                        break;
                    }
                    if (len != bfs)
                    {
                        printk("Error decomp size %d != bfs %d\n", len, bfs);
                        break;
                    } //else printk("LZ4 decompress done (%db)\n", len);

                    
                    int align_val = 32;
                    // Push back 32b align buffer
                    if(*buf32_s && len < (1024*1024) - *buf32_s)
                    {
                        //printk("Push back 32b align buffer\n");
                        memcpy(&lz4_buffer[len], &buf32[0], *buf32_s);
                        len += *buf32_s;
                        *buf32_s = 0;
                    }

                    // Save 32b align buffer
                    if (len % align_val)
                    {
                        //printk("Save 32b align buffer\n");
                        *buf32_s = len - ((len / align_val) * align_val);
                        memcpy(&buf32[0], &lz4_buffer[len - *buf32_s], *buf32_s);
                        len -= *buf32_s;
                    }

                    if (len % align_val)
                    {
                        printk("Buffer is not 32b aligned\n");
                        break;
                    }

                    // Write                             
                    bytesWrite = 0;
                    if (f_write(&fp, lz4_buffer, len, &bytesWrite) != FR_OK)
                    {
                        printk("Failed to write tmp_buffer\n");
                        break;
                    }
                    if (!(bytesWrite > 0))
                    {
                        printk("ERROR bytesWrite = 0\n");
                        break;
                    }
                    *bytesCount += bytesWrite;     

                    free(lz4_buffer);

                    bytesWrite = 0;
                    if (lz4compress && *bytesCount + *buf32_s > *bytesTotal && *buf32_s && f_write_buffer(&fp, buf32, *buf32_s > 32 ? 64 : 32, &bytesWrite) != FR_OK)
                    {
                        printk("Failed to write tmp_buffer\n");
                        break;
                    }
                    *bytesCount += bytesWrite;     


                }
                else
                {
                    // Write (uncompressed bin)
                    bytesWrite = 0;
                    if (f_write(&fp, &usb_read_buff[32], bh->block_size, &bytesWrite) != FR_OK)
                    {
                        printk("Failed to write tmp_buffer\n");
                        break;
                    }
                    if (!(bytesWrite > 0))
                    {
                        printk("ERROR bytesWrite = 0\n");
                        break;
                    }
                    *bytesCount += bytesWrite;  
                }

                int pct = (u64)(*bytesCount) * 100 / *bytesTotal;
                if (pct != *pre_pct || *bytesCount >= *bytesTotal) 
                {
                    printk("\rDownloading %s - %d%% (%d MB)              ", path, pct, *bytesCount / 1024 / 1024 );
                    *pre_pct = pct;
                }

            }
            else
            {
                printk("Failed to receive file at off. %u \n", *bytesCount);
                break;
            }    
            */                
        }
        printk("\nBytes received. %u (%u seconds)\n", *bytesCount, (get_tmr_s() - *begin_time));
        f_close(&fp);
        free(bytesCount); free(bytesTotal); free(begin_time);
        free(path); free(buf32); free(buf32_s); free(pre_pct);
    }
    else
    {
        printk("Failed to open file. Err %d\n", res);
    }
}

void on_PushPayloadCommand()
{
    printk("Received a push payload command\n");
    video_clear_line();
    UC_EXEC* uc = (UC_EXEC*)usb_read_buff;

    if (uc->command != PUSH_PAYLOAD)
    {
        printk("Wrong PUSH_PAYLOAD command\n");
        video_clear_line();
        return;
    }

    void *buf = (void *)RCM_PAYLOAD_ADDR;
    u32 size = uc->bin_size;
    
    u32 bytesCount = 0;
    printk("Preparing to receive payload, size = %u\n", size);
    video_clear_line();
    while (bytesCount < size)
    {
        u32 bytesTransferred = 0;
        if (!recv_bin_packet(&bytesTransferred))
        {
            printk("Failed to receive file at off. %u \n", bytesCount);
            break;
        }

        memcpy(buf + bytesCount, &usb_read_buff[sizeof(UC_BlockHeader)], bytesTransferred);
        bytesCount += bytesTransferred;
    }    

    if (bytesCount != size)
        return;        

    /*
    uint32_t mrt_time = 2;
    uint8_t reg_val = mrt_time - 2;
    //bit 3..5 is the value we want to set
    reg_val &= 0x7;
    reg_val <<= 3;
    reg_val |= 0x40; //always set normally
    max77620_send_byte(MAX77620_REG_ONOFFCNFG1, reg_val);
    */
    printk("Payload received and copied in RAM\n");
    video_clear_line();
    deinitialize_storages();
    msleep(200);
    //mc_disable_ahb_redirect();
    
    reloc_patcher(PATCHED_RELOC_ENTRY, EXT_PAYLOAD_ADDR, ALIGN(size, 0x10));
    //reconfig_hw_workaround(false, byte_swap_32(*(u32 *)(buf + size - sizeof(u32))));    

    //printk("Launch payload\n");
    display_end();
    
    void (*ext_payload_ptr)() = (void *)EXT_PAYLOAD_ADDR;
    (*ext_payload_ptr)();    

    printk("Failed to launch payload\n");
    clock_halt_bpmp();
    return;
}

void on_RebootRCMCommand()
{
    deinitialize_storages();
    mc_disable_ahb_redirect();
    display_end();

    bool confirm = true;
    send_response((const void*)&confirm, sizeof(bool)); // Notify caller
    msleep(10);

    // Reboot to RCM
    PMC(APBDEV_PMC_SCRATCH0) = (1 << 1);
	PMC(0x0) |= PMC_CNTRL_MAIN_RST;

	while (true)
		clock_halt_bpmp();
}

bool get_autoRCM_state(bool *state)
{
    if(!initialize_mount(NULL, 1))
    {
        printk("Failed to mount EMMC BOOT0\n");
        return false;
    }

    bool bct_state[4];
    bool autoRcmOn[4] = {false, false, false, false};
    sdmmc_storage_t* currStor = get_storage_for_index(1);
    for (int i = 0; i < 4; i++)
    {
        u8 buff[0x200];
        memset(buff, 0, 0x200);
        if (!sdmmc_storage_read(currStor, (0x200 + (0x4000 * i)) / 0x200, 1, &buff[0]))
        {
            printk("Error reading BCT %d from BOOT0.\n", i);
            deinitialize_storage(1);
            return false;
        }

        if (buff[0x10] == 0xF7) bct_state[i] = true;
        else bct_state[i] = false;
    }

    if (!memcmp(autoRcmOn, bct_state, 4))
        *state = true;
    else
        *state = false;

    deinitialize_storage(1);
    return true;
}

bool set_autoRCM_state(bool autoRCM)
{
    if(!initialize_mount(NULL, 1))
    {
        printk("Failed to mount EMMC BOOT0\n");
        return false;
    }
    sdmmc_storage_t* currStor = get_storage_for_index(1);
    for (int i = 0; i < 4; i++)
    {
        u8 buff[0x200];
        memset(buff, 0, 0x200);
        if (!sdmmc_storage_read(currStor, (0x200 + (0x4000 * i)) / 0x200, 1, &buff[0]))
        {
            printk("Error reading BCT %d from BOOT0.\n", i);
            deinitialize_storage(1);
            return false;
        }
        if (!autoRCM)
            buff[0x10] = 0xF7;
        else
        {
            u8 randomXor = 0;
            do randomXor = (unsigned)get_tmr_ms() & 0xFF;
            while (!randomXor);
            buff[0x10] ^= randomXor;
        }
        if (!sdmmc_storage_write(currStor, (0x200 + (0x4000 * i)) / 0x200, 1, &buff[0]))
        {
            printk("Error writing BCT %d from BOOT0.\n", i);
            deinitialize_storage(1);
            return false;
        }
    }
    deinitialize_storage(1);
    return true;
}

void on_getDeviceInfoCommand(FATFS *fs)
{
    // Init a device info struct
    UC_DeviceInfo di;
    di.signature = DEVINFO;

    // AutoRCM
    get_autoRCM_state(&di.autoRCM);

    // Burnt fuses
    di.burnt_fuses = 0;
    for (u32 i = 0; i < 32; i++)
    {
        if ((fuse_read_odm(7) >> i) & 1)
            di.burnt_fuses++;
    }

    // Battery
    int value = 0;
    i2c_recv_buf_small((u8 *)&value, 2, I2C_1, 0x36, 0x06);
    di.battery_capacity = value >> 8;

    // FS info
    if (fs == NULL || fs->fs_type == 0)
    {
        di.sdmmc_initialized = false;
        di.cfw_sxos = false;
        di.cfw_ams = false;
        di.cbl_hekate = false;
    }
    else
    {
        di.sdmmc_initialized = true;
        di.emmc_fs_type = fs->fs_type;
        di.emmc_fs_cl_size = fs->csize;
        di.emmc_fs_last_cl = fs->n_fatent - 2;
        printk("di.emmc_fs_last_cl = %lu\n", emmc_fs_last_cl);
        FATFS *ffs;
        f_getfree("", &di.emmc_fs_free_cl, &ffs);
        printk("di.emmc_fs_free_cl = %lu\n", di.emmc_fs_free_cl);
        FILINFO fno;
        di.cfw_sxos = (f_stat("boot.dat", &fno) == FR_OK);
        di.cbl_hekate = (f_stat("bootloader/hekate_ipl.ini", &fno) == FR_OK);
        di.cfw_ams = (f_stat("BCT.ini", &fno) == FR_OK)
                  || (f_stat("atmosphere/BCT.ini", &fno) == FR_OK)
                  || (f_stat("atmosphere/config/BCT.ini", &fno) == FR_OK);

        /*
         *  ToDo:
         *  - emunand detection
         *  - fw detection (sys + emu)
         *  - get Nintendo device ID
         *  - check for ipatched switch
         */

    }


    u32 bytes;
    rcm_usb_device_write_ep1_in_sync((u8*)&di, sizeof(UC_DeviceInfo), &bytes);
}

int main(void) 
{

    u32* lfb_base;      

    config_hw();

    // Pivot the stack so we have enough space.
	pivot_stack(IPL_STACK_TOP);

    //Tegra/Horizon configuration goes to 0x80000000+, package2 goes to 0xA9800000, we place our heap in between.
	heap_init(IPL_HEAP_START);    
    //heap_init(0x90020000);

    // Train DRAM and switch to max frequency.
	//minerva_init();

    display_enable_backlight(false);
    display_init();

    // Set up the display, and register it as a printk provider.
    lfb_base = display_init_framebuffer();
    video_init(lfb_base);
    
    //Init the CBFS memory store in case we are booting coreboot
    cbmem_initialize_empty();

    mc_enable_ahb_redirect();

    /* Turn on the backlight after initializing the lfb */
    /* to avoid flickering. */
    display_enable_backlight(true);

    if (!rcm_usb_device_ready()) //somehow usb isn't initialized    
        printk("ERROR RCM USB not initialized, press the power button to turn off the system.\n");

    //u8 *holdingBuffer = (void*)malloc(USB_BUFFER_LENGTH*2);
    //usbBuffer = (void*)ALIGN_UP((u32)&holdingBuffer[0], USB_BUFFER_LENGTH);    
    printk("ARIANE\n");
    
    FATFS fs;
    memset(&fs, 0, sizeof(FATFS));
    if(!initialize_mount(&fs, 0))
    {
        printk("Failed to initialise SD Card\n");
        memset(&fs, 0, sizeof(FATFS));
    }

    // AutoRCM
    bool autoRCM;
    if (get_autoRCM_state(&autoRCM))
        printk("autoRCM state : %s\n", autoRCM ? "ON" : "OFF");

    // Fuses
    u32 burntFuses = 0;
    for (u32 i = 0; i < 32; i++)
    {
        if ((fuse_read_odm(7) >> i) & 1)
            burntFuses++;
    }
    printk("Burnt Fuses : %u\n", burntFuses);

    // Battery
    int value = 0;
    i2c_recv_buf_small((u8 *)&value, 2, I2C_1, 0x36, 0x06);
    printk("Battery: %3d%\n", value >> 8);

    static const char READY_NOTICE[] = "READY.\n";   
    unsigned int bytesTransferred = 0;

    while(1)
    {
        if (btn_read() == BTN_POWER)
        {
            printk("EXIT\n");
            goto progend;
        }
        
        /*
         *  ToDo
         *  -> Detect USB unplug
         */
        
        // Dispatcher
        switch (usb_command_read())
        {
            case GET_STATUS :
                rcm_ready_notice();
                break;
            case WRITE_SD_FILE :
                on_WriteSDFileCommand();
                break;
            
            case PUSH_PAYLOAD :
                on_PushPayloadCommand();
                break;
                
            case REBOOT_RCM :
                on_RebootRCMCommand();
                break;

            case GET_DEVICE_INFO:
                on_getDeviceInfoCommand(&fs);
                break;
        }
    }
    
progend_user:
    printk("Press POWER button to exit\n");
    while(1) { if (btn_read() == BTN_POWER) goto progend;}

progend:
    // Tell the PMIC to turn everything off
    shutdown_using_pmic();

    /* Do nothing for now */
    return 0;
}
