#pragma once

//#include "Win32Def.h"
#include <windows.h>
#include <libusbk.h>
#include <winioctl.h>

#pragma pack(push, 1)

namespace libusbk
{
	struct interface_request_t
	{
		unsigned int interface_number;
		unsigned int altsetting_number;

		unsigned char intf_use_index: 1;	// libusbK Only
		unsigned char altf_use_index: 1;	// libusbK Only
		unsigned char: 6;

		short interface_index;		// libusbK Only
		short altsetting_index;		// libusbK Only
	};

	struct version_t
	{
		unsigned int major;
		unsigned int minor;
		unsigned int micro;
		unsigned int nano;
		unsigned int mod_value;
	};

	struct libusb_request
	{
		unsigned int timeout;
		union
		{
			struct
			{
				unsigned int configuration;
			} configuration;

			interface_request_t intf; // libusbK Only

			version_t version;

			struct
			{
				unsigned int endpoint;
				unsigned int packet_size;

				unsigned int unused; // max_transfer_size is deprecated; (see pipe policies)
				unsigned int transfer_flags;
				unsigned int iso_start_frame_latency;
			} endpoint;

			struct
			{
				UCHAR PipeID;
				ULONG IsoContextSize;
				PKISO_CONTEXT IsoContext;
			} IsoEx;
			struct
			{
				UCHAR PipeID;
			} AutoIsoEx;
			struct
			{
				unsigned int type;
				unsigned int recipient;
				unsigned int request;
				unsigned int value;
				unsigned int index;
			} vendor;
			struct
			{
				unsigned int recipient;
				unsigned int feature;
				unsigned int index;
			} feature;
			struct
			{
				unsigned int recipient;
				unsigned int index;
				unsigned int status;
			} status;
			struct
			{
				unsigned int type;
				unsigned int index;
				unsigned int language_id;
				unsigned int recipient;
			} descriptor;
			struct
			{
				unsigned int level;
			} debug;

			struct
			{
				unsigned int property;
			} device_property;
			struct
			{
				unsigned int key_type;
				unsigned int name_offset;
				unsigned int value_offset;
				unsigned int value_length;
			} device_registry_key;
			struct
			{
				ULONG information_type;
			} query_device;
			struct
			{
				unsigned int interface_index;
				unsigned int pipe_id;
				unsigned int policy_type;
			} pipe_policy;
			struct
			{
				unsigned int policy_type;
			} power_policy;

			// WDF_USB_CONTROL_SETUP_PACKET control;
			union
			{
				struct
				{
					UCHAR   RequestType;
					UCHAR   Request;
					USHORT  Value;
					USHORT  Index;
					USHORT  Length;
				} control;
			};
		};
	};

	constexpr DWORD LIBUSB_IOCTL_GET_VERSION = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x812, METHOD_BUFFERED, FILE_ANY_ACCESS);
	constexpr DWORD LIBUSB_IOCTL_GET_STATUS = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x807, METHOD_BUFFERED, FILE_ANY_ACCESS);	
};
#pragma pack(pop)

static inline bool libusbk_getInternals(KUSB_HANDLE handle, HANDLE* outMasterHandle=nullptr, HANDLE* outMasterInterfaceHandle=nullptr, LPCSTR* outDevicePath=nullptr, PUSB_CONFIGURATION_DESCRIPTOR* outConfigDescriptor=nullptr)
{
	if (handle == nullptr)
		return false;

	auto byteHandle = (const unsigned char*)handle;
#if !_WIN64
	memcpy(&byteHandle, &byteHandle[0x1C], sizeof(void*));

	if (outMasterHandle != nullptr)
		memcpy(outMasterHandle, &byteHandle[0x1C], sizeof(HANDLE));
	if (outMasterInterfaceHandle != nullptr)
		memcpy(outMasterInterfaceHandle, &byteHandle[0x20], sizeof(HANDLE));
	if (outDevicePath != nullptr)
		memcpy(outDevicePath, &byteHandle[0x24], sizeof(void*));
	if (outConfigDescriptor != nullptr)
		memcpy(outConfigDescriptor, &byteHandle[0x28], sizeof(void*));
#else
	memcpy(&byteHandle, &byteHandle[0x30], sizeof(void*));

	if (outMasterHandle != nullptr)
		memcpy(outMasterHandle, &byteHandle[0x30], sizeof(HANDLE));
	if (outMasterInterfaceHandle != nullptr)
		memcpy(outMasterInterfaceHandle, &byteHandle[0x38], sizeof(HANDLE));
	if (outDevicePath != nullptr)
		memcpy(outDevicePath, &byteHandle[0x40], sizeof(void*));
	if (outConfigDescriptor != nullptr)
		memcpy(outConfigDescriptor, &byteHandle[0x48], sizeof(void*));
#endif

	return true;
}
