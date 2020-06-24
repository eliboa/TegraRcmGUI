#ifndef RCMDEVICE_H
#define RCMDEVICE_H

#include <windows.h>
#include "libusbk_int.h"
#include "../types.h"
#include <string>

#define PACKET_SIZE 0x1000

static int RCM_VID = 0x0955;
static int RCM_PID = 0x7321;

typedef enum _RESULT
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
    DEVICEID_READ_FAILED    = 0x00A

} RRESULT;

class RcmDevice
{

////////////////////////////////////////////
/////          PUBLIC METHODS          /////
////////////////////////////////////////////
public:
//! Rcm device constructor
    RcmDevice();

    /*!
     * \brief Initialize a RCM device. This function must be called at least on time before any other method call
     * \param[in] deviceInfo
     * Pointer to a KLST_DEVINFO struct. If not provided, the system will try to detect a plugged RCM device (a.k.a Nintendo Switch in RCM mode)
     *
     * \returns
     * - on success, true.
     * - on failure, false.
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
     * \brief Checks whether the device is ready or not
     * \return true if device is ready
     */
    bool isDeviceReady() { return m_b_RcmReady; };

// member variables
private:
    bool m_is_devicePlugged;
    KLST_DEVINFO_HANDLE m_devInfo = nullptr;
    KUSB_HANDLE m_usbHandle;
    KUSB_DRIVER_API m_usbApi;
    KLST_HANDLE m_devList = nullptr;
    u32 m_currentBuffer = 0;
    bool m_b_RcmReady;

// private methods
private:
    RRESULT loadDevice();
    bool getPluggedDevice();
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
