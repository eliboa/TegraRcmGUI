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
 * Kourou is a C++ child class derived from base class "RcmDevice"
 * This class provides additionnal functions for communicating with Ariane (backend payload based program)
 *
 */

#ifndef KOUROU_H
#define KOUROU_H
#include "libs/rcm_device.h"
#include "usb_command.h"

typedef enum _KRESULT : DWORD
{
    RCM_REBOOT_FAILED       = 0x00F,
    ARIANE_NOT_READY        = 0x010,
    WRONG_PARAM_GENERIC     = 0x011,
    FAILED_TO_SET_AUTORCM   = 0x012

} KRESULT;

class Kourou : public RcmDevice
{
public:
    Kourou();
    bool initDevice(KLST_DEVINFO_HANDLE deviceInfo = nullptr);
    void disconnect();

    int sdmmc_writeFile(const char* in_path, const char* out_path, bool create_always = false);
    int getDeviceInfo(UC_DeviceInfo* di);
    bool arianeIsReady() { return m_ariane_ready; }
    bool rcmIsReady() { return m_rcm_ready; }
    void setRcmReady(bool b) { m_rcm_ready = b; }
    bool arianeIsReady_sync();
    void setArianeReady(bool b) { m_ariane_ready = b; }
    bool rebootToRcm();
    bool setAutoRcmEnabled(bool state);

private:
    int sendBinPackets(char* buffer, u32 len);
    bool readResponse(void* buffer, u32 size);    
    bool m_ariane_ready = false;
    bool m_rcm_ready = false;
};

#endif // KOUROU_H
