/*
 *   TegraRcmShash.cpp (by rajkosto)
 *	 A reimplementation of fusee-launcher by ktemkin in C++ for Windows platforms.
 *	 https://github.com/rajkosto/TegraRcmSmash
 *
 *
 */

#include "stdafx.h"
#include "TegraRcmSmash.h"
class RCMDeviceHacker
{
public:
	RCMDeviceHacker(KUSB_DRIVER_API& usbDriver_, KUSB_HANDLE usbHandle_) : usbHandle(usbHandle_), usbDriver(&usbDriver_), totalWritten(0), currentBuffer(0) {}
	~RCMDeviceHacker()
	{
		if (usbHandle != nullptr)
		{
			usbDriver->Free(usbHandle);
			usbHandle = nullptr;
		}
	}

	static constexpr u32 PACKET_SIZE = 0x1000;

	int getDriverVersion(libusbk::version_t& outVersion)
	{
		HANDLE masterHandle = INVALID_HANDLE_VALUE;
		if (!libusbk_getInternals(usbHandle, &masterHandle) || masterHandle == nullptr || masterHandle == INVALID_HANDLE_VALUE)
			return -int(ERROR_INVALID_HANDLE);

		libusbk::libusb_request myRequest;
		memset(&myRequest, 0, sizeof(myRequest));

		const auto retVal = BlockingIoctl(masterHandle, libusbk::LIBUSB_IOCTL_GET_VERSION, &myRequest, sizeof(myRequest), &myRequest, sizeof(myRequest));
		if (retVal > 0)
			outVersion = myRequest.version;

		return retVal;
	}
	int read(u8* outBuf, size_t outBufSize)
	{
		UINT lengthTransferred = 0;
		const auto retVal = usbDriver->ReadPipe(usbHandle, 0x81, outBuf, (UINT)outBufSize, &lengthTransferred, nullptr);
		if (retVal == FALSE)
			return -int(GetLastError());
		else
			return int(lengthTransferred);
	}
	int write(const u8* data, size_t dataLen, size_t packetSize = PACKET_SIZE)
	{
		int bytesRemaining = (int)dataLen;
		size_t bytesWritten = 0;
		while (bytesRemaining > 0)
		{
			const size_t bytesToWrite = (bytesRemaining < (int)packetSize) ? bytesRemaining : (int)packetSize;
			const auto retVal = writeSingleBuffer(&data[bytesWritten], bytesToWrite);
			if (retVal < 0)
				return retVal;
			else if (retVal < (int)bytesToWrite)
				return int(bytesWritten) + retVal;

			bytesWritten += retVal;
			bytesRemaining -= retVal;
		}

		return (int)bytesWritten;
	}
	int readDeviceId(u8* deviceIdBuf, size_t idBufSize)
	{
		if (idBufSize < 0x10)
			return -int(ERROR_INSUFFICIENT_BUFFER);

		return read(deviceIdBuf, 0x10);
	}
	int switchToHighBuffer()
	{
		if (currentBuffer == 0)
		{
			u8 tempZeroDatas[PACKET_SIZE];
			memset(tempZeroDatas, 0, sizeof(tempZeroDatas));

			const auto writeRes = write(tempZeroDatas, sizeof(tempZeroDatas));
			if (writeRes < 0)
				return writeRes;

			assert(currentBuffer != 0);
			return writeRes;
		}
		else
			return 0;
	}
	int smashTheStack(int length = -1)
	{
		constexpr u32 STACK_END = 0x40010000;

		if (length < 0)
			length = STACK_END - getCurrentBufferAddress();

		if (length < 1)
			return 0;

		HANDLE masterHandle = INVALID_HANDLE_VALUE;
		if (!libusbk_getInternals(usbHandle, &masterHandle) || masterHandle == nullptr || masterHandle == INVALID_HANDLE_VALUE)
			return -int(ERROR_INVALID_HANDLE);

		libusbk::libusb_request rawRequest;
		memset(&rawRequest, 0, sizeof(rawRequest));
		rawRequest.timeout = 1000; //ms
		rawRequest.status.index = 0;
		rawRequest.status.recipient = 0x02; //RECIPIENT_ENDPOINT

		ByteVector threshBuf(length, 0);
		const auto retVal = BlockingIoctl(masterHandle, libusbk::LIBUSB_IOCTL_GET_STATUS, &rawRequest, sizeof(rawRequest), &threshBuf[0], threshBuf.size());
		if (retVal < 0)
		{
			const auto theError = -retVal;
			if (theError == ERROR_SEM_TIMEOUT) //timed out, which means it probably smashed
				return (int)threshBuf.size();

			return theError;
		}
		else
			return retVal;
	}
protected:
	u32 getCurrentBufferAddress() const
	{
		return (currentBuffer == 0) ? 0x40005000u : 0x40009000u;
	}
	u32 toggleBuffer()
	{
		const auto prevBuffer = currentBuffer;
		currentBuffer = (currentBuffer == 0) ? 1u : 0u;
		return prevBuffer;
	}
	int writeSingleBuffer(const u8* data, size_t dataLen)
	{
		toggleBuffer();

		UINT lengthTransferred = 0;
		const auto retVal = usbDriver->WritePipe(usbHandle, 0x01, (u8*)data, (UINT)dataLen, &lengthTransferred, nullptr);
		if (retVal == FALSE)
			return -int(GetLastError());
		else
			return (int)lengthTransferred;
	}

