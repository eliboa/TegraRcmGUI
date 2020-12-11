/*
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

/*
 * Kourou class is derived from RcmDevice class
 * Provides additionnal functions to communicate with device with Ariane (backend payload based program)
 */

#ifndef KOUROU_H
#define KOUROU_H
#include "libs/rcm_device.h"
#include "usb_command.h"

typedef enum _KRESULT : DWORD
{
    RCM_REBOOT_FAILED       = 0x100,
    ARIANE_NOT_READY        = 0x101,
    WRONG_PARAM_GENERIC     = 0x102,
    FAILED_TO_SET_AUTORCM   = 0x103,
    FAILED_TO_MKDIR         = 0x104,
    SD_FILE_WRITE_FAILED    = 0x105

} KRESULT;

class Kourou : public RcmDevice
{
public:
    Kourou();
    bool initDevice(KLST_DEVINFO_HANDLE deviceInfo = nullptr);
    int  hack(const char* payload_path);
    int  hack(u8 *payload_buff, u32 buff_size);
    void disconnect();
    bool readResponse(void* buffer, u32 size);
    bool sendResponse(void* buffer, u32 size);
    u32  sdmmc_fileSize(const char* path);
    bool sdmmc_isDir(const char* path);
    bool sdmmc_mkDir(const char* path);
    bool sdmmc_mkPath(const char* path);
    int  sdmmc_readFile(const char* path, u8 *buffer, u32 size, u32 *bytesRead);
    int  sdmmc_readFile(const char* path, Bytes *dest, u32 *bytesRead = nullptr);
    int  sdmmc_writeFile(const char* in_path, const char* out_path, bool create_always = false);
    int  sdmmc_writeFile(Bytes *in_bytes, const char *out_path, bool create_always = false);
    int  pushPayload(const char* path);
    int  getDeviceInfo(UC_DeviceInfo* di);
    bool arianeIsReady() { return m_ariane_ready; }
    bool rcmIsReady() { return m_rcm_ready; }
    void setRcmReady(bool b) { m_rcm_ready = b; }
    bool arianeIsReady_sync(bool skip_rcm = false);
    void setArianeReady(bool b) { m_ariane_ready = b; }
    bool rebootToRcm();
    bool setAutoRcmEnabled(bool state);
    bool isReadyToReceivePayload() { return (m_rcm_ready || m_ariane_ready); }
    bool isDeviceInfoAvailable() { return m_di_set; }
    UC_DeviceInfo deviceInfo() { return m_di; }

private:    
    bool m_ariane_ready = false;
    bool m_rcm_ready = false;
    UC_DeviceInfo m_di;
    bool m_di_set = false;

    int sendBinPackets(char* buffer, u32 len);

};

#endif // KOUROU_H
