/*
 * Ariane, payload based USB backend
 *
 * Copyright (c) 2020 eliboa
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

#include "libs/hekate_config.h"
extern hekate_config h_cfg;
extern boot_cfg_t b_cfg;
extern emummc_cfg_t emu_cfg;

#include "ariane.h"
#include "gfx/gfx.h"
#include "storage/nx_emmc.h"
#include "storage/nx_emmc_bis.h"
#include "libs/hos/hos.h"
#include "libs/hos/sept.h"
#include "libs/hos/pkg1.h"

#include <string.h>
#include <stdlib.h>
#include <gfx/di.h>
#include <gfx_utils.h>
#include <soc/t210.h>
#include <soc/i2c.h>
#include <soc/bpmp.h>
#include <soc/fuse.h>
#include <soc/hw_init.h>
#include <storage/nx_sd.h>
#include <storage/sdmmc.h>
#include <memory_map.h>
#include <mem/heap.h>
#include <mem/minerva.h>
#include <mem/sdram.h>
#include <utils/ini.h>
#include <utils/dirlist.h>
#include <usb/usbd.h>

#include "storage/emummc.h"

bool keygen_done = false;
bool is_init = true;
extern bool usb_ready;
UC_DeviceInfo di;
u8* usb_write_buff = (u8*)USB_EP_BULK_IN_BUF_ADDR;
u8* usb_read_buff  = (u8*)USB_EP_BULK_OUT_BUF_ADDR;

const pkg1_id_t *sys_pkg1_id;
const pkg1_id_t *emu_pkg1_id;

static DispatchEntry dispatchEntries[] =
{
    { GET_STATUS, &rcm_ready_notice },
    { GET_DEVICE_INFO, &get_deviceinfo_command },
    { SET_AUTORCM_ON, &set_autorcm_on_command },
    { SET_AUTORCM_OFF, &set_autorcm_off_command },
    { READ_SD_FILE, &sdmmc_readfile_command },
    { WRITE_SD_FILE, &sdmmc_writefile_command },
    { SIZE_SD_FILE, &sdmmc_filesize_command },
    { REBOOT_RCM, &rcm_reboot_command },
    { GET_KEYS, &get_keys_command },
    { ISDIR_SD, &sdmmc_isdir_command },
    { MKDIR_SD, &sdmmc_mkdir_command },
    { MKPATH_SD, &sdmmc_mkpath_command },
    { PUSH_PAYLOAD, &launch_payload_command }
};

// Title ID 0100000000000809 (SystemVersion)
// Title ID 010000000000081B (BootImagePackageExFat)
static SystemTitle systemTitles[] =
{
    { "10.1.1", "5077973537f6735b564dd7475b779f87.nca", 10, "3df13daa7f553c8fa85bbff79a189d6c.nca"}, /* CHINA */
    { "10.1.0", "fd1faed0ca750700d254c0915b93d506.nca", 10, "3df13daa7f553c8fa85bbff79a189d6c.nca"},
    { "10.0.4", "34728c771299443420820d8ae490ea41.nca", 10, "d5bc167565842ee61f9670d23759844d.nca"},
    { "10.0.3", "5b1df84f88c3334335bbb45d8522cbb4.nca", 10, "d5bc167565842ee61f9670d23759844d.nca"},
    { "10.0.2", "e951bc9dedcd54f65ffd83d4d050f9e0.nca", 10, "d5bc167565842ee61f9670d23759844d.nca"},
    { "10.0.1", "36ab1acf0c10a2beb9f7d472685f9a89.nca", 10, "d5bc167565842ee61f9670d23759844d.nca"},
    { "10.0.0", "5625cdc21d5f1ca52f6c36ba261505b9.nca", 10, "d5bc167565842ee61f9670d23759844d.nca"},
    { "9.2.0",  "09ef4d92bb47b33861e695ba524a2c17.nca", 10, "2416b3794964b3482c7bc506d12c44df.nca"},
    { "9.1.0",  "c5fbb49f2e3648c8cfca758020c53ecb.nca", 10, "c9bd4eda34c91a676de09951bb8179ae.nca"},
    { "9.0.1",  "fd1ffb82dc1da76346343de22edbc97c.nca", 9 , "3b444768f8a36d0ddd85635199f9676f.nca"},
    { "9.0.0",  "a6af05b33f8f903aab90c8b0fcbcc6a4.nca", 9 , "3b444768f8a36d0ddd85635199f9676f.nca"},
    { "8.1.1",  "e9bb0602e939270a9348bddd9b78827b.nca", 8 , "96f4b8b729ade072cc661d9700955258.nca"}, /* 8.1.1-12  from chinese gamecard */
    { "8.1.1",  "724d9b432929ea43e787ad81bf09ae65.nca", 8 , "96f4b8b729ade072cc661d9700955258.nca"}, /* 8.1.1-100 from Lite */
    { "8.1.0",  "7eedb7006ad855ec567114be601b2a9d.nca", 8 , "96f4b8b729ade072cc661d9700955258.nca"},
    { "8.0.1",  "6c5426d27c40288302ad616307867eba.nca", 7 , "b2708136b24bbe206e502578000b1998.nca"},
    { "8.0.0",  "4fe7b4abcea4a0bcc50975c1a926efcb.nca", 7 , "b2708136b24bbe206e502578000b1998.nca"},
    { "7.0.1",  "e6b22c40bb4fa66a151f1dc8db5a7b5c.nca", 7 , "02a2cbfd48b2f2f3a6cec378d20a5eff.nca"},
    { "7.0.0",  "c613bd9660478de69bc8d0e2e7ea9949.nca", 7 , "58c731cdacb330868057e71327bd343e.nca"},
    { "6.2.0",  "6dfaaf1a3cebda6307aa770d9303d9b6.nca", 6 , "97cb7dc89421decc0340aec7abf8e33b.nca"},
    { "6.1.0",  "1d21680af5a034d626693674faf81b02.nca", 5 , "d5186022d6080577b13f7fd8bcba4dbb.nca"},
    { "6.0.1",  "663e74e45ffc86fbbaeb98045feea315.nca", 5 , "d5186022d6080577b13f7fd8bcba4dbb.nca"},
    { "6.0.0",  "258c1786b0f6844250f34d9c6f66095b.nca", 5 , "d5186022d6080577b13f7fd8bcba4dbb.nca"},
    { "6.0.0",  "286e30bafd7e4197df6551ad802dd815.nca", 5 , "711b5fc83a1f07d443dfc36ba606033b.nca"},
    { "5.1.0",  "fce3b0ea366f9c95fe6498b69274b0e7.nca", 4 , "c9e500edc7bb0fde52eab246028ef84c.nca"},
    { "5.0.2",  "c5758b0cb8c6512e8967e38842d35016.nca", 4 , "432f5cc48e6c1b88de2bc882204f03a1.nca"},
    { "5.0.1",  "7f5529b7a092b77bf093bdf2f9a3bf96.nca", 4 , "432f5cc48e6c1b88de2bc882204f03a1.nca"},
    { "5.0.0",  "faa857ad6e82f472863e97f810de036a.nca", 4 , "432f5cc48e6c1b88de2bc882204f03a1.nca"},
    { "4.1.0",  "77e1ae7661ad8a718b9b13b70304aeea.nca", 3 , "458a54253f9e49ddb044642286ca6485.nca"},
    { "4.0.1",  "d0e5d20e3260f3083bcc067483b71274.nca", 3 , "090b012b110973fbdc56a102456dc9c6.nca"},
    { "4.0.0",  "f99ac61b17fdd5ae8e4dda7c0b55132a.nca", 3 , "090b012b110973fbdc56a102456dc9c6.nca"},
    { "3.0.2",  "704129fc89e1fcb85c37b3112e51b0fc.nca", 2 , "e7dd3c6cf68953e86cce54b69b333256.nca"},
    { "3.0.1",  "9a78e13d48ca44b1987412352a1183a1.nca", 2 , "17f9864ce7fe3a35cbe3e3b9f6185ffb.nca"},
    { "3.0.0",  "7bef244b45bf63efb4bf47a236975ec6.nca", 1 , "9e5c73ec938f3e1e904a4031aa4240ed.nca"},
    { "2.3.0",  "d1c991c53a8a9038f8c3157a553d876d.nca", 0 , "4a94289d2400b301cbe393e64831f84c.nca"},
    { "2.2.0",  "7f90353dff2d7ce69e19e07ebc0d5489.nca", 0 , "4a94289d2400b301cbe393e64831f84c.nca"},
    { "2.1.0",  "e9b3e75fce00e52fe646156634d229b4.nca", 0 , "4a94289d2400b301cbe393e64831f84c.nca"},
    { "2.0.0",  "7a1f79f8184d4b9bae1755090278f52c.nca", 0 , "f55a04978465ebf5666ca93e21b26dd2.nca"},
    { "1.0.0",  "a1b287e07f8455e8192f13d0e45a2aaf.nca", 0 , "3b7cd379e18e2ee7e1c6d0449d540841.nca"}
};

