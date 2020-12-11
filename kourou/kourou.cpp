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

int Kourou::hack(const char* payload_path)
{
    if (!arianeIsReady())
        return RcmDevice::hack(payload_path);

    ifstream userPayload(payload_path, ios::in | ios::binary | ios::ate);

    if (!userPayload.is_open())
        return OPEN_FILE_FAILED;

    const auto userPayloadSize = int(userPayload.tellg());
    if (userPayloadSize > PAYLOAD_MAX_SIZE)
        return PAYLOAD_TOO_LARGE;

    userPayload.seekg(0, ios::beg);
    char *userPayloadBuffer = new char[userPayloadSize];
    userPayload.read(&userPayloadBuffer[0], userPayloadSize);
    bool error = !(userPayload) || int(userPayload.tellg()) != userPayloadSize;
    userPayload.close();
    if (error)
    {
        delete[] userPayloadBuffer;
        return OPEN_FILE_FAILED;
    }

    int res = hack((u8*)userPayloadBuffer, (u32)userPayloadSize);

    delete[] userPayloadBuffer;
    return res;
}

int Kourou::hack(u8 *payload_buff, u32 buff_size)
{
    if (!arianeIsReady())
        return RcmDevice::hack(payload_buff, buff_size);

    UC_EXEC uc;
    uc.command = PUSH_PAYLOAD;
    uc.bin_size = buff_size;

    if (write((const u8*)&uc, sizeof(uc)) != sizeof(uc))
        return USB_WRITE_FAILED;

    // Get response
    bool response = false;
    if (!readResponse(&response, sizeof(bool)) && !response)
        return USB_WRITE_FAILED;

    if (sendBinPackets((char*)payload_buff, buff_size) != buff_size)
        return USB_WRITE_FAILED;

    m_ariane_ready = false;

    return SUCCESS;
}

void Kourou::disconnect()
{
    RcmDevice::disconnect();
    m_rcm_ready = false;
    m_ariane_ready = false;
    memset(&m_di, 0, sizeof(UC_DeviceInfo));
    m_di_set = false;
}

bool Kourou::sdmmc_isDir(const char* path)
{
    if (path == nullptr)
        return false;

    UC_SDIO uc;
    uc.command = ISDIR_SD;
    strcpy_s(uc.path, array_countof(uc.path), path);

    if (write((const u8*)&uc, sizeof(uc)) != sizeof(uc))
        return false;

    u32 res = 0;
    if (!readResponse(&res, sizeof(u32)))
        return false;

    return res == SUCCESS ? true : false;
}

bool Kourou::sdmmc_mkDir(const char* path)
{
    if (path == nullptr)
        return false;

    UC_SDIO uc;
    uc.command = MKDIR_SD;
    strcpy_s(uc.path, array_countof(uc.path), path);

    if (write((const u8*)&uc, sizeof(uc)) != sizeof(uc))
        return false;

    u32 res = 0;
    if (!readResponse(&res, sizeof(u32)))
        return false;

    return res == SUCCESS ? true : false;
}

bool Kourou::sdmmc_mkPath(const char* path)
{
    if (path == nullptr)
        return false;

    UC_SDIO uc;
    uc.command = MKPATH_SD;
    strcpy_s(uc.path, array_countof(uc.path), path);

    if (write((const u8*)&uc, sizeof(uc)) != sizeof(uc))
        return false;

    u32 res = 0;
    if (!readResponse(&res, sizeof(u32)))
        return false;

    return res == SUCCESS ? true : false;
}

u32 Kourou::sdmmc_fileSize(const char* path)
{
    if (path == nullptr)
        return 0;

    UC_SDIO uc;
    uc.command = SIZE_SD_FILE;
    strcpy_s(uc.path, array_countof(uc.path), path);

    int bytesSent = write((const u8*)&uc, sizeof(uc));
    if (bytesSent != sizeof(uc))
        return 0;

    u32 size = 0;
    if (!readResponse(&size, sizeof(u32)))
        return 0;

    return size;
}

int Kourou::sdmmc_readFile(const char* path, Bytes *dest, u32 *bytesRead)
{
    if (path == nullptr)
        return BAD_ARGUMENT;

    UC_SDIO uc;
    if (strlen(path) > array_countof(uc.path)-1)
        return PATH_TOO_LONG;

    if (dest->size())
        dest->clear();

    uc.command = READ_SD_FILE;
    strcpy_s(uc.path, array_countof(uc.path), path);

    // Send command to device
    int bytesSent = write((const u8*)&uc, sizeof(uc));
    if (bytesSent != sizeof(uc))
        return SEND_COMMAND_FAILED;

    u32 file_size = 0;
    if (!readResponse(&file_size, sizeof(u32)))
        return USB_WRITE_FAILED;

    if (!file_size)
        return OPEN_FILE_FAILED;

    u8 usb_buffer[USB_BUFFER_LENGTH];
    UC_BlockHeader *bh = (UC_BlockHeader*)&usb_buffer[0];
    u32 offset = 0;

    while (offset < file_size)
    {
        memset(&usb_buffer[0], 0, USB_BUFFER_LENGTH);
        if (read(usb_buffer, USB_BUFFER_LENGTH) != USB_BUFFER_LENGTH)
            return USB_READ_FAILED;

        if (bh->signature != BIN_PACKET || !bh->block_size)
            return USB_READ_FAILED;

        dest->insert(dest->end(),  &usb_buffer[32],  &usb_buffer[32] + bh->block_size);
        offset += bh->block_size;
        bool res = true;
        sendResponse(&res, sizeof(res));
    }

    if (nullptr != bytesRead)
        *bytesRead = offset;

    return offset == file_size ? SUCCESS : USB_READ_FAILED;
}

