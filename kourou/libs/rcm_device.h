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

#ifndef RCMDEVICE_H
#define RCMDEVICE_H

#include <windows.h>
#include <setupapi.h>
#include <stdio.h>
#include <tchar.h>
#include "libusbk_int.h"
#include <string>
#include "../../types.h"

#define PACKET_SIZE 0x1000
#define PAYLOAD_MAX_SIZE 0x1ED58

static int RCM_VID = 0x0955;
static int RCM_PID = 0x7321;
static UCHAR READ_PIPE_ID = 0x81;
static UCHAR WRITE_PIPE_ID = 0x01;

typedef enum _RESULT : DWORD
{
    SUCCESS                 = 0x000,
    LIBUSBK_WRONG_DRIVER    = 0x001,
    DEVICE_NOT_FOUND        = 0x002,
    DEVICE_HANDLE_FAILED    = 0x003,
    MISSING_LIBUSBK_DRIVER  = 0x004,
    OPEN_FILE_FAILED        = 0x005,
    PAYLOAD_TOO_LARGE       = 0x006,
    USB_WRITE_FAILED        = 0x007,
    SW_HIGHBUFF_FAILED      = 0x008,
    STACK_SMASH_FAILED      = 0x009,
    DEVICEID_READ_FAILED    = 0x00A,
    DEVICE_NOT_READY        = 0x00B,
    DEVICE_NOT_SET          = 0x00C,
    WRONG_DEVICE_VID_PID    = 0x00D,
    DEVICE_DISCONNECTED     = 0x00E

} RRESULT;

typedef enum _DEVICE_STATUS {
    DISCONNECTED,
    CONNECTED
} DEVICE_STATUS;

class RcmDevice
{
public:
    //! Rcm device constructor
    RcmDevice();
    ~RcmDevice();
    ////////////////////////////////////////////
    /////          PUBLIC METHODS          /////
    ////////////////////////////////////////////

    /*!
     * \brief Initialize a RCM device. This function must be called at least on time before any other method call
     * \param[in] deviceInfo
     * Pointer to a KLST_DEVINFO struct. If not provided, the system will try to detect a plugged RCM device (a.k.a Nintendo Switch in RCM mode)
     *
     * \returns
     * - on success, true.
     * - on failure, false (use GetLastError() to get error code)
     */
    bool initDevice(KLST_DEVINFO_HANDLE deviceInfo = nullptr);

    /*!
     * \brief Constructs a RCM payload and smash the device stack (fusée gelée exploit)
     * \param payload_path
     * Path to the user payload file to build final payload with
     * \returns
     * - on success, SUCCESS (0)
     * - on failure, see enum RRESULT
     */
    RRESULT hack(const char* payload_path);
    /*!
     * \brief Constructs a RCM payload and smash the device stack (fusée gelée exploit)
     * \param payload_buff
     * Pointer to the payload data
     * \param buff_size
     * Payload size
     * \returns
     * - on success, SUCCESS (0)
     * - on failure, see enum RRESULT
     */
    RRESULT hack(u8 *payload_buff, u32 buff_size);

    /*!
     * \brief Read a buffer from USB pipe
     * \param buffer
     * \param bufferSize
     * \returns
     * - on success, number of bytes read from pipe
     * - on failure, error code < 0
     */
    int read(u8* buffer, size_t bufferSize);

    /*!
     * \brief Write a buffer to USB pipe
     * \param buffer
     * \param bufferSize
     * \returns
     * - on success, number of bytes written to pipe
     * - on failure, error code < 0
     */
    int write(const u8* buffer, size_t bufferSize);

    /*!
     * \brief Check whether the device is ready to receive RCM commands
     * \return true if device is ready
     */
    bool deviceIsReady();

    //! Get the RCM device status (CONNECTED / DISCONNECTED)
    DEVICE_STATUS getStatus() { return m_devStatus; }
    //! Set the RCM device status to DISCONNECTED
    void disconnect() { m_devStatus = DISCONNECTED; }

    // API Getters
    bool flushPipe(UCHAR pipeId) { return m_usbApi.FlushPipe(m_usbHandle, pipeId); }

    // Reset the current buffer to lower one. Use this with caution.
    // Current buffer should not be reset manually, except after a controlled RCM reboot or disconnection.
    // disconnect() method should be preferred
    void resetCurrentBuffer() { m_currentBuffer = 0; }

private:
    bool m_is_devicePlugged;
    KLST_DEVINFO_HANDLE m_devInfo = nullptr;
    KUSB_HANDLE m_usbHandle;
    KUSB_DRIVER_API m_usbApi;
    KLST_HANDLE m_devList = nullptr;
    u32 m_currentBuffer = 0;
    bool m_devIsInitialized = false;
    bool m_usbAPI_loaded = false;
    DEVICE_STATUS m_devStatus;

private:    
    bool getPluggedDevice(KLST_DEVINFO_HANDLE *devinfo);
    int switchToHighBuffer();
    u32 getCurrentBufferAddress() const { return m_currentBuffer == 0 ? 0x40005000u : 0x40009000u; }
    int Ioctl(DWORD ioctlCode, const void* inputBytes, size_t numInputBytes, void* outputBytes, size_t numOutputBytes);
};

class WinHandle
{
public:
    WinHandle(HANDLE srcHandle = INVALID_HANDLE_VALUE) noexcept : _handle(srcHandle) {}
    ~WinHandle()
    {
        if (_handle != INVALID_HANDLE_VALUE)
        {
            ::CloseHandle(_handle);
            _handle = INVALID_HANDLE_VALUE;
        }
    }
    void swap(WinHandle&& other) noexcept { std::swap(_handle, other._handle); }

    WinHandle(WinHandle&& moved) noexcept : _handle(INVALID_HANDLE_VALUE) { swap(std::move(moved)); }
    WinHandle(const WinHandle& copied) = delete;

    WinHandle& operator=(WinHandle&& moved) { swap(std::move(moved)); return *this; }
    WinHandle& operator=(const WinHandle& copied) = delete;

    HANDLE get() const { return _handle; }
    HANDLE release() { HANDLE retHandle = _handle; _handle = INVALID_HANDLE_VALUE; return retHandle; }
protected:
    HANDLE _handle;
};

#endif // RCMDEVICE_H