bool command_dispatcher(UC_CommandType command)
{
    for (u32 i = 0; i < ARRAY_SIZE(dispatchEntries); i++) if (dispatchEntries[i].command == command)
    {
        dispatchEntries[i].f_ptr();
        return true;
    }
    return false;
}

u32 cur_backlight;
u32 last_activity;
UC_CommandType usb_command_read()
{
    UC_CommandType command = NONE;

    if (!cur_backlight)
        cur_backlight = h_cfg.backlight;

    memset(usb_read_buff, 0, USB_BUFFER_LENGTH);
    unsigned int bytesTransferred = 0;
    int ret = rcm_usb_device_read_ep1_out_sync(usb_read_buff, USB_BUFFER_LENGTH, &bytesTransferred);
    if (ret || !(bytesTransferred > 2))
    {
        if (ret == 28) // Reconnection
            reboot_to_rcm();

        goto end;
    }
    UC_Header* uc = (UC_Header*)usb_read_buff;
    if (uc->signature == COMMAND || uc->signature == EXEC_COMMAND)
    {
        command = uc->command;
        if (command != GET_DEVICE_INFO && command != GET_STATUS)
        {
            h_cfg.backlight = 100;
            last_activity = get_tmr_s();
        }
    }

end:
    // Lower backlight brightness when inactive
    if (command == NONE && cur_backlight == 100 && last_activity + 40 < get_tmr_s())
        h_cfg.backlight = 15;

    // Switch brightness
    if (cur_backlight != h_cfg.backlight)
    {
        display_backlight_brightness(h_cfg.backlight, h_cfg.backlight == 15 ? 1000 : 0);
        cur_backlight = h_cfg.backlight;
    }
    return command;
}

