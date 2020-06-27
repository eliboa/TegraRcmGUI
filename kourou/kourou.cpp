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

#include "kourou.h"
#include <iostream>
#include <fstream>

using namespace std;

Kourou::Kourou()
{

}

bool Kourou::initDevice(KLST_DEVINFO_HANDLE deviceInfo)
{
    m_rcm_ready = RcmDevice::initDevice(deviceInfo);
    m_ariane_ready = false;

    // If RCM device is not ready, check if Ariane is already loaded
    if (!m_rcm_ready && getStatus() == CONNECTED)
    {
        m_ariane_ready = arianeIsReady_sync();
        return m_ariane_ready;
    }
    return m_rcm_ready;
}

void Kourou::disconnect()
{
    RcmDevice::disconnect();
    m_rcm_ready = false;
    m_ariane_ready = false;
}

int Kourou::sdmmc_writeFile(const char* in_path, const char* out_path, bool create_always)
{
    char* fileMemBlock = nullptr;
    ifstream file(in_path, ios::in | ios::binary | ios::ate);
    UC_SDIO uc;
    auto end = [&] (int rc) {
        if(file.is_open()) file.close();
        if (fileMemBlock != nullptr) delete[] fileMemBlock;
        return rc;
    };

    if (strlen(out_path) > array_countof(uc.path)-1)
        return PATH_TOO_LONG;

    if (!file.is_open())
        return OPEN_FILE_FAILED;

    // Check file size
    if (file.tellg() > MAX_FILE_SIZE) // 100MB max
        return end(FILE_TOO_LARGE);

    // Create command
    memset(uc.path, 0, array_countof(uc.path));
    uc.command = WRITE_SD_FILE;
    strcpy_s(uc.path, array_countof(uc.path), out_path);
    uc.file_size = file.tellg();
    file.seekg(0, ios::beg);

    // Send command to device
    int bytesSent = write((const u8*)&uc, sizeof(uc));
    if (bytesSent != sizeof(uc))
        return end(SEND_COMMAND_FAILED);

    // Allocate memory for file
    fileMemBlock = new char[uc.file_size];
    file.read(fileMemBlock, uc.file_size);

    return end(sendBinPackets(fileMemBlock, uc.file_size));
}

int Kourou::sendBinPackets(char* buffer, u32 len)
{
    u32 curOffset = 0, bytesRemaining = len;
    u8 buf[USB_BUFFER_LENGTH];
    u32 dataBufSize = USB_BUFFER_LENGTH - 32;
    while(bytesRemaining > 0)
    {
        memset(buf, 0, USB_BUFFER_LENGTH);
        UC_BlockHeader bh;
        bh.block_size = bytesRemaining > dataBufSize ? dataBufSize : bytesRemaining;
        bh.block_full_size = 0; // no compression
        memcpy_s(&buf[0], sizeof(UC_BlockHeader), &bh, sizeof(UC_BlockHeader));
        memcpy_s(&buf[32], bh.block_size, &buffer[curOffset], bh.block_size);
        u32 fbs = bh.block_size + 32;

        if (write((const u8*)&buf[0], fbs) != int(fbs))
            return USB_WRITE_FAILED;

        // Get confirmation
        u32 bytesReceived = 0;
        if (!readResponse(&bytesReceived, sizeof(u32)))
            return USB_WRITE_FAILED;
        if (bytesReceived != bh.block_size)
            return USB_WRITE_FAILED;

        bytesRemaining -= bh.block_size;
    }
    return int(len);
}

bool Kourou::readResponse(void* buffer, u32 size)
{
    u32 res_size = RESPONSE_MAX_SIZE - sizeof(u16);
    u8 tmp_buffer[RESPONSE_MAX_SIZE - sizeof(u16)];
    if (size > res_size)
        return false;
    else res_size = size + sizeof(u16);

    if (read(tmp_buffer, res_size) != int(res_size))
        return false;

    u16* signature = (u16*)tmp_buffer;
    if (*signature != RESPONSE)
        return false;

    memcpy(buffer, &tmp_buffer[sizeof(u16)], size);
    return true;
}

bool Kourou::arianeIsReady_sync()
{
    auto end = [&](bool ready) {
        m_ariane_ready = ready;
        return ready;
    };

    if (RcmDevice::deviceIsReady())
    {
        m_rcm_ready = true;
        return end(false);
    }
    else m_rcm_ready = false;

    u8 buff[0x10];
    static const char READY_INDICATOR[] = "READY.\n";
    int bytesRead = 0;

    flushPipe(READ_PIPE_ID);

    UC_Header uc;
    uc.command = GET_STATUS;
    if (write((const u8*)&uc, sizeof(uc)) != sizeof(uc))
        return end(false);

    if ((bytesRead = read(&buff[0], 0x10)) > 0 && memcmp(&buff[0], READY_INDICATOR, array_countof(READY_INDICATOR) - 1) == 0)
        return end(true);

    return end(false);
}

int Kourou::getDeviceInfo(UC_DeviceInfo* di)
{
    UC_Header uc;
    uc.command = GET_DEVICE_INFO;
    if(write((const u8*)&uc, sizeof(uc)) != sizeof(uc))
        return SEND_COMMAND_FAILED;

    if(read((u8*)di, sizeof(UC_DeviceInfo)) != sizeof(UC_DeviceInfo))
        return RECEIVE_COMMAND_FAILED;

    if (di->signature != DEVINFO)
        return RECEIVE_COMMAND_FAILED;

    return 0;
}

bool Kourou::rebootToRcm()
{
    if (!arianeIsReady_sync())
        return false;

    UC_Header uc;
    uc.command = REBOOT_RCM;
    // Send command
    write((const u8*)&uc, sizeof(uc));

    // Get response
    bool response = false;
    if (!readResponse(&response, sizeof(bool)))
        return false;

    return response;
}

bool Kourou::setAutoRcmEnabled(bool state)
{
    if (!arianeIsReady_sync())
        return false;

    UC_Header uc;
    uc.command = state ? SET_AUTORCM_ON : SET_AUTORCM_OFF;
    // Send command
    write((const u8*)&uc, sizeof(uc));

    // Get response
    bool response = false;
    if (!readResponse(&response, sizeof(bool)))
        return false;

    return response;

}