int Kourou::sdmmc_readFile(const char* path, u8 *buffer, u32 size, u32 *bytesRead)
{
    if (path == nullptr || !size)
        return BAD_ARGUMENT;

    UC_SDIO uc;
    if (strlen(path) > array_countof(uc.path)-1)
        return PATH_TOO_LONG;

    uc.command = READ_SD_FILE;
    strcpy_s(uc.path, array_countof(uc.path), path);

    // Send command to device
    int bytesSent = write((const u8*)&uc, sizeof(uc));
    if (bytesSent != sizeof(uc))
        return SEND_COMMAND_FAILED;

    u32 file_size = 0;
    if (!readResponse(&file_size, sizeof(u32)))
        return USB_WRITE_FAILED;

    if (!file_size)
        return OPEN_FILE_FAILED;

    if (file_size > size)
        return BUFFER_TOO_SMALL;

    u8 usb_buffer[USB_BUFFER_LENGTH];
    u32 offset = 0;
    UC_BlockHeader *bh = (UC_BlockHeader*)&usb_buffer[0];
    while (offset < file_size)
    {
        memset(&usb_buffer[0], 0, USB_BUFFER_LENGTH);
        if (read(usb_buffer, USB_BUFFER_LENGTH) != USB_BUFFER_LENGTH)
            return USB_READ_FAILED;

        if (bh->signature != BIN_PACKET || !bh->block_size)
            return USB_READ_FAILED;

        memcpy(&buffer[offset], &usb_buffer[32], bh->block_size);
        offset += bh->block_size;
        bool res = true;
        sendResponse(&res, sizeof(res));
    }

    if (nullptr != bytesRead)
        *bytesRead = offset;

    bool res = USB_READ_FAILED;
    readResponse(&res, sizeof(bool));

    return res ? SUCCESS : USB_READ_FAILED;
}

int Kourou::

sdmmc_writeFile(Bytes *in_bytes, const char *out_path, bool create_always)
{
    UC_SDIO uc;

    if (strlen(out_path) > array_countof(uc.path)-1)
        return PATH_TOO_LONG;

    if (!in_bytes->size())
        return OPEN_FILE_FAILED;

    if (in_bytes->size() > MAX_FILE_SIZE)
        return FILE_TOO_LARGE;

    uc.command = WRITE_SD_FILE;
    memset(uc.path, 0, array_countof(uc.path));
    uc.create_always = create_always;
    strcpy_s(uc.path, array_countof(uc.path), out_path);
    uc.file_size = in_bytes->size();

    // Send command to device
    int bytesSent = write((const u8*)&uc, sizeof(uc));
    if (bytesSent != sizeof(uc))
        return SEND_COMMAND_FAILED;

    return sendBinPackets((char*)in_bytes->data(), uc.file_size);
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
    uc.command = WRITE_SD_FILE;
    memset(uc.path, 0, array_countof(uc.path));    
    uc.create_always = create_always;
    strcpy_s(uc.path, array_countof(uc.path), out_path);
    uc.file_size = file.tellg();
    file.seekg(0, ios::beg);

    // Allocate memory for file
    fileMemBlock = new char[uc.file_size];
    file.read(fileMemBlock, uc.file_size);

    // Send command to device
    int bytesSent = write((const u8*)&uc, sizeof(uc));
    if (bytesSent != sizeof(uc))
        return end(SEND_COMMAND_FAILED);

    //Sleep(100); // Make sure Ariane has enough time to open file

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

        //if (write((const u8*)&buf[0], fbs) != int(fbs))
        int res = write((const u8*)&buf[0], USB_BUFFER_LENGTH);
        if (res != USB_BUFFER_LENGTH)
            return USB_WRITE_FAILED;

        // Get confirmation
        u32 bytesReceived = 0;
        if (!readResponse(&bytesReceived, sizeof(u32)))
            return USB_WRITE_FAILED;
        if (bytesReceived != bh.block_size)
            return USB_WRITE_FAILED;

        bytesRemaining -= bh.block_size;
        curOffset += bh.block_size;
    }
    return int(len - bytesRemaining);
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

bool Kourou::sendResponse(void* buffer, u32 size)
{
    u32 res_size = RESPONSE_MAX_SIZE - sizeof(u16);
    u8 tmp_buffer[RESPONSE_MAX_SIZE];
    if (size > res_size)
        return false;
    else res_size = size + sizeof(u16);

    u16* signature = (u16*)tmp_buffer;
    *signature = RESPONSE;
    memcpy(&tmp_buffer[sizeof(u16)], buffer, size);

    return write(tmp_buffer, res_size) == int(res_size);
}

bool Kourou::arianeIsReady_sync(bool skip_rcm)
{
    auto end = [&](bool ready) {
        m_ariane_ready = ready;
        return ready;
    };

    if (!skip_rcm)
    {
        if (RcmDevice::deviceIsReady())
        {
            m_rcm_ready = true;
            return end(false);
        }
        else m_rcm_ready = false;
    }

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
    if (di != nullptr) memset(di, 0, sizeof(UC_DeviceInfo));
    UC_Header uc;
    uc.command = GET_DEVICE_INFO;
    if(write((const u8*)&uc, sizeof(uc)) != sizeof(uc))
        return SEND_COMMAND_FAILED;

    int res = read((u8*)(di != nullptr ? di : &m_di), sizeof(UC_DeviceInfo));
    if(res != sizeof(UC_DeviceInfo))
        return RECEIVE_COMMAND_FAILED;

    if (di->signature != DEVINFO)
        return RECEIVE_COMMAND_FAILED;

    // Save device info
    if (di != nullptr) memcpy(&m_di, di, sizeof(UC_DeviceInfo));
    m_di_set = true;

    return SUCCESS;
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