void reboot_to_rcm()
{
    sd_end();

    memset(&b_cfg, 0, sizeof(b_cfg));
    memset(&h_cfg, 0, sizeof(h_cfg));
    reconfig_hw_workaround(false, 0);

    // Reboot to RCM
    PMC(0x50) = (1 << 1);
    PMC(0x0) |= (1 << 4);

    while (true)
        bpmp_halt();
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
    memset(&usb_read_buff[0], 0, sizeof(UC_BlockHeader));
    *bytesReceived = 0;
    u32 bytesTransferred = 0;
    if(rcm_usb_device_read_ep1_out_sync(usb_read_buff, USB_BUFFER_LENGTH, &bytesTransferred) || !bytesTransferred)
        return false;

    UC_BlockHeader *bh = (UC_BlockHeader*)&usb_read_buff[0];
    if (bh->signature != BIN_PACKET)
        return false;

    // Send bloc size in response
    send_response((const void*)&bh->block_size, sizeof(bh->block_size));
    *bytesReceived = bh->block_size;
    return true;
}

void hos_set_keys(bool allow_sept_reboot)
{
    // Read package1.
    u8 *pkg1 = (u8 *)malloc(0x40000);
    sdmmc_storage_init_mmc(&emmc_storage, &emmc_sdmmc, SDMMC_BUS_WIDTH_8, SDHCI_TIMING_MMC_HS400);
    sdmmc_storage_set_mmc_partition(&emmc_storage, EMMC_BOOT0);
    sdmmc_storage_read(&emmc_storage, 0x100000 / NX_EMMC_BLOCKSIZE, 0x40000 / NX_EMMC_BLOCKSIZE, pkg1);
    char *build_date = malloc(32);
    sys_pkg1_id = pkg1_identify(pkg1, build_date);
    if (!sys_pkg1_id)
    {
        gfx_printf("Failed to identify pkg1\n");
        goto out;
    }

    tsec_ctxt_t tsec_ctxt;
    tsec_ctxt.fw = (u8 *)pkg1 + sys_pkg1_id->tsec_off;
    tsec_ctxt.pkg1 = pkg1;
    tsec_ctxt.pkg11_off = sys_pkg1_id->pkg11_off;
    tsec_ctxt.secmon_base = sys_pkg1_id->secmon_base;

    hos_eks_get(); // Get keys
    if (sys_pkg1_id->kb >= KB_FIRMWARE_VERSION_700 && !h_cfg.sept_run)
    {
        u32 key_idx = 0;
        if (sys_pkg1_id->kb >= KB_FIRMWARE_VERSION_810)
            key_idx = 1;

        if (h_cfg.eks && h_cfg.eks->enabled[key_idx] >= sys_pkg1_id->kb)
            h_cfg.sept_run = true;
        else
        {
            if (!allow_sept_reboot || b_cfg.extra_cfg & EXTRA_CFG_NYX_BIS)
            {
                gfx_printf("Need to launch Sept first (kb = %d)\n", sys_pkg1_id->kb);
                goto out;
            }
            else
            {
                b_cfg.extra_cfg = EXTRA_CFG_NYX_BIS;
                sdmmc_storage_end(&emmc_storage);
                sdram_lp0_save_params(sdram_get_params_patched());
                if (!reboot_to_sept((u8 *)tsec_ctxt.fw, sys_pkg1_id->kb))
                {
                    gfx_printf("Failed to run sept\n");
                    goto out;
                }
            }
        }
    }

    // Read the correct keyblob.
    u8 *keyblob = (u8 *)calloc(NX_EMMC_BLOCKSIZE, 1);
    sdmmc_storage_read(&emmc_storage, 0x180000 / NX_EMMC_BLOCKSIZE + sys_pkg1_id->kb, 1, keyblob);

    // Derive & save hos keys
    if (hos_bis_keygen(keyblob, sys_pkg1_id->kb, &tsec_ctxt) == 1)
    {
        hos_eks_bis_save();
        keygen_done = true;
        gfx_printf("hos_eks_bis_save()\n");

        for (u32 i = 0; i < 0x10; i++)
        {
            gfx_printf("%02x", h_cfg.eks->bis_keys[3].crypt[i]);
        }
        gfx_printf("\n");

    }
    else gfx_printf("Failed to generate keys\n");

    free(keyblob);
out:
    sdmmc_storage_end(&emmc_storage);
    free(pkg1);
}