	static int BlockingIoctl(HANDLE driverHandle, DWORD ioctlCode, const void* inputBytes, size_t numInputBytes, void* outputBytes, size_t numOutputBytes)
	{
		WinHandle theEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
		if (theEvent.get() == nullptr || theEvent.get() == INVALID_HANDLE_VALUE)
			return false;

		OVERLAPPED overlapped;
		memset(&overlapped, 0, sizeof(overlapped));
		if (DeviceIoControl(driverHandle, ioctlCode, (LPVOID)inputBytes, (DWORD)numInputBytes, (LPVOID)outputBytes, (DWORD)numOutputBytes, nullptr, &overlapped) == FALSE)
		{
			const auto errCode = GetLastError();
			if (errCode != ERROR_IO_PENDING)
				return -int(errCode);
		}

		DWORD bytesReceived = 0;
		if (GetOverlappedResult(driverHandle, &overlapped, &bytesReceived, TRUE) == FALSE)
		{
			const auto errCode = GetLastError();
			return -int(errCode);
		}

		return (int)bytesReceived;
	}

	KUSB_HANDLE usbHandle;
	KUSB_DRIVER_API* usbDriver;
	size_t totalWritten;
	u32 currentBuffer;
};


static KLST_DEVINFO pluggedInDevice;
static WinHandle gotDeviceEvent;

static u32 deviceVid = 0x0955;
static u32 devicePid = 0x7321;
static void KUSB_API HotPlugEventCallback(KHOT_HANDLE Handle, KLST_DEVINFO_HANDLE DeviceInfo, KLST_SYNC_FLAG NotificationType)
{
	if (NotificationType == KLST_SYNC_FLAG_ADDED && DeviceInfo != nullptr &&
		DeviceInfo->Common.Vid == deviceVid && DeviceInfo->Common.Pid == devicePid)
	{
		memcpy(&pluggedInDevice, DeviceInfo, sizeof(pluggedInDevice));
		SetEvent(gotDeviceEvent.get());
	}
}

static WinHandle finishedUpEvent;
static BOOL WINAPI ConsoleSignalHandler(DWORD signal)
{
	switch (signal)
	{
	case CTRL_CLOSE_EVENT:
	case CTRL_LOGOFF_EVENT:
	case CTRL_SHUTDOWN_EVENT:
	case CTRL_C_EVENT:
		memset(&pluggedInDevice, 0, sizeof(pluggedInDevice));
		SetEvent(gotDeviceEvent.get());
		if (WaitForSingleObject(finishedUpEvent.get(), 1000) == WAIT_OBJECT_0)
			finishedUpEvent = WinHandle();
		else
			_ftprintf(stderr, TEXT("Timed out waiting for cleanup, forcibly closing\n"));
	default:
		break;
	}

	return TRUE;
}


TegraRcmSmash::TegraRcmSmash()
{
}


TegraRcmSmash::~TegraRcmSmash()
{
}


int TegraRcmSmash::RcmStatus()
{
	KLST_DEVINFO_HANDLE deviceInfo = nullptr;

	KLST_HANDLE deviceList = nullptr;
	if (!LstK_Init(&deviceList, KLST_FLAG_NONE))
	{
		const auto errorCode = GetLastError();
		// Win32 error trying to list USB devices
		return -6;
	}
	auto lstKgrd = MakeScopeGuard([&deviceList]()
	{
		if (deviceList != nullptr)
		{
			LstK_Free(deviceList);
			deviceList = nullptr;
		}
	});

	// Get the number of devices contained in the device list.
	UINT deviceCount = 0;
	LstK_Count(deviceList, &deviceCount);

	if (deviceCount == 0 || LstK_FindByVidPid(deviceList, deviceVid, devicePid, &deviceInfo) == FALSE)
	{
		// No device found in RCM Mode
		return -5;
	}

	if (deviceInfo != nullptr)
	{
		if (deviceInfo->DriverID != KUSB_DRVID_LIBUSBK)
		{
			/*
			Wrong driver => need to install libusbK driver
			*/
			return -4;
		}

		KUSB_DRIVER_API Usb;
		LibK_LoadDriverAPI(&Usb, deviceInfo->DriverID);

		// Initialize the device
		KUSB_HANDLE handle = nullptr;
		if (!Usb.Init(&handle, deviceInfo))
		{
			const auto errorCode = GetLastError();
			// Failed to handle device
			return -3;
		}

		RCMDeviceHacker rcmDev(Usb, handle); handle = nullptr;

		libusbk::version_t usbkVersion;
		memset(&usbkVersion, 0, sizeof(usbkVersion));
		const auto versRetVal = rcmDev.getDriverVersion(usbkVersion);
		if (versRetVal <= 0)
		{
			// Failed to get USB driver version
			return -2;
		}
		else if (usbkVersion.major != 3 || usbkVersion.minor != 0 || usbkVersion.micro != 7)
		{

			// Wrong USB driver version
			return -1;
		}
	}

	return 0;

}