void set_fs_info()
{
    if (!sdmounted())
    {
        di.sdmmc_initialized = false;
        return;
    }

    di.sdmmc_initialized = true;
    di.mmc_fs_type = sd_fs.fs_type;
    di.mmc_fs_cl_size = sd_fs.csize;
    di.mmc_fs_last_cl = sd_fs.n_fatent - 2;

    if (sd_fs.fs_type == FS_EXFAT)
        f_getfree("", &di.mmc_fs_free_cl, NULL); // This operation can be slow for FAT32

    di.cfw_sxos = (f_stat("boot.dat", NULL) == FR_OK);
    di.cbl_hekate = (f_stat("bootloader/hekate_ipl.ini", NULL) == FR_OK);


    FIL fp;
    if (f_open(&fp, "bootloader/sys/nyx.bin", FA_READ) == FR_OK)
    {
        di.cbl_nyx = true;
        u32 bytesTransferred = 0;
        u8 buff[0x200];
        memset(di.nyx_version, 0, 3);
        if (f_read(&fp, &buff, 0x200, &bytesTransferred) == FR_OK && bytesTransferred == 0x200 && !memcmp(&buff[0x99], "CTC", 3))
            memcpy(di.nyx_version, &buff[0x9C], 3);
        f_close(&fp);
    }

    if (f_open(&fp, "atmosphere/fusee-secondary.bin", FA_READ) == FR_OK)
    {
        di.cfw_ams = true;
        u32 bytesTransferred = 0;
        char buff[0x200];
        memset(di.ams_version, 0, 3);
        if (f_read(&fp, &buff, 0x200, &bytesTransferred) == FR_OK) {
            for (int i=0; i <= 0x200-4; i += 4) if (!memcmp(&buff[i], "FSS0", 4)) {
                memcpy(&di.ams_version[2], &buff[i+0x19], 1);
                memcpy(&di.ams_version[1], &buff[i+0x1A], 1);
                memcpy(&di.ams_version[0], &buff[i+0x1B], 1);
                break;
            }
        }
        f_close(&fp);
    }

    // Get emunand config
    emummc_load_cfg();
    if (emu_cfg.enabled)
    {
        di.emunand_enabled = true;
        di.emunand_type = FILEBASED;
        strcpy(emu_cfg.emummc_file_based_path, emu_cfg.path);
        strcat(emu_cfg.emummc_file_based_path, "/raw_based");
        if (!f_open(&fp, emu_cfg.emummc_file_based_path, FA_READ))
        {
            if (!f_read(&fp, &emu_cfg.sector, 4, NULL))
                if (emu_cfg.sector)
                    di.emunand_type = RAWBASED;
        }
    }
}

void get_fw_version(const char *path, const pkg1_id_t *pkg1_id, char *fw_version, bool *exfat)
{
    /*
    char *filelist = NULL;
    // Check NCAs to get sys firmware version
    for (u32 i=0; i < ARRAY_SIZE(systemTitles); i++) if (systemTitles[i].kb == pkg1_id->kb)
    {

        gfx_printf("Testing %s\n", systemTitles[i].nca_filename);
        minerva_periodic_training();
        filelist = dirlist(path, systemTitles[i].nca_filename, true, true);
        if (filelist && !strcmp(&filelist[0], systemTitles[i].nca_filename))
        {
            strcpy(fw_version, systemTitles[i].fw_version);

            filelist = dirlist(path, systemTitles[i].nca_filename_exfat, true, true);
            if (!strcmp(&filelist[0], systemTitles[i].nca_filename_exfat))
                *exfat = true;

            gfx_printf("FOUND FW %s %s\n", fw_version, *exfat ? "exFAT" : "");
            break;
        }
    }

    if (filelist)
        free(filelist);
    */

    DIR dir;
    FILINFO fno;
    bool entry_found = false;
    u32 title_entry = 0;
    if(f_opendir(&dir, path))
        return;

    // Look for title ID 0100000000000809 (SystemVersion)
    gfx_printf("Look for title ID 0100000000000809\n");
    for (;;)
    {
        int res = f_readdir(&dir, &fno);
        if (res || !fno.fname[0] || entry_found)
            break;

        if (fno.fname[0] == '.')
            continue;

        for (u32 i=0; i < ARRAY_SIZE(systemTitles); i++) if (systemTitles[i].kb == pkg1_id->kb)
        {
            if (!strcmp(fno.fname, systemTitles[i].nca_filename))
            {
                strcpy(fw_version, systemTitles[i].fw_version);
                entry_found = true;
                title_entry = i;
                break;
            }
        }
    }

    f_closedir(&dir);

    if (!entry_found)
        return;

    // Look for title ID 010000000000081B (BootImagePackageExFat)
    gfx_printf("Look for title ID 010000000000081B\n");
    f_opendir(&dir, path);
    for (;;)
    {
        int res = f_readdir(&dir, &fno);
        if (res || !fno.fname[0])
            break;

        if (fno.fname[0] == '.')
            continue;

        if (!strcmp(fno.fname, systemTitles[title_entry].nca_filename_exfat))
        {
            *exfat = true;
            break;
        }
    }

    f_closedir(&dir);
}

void set_deviceinfo()
{
    /*
     *  ToDo:
     *  - emunand type detection
     *  - get Nintendo device ID
     */

    u32 start_tmr = get_tmr_ms();
    memset(&di, 0, sizeof(UC_DeviceInfo));
    di.signature = DEVINFO;

    // AutoRCM
    get_autorcm_state(&di.autoRCM);

    // Burnt fuses
    di.burnt_fuses = 0;
    for (u32 i = 0; i < 32; i++) if ((fuse_read_odm(7) >> i) & 1)
        di.burnt_fuses++;

    // Battery
    int value = 0;
    i2c_recv_buf_small((u8 *)&value, 2, I2C_1, 0x36, 0x06);
    di.battery_capacity = value >> 8;

    // Set sdmmc FS info
    set_fs_info();

    if (!sd_mount())
        goto out;

    // Generate keys
    hos_set_keys(FORBID_SEPT_REBOOT);

    if (!keygen_done || !sys_pkg1_id)
    {
        gfx_printf("KEYGEN NOT DONE\n");
        return;
    }
    // Get sys fw version & exFat driver    
    sdmmc_storage_init_mmc(&emmc_storage, &emmc_sdmmc, SDMMC_BUS_WIDTH_8, SDHCI_TIMING_MMC_HS400);
    sdmmc_storage_set_mmc_partition(&emmc_storage, EMMC_GPP);
    LIST_INIT(gpt);
    nx_emmc_gpt_parse(&gpt, &emmc_storage);
    emmc_part_t *system = nx_emmc_part_find(&gpt, "SYSTEM");
    if (!system)
    {
        gfx_printf("SYSTEM not found ! \n");
        goto out;
    }

    h_cfg.emummc_force_disable = true;
    nx_emmc_bis_init(system);
    if(f_mount(&emmc_fs, "bis:", 1))
    {
        gfx_printf("Failed to mount SYSTEM\n");
        goto out;
    }

    get_fw_version("bis:/Contents/registered", sys_pkg1_id, &di.fw_version[0], &di.exFat_driver);

    gfx_printf("di.fw_version = %s %s\n", di.fw_version, di.exFat_driver ? " exFat" : "");

    // Unmount cur bis storage
    system = NULL;
    f_mount(NULL, "bis:", 1);

    // Retrieve device id from CAL0
    emmc_part_t *cal0 = nx_emmc_part_find(&gpt, "PRODINFO");
    if (cal0)
    {
        nx_emmc_bis_init(cal0);
        u8 buffer[0x200];
        if (nx_emmc_bis_read(2, 1, &buffer) == SUCCESS)
        {
            memcpy(&di.deviceId, &buffer[0x144], 20);
            gfx_printf("NX ID = %s\n", di.deviceId);
        }
    }

    sdmmc_storage_end(&emmc_storage);

    if (!emu_cfg.enabled || di.emunand_type == FILEBASED) // Don't look up for emu fw if filebased emunand (too slow)
        goto out;

    gfx_printf("Read emunand\n");

    // Read emu pkg1
    sdmmc_storage_end(&emmc_storage);
    u8 *pkg1 = (u8 *)malloc(0x40000);
    h_cfg.emummc_force_disable = false;
    emummc_storage_init_mmc(&emmc_storage, &emmc_sdmmc);
    emummc_storage_set_mmc_partition(&emmc_storage, EMMC_BOOT0);
    emummc_storage_read(&emmc_storage, 0x100000 / NX_EMMC_BLOCKSIZE, 0x40000 / NX_EMMC_BLOCKSIZE, pkg1);
    //gfx_printf("emu pkg1 read\n");
    char *build_date = malloc(32);
    emu_pkg1_id = pkg1_identify(pkg1, build_date);
    free(pkg1);
    if (!emu_pkg1_id)
    {
        gfx_printf("Failed to read/id emu pkg1\n");
        goto out;
    }
    gfx_printf("emu pkg1 identify %d\n", emu_pkg1_id->kb);

    // Mount emu bis storage
    emummc_storage_set_mmc_partition(&emmc_storage, EMMC_GPP);
    LIST_INIT(gpt_emu);
    nx_emmc_gpt_parse(&gpt_emu, &emmc_storage);
    system = nx_emmc_part_find(&gpt_emu, "SYSTEM");
    if (!system)
    {
        gfx_printf("EMU SYSTEM not found ! \n");
        nx_emmc_gpt_free(&gpt_emu);
        goto out_emu;
    }       

    nx_emmc_bis_init(system);
    if(f_mount(&emmc_fs, "bis:", 1))
    {
        gfx_printf("EMU Failed to mount SYSTEM");
        goto out_emu;
    }

    get_fw_version("bis:/Contents/registered", emu_pkg1_id, &di.emu_fw_version[0], &di.emu_exFat_driver);
    gfx_printf("di.emu_fw_version = %s %s\n", di.emu_fw_version, di.emu_exFat_driver ? " exFat" : "");

    f_mount(NULL, "bis:", 1);

out_emu :
    nx_emmc_gpt_free(&gpt_emu);

out :
    nx_emmc_gpt_free(&gpt);
    sdmmc_storage_end(&emmc_storage);
}

bool get_autorcm_state(bool *state)
{
    sdmmc_storage_t storage;
    sdmmc_t sdmmc;

    if (!sdmmc_storage_init_mmc(&storage, &sdmmc, SDMMC_BUS_WIDTH_8, SDHCI_TIMING_MMC_HS400))
    {
        gfx_printf("Failed to mount EMMC BOOT0\n");
        return false;
    }

    bool bct_state[4];
    bool autoRcmOn[4] = {false, false, false, false};
    for (int i = 0; i < 4; i++)
    {
        u8 buff[0x200];
        memset(buff, 0, 0x200);
        if (!sdmmc_storage_read(&storage, (0x200 + (0x4000 * i)) / 0x200, 1, &buff[0]))
        {
            sdmmc_storage_end(&storage);
            return false;
        }

        if (buff[0x10] == 0xF7) bct_state[i] = true;
        else bct_state[i] = false;
    }

    *state = !memcmp(autoRcmOn, bct_state, 4) ? true : false;
    sdmmc_storage_end(&storage);
    return true;
}

bool set_autorcm_state(bool autoRCM)
{
    sdmmc_storage_t storage;
    sdmmc_t sdmmc;

    if (!sdmmc_storage_init_mmc(&storage, &sdmmc, SDMMC_BUS_WIDTH_8, SDHCI_TIMING_MMC_HS400))
    {
        gfx_printf("Failed to mount EMMC BOOT0\n");
        return false;
    }
    for (int i = 0; i < 4; i++)
    {
        u8 buff[0x200];
        memset(buff, 0, 0x200);
        if (!sdmmc_storage_read(&storage, (0x200 + (0x4000 * i)) / 0x200, 1, &buff[0]))
        {
            sdmmc_storage_end(&storage);
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
        if (!sdmmc_storage_write(&storage, (0x200 + (0x4000 * i)) / 0x200, 1, &buff[0]))
        {
            sdmmc_storage_end(&storage);
            return false;
        }
    }
    sdmmc_storage_end(&storage);
    return true;
}

void get_deviceinfo_command()
{
    if (!is_init)
    {
        // Update battery charge
        int value = 0;
        i2c_recv_buf_small((u8 *)&value, 2, I2C_1, 0x36, 0x06);
        di.battery_capacity = value >> 8;

        // Update FS info
        set_fs_info();
    }

    //gfx_printf("Received get_deviceinfo_command %s\n", di.fw_version);

    u32 bytes = 0;
    rcm_usb_device_write_ep1_in_sync((u8*)&di, sizeof(UC_DeviceInfo), &bytes);

    is_init = false;
}

void sdmmc_filesize_command()
{
    UC_SDIO* uc = (UC_SDIO*)usb_read_buff;
    u32 res = 0;
    FIL fp;

    if (f_open(&fp, uc->path, FA_READ) == FR_OK)
        res = f_size(&fp);

    send_response((const void*)&res, sizeof(u32)); // Notify caller
}

void sdmmc_isdir_command()
{
    UC_SDIO* uc = (UC_SDIO*)usb_read_buff;
    DIR dir;

    FRESULT res = f_opendir(&dir, uc->path);
    if (res == FR_OK)
        f_closedir(&dir);

    send_response((const void*)&res, sizeof(u32)); // Notify caller
}

void sdmmc_mkdir_command()
{
    UC_SDIO* uc = (UC_SDIO*)usb_read_buff;
    DIR dir;

    FRESULT res = f_opendir(&dir, uc->path);
    if (res != FR_OK)
        res = f_mkdir(uc->path);
    else f_closedir(&dir);

    send_response((const void*)&res, sizeof(u32)); // Notify caller
}

void sdmmc_mkpath_command()
{

    UC_SDIO* uc = (UC_SDIO*)usb_read_buff;
    DIR dir;

    FRESULT res = f_opendir(&dir, uc->path);
    if (res != FR_OK)
    {
        char *cdir = malloc(ARRAY_SIZE(uc->path));
        memcpy(cdir, uc->path, ARRAY_SIZE(uc->path));
        char *cur_path = malloc(ARRAY_SIZE(uc->path));
        char *dir;
        int pos = 0;

        while ((dir = strtok(cdir, "/")) != NULL)
        {
            free(cdir);
            cdir = NULL;
            memcpy(&cur_path[pos], &dir[0], strlen(dir)+1);
            pos += strlen(dir)+1;
            DIR s_dir;
            res = f_opendir(&s_dir, cur_path);
            if (res == FR_OK) {
                f_closedir(&s_dir);
            } else {
                res = f_mkdir(cur_path);
                if (res != FR_OK)
                    break;
            }
            cur_path[pos-1] = '/';
        }
        free(cur_path);
    }
    else f_closedir(&dir);
    send_response((const void*)&res, sizeof(u32)); // Notify caller
}

void sdmmc_readfile_command()
{
    u32 start_tmr = get_tmr_ms();
    UC_SDIO* uc = (UC_SDIO*)usb_read_buff;
    FIL fp;

    if (f_open(&fp, uc->path, FA_READ) != FR_OK)
        goto out;

    u32 size = f_size(&fp);
    u32 bytesRemaining = size;

    send_response((const void*)&size, sizeof(u32)); // Notify caller
    while (bytesRemaining > 0)
    {
        minerva_periodic_training();

        u32 buf_size = bytesRemaining > USB_BUFFER_LENGTH - 32 ? USB_BUFFER_LENGTH - 32 : bytesRemaining;
        u32 bytesTransferred = 0;
        memset(&usb_read_buff[0], 0, USB_BUFFER_LENGTH);

        UC_BlockHeader *bh = (UC_BlockHeader*)&usb_write_buff[0];
        bh->signature = BIN_PACKET;
        bh->block_size = buf_size;

        if (f_read(&fp, &usb_write_buff[32], buf_size, &bytesTransferred) != FR_OK)
            goto out;

        if (bytesTransferred != buf_size)
            goto out;

        bytesTransferred = 0;
        if (rcm_usb_device_write_ep1_in_sync(&usb_write_buff[0], USB_BUFFER_LENGTH, &bytesTransferred) && !bytesTransferred)
            goto out;

        // Read confirm
        memset(&usb_read_buff[0], 0, sizeof(u16) + sizeof(bool));
        bytesTransferred = 0;
        if(rcm_usb_device_read_ep1_out_sync(usb_read_buff, sizeof(u16) + sizeof(bool), &bytesTransferred) && !bytesTransferred)
            goto out;

        u16 *signature = (u16*)usb_read_buff;
        bool *res = (bool*)&usb_read_buff[sizeof(u16)];
        if (*signature != RESPONSE || *res != true)
            goto out;

        bytesRemaining -= buf_size;
    }

out:
    //gfx_printf("readfile_command ends in %dms\n", get_tmr_ms() - start_tmr);
    f_close(&fp);
}

void sdmmc_writefile_command()
{
    UC_SDIO* uc = (UC_SDIO*)usb_read_buff;
    FIL fp;

    if (f_open (&fp, uc->path, uc->create_always ? FA_WRITE | FA_CREATE_ALWAYS : FA_WRITE | FA_CREATE_NEW) != FR_OK)
        goto out;

    u32 fileSize = uc->file_size;
    u32 bytesRemaining = fileSize;

    while (bytesRemaining > 0)
    {
        minerva_periodic_training();
        u32 bytesTransferred = 0;
        if (!recv_bin_packet(&bytesTransferred))
            break;

        u32 bytesWrite = 0;
        if (f_write(&fp, &usb_read_buff[32], bytesTransferred, &bytesWrite) != FR_OK || bytesWrite <= 0)
            break;

        bytesRemaining -= bytesWrite;
    }

out:
    f_close(&fp);
}

void rcm_reboot_command()
{
    bool confirm = true;
    send_response((const void*)&confirm, sizeof(bool)); // Notify caller

    reboot_to_rcm();
}

void set_autorcm_on_command()
{
    bool res = set_autorcm_state(true);
    send_response((const void*)&res, sizeof(bool)); // Notify caller
}

void set_autorcm_off_command()
{
    bool res = set_autorcm_state(true);
    send_response((const void*)&res, sizeof(bool)); // Notify caller
}

void get_keys_command()
{
    hos_set_keys(ALLOW_SEPT_REBOOT);
}

extern void reloc_patcher(u32 payload_dst, u32 payload_src, u32 payload_size);

void launch_payload_command()
{
    UC_EXEC* uc = (UC_EXEC*)usb_read_buff;
    u32 bytesCount = 0;

    if (uc->command != PUSH_PAYLOAD)
        goto out;

    void *buf = (void *)RCM_PAYLOAD_ADDR;
    u32 size = uc->bin_size;

    bool confirm = true;
    send_response((const void*)&confirm, sizeof(bool)); // Notify caller

    while (bytesCount < size)
    {
        minerva_periodic_training();
        u32 bytesTransferred = 0;
        if (!recv_bin_packet(&bytesTransferred))
            break;

        memcpy(buf + bytesCount, &usb_read_buff[32], bytesTransferred);
        bytesCount += bytesTransferred;
    }

    if (bytesCount != size)
        goto out;

    sd_end();

    memset(&b_cfg, 0, sizeof(b_cfg));
    memset(&h_cfg, 0, sizeof(h_cfg));

    reloc_patcher(PATCHED_RELOC_ENTRY, EXT_PAYLOAD_ADDR, ALIGN(size, 0x10));
    reconfig_hw_workaround(false, 0);

    sdmmc_storage_init_wait_sd();

    void (*ext_payload_ptr)() = (void *)EXT_PAYLOAD_ADDR;
    (*ext_payload_ptr)();

out:
    gfx_printf("launch_payload_command failed (%db)\n", bytesCount);
    return;
}
