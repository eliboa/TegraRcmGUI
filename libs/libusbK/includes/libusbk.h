/*! \file libusbk.h
* \brief functions for usb device communication.
*
* \note
* This is the \b main libusbK USB user include file.
*/

#ifndef _LIBUSBK_H__
#define _LIBUSBK_H__

#include "lusbk_shared.h"

///////////////////////////////////////////////////////////////////////
// L I B U S B K  PUBLIC STRUCTS, DEFINES, AND ENUMS //////////////////
///////////////////////////////////////////////////////////////////////

#ifndef _LIBUSBK_LIBK_TYPES

/*! \addtogroup libk
* @{
*/

#define _in
#define _inopt
#define _out
#define _outopt
#define _ref
#define _refopt

//! UsbK base function pointer, See \ref LibK_GetProcAddress.
typedef INT_PTR (FAR WINAPI* KPROC)();

//! Indicates that a function is an exported API call.
#if defined(DYNAMIC_DLL)
#define KUSB_EXP
#else
#define KUSB_EXP
#endif

//! Indicates the calling convention. This is always WINAPI (stdcall) by default.
#if !defined(KUSB_API)
#define KUSB_API WINAPI
#endif

#pragma warning(disable:4201)

#if _MSC_VER >= 1200
#pragma warning(push)
#endif
#pragma warning (disable:4201)
#pragma warning(disable:4214) // named type definition in parentheses

//! User defined handle context space, see \ref LibK_GetContext.
typedef INT_PTR KLIB_USER_CONTEXT;

//! KUSB control setup packet.
/*!
* This union structure is identical in size to a \ref WINUSB_SETUP_PACKET,
* but provides additional field accessors. (see \ref libusbk.h for structure details)
*/
typedef union _KUSB_SETUP_PACKET
{
	UCHAR	Bytes[8];
	USHORT	Words[4];
	struct
	{
		//! Request value
		struct
		{
			UCHAR Recipient: 2;
			UCHAR Reserved: 3;
			UCHAR Type: 2;
			UCHAR Dir: 1;
		} BmRequest;

		//! Request type value
		UCHAR Request;

		//! wValue
		USHORT Value;

		//! wIndex
		USHORT Index;

		//! wLength ushort value
		USHORT Length;
	};
	struct
	{
		struct
		{
			UCHAR b0: 1;
			UCHAR b1: 1;
			UCHAR b2: 1;
			UCHAR b3: 1;
			UCHAR b4: 1;
			UCHAR b5: 1;
			UCHAR b6: 1;
			UCHAR b7: 1;
		} BmRequestBits;

		struct
		{
			UCHAR b0: 1;
			UCHAR b1: 1;
			UCHAR b2: 1;
			UCHAR b3: 1;
			UCHAR b4: 1;
			UCHAR b5: 1;
			UCHAR b6: 1;
			UCHAR b7: 1;
		} RequestBits;

		UCHAR ValueLo;
		UCHAR ValueHi;
		UCHAR IndexLo;
		UCHAR IndexHi;
		UCHAR LengthLo;
		UCHAR LengthHi;
	};
} KUSB_SETUP_PACKET;
// setup packet is eight bytes -- defined by spec
C_ASSERT(sizeof(KUSB_SETUP_PACKET) == 8);

#if _MSC_VER >= 1200
#pragma warning(pop)
#endif


//! Base handle type for all library handles, See \ref KLIB_HANDLE_TYPE.
typedef void* KLIB_HANDLE;

//! Opaque UsbK handle, see \ref UsbK_Init.
typedef KLIB_HANDLE KUSB_HANDLE;

//! Opaque LstK handle, see \ref LstK_Init.
typedef KLIB_HANDLE KLST_HANDLE;

//! Opaque HotK handle, see \ref HotK_Init.
typedef KLIB_HANDLE KHOT_HANDLE;

//! Opaque OvlK handle, see \ref OvlK_Acquire.
typedef KLIB_HANDLE KOVL_HANDLE;


//! Opaque OvlPoolK handle, see \ref OvlK_Init.
typedef KLIB_HANDLE KOVL_POOL_HANDLE;

//! Opaque StmK handle, see \ref StmK_Init.
typedef KLIB_HANDLE KSTM_HANDLE;

//! Handle type enumeration.
typedef enum _KLIB_HANDLE_TYPE
{
    //! Hot plug handle. \ref KHOT_HANDLE
    KLIB_HANDLE_TYPE_HOTK,

    //! USB handle. \ref KUSB_HANDLE
    KLIB_HANDLE_TYPE_USBK,

    //! Shared USB handle. \ref KUSB_HANDLE
    KLIB_HANDLE_TYPE_USBSHAREDK,

    //! Device list handle. \ref KLST_HANDLE
    KLIB_HANDLE_TYPE_LSTK,

    //! Device info handle. \ref KLST_DEVINFO_HANDLE
    KLIB_HANDLE_TYPE_LSTINFOK,

    //! Overlapped handle. \ref KOVL_HANDLE
    KLIB_HANDLE_TYPE_OVLK,

    //! Overlapped pool handle. \ref KOVL_POOL_HANDLE
    KLIB_HANDLE_TYPE_OVLPOOLK,

    //! Pipe stream handle. \ref KSTM_HANDLE
    KLIB_HANDLE_TYPE_STMK,

    //! Max handle type count.
    KLIB_HANDLE_TYPE_COUNT
} KLIB_HANDLE_TYPE;

//! Function typedef for \ref LibK_SetCleanupCallback.
typedef INT KUSB_API KLIB_HANDLE_CLEANUP_CB (_in KLIB_HANDLE Handle, _in KLIB_HANDLE_TYPE HandleType, _in KLIB_USER_CONTEXT UserContext);

//! libusbK verson information structure.
typedef struct _KLIB_VERSION
{
	//! Major version number.
	INT Major;

	//! Minor version number.
	INT Minor;

	//! Micro version number.
	INT Micro;

	//! Nano version number.
	INT Nano;
} KLIB_VERSION;
//! Pointer to a \copybrief KLIB_VERSION
typedef KLIB_VERSION* PKLIB_VERSION;

/*! @} */
#endif

#ifndef _LIBUSBK_ISOK_TYPES
/*! \addtogroup isok
* @{
*/


//! Callback function typedef for \ref IsoK_EnumPackets
typedef BOOL KUSB_API KISO_ENUM_PACKETS_CB (_in UINT PacketIndex, _in PKISO_PACKET IsoPacket, _in PVOID UserState);

/*! @} */
#endif

#ifndef _LIBUSBK_LSTK_TYPES

/*! \addtogroup lstk
* @{
*/

//!  Allocated length for all strings in a \ref KLST_DEVINFO structure.
#define KLST_STRING_MAX_LEN 256

//! Device list sync flags.
/*!
* These sync flags are also use by the hot plug module to indicate device
* arrival/removal notifications:
* - \b DeviceArrival = KLST_SYNC_FLAG_ADDED
* - \b DeviceRemoval = KLST_SYNC_FLAG_REMOVED
*/
typedef enum _KLST_SYNC_FLAG
{
    //! Cleared/invalid state.
    KLST_SYNC_FLAG_NONE				= 0L,

    //! Unchanged state,
    KLST_SYNC_FLAG_UNCHANGED		= 0x0001,

    //! Added (Arrival) state,
    KLST_SYNC_FLAG_ADDED			= 0x0002,

    //! Removed (Unplugged) state,
    KLST_SYNC_FLAG_REMOVED			= 0x0004,

    //! Connect changed state.
    KLST_SYNC_FLAG_CONNECT_CHANGE	= 0x0008,

    //! All states.
    KLST_SYNC_FLAG_MASK				= 0x000F,
} KLST_SYNC_FLAG;

//! Common usb device information structure
typedef struct _KLST_DEV_COMMON_INFO
{
	//! VendorID parsed from \ref KLST_DEVINFO::DeviceID
	INT Vid;

	//! ProductID parsed from \ref KLST_DEVINFO::DeviceID
	INT Pid;

	//! Composite interface number parsed from \ref KLST_DEVINFO::DeviceID.  Set to \b -1 for devices that do not have the composite parent driver.
	INT MI;

	// An ID that uniquely identifies a USB device.
	CHAR InstanceID[KLST_STRING_MAX_LEN];

} KLST_DEV_COMMON_INFO;
//! Pointer to a \c KLST_DEV_COMMON_INFO structure.
typedef KLST_DEV_COMMON_INFO* PKLST_DEV_COMMON_INFO;

//! Semi-opaque device information structure of a device list.
/*!
*
* \attention This structure is semi-opaque.
*
*/
typedef struct _KLST_DEVINFO
{
	//! Common usb device information
	KLST_DEV_COMMON_INFO Common;

	//! Driver id this device element is using
	INT DriverID;

	//! Device interface GUID
	CHAR DeviceInterfaceGUID[KLST_STRING_MAX_LEN];

	//! Device instance ID.
	/*!
	* A Device instance ID has the following format:
	* [enumerator]\[enumerator-specific-device-ID]\[instance-specific-ID]
	* - [enumerator]
	*   - For USB device, the enumerator is always \c USB
	* - [enumerator-specific-device-ID]
	*   - Contains the vendor and product id (VID_xxxx&PID_xxxx)
	*   - If present, contains the usbccgp (windows composite device layer) interface number (MI_xx)
	* - [instance-specific-ID]
	*   - If the device is composite, contains a unique interface ID generated by Windows.
	*   - If the device is not composite and has a serial number, contains the devices serial number.
	*   - If the device does not have a serial number, contains a unique ID generated by Windows.
	*/
	CHAR DeviceID[KLST_STRING_MAX_LEN];

	//! Class GUID.
	CHAR ClassGUID[KLST_STRING_MAX_LEN];

	//! Manufacturer name as specified in the INF file.
	CHAR Mfg[KLST_STRING_MAX_LEN];

	//! Device description as specified in the INF file.
	CHAR DeviceDesc[KLST_STRING_MAX_LEN];

	//! Driver service name.
	CHAR Service[KLST_STRING_MAX_LEN];

	//! Unique identifier.
	CHAR SymbolicLink[KLST_STRING_MAX_LEN];

	//! physical device filename used with the Windows \c CreateFile()
	CHAR DevicePath[KLST_STRING_MAX_LEN];

	//! libusb-win32 filter index id.
	INT LUsb0FilterIndex;

	//! Indicates the devices connection state.
	BOOL Connected;

	//! Synchronization flags. (internal use only)
	KLST_SYNC_FLAG SyncFlags;

	INT BusNumber;

	INT DeviceAddress;

	//! If the the device is serialized, represents the string value of \ref USB_DEVICE_DESCRIPTOR::iSerialNumber. For Devices without a \b iSerialNumber, represents the unique \b InstanceID assigned by \b Windows.
	CHAR SerialNumber[KLST_STRING_MAX_LEN];

} KLST_DEVINFO;
//! Pointer to a \ref KLST_DEVINFO structure. (semi-opaque)
typedef KLST_DEVINFO* KLST_DEVINFO_HANDLE;

//! Device list initialization flags.
typedef enum _KLST_FLAG
{
    //! No flags (or 0)
    KLST_FLAG_NONE = 0L,

    //! Enable listings for the raw device interface GUID \b only. {A5DCBF10-6530-11D2-901F-00C04FB951ED}
    KLST_FLAG_INCLUDE_RAWGUID = 0x0001,

    //! List all libusbK devices including those not currently connected.
    KLST_FLAG_INCLUDE_DISCONNECT = 0x0002,

} KLST_FLAG;

//! Device list/hot-plug pattern match structure.
/*!
* \fixedstruct{1024}
*
* These ansi char strings are used to specify which devices should be included in a device list.
* All strings file pattern match strings allowing asterisk or question mark chars as wildcards.
*
*/
typedef struct _KLST_PATTERN_MATCH
{
	//! Pattern match a device instance id.
	CHAR DeviceID[KLST_STRING_MAX_LEN];

	//! Pattern match a device interface guid.
	CHAR DeviceInterfaceGUID[KLST_STRING_MAX_LEN];

	//! Pattern match a symbolic link.
	CHAR ClassGUID[KLST_STRING_MAX_LEN];

	//! fixed structure padding.
	UCHAR z_F_i_x_e_d[1024 - KLST_STRING_MAX_LEN * 3];

} KLST_PATTERN_MATCH;
C_ASSERT(sizeof(KLST_PATTERN_MATCH) == 1024);

//! Pointer to a \ref KLST_PATTERN_MATCH structure.
typedef KLST_PATTERN_MATCH* PKLST_PATTERN_MATCH;

//! Device list enumeration function callback typedef.
/*!
*
* \param DeviceList
* The device list \c DeviceInfo belongs to
*
* \param DeviceInfo
* Device information
*
* \param Context
* User context that was passed into \ref LstK_Enumerate
*
* Use this typedef as a prototype for an enumeration function with \ref LstK_Enumerate.
*
*/
typedef BOOL KUSB_API KLST_ENUM_DEVINFO_CB (
    _in KLST_HANDLE DeviceList,
    _in KLST_DEVINFO_HANDLE DeviceInfo,
    _in PVOID Context);

/*! @} */

#endif

#ifndef   __USB_H__

#include <pshpack1.h>

/*! \addtogroup libk
* @{
*/

//! Maximum value that can be added to the current start frame.
#define USBD_ISO_START_FRAME_RANGE 1024


//! bmRequest.Dir
typedef enum _BMREQUEST_DIR
{
    BMREQUEST_DIR_HOST_TO_DEVICE = 0,
    BMREQUEST_DIR_DEVICE_TO_HOST = 1,
} BMREQUEST_DIR;

//! bmRequest.Type
typedef enum _BMREQUEST_TYPE
{
    //! Standard request. See \ref USB_REQUEST_ENUM
    BMREQUEST_TYPE_STANDARD = 0,

    //! Class-specific request.
    BMREQUEST_TYPE_CLASS = 1,

    //! Vendor-specific request
    BMREQUEST_TYPE_VENDOR = 2,
} BMREQUEST_TYPE;

//! bmRequest.Recipient
typedef enum _BMREQUEST_RECIPIENT
{
    //! Request is for a device.
    BMREQUEST_RECIPIENT_DEVICE = 0,

    //! Request is for an interface of a device.
    BMREQUEST_RECIPIENT_INTERFACE = 1,

    //! Request is for an endpoint of a device.
    BMREQUEST_RECIPIENT_ENDPOINT = 2,

    //! Request is for a vendor-specific purpose.
    BMREQUEST_RECIPIENT_OTHER = 3,
} BMREQUEST_RECIPIENT;

//! Maximum length (in bytes) of a usb string. USB strings are always stored in wide-char format.
#define MAXIMUM_USB_STRING_LENGTH 255

//! Values for the bits returned by the \ref USB_REQUEST_GET_STATUS request.
typedef enum _USB_GETSTATUS
{
    //! Device is self powered
    USB_GETSTATUS_SELF_POWERED = 0x01,

    //! Device can wake the system from a low power/sleeping state.
    USB_GETSTATUS_REMOTE_WAKEUP_ENABLED = 0x02
} USB_GETSTATUS;

//! Standard USB descriptor types. For more information, see section 9-5 of the USB 3.0 specifications.
typedef enum _USB_DESCRIPTOR_TYPE
{
    //! Device descriptor type.
    USB_DESCRIPTOR_TYPE_DEVICE = 0x01,

    //! Configuration descriptor type.
    USB_DESCRIPTOR_TYPE_CONFIGURATION = 0x02,

    //! String descriptor type.
    USB_DESCRIPTOR_TYPE_STRING = 0x03,

    //! Interface descriptor type.
    USB_DESCRIPTOR_TYPE_INTERFACE = 0x04,

    //! Endpoint descriptor type.
    USB_DESCRIPTOR_TYPE_ENDPOINT = 0x05,

    //! Device qualifier descriptor type.
    USB_DESCRIPTOR_TYPE_DEVICE_QUALIFIER = 0x06,

    //! Config power descriptor type.
    USB_DESCRIPTOR_TYPE_CONFIG_POWER = 0x07,

    //! Interface power descriptor type.
    USB_DESCRIPTOR_TYPE_INTERFACE_POWER = 0x08,

    //! Interface association descriptor type.
    USB_DESCRIPTOR_TYPE_INTERFACE_ASSOCIATION = 0x0B,
} USB_DESCRIPTOR_TYPE;

//! Makes \c wValue for a \ref USB_REQUEST_GET_DESCRIPTOR or \ref USB_REQUEST_SET_DESCRIPTOR request.
#define USB_DESCRIPTOR_MAKE_TYPE_AND_INDEX(d, i)	\
	((USHORT)((USHORT)d<<8 | i))

//! Endpoint type mask for the \c bmAttributes field of a \ref USB_ENDPOINT_DESCRIPTOR
#define USB_ENDPOINT_TYPE_MASK                    0x03

//! Indicates a control endpoint
#define USB_ENDPOINT_TYPE_CONTROL                 0x00

//! Indicates an isochronous endpoint
#define USB_ENDPOINT_TYPE_ISOCHRONOUS             0x01

//! Indicates a bulk endpoint
#define USB_ENDPOINT_TYPE_BULK                    0x02

//! Indicates an interrupt endpoint
#define USB_ENDPOINT_TYPE_INTERRUPT               0x03

//! Config power mask for the \c bmAttributes field of a \ref USB_CONFIGURATION_DESCRIPTOR
#define USB_CONFIG_POWERED_MASK                   0xc0

//! Values used in the \c bmAttributes field of a \ref USB_CONFIGURATION_DESCRIPTOR
enum USB_CONFIG_BM_ATTRIBUTE_ENUM
{
    //! The device is powered by it's host.
    USB_CONFIG_BUS_POWERED = 0x80,

    //! The device has an external power source.
    USB_CONFIG_SELF_POWERED = 0x40,

    //! The device is capable of waking the the host from a low power/sleeping state.
    USB_CONFIG_REMOTE_WAKEUP = 0x20,
};

//! Endpoint direction mask for the \c bEndpointAddress field of a \ref USB_ENDPOINT_DESCRIPTOR
#define USB_ENDPOINT_DIRECTION_MASK               0x80

//! Endpoint address mask for the \c bEndpointAddress field of a \ref USB_ENDPOINT_DESCRIPTOR
#define USB_ENDPOINT_ADDRESS_MASK 0x0F

//! Tests the \c bEndpointAddress direction bit. TRUE if the endpoint address is an OUT endpoint. (HostToDevice, PC Write)
/*!
* \param addr \c bEndpointAddress field of a \ref USB_ENDPOINT_DESCRIPTOR
*/
#define USB_ENDPOINT_DIRECTION_OUT(addr)          (!((addr) & USB_ENDPOINT_DIRECTION_MASK))

//!  Tests the \c bEndpointAddress direction bit. TRUE if the endpoint address is an IN endpoint. (DeviceToHost, PC Read)
/*!
* \param addr \c bEndpointAddress field of a \ref USB_ENDPOINT_DESCRIPTOR
*/
#define USB_ENDPOINT_DIRECTION_IN(addr)           ((addr) & USB_ENDPOINT_DIRECTION_MASK)

//! USB defined request codes
/*
* see Chapter 9 of the USB 2.0 specification for
* more information.
*
* These are the correct values based on the USB 2.0 specification.
*/
enum USB_REQUEST_ENUM
{
    //! Request status of the specific recipient
    USB_REQUEST_GET_STATUS = 0x00,

    //! Clear or disable a specific feature
    USB_REQUEST_CLEAR_FEATURE = 0x01,

    //! Set or enable a specific feature
    USB_REQUEST_SET_FEATURE = 0x03,

    //! Set device address for all future accesses
    USB_REQUEST_SET_ADDRESS = 0x05,

    //! Get the specified descriptor
    USB_REQUEST_GET_DESCRIPTOR = 0x06,

    //! Update existing descriptors or add new descriptors
    USB_REQUEST_SET_DESCRIPTOR = 0x07,

    //! Get the current device configuration value
    USB_REQUEST_GET_CONFIGURATION = 0x08,

    //! Set device configuration
    USB_REQUEST_SET_CONFIGURATION = 0x09,

    //! Return the selected alternate setting for the specified interface
    USB_REQUEST_GET_INTERFACE = 0x0A,

    //! Select an alternate interface for the specified interface
    USB_REQUEST_SET_INTERFACE = 0x0B,

    //! Set then report an endpoint's synchronization frame
    USB_REQUEST_SYNC_FRAME = 0x0C,
};

//! USB defined class codes
/*!
* see http://www.usb.org/developers/defined_class for more information.
*
*/
enum USB_DEVICE_CLASS_ENUM
{
    //! Reserved class
    USB_DEVICE_CLASS_RESERVED = 0x00,

    //! Audio class
    USB_DEVICE_CLASS_AUDIO = 0x01,

    //! Communications class
    USB_DEVICE_CLASS_COMMUNICATIONS = 0x02,

    //! Human Interface Device class
    USB_DEVICE_CLASS_HUMAN_INTERFACE = 0x03,

    //! Imaging class
    USB_DEVICE_CLASS_IMAGING = 0x06,

    //! Printer class
    USB_DEVICE_CLASS_PRINTER = 0x07,

    //! Mass storage class
    USB_DEVICE_CLASS_STORAGE = 0x08,

    //! Hub class
    USB_DEVICE_CLASS_HUB = 0x09,

    //! vendor-specific class
    USB_DEVICE_CLASS_VENDOR_SPECIFIC = 0xFF,
};

//! A structure representing the standard USB device descriptor.
/*!
* This descriptor is documented in section 9.6.1 of the USB 2.0 specification.
* All multiple-byte fields are represented in host-endian format.
*/
typedef struct _USB_DEVICE_DESCRIPTOR
{
	//! Size of this descriptor (in bytes)
	UCHAR bLength;

	//! Descriptor type
	UCHAR bDescriptorType;

	//! USB specification release number in binary-coded decimal.
	/*!
	* A value of 0x0200 indicates USB 2.0, 0x0110 indicates USB 1.1, etc.
	*/
	USHORT bcdUSB;

	//! USB-IF class code for the device
	UCHAR bDeviceClass;

	//! USB-IF subclass code for the device
	UCHAR bDeviceSubClass;

	//! USB-IF protocol code for the device
	UCHAR bDeviceProtocol;

	//! Maximum packet size for control endpoint 0
	UCHAR bMaxPacketSize0;

	//! USB-IF vendor ID
	USHORT idVendor;

	//! USB-IF product ID
	USHORT idProduct;

	//! Device release number in binary-coded decimal
	USHORT bcdDevice;

	//! Index of string descriptor describing manufacturer
	UCHAR iManufacturer;

	//! Index of string descriptor describing product
	UCHAR iProduct;

	//! Index of string descriptor containing device serial number
	UCHAR iSerialNumber;

	//! Number of possible configurations
	UCHAR bNumConfigurations;

} USB_DEVICE_DESCRIPTOR;
//! pointer to a \c USB_DEVICE_DESCRIPTOR
typedef USB_DEVICE_DESCRIPTOR* PUSB_DEVICE_DESCRIPTOR;

//! A structure representing the standard USB endpoint descriptor.
/*!
* This descriptor is documented in section 9.6.3 of the USB 2.0 specification.
* All multiple-byte fields are represented in host-endian format.
*/
typedef struct _USB_ENDPOINT_DESCRIPTOR
{
	//! Size of this descriptor (in bytes)
	UCHAR bLength;

	//! Descriptor type
	UCHAR bDescriptorType;

	//! The address of the endpoint described by this descriptor.
	/*!
	* - Bits 0:3 are the endpoint number
	* - Bits 4:6 are reserved
	* - Bit 7 indicates direction
	*/
	UCHAR bEndpointAddress;

	//! Attributes which apply to the endpoint when it is configured using the bConfigurationValue.
	/*!
	* - Bits 0:1 determine the transfer type.
	* - Bits 2:3 are only used for isochronous endpoints and refer to sync type.
	* - Bits 4:5 are also only used for isochronous endpoints and refer to usage type.
	* - Bits 6:7 are reserved.
	*/
	UCHAR bmAttributes;

	//! Maximum packet size this endpoint is capable of sending/receiving.
	USHORT wMaxPacketSize;

	//! Interval for polling endpoint for data transfers.
	UCHAR bInterval;

} USB_ENDPOINT_DESCRIPTOR;
//! pointer to a \c USB_ENDPOINT_DESCRIPTOR
typedef USB_ENDPOINT_DESCRIPTOR* PUSB_ENDPOINT_DESCRIPTOR;

//! A structure representing the standard USB configuration descriptor.
/*
*
* This descriptor is documented in section 9.6.3 of the USB 2.0 specification.
* All multiple-byte fields are represented in host-endian format.
*
*/
typedef struct _USB_CONFIGURATION_DESCRIPTOR
{
	//! Size of this descriptor (in bytes)
	UCHAR bLength;

	//! Descriptor type
	UCHAR bDescriptorType;

	//! Total length of data returned for this configuration
	USHORT wTotalLength;

	//! Number of interfaces supported by this configuration
	UCHAR bNumInterfaces;

	//! Identifier value for this configuration
	UCHAR bConfigurationValue;

	//! Index of string descriptor describing this configuration
	UCHAR iConfiguration;

	//! Configuration characteristics
	UCHAR bmAttributes;

	//! Maximum power consumption of the USB device from this bus in this configuration when the device is fully operation.
	/*!
	* Expressed in units of 2 mA.
	*/
	UCHAR MaxPower;
} USB_CONFIGURATION_DESCRIPTOR;
//! pointer to a \c USB_CONFIGURATION_DESCRIPTOR
typedef USB_CONFIGURATION_DESCRIPTOR* PUSB_CONFIGURATION_DESCRIPTOR;

//! A structure representing the standard USB interface descriptor.
/*!
* This descriptor is documented in section 9.6.5 of the USB 2.0 specification.
* All multiple-byte fields are represented in host-endian format.
*/
typedef struct _USB_INTERFACE_DESCRIPTOR
{
	//! Size of this descriptor (in bytes)
	UCHAR bLength;

	//! Descriptor type
	UCHAR bDescriptorType;

	//! Number of this interface
	UCHAR bInterfaceNumber;

	//! Value used to select this alternate setting for this interface
	UCHAR bAlternateSetting;

	//! Number of endpoints used by this interface (excluding the control endpoint)
	UCHAR bNumEndpoints;

	//! USB-IF class code for this interface
	UCHAR bInterfaceClass;

	//! USB-IF subclass code for this interface
	UCHAR bInterfaceSubClass;

	//! USB-IF protocol code for this interface
	UCHAR bInterfaceProtocol;

	//! Index of string descriptor describing this interface
	UCHAR iInterface;

} USB_INTERFACE_DESCRIPTOR;
//! pointer to a \c USB_INTERFACE_DESCRIPTOR
typedef USB_INTERFACE_DESCRIPTOR* PUSB_INTERFACE_DESCRIPTOR;

//! A structure representing the standard USB string descriptor.
/*!
* This descriptor is documented in section 9.6.5 of the USB 2.0 specification.
* All multiple-byte fields are represented in host-endian format.
*/
typedef struct _USB_STRING_DESCRIPTOR
{
	//! Size of this descriptor (in bytes)
	UCHAR bLength;

	//! Descriptor type
	UCHAR bDescriptorType;

	//! Content of the string
	WCHAR bString[1];

} USB_STRING_DESCRIPTOR;
//! pointer to a \c USB_STRING_DESCRIPTOR
typedef USB_STRING_DESCRIPTOR* PUSB_STRING_DESCRIPTOR;

//! A structure representing the common USB descriptor.
typedef struct _USB_COMMON_DESCRIPTOR
{
	//! Size of this descriptor (in bytes)
	UCHAR bLength;

	//! Descriptor type
	UCHAR bDescriptorType;

} USB_COMMON_DESCRIPTOR;
//! pointer to a \c USB_COMMON_DESCRIPTOR
typedef USB_COMMON_DESCRIPTOR* PUSB_COMMON_DESCRIPTOR;

#if _MSC_VER >= 1200
#pragma warning(push)
#endif
#pragma warning (disable:4201)
#pragma warning(disable:4214) // named type definition in parentheses

//! Allows hardware manufacturers to define groupings of interfaces.
/*!
*
* The ECN specifies a USB descriptor, called the Interface Association
* Descriptor (IAD).
*
* The Universal Serial Bus Specification, revision 2.0, does not support
* grouping more than one interface of a composite device within a single
* function. However, the USB Device Working Group (DWG) created USB device
* classes that allow for functions with multiple interfaces, and the USB
* Implementor's Forum issued an Engineering Change Notification (ECN) that
* defines a mechanism for grouping interfaces.
*
*/
typedef struct _USB_INTERFACE_ASSOCIATION_DESCRIPTOR
{
	//! Size of this descriptor (in bytes)
	UCHAR   bLength;

	//! Descriptor type
	UCHAR   bDescriptorType;

	//! First interface number of the set of interfaces that follow this descriptor
	UCHAR   bFirstInterface;

	//! The Number of interfaces follow this descriptor that are considered "associated"
	UCHAR   bInterfaceCount;

	//! \c bInterfaceClass used for this associated interfaces
	UCHAR   bFunctionClass;

	//! \c bInterfaceSubClass used for the associated interfaces
	UCHAR   bFunctionSubClass;

	//! \c bInterfaceProtocol used for the associated interfaces
	UCHAR   bFunctionProtocol;

	//! Index of string descriptor describing the associated interfaces
	UCHAR   iFunction;

} USB_INTERFACE_ASSOCIATION_DESCRIPTOR;
//! pointer to a \c USB_INTERFACE_ASSOCIATION_DESCRIPTOR
typedef USB_INTERFACE_ASSOCIATION_DESCRIPTOR* PUSB_INTERFACE_ASSOCIATION_DESCRIPTOR;

#if _MSC_VER >= 1200
#pragma warning(pop)
#endif

/*! @} */

#include <poppack.h>
#endif // __USB_H__

#ifndef _LIBUSBK_LIBK_TYPES

/*! \addtogroup libk
* @{
*/

//! Usb handle specific properties that can be retrieved with \ref UsbK_GetProperty.
typedef enum _KUSB_PROPERTY
{
    //! Get the internal device file handle used for operations such as GetOverlappedResult or DeviceIoControl.
    KUSB_PROPERTY_DEVICE_FILE_HANDLE,

    KUSB_PROPERTY_COUNT

} KUSB_PROPERTY;

//! Supported driver id enumeration.
typedef enum _KUSB_DRVID
{
    //! libusbK.sys driver ID
    KUSB_DRVID_LIBUSBK,

    //! libusb0.sys driver ID
    KUSB_DRVID_LIBUSB0,

    //! WinUSB.sys driver ID
    KUSB_DRVID_WINUSB,

    //! libusb0.sys filter driver ID
    KUSB_DRVID_LIBUSB0_FILTER,

    //! Supported driver count
    KUSB_DRVID_COUNT

} KUSB_DRVID;

//! Supported function id enumeration.
typedef enum _KUSB_FNID
{
    //! \ref UsbK_Init dynamic driver function id.
    KUSB_FNID_Init,

    //! \ref UsbK_Free dynamic driver function id.
    KUSB_FNID_Free,

    //! \ref UsbK_ClaimInterface dynamic driver function id.
    KUSB_FNID_ClaimInterface,

    //! \ref UsbK_ReleaseInterface dynamic driver function id.
    KUSB_FNID_ReleaseInterface,

    //! \ref UsbK_SetAltInterface dynamic driver function id.
    KUSB_FNID_SetAltInterface,

    //! \ref UsbK_GetAltInterface dynamic driver function id.
    KUSB_FNID_GetAltInterface,

    //! \ref UsbK_GetDescriptor dynamic driver function id.
    KUSB_FNID_GetDescriptor,

    //! \ref UsbK_ControlTransfer dynamic driver function id.
    KUSB_FNID_ControlTransfer,

    //! \ref UsbK_SetPowerPolicy dynamic driver function id.
    KUSB_FNID_SetPowerPolicy,

    //! \ref UsbK_GetPowerPolicy dynamic driver function id.
    KUSB_FNID_GetPowerPolicy,

    //! \ref UsbK_SetConfiguration dynamic driver function id.
    KUSB_FNID_SetConfiguration,

    //! \ref UsbK_GetConfiguration dynamic driver function id.
    KUSB_FNID_GetConfiguration,

    //! \ref UsbK_ResetDevice dynamic driver function id.
    KUSB_FNID_ResetDevice,

    //! \ref UsbK_Initialize dynamic driver function id.
    KUSB_FNID_Initialize,

    //! \ref UsbK_SelectInterface dynamic driver function id.
    KUSB_FNID_SelectInterface,

    //! \ref UsbK_GetAssociatedInterface dynamic driver function id.
    KUSB_FNID_GetAssociatedInterface,

    //! \ref UsbK_Clone dynamic driver function id.
    KUSB_FNID_Clone,

    //! \ref UsbK_QueryInterfaceSettings dynamic driver function id.
    KUSB_FNID_QueryInterfaceSettings,

    //! \ref UsbK_QueryDeviceInformation dynamic driver function id.
    KUSB_FNID_QueryDeviceInformation,

    //! \ref UsbK_SetCurrentAlternateSetting dynamic driver function id.
    KUSB_FNID_SetCurrentAlternateSetting,

    //! \ref UsbK_GetCurrentAlternateSetting dynamic driver function id.
    KUSB_FNID_GetCurrentAlternateSetting,

    //! \ref UsbK_QueryPipe dynamic driver function id.
    KUSB_FNID_QueryPipe,

    //! \ref UsbK_SetPipePolicy dynamic driver function id.
    KUSB_FNID_SetPipePolicy,

    //! \ref UsbK_GetPipePolicy dynamic driver function id.
    KUSB_FNID_GetPipePolicy,

    //! \ref UsbK_ReadPipe dynamic driver function id.
    KUSB_FNID_ReadPipe,

    //! \ref UsbK_WritePipe dynamic driver function id.
    KUSB_FNID_WritePipe,

    //! \ref UsbK_ResetPipe dynamic driver function id.
    KUSB_FNID_ResetPipe,

    //! \ref UsbK_AbortPipe dynamic driver function id.
    KUSB_FNID_AbortPipe,

    //! \ref UsbK_FlushPipe dynamic driver function id.
    KUSB_FNID_FlushPipe,

    //! \ref UsbK_IsoReadPipe dynamic driver function id.
    KUSB_FNID_IsoReadPipe,

    //! \ref UsbK_IsoWritePipe dynamic driver function id.
    KUSB_FNID_IsoWritePipe,

    //! \ref UsbK_GetCurrentFrameNumber dynamic driver function id.
    KUSB_FNID_GetCurrentFrameNumber,

    //! \ref UsbK_GetOverlappedResult dynamic driver function id.
    KUSB_FNID_GetOverlappedResult,

    //! \ref UsbK_GetProperty dynamic driver function id.
    KUSB_FNID_GetProperty,


    //! Supported function count
    KUSB_FNID_COUNT,

} KUSB_FNID;

typedef BOOL KUSB_API KUSB_Init (
    _out KUSB_HANDLE* InterfaceHandle,
    _in KLST_DEVINFO_HANDLE DevInfo);

typedef BOOL KUSB_API KUSB_Free (
    _in KUSB_HANDLE InterfaceHandle);

typedef BOOL KUSB_API KUSB_ClaimInterface (
    _in KUSB_HANDLE InterfaceHandle,
    _in UCHAR NumberOrIndex,
    _in BOOL IsIndex);

typedef BOOL KUSB_API KUSB_ReleaseInterface (
    _in KUSB_HANDLE InterfaceHandle,
    _in UCHAR NumberOrIndex,
    _in BOOL IsIndex);

typedef BOOL KUSB_API KUSB_SetAltInterface (
    _in KUSB_HANDLE InterfaceHandle,
    _in UCHAR NumberOrIndex,
    _in BOOL IsIndex,
    _in UCHAR AltSettingNumber);

typedef BOOL KUSB_API KUSB_GetAltInterface (
    _in KUSB_HANDLE InterfaceHandle,
    _in UCHAR NumberOrIndex,
    _in BOOL IsIndex,
    _out PUCHAR AltSettingNumber);

typedef BOOL KUSB_API KUSB_GetDescriptor (
    _in KUSB_HANDLE InterfaceHandle,
    _in UCHAR DescriptorType,
    _in UCHAR Index,
    _in USHORT LanguageID,
    _out PUCHAR Buffer,
    _in UINT BufferLength,
    _outopt PUINT LengthTransferred);

typedef BOOL KUSB_API KUSB_ControlTransfer (
    _in KUSB_HANDLE InterfaceHandle,
    _in WINUSB_SETUP_PACKET SetupPacket,
    _refopt PUCHAR Buffer,
    _in UINT BufferLength,
    _outopt PUINT LengthTransferred,
    _inopt LPOVERLAPPED Overlapped);

typedef BOOL KUSB_API KUSB_SetPowerPolicy (
    _in KUSB_HANDLE InterfaceHandle,
    _in UINT PolicyType,
    _in UINT ValueLength,
    _in PVOID Value);

typedef BOOL KUSB_API KUSB_GetPowerPolicy (
    _in KUSB_HANDLE InterfaceHandle,
    _in UINT PolicyType,
    _ref PUINT ValueLength,
    _out PVOID Value);

typedef BOOL KUSB_API KUSB_SetConfiguration (
    _in KUSB_HANDLE InterfaceHandle,
    _in UCHAR ConfigurationNumber);

typedef BOOL KUSB_API KUSB_GetConfiguration (
    _in KUSB_HANDLE InterfaceHandle,
    _out PUCHAR ConfigurationNumber);

typedef BOOL KUSB_API KUSB_ResetDevice (
    _in KUSB_HANDLE InterfaceHandle);

typedef BOOL KUSB_API KUSB_Initialize (
    _in HANDLE DeviceHandle,
    _out KUSB_HANDLE* InterfaceHandle);

typedef BOOL KUSB_API KUSB_SelectInterface (
    _in KUSB_HANDLE InterfaceHandle,
    _in UCHAR NumberOrIndex,
    _in BOOL IsIndex);

typedef BOOL KUSB_API KUSB_GetAssociatedInterface (
    _in KUSB_HANDLE InterfaceHandle,
    _in UCHAR AssociatedInterfaceIndex,
    _out KUSB_HANDLE* AssociatedInterfaceHandle);

typedef BOOL KUSB_API KUSB_Clone (
    _in KUSB_HANDLE InterfaceHandle,
    _out KUSB_HANDLE* DstInterfaceHandle);

typedef BOOL KUSB_API KUSB_QueryInterfaceSettings (
    _in KUSB_HANDLE InterfaceHandle,
    _in UCHAR AltSettingIndex,
    _out PUSB_INTERFACE_DESCRIPTOR UsbAltInterfaceDescriptor);

typedef BOOL KUSB_API KUSB_QueryDeviceInformation (
    _in KUSB_HANDLE InterfaceHandle,
    _in UINT InformationType,
    _ref PUINT BufferLength,
    _ref PVOID Buffer);

typedef BOOL KUSB_API KUSB_SetCurrentAlternateSetting (
    _in KUSB_HANDLE InterfaceHandle,
    _in UCHAR AltSettingNumber);

typedef BOOL KUSB_API KUSB_GetCurrentAlternateSetting (
    _in KUSB_HANDLE InterfaceHandle,
    _out PUCHAR AltSettingNumber);

typedef BOOL KUSB_API KUSB_QueryPipe (
    _in KUSB_HANDLE InterfaceHandle,
    _in UCHAR AltSettingNumber,
    _in UCHAR PipeIndex,
    _out PWINUSB_PIPE_INFORMATION PipeInformation);

typedef BOOL KUSB_API KUSB_SetPipePolicy (
    _in KUSB_HANDLE InterfaceHandle,
    _in UCHAR PipeID,
    _in UINT PolicyType,
    _in UINT ValueLength,
    _in PVOID Value);

typedef BOOL KUSB_API KUSB_GetPipePolicy (
    _in KUSB_HANDLE InterfaceHandle,
    _in UCHAR PipeID,
    _in UINT PolicyType,
    _ref PUINT ValueLength,
    _out PVOID Value);

typedef BOOL KUSB_API KUSB_ReadPipe (
    _in KUSB_HANDLE InterfaceHandle,
    _in UCHAR PipeID,
    _out PUCHAR Buffer,
    _in UINT BufferLength,
    _outopt PUINT LengthTransferred,
    _inopt LPOVERLAPPED Overlapped);

typedef BOOL KUSB_API KUSB_WritePipe (
    _in KUSB_HANDLE InterfaceHandle,
    _in UCHAR PipeID,
    _in PUCHAR Buffer,
    _in UINT BufferLength,
    _outopt PUINT LengthTransferred,
    _inopt LPOVERLAPPED Overlapped);

typedef BOOL KUSB_API KUSB_ResetPipe (
    _in KUSB_HANDLE InterfaceHandle,
    _in UCHAR PipeID);

typedef BOOL KUSB_API KUSB_AbortPipe (
    _in KUSB_HANDLE InterfaceHandle,
    _in UCHAR PipeID);

typedef BOOL KUSB_API KUSB_FlushPipe (
    _in KUSB_HANDLE InterfaceHandle,
    _in UCHAR PipeID);

typedef BOOL KUSB_API KUSB_IsoReadPipe (
    _in KUSB_HANDLE InterfaceHandle,
    _in UCHAR PipeID,
    _out PUCHAR Buffer,
    _in UINT BufferLength,
    _in LPOVERLAPPED Overlapped,
    _refopt PKISO_CONTEXT IsoContext);

typedef BOOL KUSB_API KUSB_IsoWritePipe (
    _in KUSB_HANDLE InterfaceHandle,
    _in UCHAR PipeID,
    _in PUCHAR Buffer,
    _in UINT BufferLength,
    _in LPOVERLAPPED Overlapped,
    _refopt PKISO_CONTEXT IsoContext);

typedef BOOL KUSB_API KUSB_GetCurrentFrameNumber (
    _in KUSB_HANDLE InterfaceHandle,
    _out PUINT FrameNumber);

typedef BOOL KUSB_API KUSB_GetOverlappedResult (
    _in KUSB_HANDLE InterfaceHandle,
    _in LPOVERLAPPED Overlapped,
    _out PUINT lpNumberOfBytesTransferred,
    _in BOOL bWait);

typedef BOOL KUSB_API KUSB_GetProperty (
    _in KUSB_HANDLE InterfaceHandle,
    _in KUSB_PROPERTY PropertyType,
    _ref PUINT PropertySize,
    _out PVOID Value);



//! USB core driver API information structure.
/*!
* This structure is part of \ref KUSB_DRIVER_API and contains
* driver and user specific information.
*
*/
typedef struct _KUSB_DRIVER_API_INFO
{
	//! \readonly Driver id of the driver api.
	INT DriverID;

	//! \readonly Number of valid functions contained in the driver API.
	INT FunctionCount;

} KUSB_DRIVER_API_INFO;

//! Driver API function set structure.
/*
* Contains the driver specific USB core function pointer set.
*
* \note
* This structure has a fixed 512 byte structure size.
*/
typedef struct _KUSB_DRIVER_API
{
	//! Driver API information.
	KUSB_DRIVER_API_INFO Info;

	/*! \fn BOOL KUSB_API Init (_out KUSB_HANDLE* InterfaceHandle, _in KLST_DEVINFO_HANDLE DevInfo)
		* \memberof KUSB_DRIVER_API
		* \copydoc UsbK_Init
		*/
	KUSB_Init* Init;

	/*! \fn BOOL KUSB_API Free (_in KUSB_HANDLE InterfaceHandle)
	* \memberof KUSB_DRIVER_API
	* \copydoc UsbK_Free
	*/
	KUSB_Free* Free;

	/*! \fn BOOL KUSB_API ClaimInterface (_in KUSB_HANDLE InterfaceHandle, _in UCHAR NumberOrIndex, _in BOOL IsIndex)
	* \memberof KUSB_DRIVER_API
	* \copydoc UsbK_ClaimInterface
	*/
	KUSB_ClaimInterface* ClaimInterface;

	/*! \fn BOOL KUSB_API ReleaseInterface (_in KUSB_HANDLE InterfaceHandle, _in UCHAR NumberOrIndex, _in BOOL IsIndex)
	* \memberof KUSB_DRIVER_API
	* \copydoc UsbK_ReleaseInterface
	*/
	KUSB_ReleaseInterface* ReleaseInterface;

	/*! \fn BOOL KUSB_API SetAltInterface (_in KUSB_HANDLE InterfaceHandle, _in UCHAR NumberOrIndex, _in BOOL IsIndex, _in UCHAR AltSettingNumber)
	* \memberof KUSB_DRIVER_API
	* \copydoc UsbK_SetAltInterface
	*/
	KUSB_SetAltInterface* SetAltInterface;

	/*! \fn BOOL KUSB_API GetAltInterface (_in KUSB_HANDLE InterfaceHandle, _in UCHAR NumberOrIndex, _in BOOL IsIndex, _out PUCHAR AltSettingNumber)
	* \memberof KUSB_DRIVER_API
	* \copydoc UsbK_GetAltInterface
	*/
	KUSB_GetAltInterface* GetAltInterface;

	/*! \fn BOOL KUSB_API GetDescriptor (_in KUSB_HANDLE InterfaceHandle, _in UCHAR DescriptorType, _in UCHAR Index, _in USHORT LanguageID, _out PUCHAR Buffer, _in UINT BufferLength, _outopt PUINT LengthTransferred)
	* \memberof KUSB_DRIVER_API
	* \copydoc UsbK_GetDescriptor
	*/
	KUSB_GetDescriptor* GetDescriptor;

	/*! \fn BOOL KUSB_API ControlTransfer (_in KUSB_HANDLE InterfaceHandle, _in WINUSB_SETUP_PACKET SetupPacket, _refopt PUCHAR Buffer, _in UINT BufferLength, _outopt PUINT LengthTransferred, _inopt LPOVERLAPPED Overlapped)
	* \memberof KUSB_DRIVER_API
	* \copydoc UsbK_ControlTransfer
	*/
	KUSB_ControlTransfer* ControlTransfer;

	/*! \fn BOOL KUSB_API SetPowerPolicy (_in KUSB_HANDLE InterfaceHandle, _in UINT PolicyType, _in UINT ValueLength, _in PVOID Value)
	* \memberof KUSB_DRIVER_API
	* \copydoc UsbK_SetPowerPolicy
	*/
	KUSB_SetPowerPolicy* SetPowerPolicy;

	/*! \fn BOOL KUSB_API GetPowerPolicy (_in KUSB_HANDLE InterfaceHandle, _in UINT PolicyType, _ref PUINT ValueLength, _out PVOID Value)
	* \memberof KUSB_DRIVER_API
	* \copydoc UsbK_GetPowerPolicy
	*/
	KUSB_GetPowerPolicy* GetPowerPolicy;

	/*! \fn BOOL KUSB_API SetConfiguration (_in KUSB_HANDLE InterfaceHandle, _in UCHAR ConfigurationNumber)
	* \memberof KUSB_DRIVER_API
	* \copydoc UsbK_SetConfiguration
	*/
	KUSB_SetConfiguration* SetConfiguration;

	/*! \fn BOOL KUSB_API GetConfiguration (_in KUSB_HANDLE InterfaceHandle, _out PUCHAR ConfigurationNumber)
	* \memberof KUSB_DRIVER_API
	* \copydoc UsbK_GetConfiguration
	*/
	KUSB_GetConfiguration* GetConfiguration;

	/*! \fn BOOL KUSB_API ResetDevice (_in KUSB_HANDLE InterfaceHandle)
	* \memberof KUSB_DRIVER_API
	* \copydoc UsbK_ResetDevice
	*/
	KUSB_ResetDevice* ResetDevice;

	/*! \fn BOOL KUSB_API Initialize (_in HANDLE DeviceHandle, _out KUSB_HANDLE* InterfaceHandle)
	* \memberof KUSB_DRIVER_API
	* \copydoc UsbK_Initialize
	*/
	KUSB_Initialize* Initialize;

	/*! \fn BOOL KUSB_API SelectInterface (_in KUSB_HANDLE InterfaceHandle, _in UCHAR NumberOrIndex, _in BOOL IsIndex)
	* \memberof KUSB_DRIVER_API
	* \copydoc UsbK_SelectInterface
	*/
	KUSB_SelectInterface* SelectInterface;

	/*! \fn BOOL KUSB_API GetAssociatedInterface (_in KUSB_HANDLE InterfaceHandle, _in UCHAR AssociatedInterfaceIndex, _out KUSB_HANDLE* AssociatedInterfaceHandle)
	* \memberof KUSB_DRIVER_API
	* \copydoc UsbK_GetAssociatedInterface
	*/
	KUSB_GetAssociatedInterface* GetAssociatedInterface;

	/*! \fn BOOL KUSB_API Clone (_in KUSB_HANDLE InterfaceHandle, _out KUSB_HANDLE* DstInterfaceHandle)
	* \memberof KUSB_DRIVER_API
	* \copydoc UsbK_Clone
	*/
	KUSB_Clone* Clone;

	/*! \fn BOOL KUSB_API QueryInterfaceSettings (_in KUSB_HANDLE InterfaceHandle, _in UCHAR AltSettingIndex, _out PUSB_INTERFACE_DESCRIPTOR UsbAltInterfaceDescriptor)
	* \memberof KUSB_DRIVER_API
	* \copydoc UsbK_QueryInterfaceSettings
	*/
	KUSB_QueryInterfaceSettings* QueryInterfaceSettings;

	/*! \fn BOOL KUSB_API QueryDeviceInformation (_in KUSB_HANDLE InterfaceHandle, _in UINT InformationType, _ref PUINT BufferLength, _ref PVOID Buffer)
	* \memberof KUSB_DRIVER_API
	* \copydoc UsbK_QueryDeviceInformation
	*/
	KUSB_QueryDeviceInformation* QueryDeviceInformation;

	/*! \fn BOOL KUSB_API SetCurrentAlternateSetting (_in KUSB_HANDLE InterfaceHandle, _in UCHAR AltSettingNumber)
	* \memberof KUSB_DRIVER_API
	* \copydoc UsbK_SetCurrentAlternateSetting
	*/
	KUSB_SetCurrentAlternateSetting* SetCurrentAlternateSetting;

	/*! \fn BOOL KUSB_API GetCurrentAlternateSetting (_in KUSB_HANDLE InterfaceHandle, _out PUCHAR AltSettingNumber)
	* \memberof KUSB_DRIVER_API
	* \copydoc UsbK_GetCurrentAlternateSetting
	*/
	KUSB_GetCurrentAlternateSetting* GetCurrentAlternateSetting;

	/*! \fn BOOL KUSB_API QueryPipe (_in KUSB_HANDLE InterfaceHandle, _in UCHAR AltSettingNumber, _in UCHAR PipeIndex, _out PWINUSB_PIPE_INFORMATION PipeInformation)
	* \memberof KUSB_DRIVER_API
	* \copydoc UsbK_QueryPipe
	*/
	KUSB_QueryPipe* QueryPipe;

	/*! \fn BOOL KUSB_API SetPipePolicy (_in KUSB_HANDLE InterfaceHandle, _in UCHAR PipeID, _in UINT PolicyType, _in UINT ValueLength, _in PVOID Value)
	* \memberof KUSB_DRIVER_API
	* \copydoc UsbK_SetPipePolicy
	*/
	KUSB_SetPipePolicy* SetPipePolicy;

	/*! \fn BOOL KUSB_API GetPipePolicy (_in KUSB_HANDLE InterfaceHandle, _in UCHAR PipeID, _in UINT PolicyType, _ref PUINT ValueLength, _out PVOID Value)
	* \memberof KUSB_DRIVER_API
	* \copydoc UsbK_GetPipePolicy
	*/
	KUSB_GetPipePolicy* GetPipePolicy;

	/*! \fn BOOL KUSB_API ReadPipe (_in KUSB_HANDLE InterfaceHandle, _in UCHAR PipeID, _out PUCHAR Buffer, _in UINT BufferLength, _outopt PUINT LengthTransferred, _inopt LPOVERLAPPED Overlapped)
	* \memberof KUSB_DRIVER_API
	* \copydoc UsbK_ReadPipe
	*/
	KUSB_ReadPipe* ReadPipe;

	/*! \fn BOOL KUSB_API WritePipe (_in KUSB_HANDLE InterfaceHandle, _in UCHAR PipeID, _in PUCHAR Buffer, _in UINT BufferLength, _outopt PUINT LengthTransferred, _inopt LPOVERLAPPED Overlapped)
	* \memberof KUSB_DRIVER_API
	* \copydoc UsbK_WritePipe
	*/
	KUSB_WritePipe* WritePipe;

	/*! \fn BOOL KUSB_API ResetPipe (_in KUSB_HANDLE InterfaceHandle, _in UCHAR PipeID)
	* \memberof KUSB_DRIVER_API
	* \copydoc UsbK_ResetPipe
	*/
	KUSB_ResetPipe* ResetPipe;

	/*! \fn BOOL KUSB_API AbortPipe (_in KUSB_HANDLE InterfaceHandle, _in UCHAR PipeID)
	* \memberof KUSB_DRIVER_API
	* \copydoc UsbK_AbortPipe
	*/
	KUSB_AbortPipe* AbortPipe;

	/*! \fn BOOL KUSB_API FlushPipe (_in KUSB_HANDLE InterfaceHandle, _in UCHAR PipeID)
	* \memberof KUSB_DRIVER_API
	* \copydoc UsbK_FlushPipe
	*/
	KUSB_FlushPipe* FlushPipe;

	/*! \fn BOOL KUSB_API IsoReadPipe (_in KUSB_HANDLE InterfaceHandle, _in UCHAR PipeID, _out PUCHAR Buffer, _in UINT BufferLength, _in LPOVERLAPPED Overlapped, _refopt PKISO_CONTEXT IsoContext)
	* \memberof KUSB_DRIVER_API
	* \copydoc UsbK_IsoReadPipe
	*/
	KUSB_IsoReadPipe* IsoReadPipe;

	/*! \fn BOOL KUSB_API IsoWritePipe (_in KUSB_HANDLE InterfaceHandle, _in UCHAR PipeID, _in PUCHAR Buffer, _in UINT BufferLength, _in LPOVERLAPPED Overlapped, _refopt PKISO_CONTEXT IsoContext)
	* \memberof KUSB_DRIVER_API
	* \copydoc UsbK_IsoWritePipe
	*/
	KUSB_IsoWritePipe* IsoWritePipe;

	/*! \fn BOOL KUSB_API GetCurrentFrameNumber (_in KUSB_HANDLE InterfaceHandle, _out PUINT FrameNumber)
	* \memberof KUSB_DRIVER_API
	* \copydoc UsbK_GetCurrentFrameNumber
	*/
	KUSB_GetCurrentFrameNumber* GetCurrentFrameNumber;

	/*! \fn BOOL KUSB_API GetOverlappedResult (_in KUSB_HANDLE InterfaceHandle, _in LPOVERLAPPED Overlapped, _out PUINT lpNumberOfBytesTransferred, _in BOOL bWait)
	* \memberof KUSB_DRIVER_API
	* \copydoc UsbK_GetOverlappedResult
	*/
	KUSB_GetOverlappedResult* GetOverlappedResult;

	/*! \fn BOOL KUSB_API GetProperty (_in KUSB_HANDLE InterfaceHandle, _in KUSB_PROPERTY PropertyType, _ref PUINT PropertySize, _out PVOID Value)
	* \memberof KUSB_DRIVER_API
	* \copydoc UsbK_GetProperty
	*/
	KUSB_GetProperty* GetProperty;

	//! fixed structure padding.
	UCHAR z_F_i_x_e_d[512 - sizeof(KUSB_DRIVER_API_INFO) -  sizeof(UINT_PTR) * KUSB_FNID_COUNT];

} KUSB_DRIVER_API;
typedef KUSB_DRIVER_API* PKUSB_DRIVER_API;
C_ASSERT(sizeof(KUSB_DRIVER_API) == 512);
/**@}*/
#endif

#ifndef _LIBUSBK_HOTK_TYPES

/*! \addtogroup hotk
* @{
*/

//! Hot plug config flags.
typedef enum _KHOT_FLAG
{
    //! No flags (or 0)
    KHOT_FLAG_NONE,

    //! Notify all devices which match upon a succuessful call to \ref HotK_Init.
    KHOT_FLAG_PLUG_ALL_ON_INIT				= 0x0001,

    //! Allow other \ref KHOT_HANDLE instances to consume this match.
    KHOT_FLAG_PASS_DUPE_INSTANCE			= 0x0002,

    //! If a \c UserHwnd is specified, use \c PostMessage instead of \c SendMessage.
    KHOT_FLAG_POST_USER_MESSAGE				= 0x0004,
} KHOT_FLAG;

//! Hot plug event function definition.
typedef VOID KUSB_API KHOT_PLUG_CB(
    _in KHOT_HANDLE HotHandle,
    _in KLST_DEVINFO_HANDLE DeviceInfo,
    _in KLST_SYNC_FLAG PlugType);

//! Power broadcast event function definition.
typedef VOID KUSB_API KHOT_POWER_BROADCAST_CB(
    _in KHOT_HANDLE HotHandle,
    _in KLST_DEVINFO_HANDLE DeviceInfo,
    _in UINT PbtEvent);

//! Hot plug parameter structure.
/*!
* \fixedstruct{2048}
*
* This structure is intially passed as a parameter to \ref HotK_Init.
*
*/
typedef struct _KHOT_PARAMS
{
	//! Hot plug event window handle to send/post messages when notifications occur.
	HWND UserHwnd;

	//! WM_USER message start offset used when sending/posting messages, See details.
	/*!
	* \attention The \ref hotk will send UserMessage+1 for arrival notifications and UserMessage+0 for removal notifications.
	*
	* - WPARAM = \ref KHOT_HANDLE
	* - LPARAM = \ref KLST_DEVINFO_HANDLE
	*/
	UINT UserMessage;

	//! Additional init/config parameters
	KHOT_FLAG Flags;

	//! File pattern matches for restricting notifcations to a single/group or all supported usb devices.
	KLST_PATTERN_MATCH PatternMatch;

	//! Hot plug event callback function invoked when notifications occur.
	/*! \fn VOID KUSB_API OnHotPlug (_in KHOT_HANDLE HotHandle, _in KLST_DEVINFO_HANDLE DeviceInfo, _in KLST_SYNC_FLAG PlugType)
	* \memberof KHOT_PARAMS
	*/
	KHOT_PLUG_CB* OnHotPlug;

	//! \b WM_POWERBROADCAST event callback function invoked when a power-management event has occurred.
	/*! \fn VOID KUSB_API OnPowerBroadcast (_in KHOT_HANDLE HotHandle, _in KLST_DEVINFO_HANDLE DeviceInfo, _in UINT PbtEvent)
	* \memberof KHOT_PARAMS
	*/
	KHOT_POWER_BROADCAST_CB* OnPowerBroadcast;

	//! fixed structure padding.
	UCHAR z_F_i_x_e_d[2048 - sizeof(KLST_PATTERN_MATCH) - sizeof(UINT_PTR) * 3 - sizeof(UINT) * 2];

} KHOT_PARAMS;
C_ASSERT(sizeof(KHOT_PARAMS) == 2048);

//! Pointer to a \ref KHOT_PARAMS structure.
typedef KHOT_PARAMS* PKHOT_PARAMS;

/**@}*/

#endif

#ifndef _LIBUSBK_OVLK_TYPES

/*! \addtogroup ovlk
*  @{
*/

//! \c WaitFlags used by \ref OvlK_Wait.
/*!
*
*/
typedef enum _KOVL_WAIT_FLAG
{
    //! Do not perform any additional actions upon exiting \ref OvlK_Wait.
    KOVL_WAIT_FLAG_NONE							= 0L,

    //! If the i/o operation completes successfully, release the OverlappedK back to it's pool.
    KOVL_WAIT_FLAG_RELEASE_ON_SUCCESS			= 0x0001,

    //! If the i/o operation fails, release the OverlappedK back to it's pool.
    KOVL_WAIT_FLAG_RELEASE_ON_FAIL				= 0x0002,

    //! If the i/o operation fails or completes successfully, release the OverlappedK back to its pool. Perform no actions if it times-out.
    KOVL_WAIT_FLAG_RELEASE_ON_SUCCESS_FAIL		= 0x0003,

    //! If the i/o operation times-out cancel it, but do not release the OverlappedK back to its pool.
    KOVL_WAIT_FLAG_CANCEL_ON_TIMEOUT			= 0x0004,

    //! If the i/o operation times-out, cancel it and release the OverlappedK back to its pool.
    KOVL_WAIT_FLAG_RELEASE_ON_TIMEOUT			= 0x000C,

    //! Always release the OverlappedK back to its pool.  If the operation timed-out, cancel it before releasing back to its pool.
    KOVL_WAIT_FLAG_RELEASE_ALWAYS				= 0x000F,

    //! Uses alterable wait functions.  See http://msdn.microsoft.com/en-us/library/windows/desktop/ms687036%28v=vs.85%29.aspx
    KOVL_WAIT_FLAG_ALERTABLE					= 0x0010,

} KOVL_WAIT_FLAG;

//! \c Overlapped pool config flags.
/*!
* \attention Currently not used.
*/
typedef enum _KOVL_POOL_FLAG
{
    KOVL_POOL_FLAG_NONE	= 0L,
} KOVL_POOL_FLAG;

/**@}*/

#endif

#ifndef _LIBUSBK_STMK_TYPES

/*! \addtogroup stmk
*  @{
*/
//! Stream config flags.
/*!
* \attention Currently not used.
*/
typedef enum _KSTM_FLAG
{
    //! None
    KSTM_FLAG_NONE			   = 0L,
    KSTM_FLAG_NO_PARTIAL_XFERS = 0x00100000,
    KSTM_FLAG_USE_TIMEOUT	= 0x80000000,
    KSTM_FLAG_TIMEOUT_MASK	= 0x0001FFFF
} KSTM_FLAG;

//! Stream config flags.
/*!
* \attention Currently not used.
*/
typedef enum _KSTM_COMPLETE_RESULT
{
    //! Valid
    KSTM_COMPLETE_RESULT_VALID		= 0L,
    //! Invalid
    KSTM_COMPLETE_RESULT_INVALID,
} KSTM_COMPLETE_RESULT;

//! Stream transfer context structure.
/*!
* This structure is passed into the stream callback functions.
* The stream transfer context list is allocated upon a successful call to \ref StmK_Init.  There is one
* transfer context for each transfer. (0 to \c MaxPendingTransfers).
*
*/
typedef struct _KSTM_XFER_CONTEXT
{

	//! Internal stream buffer.
	PUCHAR Buffer;

	//! Size of internal stream buffer.
	INT BufferSize;

	//! Number of bytes to write or number of bytes read.
	INT TransferLength;

	//! User defined state.
	PVOID UserState;

} KSTM_XFER_CONTEXT;
//! Pointer to a \ref KSTM_XFER_CONTEXT structure.
typedef KSTM_XFER_CONTEXT* PKSTM_XFER_CONTEXT;

//! Stream information structure.
/*!
* This structure is passed into the stream callback functions.
* The stream context is allocated upon a successful call to \ref StmK_Init.  There is only one
* stream context per stream.
*
*/
typedef struct _KSTM_INFO
{
	//! \ref KUSB_HANDLE this stream uses.
	KUSB_HANDLE UsbHandle;

	//! This parameter corresponds to the bEndpointAddress field in the endpoint descriptor.
	UCHAR PipeID;

	//! Maximum transfer read/write request allowed pending.
	INT MaxPendingTransfers;

	//! Maximum transfer sage size.
	INT MaxTransferSize;

	//! Maximum number of I/O request allowed pending.
	INT MaxPendingIO;

	//! Populated with the endpoint descriptor for the specified \c PipeID.
	USB_ENDPOINT_DESCRIPTOR EndpointDescriptor;

	//! Populated with the driver api for the specified \c UsbHandle.
	KUSB_DRIVER_API DriverAPI;

	//! Populated with the device file handle for the specified \c UsbHandle.
	HANDLE DeviceHandle;

	//! Stream handle.
	KSTM_HANDLE StreamHandle;

	//! Stream info user defined state.
	PVOID UserState;

} KSTM_INFO;
//! Pointer to a \ref KSTM_INFO structure.
typedef KSTM_INFO* PKSTM_INFO;

//! Function definition for an optional user-defined callback; executed when a transfer error occurs.
/*! \fn INT KUSB_API KSTM_ERROR_CB(_in PKSTM_INFO StreamInfo, _in PKSTM_XFER_CONTEXT XferContext, _in INT XferContextIndex, _in INT ErrorCode)
* \memberof KSTM_CALLBACK
*/
typedef INT KUSB_API KSTM_ERROR_CB(_in PKSTM_INFO StreamInfo, _in PKSTM_XFER_CONTEXT XferContext, _in INT XferContextIndex, _in INT ErrorCode);

//! Function definition for an optional user-defined callback; executed to submit a transfer.
/*! \fn INT KUSB_API KSTM_SUBMIT_CB(_in PKSTM_INFO StreamInfo, _in PKSTM_XFER_CONTEXT XferContext, _in INT XferContextIndex, _in LPOVERLAPPED Overlapped)
* \memberof KSTM_CALLBACK
*/
typedef INT KUSB_API KSTM_SUBMIT_CB(_in PKSTM_INFO StreamInfo, _in PKSTM_XFER_CONTEXT XferContext, _in INT XferContextIndex, _in LPOVERLAPPED Overlapped);

//! Function definition for an optional user-defined callback; executed for each transfer context when the stream is started with \ref StmK_Start.
/*! \fn INT KUSB_API KSTM_STARTED_CB(_in PKSTM_INFO StreamInfo, _in PKSTM_XFER_CONTEXT XferContext, _in INT XferContextIndex)
* \memberof KSTM_CALLBACK
*/
typedef INT KUSB_API KSTM_STARTED_CB(_in PKSTM_INFO StreamInfo, _in PKSTM_XFER_CONTEXT XferContext, _in INT XferContextIndex);

//! Function definition for an optional user-defined callback; executed for each transfer context when the stream is stopped with \ref StmK_Stop.
/*! \fn INT KUSB_API KSTM_STOPPED_CB(_in PKSTM_INFO StreamInfo, _in PKSTM_XFER_CONTEXT XferContext, _in INT XferContextIndex)
* \memberof KSTM_CALLBACK
*/
typedef INT KUSB_API KSTM_STOPPED_CB(_in PKSTM_INFO StreamInfo, _in PKSTM_XFER_CONTEXT XferContext, _in INT XferContextIndex);

//! Function definition for an optional user-defined callback; executed when a valid transfer completes.
/*! \fn INT KUSB_API KSTM_COMPLETE_CB(_in PKSTM_INFO StreamInfo, _in PKSTM_XFER_CONTEXT XferContext, _in INT XferContextIndex, _in INT ErrorCode)
* \memberof KSTM_CALLBACK
*/
typedef INT KUSB_API KSTM_COMPLETE_CB(_in PKSTM_INFO StreamInfo, _in PKSTM_XFER_CONTEXT XferContext, _in INT XferContextIndex, _in INT ErrorCode);

//! Function definition for an optional user-defined callback; executed immediately after a transfer completes.
/*! \fn KSTM_COMPLETE_RESULT KUSB_API KSTM_BEFORE_COMPLETE_CB(_in PKSTM_INFO StreamInfo, _in PKSTM_XFER_CONTEXT XferContext, _in INT XferContextIndex, _ref PINT ErrorCode)
* \memberof KSTM_CALLBACK
*
* This callback function allows the user to accept or reject the transfer:
* - IN (Read, DeviceToHost) endpoints.
*   - KSTM_COMPLETE_RESULT_VALID
*     Continue normal processing; add the transfer to the internal complete list and make it available to \ref StmK_Read.
*   - KSTM_COMPLETE_RESULT_INVALID
*     Ignore this transfer.
* - OUT (Write, HostToDevice) endpoints.
*   - KSTM_COMPLETE_RESULT_VALID
*     Continue normal processing; add the transfer to the internal complete list and make it available subsequent \ref StmK_Write requests.
*   - KSTM_COMPLETE_RESULT_INVALID
*     Return this transfer to the internal queued list for automatic resubmission to the device.
*
*/
typedef KSTM_COMPLETE_RESULT KUSB_API KSTM_BEFORE_COMPLETE_CB(_in PKSTM_INFO StreamInfo, _in PKSTM_XFER_CONTEXT XferContext, _in INT XferContextIndex, _in PINT ErrorCode);

//! Stream callback structure.
/*!
* \fixedstruct{64}
*
* These optional callback functions are executed from the streams internal thread at various stages of operation.
*
*/
typedef struct _KSTM_CALLBACK
{
	//! Executed when a transfer error occurs.
	KSTM_ERROR_CB* Error;

	//! Executed to submit a transfer.
	KSTM_SUBMIT_CB* Submit;

	//! Executed when a valid transfer completes.
	KSTM_COMPLETE_CB* Complete;

	//! Executed for every transfer context when the stream is started with \ref StmK_Start.
	KSTM_STARTED_CB* Started;

	//! Executed for every transfer context when the stream is stopped with \ref StmK_Stop.
	KSTM_STOPPED_CB* Stopped;

	//! Executed immediately after a transfer completes.
	KSTM_BEFORE_COMPLETE_CB* BeforeComplete;

	//! fixed structure padding.
	UCHAR z_F_i_x_e_d[64 - sizeof(UINT_PTR) * 6];

} KSTM_CALLBACK;
//! Pointer to a \ref KSTM_CALLBACK structure.
typedef KSTM_CALLBACK* PKSTM_CALLBACK;
C_ASSERT(sizeof(KSTM_CALLBACK) == 64);

/**@}*/

#endif
///////////////////////////////////////////////////////////////////////
// L I B U S B K  PUBLIC FUNCTIONS ////////////////////////////////////
///////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _LIBUSBK_LIBK_FUNCTIONS
	/*! \addtogroup libk
	* @{
	*/

//! Gets the internal user context for the specified \ref KLIB_HANDLE.
	/*!
	*
	* \param[out] Version
	* Receives the libusbK library verson information.
	*
	* \returns NONE
	*/
	KUSB_EXP VOID KUSB_API LibK_GetVersion(_out PKLIB_VERSION Version);

//! Gets the internal user context for the specified \ref KLIB_HANDLE.
	/*!
	*
	* \param[in] Handle
	* The handle containg the context to retrieve.
	*
	* \param[in] HandleType
	* Handle type of \c Handle.
	*
	* \returns
	* - on success, The user context value.
	* - On failure, returns NULL and sets last error to \c ERROR_INVALID_HANDLE.
	*
	*/
	KUSB_EXP KLIB_USER_CONTEXT KUSB_API LibK_GetContext(
	    _in KLIB_HANDLE Handle,
	    _in KLIB_HANDLE_TYPE HandleType);

//! Sets internal user context for the specified \ref KLIB_HANDLE.
	/*!
	*
	* \param[in] Handle
	* The handle containg the context to set.
	*
	* \param[in] HandleType
	* Handle type of \c Handle.
	*
	* \param[in] ContextValue
	* Value to assign to the handle user context space.
	*
	* \returns On success, TRUE. Otherwise FALSE. Use \c GetLastError() to get extended error information.
	*
	*/
	KUSB_EXP BOOL KUSB_API LibK_SetContext(
	    _in KLIB_HANDLE Handle,
	    _in KLIB_HANDLE_TYPE HandleType,
	    _in KLIB_USER_CONTEXT ContextValue);

//! Assigns a cleanup callback function to a \ref KLIB_HANDLE.
	/*!
	*
	* \param[in] Handle
	* The handle containg the cleanup callback function to set.
	*
	* \param[in] HandleType
	* Handle type of \c Handle.
	*
	* \param[in] CleanupCB
	* User supplied callback function to execute when the handles internal reference count reaches 0.
	*
	* \returns On success, TRUE. Otherwise FALSE. Use \c GetLastError() to get extended error information.
	*
	*/
	KUSB_EXP BOOL KUSB_API LibK_SetCleanupCallback(
	    _in KLIB_HANDLE Handle,
	    _in KLIB_HANDLE_TYPE HandleType,
	    _in KLIB_HANDLE_CLEANUP_CB* CleanupCB);

//! Initialize a driver API set.
	/*!
	*
	* \param[out] DriverAPI
	* A driver API structure to populate.
	*
	* \param[in] DriverID
	* The driver id of the API set to retrieve. See \ref KUSB_DRVID
	*
	* \returns On success, TRUE. Otherwise FALSE. Use \c GetLastError() to get extended error information.
	*
	*/
	KUSB_EXP BOOL KUSB_API LibK_LoadDriverAPI(
	    _out PKUSB_DRIVER_API DriverAPI,
	    _in INT DriverID);

//! Copies the driver API set out of a \ref KUSB_HANDLE
	/*!
	*
	* \param[out] DriverAPI
	* A driver API structure to populate.
	*
	* \param[in] UsbHandle
	* Handle containing the desired driver API.
	*
	* \returns On success, TRUE. Otherwise FALSE. Use \c GetLastError() to get extended error information.
	*
	*/
	KUSB_EXP BOOL KUSB_API LibK_CopyDriverAPI(
	    _out PKUSB_DRIVER_API DriverAPI,
	    _in KUSB_HANDLE UsbHandle);

//! Initialize a driver API function.
	/*!
	* \param[out] ProcAddress
	* Reference to a function pointer that will receive the API function pointer.
	*
	* \param[in] DriverID
	* The driver id of the API to use. See \ref KUSB_DRVID
	*
	* \param[in] FunctionID
	* The function id. See \ref KUSB_FNID
	*
	* \returns On success, TRUE. Otherwise FALSE. Use \c GetLastError() to get extended error information.
	*
	*/
	KUSB_EXP BOOL KUSB_API LibK_GetProcAddress(
	    _out KPROC* ProcAddress,
	    _in INT DriverID,
	    _in INT FunctionID);

//! Sets the default user context for the specified \ref KLIB_HANDLE_TYPE.
	/*!
	*
	* \param[in] HandleType
	* The handle type which will be assigned the default ContextValue.
	*
	* \param[in] ContextValue
	* Value assigned to the default user context for the specified \ref KLIB_HANDLE_TYPE.
	*
	* \returns On success, TRUE. Otherwise FALSE. Use \c GetLastError() to get extended error information.
	*
	*/
	KUSB_EXP BOOL KUSB_API LibK_SetDefaultContext(
	    _in KLIB_HANDLE_TYPE HandleType,
	    _in KLIB_USER_CONTEXT ContextValue);

//! Gets the default user context for the specified \ref KLIB_HANDLE_TYPE.
	/*!
	*
	* \param[in] HandleType
	* Handle type used to retrieve the default user context.
	*
	* \returns
	* - on success, The default user context value.
	* - On failure, returns NULL and sets last error to \c ERROR_INVALID_HANDLE.
	*
	*/
	KUSB_EXP KLIB_USER_CONTEXT KUSB_API LibK_GetDefaultContext(
	    _in KLIB_HANDLE_TYPE HandleType);

//! Initializes the global libusbK process context.
	/*!
	*
	* If this function is not called at startup, libusbK initializes the global libusbK process context automatically.
	*
	* \param[in] Heap
	* A handle to the memory heap libusbK will use for dynamic memory allocation.
	* \note The process context itself is always allocated from the proccess heap.
	* \note If \b Heap is \b NULL, dynamic memory is allocated from the proccess heap.
	*
	* \param[in] Reserved
	* Reserved for future use.  Must set to \b NULL.
	*
	* \returns On success, TRUE. Otherwise FALSE. Use \c GetLastError() to get extended error information.
	*
	*/
	KUSB_EXP BOOL KUSB_API LibK_Context_Init(
	    _inopt HANDLE Heap,
	    _in PVOID Reserved);

//! Frees the global libusbK process context.
	/*!
	*
	* If this function is not called on exit, libusbK frees the global libusbK process context automatically when it terminates.
	*
	* \returns NONE.
	*
	*/
	KUSB_EXP VOID KUSB_API LibK_Context_Free(VOID);


	/**@}*/
#endif

#ifndef _LIBUSBK_USBK_FUNCTIONS
	/*! \addtogroup usbk
	*  @{
	*/

//! Creates/opens a libusbK interface handle from the device list. This is a preferred method.
	/*!
	*
	* \param[out] InterfaceHandle
	* Receives a handle configured to the first (default) interface on the device. This handle is required by
	* other libusbK routines that perform operations on the default interface. The handle is opaque. To release
	* this handle, call the \ref UsbK_Free function.
	*
	* \param[in] DevInfo
	* The device list element to open.
	*
	* \returns On success, TRUE. Otherwise FALSE. Use \c GetLastError() to get extended error information.
	*
	* \c UsbK_Init performs the same tasks as \ref UsbK_Initialize with the following exceptions:
	* - Uses a \ref KLST_DEVINFO instead of a file handle created with the Windows CreateFile() API function.
	* - File handles are managed internally and are closed when the last \ref KUSB_HANDLE is closed with
	*   \ref UsbK_Free. To obtain the internal file device handle, See \ref UsbK_GetProperty.
	*
	* \note
	* A \c KUSB_HANDLE is required by other library routines that perform operations on a device. Once
	* initialized, it can access all interfaces/endpoints of a device. An initialized handle can be cloned with
	* \ref UsbK_Clone or \ref UsbK_GetAssociatedInterface. A Cloned handle will behave just as the orignal
	* except in will maintain it's own \b selected interface setting.
	*
	*/
	KUSB_EXP BOOL KUSB_API UsbK_Init (
	    _out KUSB_HANDLE* InterfaceHandle,
	    _in KLST_DEVINFO_HANDLE DevInfo);

//! Frees a libusbK interface handle.
	/*!
	*
	* \param[in] InterfaceHandle
	* Handle to an interface on the device. This handle must be created by a previous call to \ref UsbK_Init,
	* \ref UsbK_Initialize, \ref UsbK_GetAssociatedInterface, or \ref UsbK_Clone.
	*
	* \returns TRUE.
	*
	* The \ref UsbK_Free function releases resources alocated to the InterfaceHandle.
	*
	*/
	KUSB_EXP BOOL KUSB_API UsbK_Free (
	    _in KUSB_HANDLE InterfaceHandle);

//! Claims the specified interface by number or index.
	/*!
	*
	* \param[in] InterfaceHandle
	* An initialized usb handle, see \ref UsbK_Init.
	*
	* \param[in] NumberOrIndex
	* Interfaces can be claimed or released by a interface index or \c bInterfaceNumber.
	* - Interface indexes always start from 0 and continue sequentially for all interfaces of the device.
	* - An interface number always represents the actual \ref USB_INTERFACE_DESCRIPTOR::bInterfaceNumber.
	*   Interface numbers are not guaranteed to be zero based or sequential.
	*
	* \param[in] IsIndex
	* If TRUE, \c NumberOrIndex represents an interface index.\n if FALSE \c NumberOrIndex represents a
	* \c bInterfaceNumber.
	*
	* \returns On success, TRUE. Otherwise FALSE. Use \c GetLastError() to get extended error information.
	*
	* Claiming an interface allows applications a way to prevent other applications or multiple instances of the
	* same application from using an interface at the same time.
	*
	* When an interface is claimed with \ref UsbK_ClaimInterface it performs the following actions:
	* - Checks if the interface exists. If it does not, returns FALSE and sets last error to
	*   ERROR_NO_MORE_ITEMS.
	* - The default (or current) interface for the device is changed to \c NumberOrIndex.
	* - libusb0.sys and libusbK.sys:
	*   - A request to claim the interface is sent to the driver. If the interface is not claimed or already
	*     claimed by the application the request succeeds. If the interface is claimed by another application,
	*     \ref UsbK_ClaimInterface returns FALSE and sets last error to \c ERROR_BUSY. In this case the The
	*     default (or current) interface for the device is \b still changed to \c NumberOrIndex.
	* - WinUSB.sys: All WinUSB device interfaces are claimed when the device is opened. This function performs
	*   identically to \ref UsbK_SelectInterface
	*
	*/
	KUSB_EXP BOOL KUSB_API UsbK_ClaimInterface (
	    _in KUSB_HANDLE InterfaceHandle,
	    _in UCHAR NumberOrIndex,
	    _in BOOL IsIndex);

//! Releases the specified interface by number or index.
	/*!
	*
	* \param[in] InterfaceHandle
	* An initialized usb handle, see \ref UsbK_Init.
	*
	* \param[in] NumberOrIndex
	* Interfaces can be claimed or released by a interface index or \c bInterfaceNumber.
	* - Interface indexes always start from 0 and continue sequentially for all interfaces of the device.
	* - An interface number always represents the actual \ref USB_INTERFACE_DESCRIPTOR::bInterfaceNumber.
	*   Interface numbers are not guaranteed to be zero based or sequential.
	*
	* \param[in] IsIndex
	* If TRUE, \c NumberOrIndex represents an interface index.\n if FALSE \c NumberOrIndex represents a
	* \c bInterfaceNumber.
	*
	* \returns On success, TRUE. Otherwise FALSE. Use \c GetLastError() to get extended error information.
	*
	* When an interface is release with \ref UsbK_ReleaseInterface it performs the following actions:
	* - Checks if the interface exists. If it does not, returns FALSE and sets last error to
	*   ERROR_NO_MORE_ITEMS.
	* - The default (or current) interface for the device is changed to the previously claimed interface.
	* - libusb0.sys and libusbK.sys:
	*   - A request to release the interface is sent to the driver. If the interface is not claimed by a
	*     different application the request succeeds. If the interface is claimed by another application,
	*     \ref UsbK_ReleaseInterface returns FALSE and sets last error to \c ERROR_BUSY. In this case, the
	*     default/current interface for the device is \b still changed to the previously claimed interface.
	* - WinUSB.sys: No other action needed, returns TRUE.
	*
	* \note
	* When an interface is released, it is moved to the bottom if an interface stack making a previously claimed
	* interface the current. This will continue to occur regardless of whether the interface is claimed. For
	* this reason, \ref UsbK_ReleaseInterface can be used as a means to change the current/default interface of
	* an \c InterfaceHandle without claiming the interface.
	*
	*/
	KUSB_EXP BOOL KUSB_API UsbK_ReleaseInterface (
	    _in KUSB_HANDLE InterfaceHandle,
	    _in UCHAR NumberOrIndex,
	    _in BOOL IsIndex);

//! Sets the alternate setting of the specified interface.
	/*!
	*
	* \param[in] InterfaceHandle
	* An initialized usb handle, see \ref UsbK_Init.
	*
	* \param[in] NumberOrIndex
	* Interfaces can be specified by a interface index or \c bInterfaceNumber.
	* - Interface indexes always start from 0 and continue sequentially for all interfaces of the device.
	* - An interface number always represents the actual \ref USB_INTERFACE_DESCRIPTOR::bInterfaceNumber.
	*   Interface numbers are not guaranteed to be zero based or sequential.
	*
	* \param[in] IsIndex
	* If TRUE, \c NumberOrIndex represents an interface index.\n if FALSE \c NumberOrIndex represents a
	* \c bInterfaceNumber.
	*
	* \param[in] AltSettingNumber
	* The bAlternateSetting to activate.
	*
	* \returns On success, TRUE. Otherwise FALSE. Use \c GetLastError() to get extended error information.
	*
	* \ref UsbK_SetAltInterface performs the same task as \ref UsbK_SetCurrentAlternateSetting except it
	* provides the option of specifying which interfaces alternate setting to activate.
	*
	*/
	KUSB_EXP BOOL KUSB_API UsbK_SetAltInterface (
	    _in KUSB_HANDLE InterfaceHandle,
	    _in UCHAR NumberOrIndex,
	    _in BOOL IsIndex,
	    _in UCHAR AltSettingNumber);

//! Gets the alternate setting for the specified interface.
	/*!
	*
	* \param[in] InterfaceHandle
	* An initialized usb handle, see \ref UsbK_Init.
	*
	* \param[in] NumberOrIndex
	* Interfaces can be specified by a interface index or \c bInterfaceNumber.
	* - Interface indexes always start from 0 and continue sequentially for all interfaces of the device.
	* - An interface number always represents the actual \ref USB_INTERFACE_DESCRIPTOR::bInterfaceNumber.
	*   Interface numbers are not guaranteed to be zero based or sequential.
	*
	* \param[in] IsIndex
	* If TRUE, \c NumberOrIndex represents an interface index.\n if FALSE \c NumberOrIndex represents a
	* \c bInterfaceNumber.
	*
	* \param[out] AltSettingNumber
	* On success, returns the active bAlternateSetting.
	*
	* \returns On success, TRUE. Otherwise FALSE. Use \c GetLastError() to get extended error information.
	*
	* \ref UsbK_GetAltInterface performs the same task as \ref UsbK_GetCurrentAlternateSetting except it
	* provides the option of specifying which interfaces alternate setting is to be retrieved.
	*
	*/
	KUSB_EXP BOOL KUSB_API UsbK_GetAltInterface (
	    _in KUSB_HANDLE InterfaceHandle,
	    _in UCHAR NumberOrIndex,
	    _in BOOL IsIndex,
	    _out PUCHAR AltSettingNumber);

//! Gets the requested descriptor. This is a synchronous operation.
	/*!
	*
	* \param[in] InterfaceHandle
	* An initialized usb handle, see \ref UsbK_Init.
	*
	* \param[in] DescriptorType
	* A value that specifies the type of descriptor to return. This parameter corresponds to the bDescriptorType
	* field of a standard device descriptor, whose values are described in the Universal Serial Bus
	* specification.
	*
	* \param[in] Index
	* The descriptor index. For an explanation of the descriptor index, see the Universal Serial Bus
	* specification (www.usb.org).
	*
	* \param[in] LanguageID
	* A value that specifies the language identifier, if the requested descriptor is a string descriptor.
	*
	* \param[out] Buffer
	* A caller-allocated buffer that receives the requested descriptor.
	*
	* \param[in] BufferLength
	* The length, in bytes, of Buffer.
	*
	* \param[out] LengthTransferred
	* The number of bytes that were copied into Buffer.
	*
	* \returns On success, TRUE. Otherwise FALSE. Use \c GetLastError() to get extended error information.
	*
	* If the device descriptor or active config descriptor is requested, \ref UsbK_GetDescriptor retrieves
	* cached data and this becomes a non-blocking, non I/O request.
	*
	*/
	KUSB_EXP BOOL KUSB_API UsbK_GetDescriptor (
	    _in KUSB_HANDLE InterfaceHandle,
	    _in UCHAR DescriptorType,
	    _in UCHAR Index,
	    _in USHORT LanguageID,
	    _out PUCHAR Buffer,
	    _in UINT BufferLength,
	    _outopt PUINT LengthTransferred);

//! Transmits control data over a default control endpoint.
	/*!
	*
	* \param[in] InterfaceHandle
	* A valid libusbK interface handle returned by:
	* - \ref UsbK_Init
	* - \ref UsbK_Initialize
	* - \ref UsbK_GetAssociatedInterface
	* - \ref UsbK_Clone
	*
	* \param[in] SetupPacket
	*  The 8-byte setup packet of type WINUSB_SETUP_PACKET.
	*
	* \param[in,out] Buffer
	* A caller-allocated buffer that contains the data to transfer.
	*
	* \param[in] BufferLength
	* The number of bytes to transfer, not including the setup packet. This number must be less than or equal to
	* the size, in bytes, of Buffer.
	*
	* \param[out] LengthTransferred
	* A pointer to a UINT variable that receives the actual number of transferred bytes. If the application
	* does not expect any data to be transferred during the data phase (BufferLength is zero), LengthTransferred
	* can be NULL.
	*
	* \param[in] Overlapped
	* An optional pointer to an OVERLAPPED structure, which is used for asynchronous operations. If this
	* parameter is specified, \ref UsbK_ControlTransfer immediately returns, and the event is signaled when the
	* operation is complete. If Overlapped is not supplied, the \ref UsbK_ControlTransfer function transfers
	* data synchronously.
	*
	* \returns On success, TRUE. Otherwise FALSE. Use \c GetLastError() to get extended error information. If an
	* \c Overlapped member is supplied and the operation succeeds this function returns FALSE and sets last
	* error to ERROR_IO_PENDING.
	*
	* A \ref UsbK_ControlTransfer is never cached. These requests always go directly to the usb device.
	*
	* \attention
	* This function should not be used for operations supported by the library.\n e.g.
	* \ref UsbK_SetConfiguration, \ref UsbK_SetAltInterface, etc..
	*
	*/
	KUSB_EXP BOOL KUSB_API UsbK_ControlTransfer (
	    _in KUSB_HANDLE InterfaceHandle,
	    _in WINUSB_SETUP_PACKET SetupPacket,
	    _refopt PUCHAR Buffer,
	    _in UINT BufferLength,
	    _outopt PUINT LengthTransferred,
	    _inopt LPOVERLAPPED Overlapped);

//! Sets the power policy for a device.
	/*!
	*
	* \param[in] InterfaceHandle
	* An initialized usb handle, see \ref UsbK_Init.
	*
	* \param[in] PolicyType
	* A value that specifies the power policy to set. The following table describes symbolic constants that are
	* defined in \ref lusbk_shared.h.
	*
	* - AUTO_SUSPEND (0x81)
	*   - Specifies the auto-suspend policy type; the power policy parameter must be specified by the caller in
	*     the Value parameter.
	*   - For auto-suspend, the Value parameter must point to a UCHAR variable.
	*   - If Value is TRUE (nonzero), the USB stack suspends the device if the device is idle. A device is idle
	*     if there are no transfers pending, or if the only pending transfers are IN transfers to interrupt or
	*     bulk endpoints.
	*   - The default value is determined by the value set in the DefaultIdleState registry setting. By default,
	*     this value is TRUE.
	*
	* - SUSPEND_DELAY (0x83)
	*   - Specifies the suspend-delay policy type; the power policy parameter must be specified by the caller in
	*     the Value parameter.
	*   - For suspend-delay, Value must point to a UINT variable.
	*   - Value specifies the minimum amount of time, in milliseconds, that the driver must wait post transfer
	*     before it can suspend the device.
	*   - The default value is determined by the value set in the DefaultIdleTimeout registry setting. By
	*     default, this value is five seconds.
	*
	* \param[in] ValueLength
	* The size, in bytes, of the buffer at Value.
	*
	* \param[in] Value
	* The new value for the power policy parameter. Data type and value for Value depends on the type of power
	* policy passed in PolicyType. For more information, see PolicyType.
	*
	* \returns On success, TRUE. Otherwise FALSE. Use \c GetLastError() to get extended error information.
	*
	* The following list summarizes the effects of changes to power management states:
	* - All pipe handles, interface handles, locks, and alternate settings are preserved across power management
	*   events.
	* - Any transfers that are in progress are suspended when a device transfers to a low power state, and they
	*   are resumed when the device is restored to a working state.
	* - The device and system must be in a working state before the client can restore a device-specific
	*   configuration. Clients can determine whether the device and system are in a working state from the
	*   WM_POWERBROADCAST message.
	* - The client can indicate that an interface is idle by calling \ref UsbK_SetPowerPolicy.
	*
	*/
	KUSB_EXP BOOL KUSB_API UsbK_SetPowerPolicy (
	    _in KUSB_HANDLE InterfaceHandle,
	    _in UINT PolicyType,
	    _in UINT ValueLength,
	    _in PVOID Value);

//! Gets the power policy for a device.
	/*!
	*
	* \param[in] InterfaceHandle
	* An initialized usb handle, see \ref UsbK_Init.
	*
	* \param[in] PolicyType
	* A value that specifies the power policy parameter to retrieve in Value. The following table describes
	* symbolic constants that are defined in \ref lusbk_shared.h.
	*
	* - AUTO_SUSPEND (0x81)
	*   - If the caller specifies a power policy of AUTO_SUSPEND, \ref UsbK_GetPowerPolicy returns the value of
	*     the auto suspend policy parameter in the Value parameter.
	*   - If Value is TRUE (that is, nonzero), the USB stack suspends the device when no transfers are pending
	*     or the only transfers pending are IN transfers on an interrupt or bulk endpoint.
	*   - The value of the DefaultIdleState registry value determines the default value of the auto suspend
	*     policy parameter.
	*   - The Value parameter must point to a UCHAR variable.
	*
	* - SUSPEND_DELAY (0x83)
	*   - If the caller specifies a power policy of SUSPEND_DELAY, \ref UsbK_GetPowerPolicy returns the value of
	*     the suspend delay policy parameter in Value.
	*   - The suspend delay policy parameter specifies the minimum amount of time, in milliseconds, that the
	*     driver must wait after any transfer before it can suspend the device.
	*   - Value must point to a UINT variable.
	*
	* \param[in,out] ValueLength
	* A pointer to the size of the buffer that Value. On output, ValueLength receives the size of the data that
	* was copied into the Value buffer.
	*
	* \param[out] Value
	* A buffer that receives the specified power policy parameter. For more information, see PolicyType.
	*
	* \returns On success, TRUE. Otherwise FALSE. Use \c GetLastError() to get extended error information.
	*
	*/
	KUSB_EXP BOOL KUSB_API UsbK_GetPowerPolicy (
	    _in KUSB_HANDLE InterfaceHandle,
	    _in UINT PolicyType,
	    _ref PUINT ValueLength,
	    _out PVOID Value);

//! Sets the device configuration number.
	/*!
	*
	* \param[in] InterfaceHandle
	* An initialized usb handle, see \ref UsbK_Init.
	*
	* \param[in] ConfigurationNumber
	* The configuration number to activate.
	*
	* \returns On success, TRUE. Otherwise FALSE. Use \c GetLastError() to get extended error information.
	*
	* \ref UsbK_SetConfiguration is only supported with libusb0.sys. If the driver in not libusb0.sys, this
	* function performs the following emulation actions:
	* - If the requested configuration number is the current configuration number, returns TRUE.
	* - If the requested configuration number is one other than the current configuration number, returns FALSE
	*   and set last error to \c ERROR_NO_MORE_ITEMS.
	*
	* This function will fail if there are pending I/O operations or there are other libusbK interface handles
	* referencing the device. \sa UsbK_Free
	*
	*/
	KUSB_EXP BOOL KUSB_API UsbK_SetConfiguration (
	    _in KUSB_HANDLE InterfaceHandle,
	    _in UCHAR ConfigurationNumber);

//! Gets the device current configuration number.
	/*!
	*
	* \param[in] InterfaceHandle
	* An initialized usb handle, see \ref UsbK_Init.
	*
	* \param[out] ConfigurationNumber
	* On success, receives the active configuration number.
	*
	* \returns On success, TRUE. Otherwise FALSE. Use \c GetLastError() to get extended error information.
	*
	*/
	KUSB_EXP BOOL KUSB_API UsbK_GetConfiguration (
	    _in KUSB_HANDLE InterfaceHandle,
	    _out PUCHAR ConfigurationNumber);

//! Resets the usb device of the specified interface handle. (port cycle).
	/*!
	*
	* \param[in] InterfaceHandle
	* An initialized usb handle, see \ref UsbK_Init.
	*
	* \returns On success, TRUE. Otherwise FALSE. Use \c GetLastError() to get extended error information.
	*
	*/
	KUSB_EXP BOOL KUSB_API UsbK_ResetDevice (
	    _in KUSB_HANDLE InterfaceHandle);

//! Creates a libusbK handle for the device specified by a file handle.
	/*!
	*
	* \attention
	* This function is supported for WinUSB API compatibility only and is not intended for new development.
	* libusbK library users should use \ref UsbK_Init instead. (regardless of the driver they've selected)
	*
	* \param[in] DeviceHandle
	* The handle to the device that CreateFile returned. libusbK uses overlapped I/O, so FILE_FLAG_OVERLAPPED
	* must be specified in the dwFlagsAndAttributes parameter of CreateFile call for DeviceHandle to have the
	* characteristics necessary for this to function properly.
	*
	* \param[out] InterfaceHandle
	* Receives a handle configured to the first (default) interface on the device. This handle is required by
	* other libusbK routines that perform operations on the default interface. The handle is opaque. To release
	* this handle, call the \ref UsbK_Free function.
	*
	* \returns On success, TRUE. Otherwise FALSE. Use \c GetLastError() to get extended error information.
	*
	* When \ref UsbK_Initialize is called, the policy settings of the interface are reset to the default values.
	*
	* The \ref UsbK_Initialize call queries the underlying USB stack for various descriptors and allocates
	* enough memory to store the retrieved descriptor data.
	*
	* \ref UsbK_Initialize first retrieves the device descriptor and then gets the associated configuration
	* descriptor. From the configuration descriptor, the call derives the associated interface descriptors and
	* stores them in an array. The interfaces in the array are identified by zero-based indexes. An index value
	* of 0 indicates the first interface (the default interface), a value of 1 indicates the second associated
	* interface, and so on. \ref UsbK_Initialize parses the default interface descriptor for the endpoint
	* descriptors and caches information such as the associated pipes or state specific data. The handle
	* received in the InterfaceHandle parameter will have its default interface configured to the first
	* interface in the array.
	*
	* If an application wants to use another interface on the device, it can call
	* \ref UsbK_GetAssociatedInterface, or \ref UsbK_ClaimInterface.
	*
	*/
	KUSB_EXP BOOL KUSB_API UsbK_Initialize (
	    _in HANDLE DeviceHandle,
	    _out KUSB_HANDLE* InterfaceHandle);

//! Selects the specified interface by number or index as the current interface.
	/*!
	*
	* \param[in] InterfaceHandle
	* An initialized usb handle, see \ref UsbK_Init.
	*
	* \param[in] NumberOrIndex
	* Interfaces can be claimed or released by a interface index or \c bInterfaceNumber.
	* - Interface indexes always start from 0 and continue sequentially for all interfaces of the device.
	* - An interface number always represents the actual \ref USB_INTERFACE_DESCRIPTOR::bInterfaceNumber.
	*   Interface numbers are not guaranteed to be zero based or sequential.
	*
	* \param[in] IsIndex
	* If TRUE, \c NumberOrIndex represents an interface index.\n if FALSE \c NumberOrIndex represents a
	* \c bInterfaceNumber.
	*
	* \returns On success, TRUE. Otherwise FALSE. Use \c GetLastError() to get extended error information.
	*
	* \sa UsbK_ClaimInterface
	*
	*/
	KUSB_EXP BOOL KUSB_API UsbK_SelectInterface (
	    _in KUSB_HANDLE InterfaceHandle,
	    _in UCHAR NumberOrIndex,
	    _in BOOL IsIndex);

//! Retrieves a handle for an associated interface.
	/*!
	*
	* \param[in] InterfaceHandle
	* An initialized usb handle, see \ref UsbK_Init.
	*
	* \param[in] AssociatedInterfaceIndex
	* An index that specifies the associated interface to retrieve. A value of 0 indicates the first associated
	* interface, a value of 1 indicates the second associated interface, and so on.
	*
	* \param[out] AssociatedInterfaceHandle
	* A handle for the associated interface. Callers must pass this interface handle to libusbK Functions
	* exposed by libusbK.dll. To close this handle, call \ref UsbK_Free.
	*
	* \returns On success, TRUE. Otherwise FALSE. Use \c GetLastError() to get extended error information.
	*
	* The \ref UsbK_GetAssociatedInterface function retrieves an opaque handle for an associated interface. This
	* is a synchronous operation.
	*
	* The first associated interface is the interface that immediately follows the current (or default)
	* interface of the specified /c InterfaceHandle.
	*
	* The handle that \ref UsbK_GetAssociatedInterface returns must be released by calling \ref UsbK_Free.
	*
	*/
	KUSB_EXP BOOL KUSB_API UsbK_GetAssociatedInterface (
	    _in KUSB_HANDLE InterfaceHandle,
	    _in UCHAR AssociatedInterfaceIndex,
	    _out KUSB_HANDLE* AssociatedInterfaceHandle);

//! Clones the specified interface handle.
	/*!
	*
	* Each cloned interface handle maintains it's own selected interface.
	*
	* \param[in] InterfaceHandle
	* An initialized usb handle, see \ref UsbK_Init.
	*
	* \param[out] DstInterfaceHandle
	* On success, the cloned return handle.
	*
	* \returns On success, TRUE. Otherwise FALSE. Use \c GetLastError() to get extended error information.
	*
	*/
	KUSB_EXP BOOL KUSB_API UsbK_Clone (
	    _in KUSB_HANDLE InterfaceHandle,
	    _out KUSB_HANDLE* DstInterfaceHandle);

//! Retrieves the interface descriptor for the specified alternate interface settings for a particular interface handle.
	/*!
	*
	* \param[in] InterfaceHandle
	* An initialized usb handle, see \ref UsbK_Init.
	*
	* \param[in] AltSettingIndex
	* A value that indicates which alternate setting index to return. A value of 0 indicates the first alternate
	* setting, a value of 1 indicates the second alternate setting, and so on.
	*
	* \param[out] UsbAltInterfaceDescriptor
	* A pointer to a caller-allocated \ref USB_INTERFACE_DESCRIPTOR structure that contains information about
	* the interface that AltSettingNumber specified.
	*
	* The \ref UsbK_QueryInterfaceSettings call searches the current/default interface array for the alternate
	* interface specified by the caller in the AltSettingIndex. If the specified alternate interface is found,
	* the function populates the caller-allocated USB_INTERFACE_DESCRIPTOR structure. If the specified alternate
	* interface is not found, then the call fails with the ERROR_NO_MORE_ITEMS code.
	*
	* To change the current/default interface, see \ref UsbK_SelectInterface and \ref UsbK_ClaimInterface
	*
	* \returns On success, TRUE. Otherwise FALSE. Use \c GetLastError() to get extended error information.
	*
	*/
	KUSB_EXP BOOL KUSB_API UsbK_QueryInterfaceSettings (
	    _in KUSB_HANDLE InterfaceHandle,
	    _in UCHAR AltSettingIndex,
	    _out PUSB_INTERFACE_DESCRIPTOR UsbAltInterfaceDescriptor);

//! Retrieves information about the physical device that is associated with a libusbK handle.
	/*!
	* \param[in] InterfaceHandle
	* An initialized usb handle, see \ref UsbK_Init.
	*
	* \param[in] InformationType
	* A value that specifies which interface information value to retrieve. On input, InformationType must have
	* the following value: \c DEVICE_SPEED (0x01).
	*
	* \param[in,out] BufferLength
	* The maximum number of bytes to read. This number must be less than or equal to the size, in bytes, of
	* Buffer. On output, BufferLength is set to the actual number of bytes that were copied into Buffer.
	*
	* \param[in,out] Buffer
	* A caller-allocated buffer that receives the requested value. On output, Buffer indicates the device speed:
	* - (0x01) low/full speed device.
	* - (0x03) high speed device.
	*
	* \returns On success, TRUE. Otherwise FALSE. Use \c GetLastError() to get extended error information.
	*
	*/
	KUSB_EXP BOOL KUSB_API UsbK_QueryDeviceInformation (
	    _in KUSB_HANDLE InterfaceHandle,
	    _in UINT InformationType,
	    _ref PUINT BufferLength,
	    _ref PVOID Buffer);

//! Sets the alternate setting of an interface.
	/*!
	*
	* \param[in] InterfaceHandle
	* An initialized usb handle, see \ref UsbK_Init.
	*
	* \param[in] AltSettingNumber
	* The value that is contained in the bAlternateSetting member of the \ref USB_INTERFACE_DESCRIPTOR
	* structure. This structure can be populated by the \ref UsbK_QueryInterfaceSettings routine.
	*
	* \returns On success, TRUE. Otherwise FALSE. Use \c GetLastError() to get extended error information.
	*
	* Sets the active bAlternateSetting for the current/default interface.
	*
	* To change the default/current interface see \ref UsbK_ClaimInterface and \ref UsbK_ReleaseInterface
	* \sa UsbK_SetAltInterface
	*
	*/
	KUSB_EXP BOOL KUSB_API UsbK_SetCurrentAlternateSetting (
	    _in KUSB_HANDLE InterfaceHandle,
	    _in UCHAR AltSettingNumber);

//! Gets the current alternate interface setting for an interface.
	/*!
	*
	* \param[in] InterfaceHandle
	* An initialized usb handle, see \ref UsbK_Init.
	*
	* \param[out] AltSettingNumber
	* A pointer to an unsigned character that receives an integer that indicates the current alternate setting.
	*
	* \returns On success, TRUE. Otherwise FALSE. Use \c GetLastError() to get extended error information.
	*
	* Gets the active bAlternateSetting for the current/default interface.
	*
	* To change the default/current interface see \ref UsbK_ClaimInterface and \ref UsbK_ReleaseInterface
	* \sa UsbK_GetAltInterface
	*
	*/
	KUSB_EXP BOOL KUSB_API UsbK_GetCurrentAlternateSetting (
	    _in KUSB_HANDLE InterfaceHandle,
	    _out PUCHAR AltSettingNumber);

//! Retrieves information about a pipe that is associated with an interface.
	/*!
	*
	* \param[in] InterfaceHandle
	* An initialized usb handle, see \ref UsbK_Init.
	*
	* \param[in] AltSettingNumber
	* A value that specifies the alternate interface to return the information for.
	*
	* \param[in] PipeIndex
	* A value that specifies the pipe to return information about. This value is not the same as the
	* bEndpointAddress field in the endpoint descriptor. A PipeIndex value of 0 signifies the first endpoint
	* that is associated with the interface, a value of 1 signifies the second endpoint, and so on. PipeIndex
	* must be less than the value in the bNumEndpoints field of the interface descriptor.
	*
	* \param[out] PipeInformation
	* A pointer, on output, to a caller-allocated \ref WINUSB_PIPE_INFORMATION structure that contains pipe
	* information.
	*
	* \returns On success, TRUE. Otherwise FALSE. Use \c GetLastError() to get extended error information.
	*
	* The \ref UsbK_QueryPipe function does not retrieve information about the control pipe.
	*
	* Each interface on the USB device can have multiple endpoints. To communicate with each of these endpoints,
	* the bus driver creates pipes for each endpoint on the interface. The pipe indices are zero-based.
	* Therefore for n number of endpoints, the pipes' indices are set from n-1. \ref UsbK_QueryPipe parses the
	* configuration descriptor to get the interface specified by the caller. It searches the interface
	* descriptor for the endpoint descriptor associated with the caller-specified pipe. If the endpoint is
	* found, the function populates the caller-allocated \ref WINUSB_PIPE_INFORMATION structure with information
	* from the endpoint descriptor.
	*
	*/
	KUSB_EXP BOOL KUSB_API UsbK_QueryPipe (
	    _in KUSB_HANDLE InterfaceHandle,
	    _in UCHAR AltSettingNumber,
	    _in UCHAR PipeIndex,
	    _out PWINUSB_PIPE_INFORMATION PipeInformation);

//! Sets the policy for a specific pipe associated with an endpoint on the device. This is a synchronous operation.
	/*!
	*
	* \param[in] InterfaceHandle
	* An initialized usb handle, see \ref UsbK_Init.
	*
	* \param[in] PipeID
	* An 8-bit value that consists of a 7-bit address and a direction bit. This parameter corresponds to the
	* bEndpointAddress field in the endpoint descriptor.
	*
	* \param[in] PolicyType
	* A UINT variable that specifies the policy parameter to change. The Value parameter contains the new value
	* for the policy parameter. See the remarks section for information about each of the pipe policies and the
	* resulting behavior.
	*
	* \param[in] ValueLength
	* The size, in bytes, of the buffer at Value.
	*
	* \param[in] Value
	* The new value for the policy parameter that PolicyType specifies. The size of this input parameter depends
	* on the policy to change. For information about the size of this parameter, see the description of the
	* PolicyType parameter.
	*
	* \returns On success, TRUE. Otherwise FALSE. Use \c GetLastError() to get extended error information.
	*
	* The following list describes symbolic constants that are defined in \ref lusbk_shared.h
	*
	* - \c SHORT_PACKET_TERMINATE (0x01)
	*   - The default value is \c FALSE.
	*   - To enable \c SHORT_PACKET_TERMINATE, in Value pass the address of a caller-allocated \c UCHAR variable
	*     set to \c TRUE (nonzero).
	*   - Enabling \c SHORT_PACKET_TERMINATE causes the driver to send a zero-length packet at the end of every
	*     write request to the host controller.
	*
	* - \c AUTO_CLEAR_STALL (0x02)
	*   - The default value is \c FALSE. To enable \c AUTO_CLEAR_STALL, in Value pass the address of a
	*     caller-allocated \c UCHAR variable set to \c TRUE (nonzero).
	*   - Enabling \c AUTO_CLEAR_STALL causes libusbK to reset the pipe in order to automatically clear the
	*     stall condition. Data continues to flow on the bulk and interrupt \c IN endpoints again as soon as a
	*     new or a queued transfer arrives on the endpoint. This policy parameter does not affect control pipes.
	*   - Disabling \c AUTO_CLEAR_STALL causes all transfers (that arrive to the endpoint after the stalled
	*     transfer) to fail until the caller manually resets the endpoint's pipe by calling \ref UsbK_ResetPipe.
	*
	* - \c PIPE_TRANSFER_TIMEOUT (0x03)
	*   - The default value is zero. To set a time-out value, in Value pass the address of a caller-allocated
	*     \c UINT variable that contains the time-out interval.
	*   - The \c PIPE_TRANSFER_TIMEOUT value specifies the time-out interval, in milliseconds. The host
	*     controller cancels transfers that do not complete within the specified time-out interval.
	*   - A value of zero (default) indicates that transfers do not time out because the host controller never
	*     cancels the transfer.
	*
	* - \c IGNORE_SHORT_PACKETS (0x04)
	*   - The default value is \c FALSE. To enable \c IGNORE_SHORT_PACKETS, in Value pass the address of a
	*     caller-allocated \c UCHAR variable set to \c TRUE (nonzero).
	*   - Enabling \c IGNORE_SHORT_PACKETS causes the host controller to not complete a read operation after it
	*     receives a short packet. Instead, the host controller completes the operation only after the host has
	*     read the specified number of bytes.
	*   - Disabling \c IGNORE_SHORT_PACKETS causes the host controller to complete a read operation when either
	*     the host has read the specified number of bytes or the host has received a short packet.
	*
	* - \c ALLOW_PARTIAL_READS (0x05)
	*   - The default value is \c TRUE (nonzero). To disable \c ALLOW_PARTIAL_READS, in Value pass the address
	*     of a caller-allocated \c UCHAR variable set to \c FALSE (zero).
	*   - Disabling \c ALLOW_PARTIAL_READS causes the read requests to fail whenever the device returns more
	*     data (on bulk and interrupt \c IN endpoints) than the caller requested.
	*   - Enabling \c ALLOW_PARTIAL_READS causes libusbK to save or discard the extra data when the device
	*     returns more data (on bulk and interrupt \c IN endpoints) than the caller requested. This behavior is
	*     defined by setting the \c AUTO_FLUSH value.
	*
	* - \c AUTO_FLUSH (0x06)
	*   - The default value is \c FALSE (zero). To enable \c AUTO_FLUSH, in Value pass the address of a
	*     caller-allocated \c UCHAR variable set to \c TRUE (nonzero).
	*   - \c AUTO_FLUSH must be used with \c ALLOW_PARTIAL_READS enabled. If \c ALLOW_PARTIAL_READS is \c TRUE,
	*     the value of \c AUTO_FLUSH determines the action taken by libusbK when the device returns more data
	*     than the caller requested.
	*   - Disabling \c ALLOW_PARTIAL_READS causes libusbK to ignore the \c AUTO_FLUSH value.
	*   - Disabling \c AUTO_FLUSH with \c ALLOW_PARTIAL_READS enabled causes libusbK to save the extra data, add
	*     the data to the beginning of the caller's next read request, and send it to the caller in the next
	*     read operation.
	*   - Enabling \c AUTO_FLUSH with \c ALLOW_PARTIAL_READS enabled causes libusbK to discard the extra data
	*     remaining from the read request.
	*
	* - \c RAW_IO (0x07)
	*   - The default value is \c FALSE (zero). To enable \c RAW_IO, in Value pass the address of a
	*     caller-allocated \c UCHAR variable set to \c TRUE (nonzero).
	*   - Enabling \c RAW_IO causes libusbK to send data directly to the \c USB driver stack, bypassing
	*     libusbK's queuing and error handling mechanism.
	*   - The buffers that are passed to \ref UsbK_ReadPipe must be configured by the caller as follows:
	*     - The buffer length must be a multiple of the maximum endpoint packet size.
	*     - The length must be less than or equal to the value of \c MAXIMUM_TRANSFER_SIZE retrieved by
	*       \ref UsbK_GetPipePolicy.
	*   - Disabling \c RAW_IO (\c FALSE) does not impose any restriction on the buffers that are passed to
	*     \ref UsbK_ReadPipe.
	*
	* - \c RESET_PIPE_ON_RESUME (0x09)
	*   - The default value is \c FALSE (zero). To enable \c RESET_PIPE_ON_RESUME, in Value pass the address of
	*     a caller-allocated \c UCHAR variable set to \c TRUE (nonzero).
	*   - \c TRUE (or a nonzero value) indicates that on resume from suspend, libusbK resets the endpoint before
	*     it allows the caller to send new requests to the endpoint.
	*
	*/
	KUSB_EXP BOOL KUSB_API UsbK_SetPipePolicy (
	    _in KUSB_HANDLE InterfaceHandle,
	    _in UCHAR PipeID,
	    _in UINT PolicyType,
	    _in UINT ValueLength,
	    _in PVOID Value);

//! Gets the policy for a specific pipe (endpoint).
	/*!
	*
	* \param[in] InterfaceHandle
	* An initialized usb handle, see \ref UsbK_Init.
	*
	* \param[in] PipeID
	* An 8-bit value that consists of a 7-bit address and a direction bit. This parameter corresponds to the
	* bEndpointAddress field in the endpoint descriptor.
	*
	* \param[in] PolicyType
	* A UINT variable that specifies the policy parameter to retrieve. The current value for the policy
	* parameter is retrieved the Value parameter.
	*
	* \param[in,out] ValueLength
	* A pointer to the size, in bytes, of the buffer that Value points to. On output, ValueLength receives the
	* size, in bytes, of the data that was copied into the Value buffer.
	*
	* \param[out] Value
	* A pointer to a buffer that receives the specified pipe policy value.
	*
	* \returns On success, TRUE. Otherwise FALSE. Use \c GetLastError() to get extended error information.
	*
	*/
	KUSB_EXP BOOL KUSB_API UsbK_GetPipePolicy (
	    _in KUSB_HANDLE InterfaceHandle,
	    _in UCHAR PipeID,
	    _in UINT PolicyType,
	    _ref PUINT ValueLength,
	    _out PVOID Value);

//! Reads data from the specified pipe.
	/*!
	*
	* \param[in] InterfaceHandle
	* An initialized usb handle, see \ref UsbK_Init.
	*
	* \param[in] PipeID
	* An 8-bit value that consists of a 7-bit address and a direction bit. This parameter corresponds to the
	* bEndpointAddress field in the endpoint descriptor.
	*
	* \param[out] Buffer
	* A caller-allocated buffer that receives the data that is read.
	*
	* \param[in] BufferLength
	* The maximum number of bytes to read. This number must be less than or equal to the size, in bytes, of
	* Buffer.
	*
	* \param[out] LengthTransferred
	* A pointer to a UINT variable that receives the actual number of bytes that were copied into Buffer. For
	* more information, see Remarks.
	*
	* \param[in] Overlapped
	* An optional pointer to an overlapped structure for asynchronous operations. This can be a \ref KOVL_HANDLE
	* or a pointer to a standard windows OVERLAPPED structure. If this parameter is specified, \c UsbK_ReadPipe
	* returns immediately rather than waiting synchronously for the operation to complete before returning. An
	* event is signaled when the operation is complete.
	*
	* \returns On success, TRUE. Otherwise FALSE. Use \c GetLastError() to get extended error information.
	*
	*/
	KUSB_EXP BOOL KUSB_API UsbK_ReadPipe (
	    _in KUSB_HANDLE InterfaceHandle,
	    _in UCHAR PipeID,
	    _out PUCHAR Buffer,
	    _in UINT BufferLength,
	    _outopt PUINT LengthTransferred,
	    _inopt LPOVERLAPPED Overlapped);

//! Writes data to a pipe.
	/*!
	*
	* \param[in] InterfaceHandle
	* An initialized usb handle, see \ref UsbK_Init.
	*
	* \param[in] PipeID
	* An 8-bit value that consists of a 7-bit address and a direction bit. This parameter corresponds to the
	* bEndpointAddress field in the endpoint descriptor.
	*
	* \param[in] Buffer
	* A caller-allocated buffer the data is written from.
	*
	* \param[in] BufferLength
	* The maximum number of bytes to write. This number must be less than or equal to the size, in bytes, of
	* Buffer.
	*
	* \param[out] LengthTransferred
	* A pointer to a UINT variable that receives the actual number of bytes that were transferred from Buffer.
	* For more information, see Remarks.
	*
	* \param[in] Overlapped
	* An optional pointer to an overlapped structure for asynchronous operations. This can be a \ref KOVL_HANDLE
	* or a pointer to a standard windows OVERLAPPED structure. If this parameter is specified, \c UsbK_WritePipe
	* returns immediately rather than waiting synchronously for the operation to complete before returning. An
	* event is signaled when the operation is complete.
	*
	* \returns On success, TRUE. Otherwise FALSE. Use \c GetLastError() to get extended error information.
	*
	*/
	KUSB_EXP BOOL KUSB_API UsbK_WritePipe (
	    _in KUSB_HANDLE InterfaceHandle,
	    _in UCHAR PipeID,
	    _in PUCHAR Buffer,
	    _in UINT BufferLength,
	    _outopt PUINT LengthTransferred,
	    _inopt LPOVERLAPPED Overlapped);

//! Resets the data toggle and clears the stall condition on a pipe.
	/*!
	*
	* \param[in] InterfaceHandle
	* An initialized usb handle, see \ref UsbK_Init.
	*
	* \param[in] PipeID
	* An 8-bit value that consists of a 7-bit address and a direction bit. This parameter corresponds to the
	* bEndpointAddress field in the endpoint descriptor.
	*
	* \returns On success, TRUE. Otherwise FALSE. Use \c GetLastError() to get extended error information.
	*
	*/
	KUSB_EXP BOOL KUSB_API UsbK_ResetPipe (
	    _in KUSB_HANDLE InterfaceHandle,
	    _in UCHAR PipeID);

//! Aborts all of the pending transfers for a pipe.
	/*!
	*
	* \param[in] InterfaceHandle
	* An initialized usb handle, see \ref UsbK_Init.
	*
	* \param[in] PipeID
	* An 8-bit value that consists of a 7-bit address and a direction bit. This parameter corresponds to the
	* bEndpointAddress field in the endpoint descriptor.
	*
	* \returns On success, TRUE. Otherwise FALSE. Use \c GetLastError() to get extended error information.
	*
	*/
	KUSB_EXP BOOL KUSB_API UsbK_AbortPipe (
	    _in KUSB_HANDLE InterfaceHandle,
	    _in UCHAR PipeID);

//! Discards any data that is cached in a pipe.
	/*!
	*
	* \param[in] InterfaceHandle
	* An initialized usb handle, see \ref UsbK_Init.
	*
	* \param[in] PipeID
	* An 8-bit value that consists of a 7-bit address and a direction bit. This parameter corresponds to the
	* bEndpointAddress field in the endpoint descriptor.
	*
	* \returns On success, TRUE. Otherwise FALSE. Use \c GetLastError() to get extended error information.
	*
	*/
	KUSB_EXP BOOL KUSB_API UsbK_FlushPipe (
	    _in KUSB_HANDLE InterfaceHandle,
	    _in UCHAR PipeID);

//! Reads from an isochronous pipe.
	/*!
	*
	* \param[in] InterfaceHandle
	* An initialized usb handle, see \ref UsbK_Init.
	*
	* \param[in] PipeID
	* An 8-bit value that consists of a 7-bit address and a direction bit. This parameter corresponds to the
	* bEndpointAddress field in the endpoint descriptor.
	*
	* \param[out] Buffer
	* A caller-allocated buffer that receives the data that is read.
	*
	* \param[in] BufferLength
	* The maximum number of bytes to read. This number must be less than or equal to the size, in bytes, of
	* Buffer.
	*
	* \param[in] Overlapped
	* A \b required pointer to an overlapped structure for asynchronous operations. This can be a
	* \ref KOVL_HANDLE or a pointer to a standard windows OVERLAPPED structure. If this parameter is specified,
	* \c UsbK_IsoReadPipe returns immediately rather than waiting synchronously for the operation to complete
	* before returning. An event is signaled when the operation is complete.
	*
	* \param[in,out] IsoContext
	* Pointer to an isochronous transfer context created with \ref IsoK_Init. If \c IsoContext is NULL,
	*
	* \returns On success, TRUE. Otherwise FALSE. Use \c GetLastError() to get extended error information.
	*
	* \par Overlapped I/O considerations
	* If an \c Overlapped parameter is specified and the transfer is submitted successfully, the function
	* returns \b FALSE and sets last error to \c ERROR_IO_PENDING. When using overlapped I/O, users may ignore
	* the return results of this function and instead use the return results from one of the \ref ovlk wait
	* functions or from \ref UsbK_GetOverlappedResult.
	*
	*/
	KUSB_EXP BOOL KUSB_API UsbK_IsoReadPipe (
	    _in KUSB_HANDLE InterfaceHandle,
	    _in UCHAR PipeID,
	    _out PUCHAR Buffer,
	    _in UINT BufferLength,
	    _in LPOVERLAPPED Overlapped,
	    _refopt PKISO_CONTEXT IsoContext);

//! Writes to an isochronous pipe.
	/*!
	*
	* \param[in] InterfaceHandle
	* An initialized usb handle, see \ref UsbK_Init.
	*
	* \param[in] PipeID
	* An 8-bit value that consists of a 7-bit address and a direction bit. This parameter corresponds to the
	* bEndpointAddress field in the endpoint descriptor.
	*
	* \param[in] Buffer
	* A caller-allocated buffer that receives the data that is read.
	*
	* \param[in] BufferLength
	* The maximum number of bytes to write. This number must be less than or equal to the size, in bytes, of
	* Buffer.
	*
	* \param[in] Overlapped
	* An optional pointer to an overlapped structure for asynchronous operations. This can be a \ref KOVL_HANDLE
	* or a pointer to a standard windows OVERLAPPED structure. If this parameter is specified,
	* \c UsbK_IsoWritePipe returns immediately rather than waiting synchronously for the operation to complete
	* before returning. An event is signaled when the operation is complete.
	*
	* \param[in,out] IsoContext
	* Pointer to an isochronous transfer context created with \ref IsoK_Init. See remarks below.
	*
	* \returns On success, TRUE. Otherwise FALSE. Use \c GetLastError() to get extended error information.
	*
	*/
	KUSB_EXP BOOL KUSB_API UsbK_IsoWritePipe (
	    _in KUSB_HANDLE InterfaceHandle,
	    _in UCHAR PipeID,
	    _in PUCHAR Buffer,
	    _in UINT BufferLength,
	    _in LPOVERLAPPED Overlapped,
	    _refopt PKISO_CONTEXT IsoContext);

//! Retrieves the current USB frame number.
	/*!
	*
	* \param[in] InterfaceHandle
	* An initialized usb handle, see \ref UsbK_Init.
	*
	* \param[out] FrameNumber
	* A pointer to a location that receives the current 32-bit frame number on the USB bus (from the host
	* controller driver).
	*
	* \returns On success, TRUE. Otherwise FALSE. Use \c GetLastError() to get extended error information.
	*
	*/
	KUSB_EXP BOOL KUSB_API UsbK_GetCurrentFrameNumber (
	    _in KUSB_HANDLE InterfaceHandle,
	    _out PUINT FrameNumber);

//! Retrieves the results of an overlapped operation on the specified libusbK handle.
	/*!
	*
	* \param[in] InterfaceHandle
	* An initialized usb handle, see \ref UsbK_Init.
	*
	* \param[in] Overlapped
	* A pointer to a standard windows OVERLAPPED structure that was specified when the overlapped operation was
	* started.
	*
	* \param[out] lpNumberOfBytesTransferred
	* A pointer to a variable that receives the number of bytes that were actually transferred by a read or
	* write operation.
	*
	* \param[in] bWait
	* If this parameter is TRUE, the function does not return until the operation has been completed. If this
	* parameter is FALSE and the operation is still pending, the function returns FALSE and the GetLastError
	* function returns ERROR_IO_INCOMPLETE.
	*
	* \returns On success, TRUE. Otherwise FALSE. Use \c GetLastError() to get extended error information.
	*
	* This function is like the Win32 API routine, GetOverlappedResult, with one difference; instead of passing
	* a file handle that is returned from CreateFile, the caller passes an interface handle that is returned
	* from \ref UsbK_Initialize, \ref UsbK_Init, or \ref UsbK_GetAssociatedInterface. The caller can use either
	* API routine, if the appropriate handle is passed. The \ref UsbK_GetOverlappedResult function extracts the
	* file handle from the interface handle and then calls GetOverlappedResult. \n
	*
	* The results that are reported by the \ref UsbK_GetOverlappedResult function are those from the specified
	* handle's last overlapped operation to which the specified standard windows OVERLAPPED structure was
	* provided, and for which the operation's results were pending. A pending operation is indicated when the
	* function that started the operation returns FALSE, and the GetLastError routine returns ERROR_IO_PENDING.
	* When an I/O operation is pending, the function that started the operation resets the hEvent member of the
	* standard windows OVERLAPPED structure to the nonsignaled state. Then when the pending operation has been
	* completed, the system sets the event object to the signaled state. \n
	*
	* The caller can specify that an event object is manually reset in the standard windows OVERLAPPED
	* structure. If an automatic reset event object is used, the event handle must not be specified in any other
	* wait operation in the interval between starting the overlapped operation and the call to
	* \ref UsbK_GetOverlappedResult. For example, the event object is sometimes specified in one of the wait
	* routines to wait for the operation to be completed. When the wait routine returns, the system sets an
	* auto-reset event's state to nonsignaled, and a successive call to \ref UsbK_GetOverlappedResult with the
	* bWait parameter set to TRUE causes the function to be blocked indefinitely.
	*
	* If the bWait parameter is TRUE, \ref UsbK_GetOverlappedResult determines whether the pending operation has
	* been completed by waiting for the event object to be in the signaled state.
	*
	* If the hEvent member of the standard windows OVERLAPPED structure is NULL, the system uses the state of
	* the file handle to signal when the operation has been completed. Do not use file handles for this purpose.
	* It is better to use an event object because of the confusion that can occur when multiple concurrent
	* overlapped operations are performed on the same file. In this situation, you cannot know which operation
	* caused the state of the object to be signaled.
	*
	*/
	KUSB_EXP BOOL KUSB_API UsbK_GetOverlappedResult (
	    _in KUSB_HANDLE InterfaceHandle,
	    _in LPOVERLAPPED Overlapped,
	    _out PUINT lpNumberOfBytesTransferred,
	    _in BOOL bWait);

//! Gets a USB device (driver specific) property from usb handle.
	/*!
	*
	* \param[in] InterfaceHandle
	* USB handle of the property to retrieve.
	*
	* \param[in] PropertyType
	* The propety type to retrieve.
	*
	* \param[in,out] PropertySize
	* Size in bytes of \c Value.
	*
	* \param[out] Value
	* On success, receives the proprty data.
	*
	* \returns On success, TRUE. Otherwise FALSE. Use \c GetLastError() to get extended error information.
	*
	*/
	KUSB_EXP BOOL KUSB_API UsbK_GetProperty (
	    _in KUSB_HANDLE InterfaceHandle,
	    _in KUSB_PROPERTY PropertyType,
	    _ref PUINT PropertySize,
	    _out PVOID Value);

	/*! @} */


#endif

#ifndef _LIBUSBK_LSTK_FUNCTIONS
	/*! \addtogroup lstk
	*@{
	*/

//! Initializes a new usb device list containing all supported devices.
	/*!
	*
	* \param[out] DeviceList
	* Pointer reference that will receive a populated device list.
	*
	* \param[in] Flags
	* Search, filter, and listing options. see \c KLST_FLAG
	*
	* \returns On success, TRUE. Otherwise FALSE. Use \c GetLastError() to get extended error information.
	*
	* \c LstK_Init populates \c DeviceList with connected usb devices that can be used by libusbK.
	*
	* \note if \ref LstK_Init returns TRUE, the device list must be freed with \ref LstK_Free when it is no
	* longer needed.
	*
	*/
	KUSB_EXP BOOL KUSB_API LstK_Init(
	    _out KLST_HANDLE* DeviceList,
	    _in KLST_FLAG Flags);

//! Initializes a new usb device list containing only devices matching a specific class GUID.
	/*!
	*
	* \param[out] DeviceList
	* Pointer reference that will receive a populated device list.
	*
	* \param[in] Flags
	* Search, filter, and listing options. see \c KLST_FLAG
	*
	* \param[in] PatternMatch
	* Pattern Search filter.
	*
	* \returns On success, TRUE. Otherwise FALSE. Use \c GetLastError() to get extended error information.
	*
	* \c LstK_InitEx populates \c DeviceList with usb devices that can be used by libusbK.  Only device
	* matching the \ref KLST_PATTERN_MATCH string are included in the list.
	*
	* \note
	* This function significantly improves performance when used with a device interface guid pattern patch.
	*
	* \note if \ref LstK_InitEx returns TRUE, the device list must be freed with \ref LstK_Free when it it's no
	* longer needed.
	*
	*/
	KUSB_EXP BOOL KUSB_API LstK_InitEx(
	    _out KLST_HANDLE* DeviceList,
	    _in KLST_FLAG Flags,
	    _in PKLST_PATTERN_MATCH PatternMatch);

	//! Frees a usb device list.
	/*!
	*
	* \note if \ref LstK_Init returns TRUE, the device list must be freed with \ref LstK_Free when it is no
	* longer needed.
	*
	* \param[in] DeviceList
	* The \c DeviceList to free.
	*
	* \returns NONE
	*
	* Frees all resources that were allocated to \c DeviceList by \ref LstK_Init.
	*
	*/
	KUSB_EXP BOOL KUSB_API LstK_Free(
	    _in KLST_HANDLE DeviceList);

//! Enumerates \ref KLST_DEVINFO elements of a \ref KLST_HANDLE.
	/*!
	*
	* \param[in] DeviceList
	* The \c DeviceList to enumerate.
	*
	* \param[in] EnumDevListCB
	* Function to call for each iteration.
	*
	* \param[in] Context
	* Optional user context pointer.
	*
	* \returns On success, TRUE. Otherwise FALSE. Use \c GetLastError() to get extended error information.
	*
	* Calls \c EnumDevListCB for each element in the device list or until \c EnumDevListCB returns FALSE.
	*
	*/
	KUSB_EXP BOOL KUSB_API LstK_Enumerate(
	    _in KLST_HANDLE DeviceList,
	    _in KLST_ENUM_DEVINFO_CB* EnumDevListCB,
	    _inopt PVOID Context);

//! Gets the \ref KLST_DEVINFO element for the current position.
	/*!
	*
	* \param[in] DeviceList
	* The \c DeviceList to retrieve a current \ref KLST_DEVINFO for.
	*
	* \param[out] DeviceInfo
	* The device information.
	*
	* \returns On success, TRUE. Otherwise FALSE. Use \c GetLastError() to get extended error information.
	*
	* After a \c DeviceList is created or after the \ref LstK_MoveReset method is called, the \c LstK_MoveNext
	* method must be called to advance the device list enumerator to the first element of the \c DeviceList
	* before calling \c LstK_Current otherwise, \c DeviceInfo is undefined.
	*
	* \c LstK_Current returns \c FALSE and sets last error to \c ERROR_NO_MORE_ITEMS if the last call to
	* \c LstK_MoveNext returned \c FALSE, which indicates the end of the \c DeviceList.
	*
	* \c LstK_Current does not move the position of the device list enumerator, and consecutive calls to
	* \c LstK_Current return the same object until either \c LstK_MoveNext or \ref LstK_MoveReset is called.
	*
	*/
	KUSB_EXP BOOL KUSB_API LstK_Current(
	    _in KLST_HANDLE DeviceList,
	    _out KLST_DEVINFO_HANDLE* DeviceInfo);

//! Advances the device list current \ref KLST_DEVINFO position.
	/*!
	* \param[in] DeviceList
	* A usb device list returned by \ref LstK_Init
	*
	* \param[out] DeviceInfo
	* On success, contains a pointer to the device information for the current enumerators position.
	*
	* \returns On success, TRUE. Otherwise FALSE. Use \c GetLastError() to get extended error information.
	*
	* After a \c DeviceList is created or after \ref LstK_MoveReset is called, an enumerator is positioned
	* before the first element of the \c DeviceList and the \b first call to \c LstK_MoveNext moves the
	* enumerator over the first element of the \c DeviceList.
	*
	* If \c LstK_MoveNext passes the end of the \c DeviceList, the enumerator is positioned after the last
	* element in the \c DeviceList and \c LstK_MoveNext returns \c FALSE. When the enumerator is at this
	* position, a subsequent call to \c LstK_MoveNext will reset the enumerator and it continues from the
	* beginning.
	*
	*/
	KUSB_EXP BOOL KUSB_API LstK_MoveNext(
	    _in KLST_HANDLE DeviceList,
	    _outopt KLST_DEVINFO_HANDLE* DeviceInfo);

//! Sets the device list to its initial position, which is before the first element in the list.
	/*!
	*
	* \param[in] DeviceList
	*
	* \returns On success, TRUE. Otherwise FALSE. Use \c GetLastError() to get extended error information.
	*
	*/
	KUSB_EXP VOID KUSB_API LstK_MoveReset(
	    _in KLST_HANDLE DeviceList);

//! Find a device by vendor and product id
	/*!
	*
	* \param[in] DeviceList
	* The \c DeviceList to retrieve a current \ref KLST_DEVINFO for.
	*
	* \param[in] Vid
	* ID is used in conjunction with the \c Pid to uniquely identify USB devices, providing traceability to the
	* OEM.
	*
	* \param[in] Pid
	* ID is used in conjunction with the \c Pid to uniquely identify USB devices, providing traceability to the
	* OEM.
	*
	* \param[out] DeviceInfo
	* On success, the device information pointer, otherwise NULL.
	*
	* \returns
	* - TRUE if the device was found
	* - FALSE if the device was \b not found or an error occurred.
	*   - Sets last error to \c ERROR_NO_MORE_ITEMS if the device was \b not found.
	*
	* Searches all elements in \c DeviceList for usb device matching the specified.
	*
	*/
	KUSB_EXP BOOL KUSB_API LstK_FindByVidPid(
	    _in KLST_HANDLE DeviceList,
	    _in INT Vid,
	    _in INT Pid,
	    _out KLST_DEVINFO_HANDLE* DeviceInfo);

//! Counts the number of device info elements in a device list.
	/*!
	*
	* \param[in] DeviceList
	* The deice list to count.
	*
	* \param[in,out] Count
	* On success, receives the number of \ref KLST_DEVINFO elements in the list.
	*
	* \returns On success, TRUE. Otherwise FALSE. Use \c GetLastError() to get extended error information.
	*
	*/
	KUSB_EXP BOOL KUSB_API LstK_Count(
	    _in KLST_HANDLE DeviceList,
	    _ref PUINT Count);


	/**@}*/

#endif

#ifndef _LIBUSBK_HOTK_FUNCTIONS
	/*! \addtogroup hotk
	* @{
	*/

//! Creates a new hot-plug handle for USB device arrival/removal event monitoring.
	/*!
	*
	* \param[out] Handle
	* Reference to a handle pointer that will receive the initialized hot-plug handle.
	*
	* \param[in,out] InitParams
	* Hot plug handle initialization structure.
	*
	* \returns On success, TRUE. Otherwise FALSE. Use \c GetLastError() to get extended error information.
	*
	*/
	KUSB_EXP BOOL KUSB_API HotK_Init(
	    _out KHOT_HANDLE* Handle,
	    _ref PKHOT_PARAMS InitParams);

//! Frees the specified hot-plug handle.
	/*!
	*
	* \param[in] Handle
	* hot-plug handle pointer to free.
	*
	* \returns On success, TRUE. Otherwise FALSE. Use \c GetLastError() to get extended error information.
	*
	*/
	KUSB_EXP BOOL KUSB_API HotK_Free(
	    _in KHOT_HANDLE Handle);

	//! Frees all hot-plug handles initialized with \ref HotK_Init.
	/*!
	*
	*/
	KUSB_EXP VOID KUSB_API HotK_FreeAll(VOID);

	/**@}*/

#endif

#ifndef _LIBUSBK_OVLK_FUNCTIONS
	/*! \addtogroup ovlk
	*  @{
	*/

//! Gets a preallocated \c OverlappedK structure from the specified/default pool.
	/*!
	*
	* \param[out] OverlappedK
	* On Success, receives the overlapped handle.
	*
	* \param[in] PoolHandle
	* The overlapped pool used to retrieve the next available \c OverlappedK.
	*
	* \returns On success, the next unused overlappedK available in the pool. Otherwise NULL. Use
	* \c GetLastError() to get extended error information.
	*
	* After calling \ref OvlK_Acquire or \ref OvlK_ReUse the \c OverlappedK is ready to be used in an I/O
	* operation. See one of the \c UsbK core transfer functions such as \ref UsbK_ReadPipe or
	* \ref UsbK_WritePipe for more information.
	*
	* If the pools internal refurbished list (a re-usable list of \c OverlappedK structures) is not empty, the
	* \ref OvlK_Acquire function will choose an overlapped from the refurbished list.
	*
	*/
	KUSB_EXP BOOL KUSB_API OvlK_Acquire(
	    _out KOVL_HANDLE* OverlappedK,
	    _in KOVL_POOL_HANDLE PoolHandle);

//! Returns an \c OverlappedK structure to it's pool.
	/*!
	*
	* \param[in] OverlappedK
	* The overlappedK to release.
	*
	* \returns On success, TRUE. Otherwise FALSE. Use \c GetLastError() to get extended error information.
	*
	* When an overlapped is returned to pool, it resources are \b not freed. Instead, it is added to an internal
	* refurbished list (a re-usable list of \c OverlappedK structures).
	*
	* \warning This function must not be called when the OverlappedK is in-use. If unsure, consider using
	* \ref OvlK_WaitAndRelease instead.
	*
	*/
	KUSB_EXP BOOL KUSB_API OvlK_Release(
	    _in KOVL_HANDLE OverlappedK);


//! Creates a new overlapped pool.
	/*!
	*
	* \param[out] PoolHandle
	* On success, receives the new pool handle.
	*
	* \param[in] UsbHandle
	* USB handle to associate with the pool.
	*
	* \param[in] MaxOverlappedCount
	* Maximum number of overkappedK handles allowed in the pool.
	*
	* \param[in] Flags
	* Pool flags.
	*
	* \returns On success, TRUE. Otherwise FALSE. Use \c GetLastError() to get extended error information.
	*
	*/
	KUSB_EXP BOOL KUSB_API OvlK_Init (
	    _out KOVL_POOL_HANDLE* PoolHandle,
	    _in KUSB_HANDLE UsbHandle,
	    _in INT MaxOverlappedCount,
	    _inopt KOVL_POOL_FLAG Flags);

//! Destroys the specified pool and all resources it created.
	/*!
	*
	* \param[in] PoolHandle
	* The overlapped pool to destroy. Once destroyed, the pool and all resources which belong to it can no
	* longer be used.
	*
	* \returns On success, TRUE. Otherwise FALSE. Use \c GetLastError() to get extended error information.
	*
	* \warning A pool should not be destroyed until all OverlappedKs acquired from it are no longer in-use. For
	* more information see \ref OvlK_WaitAndRelease or \ref OvlK_Release.
	*
	*/
	KUSB_EXP BOOL KUSB_API OvlK_Free(
	    _in KOVL_POOL_HANDLE PoolHandle);


//! Returns the internal event handle used to signal IO operations.
	/*!
	*
	* \param[in] OverlappedK
	* The overlappedK used to return the internal event handle.
	*
	* \returns On success, The manual reset event handle being used by this overlappedK. Otherwise NULL. Use
	* \c GetLastError() to get extended error information.
	*
	* \ref OvlK_GetEventHandle is useful for applications that must to their own event handling. It exposes the
	* windows \c OVERLAPPED \c hEvent used for i/o completion signaling. This event handle can be used by the
	* standard event wait functions; /c WaitForMultipleObjectsEx for example.
	*
	* \warning Use \ref OvlK_GetEventHandle with caution. Event handles returned by this function should never
	* be used unless the OverlappedK has been \b acquired by the application.
	*
	*/
	KUSB_EXP HANDLE KUSB_API OvlK_GetEventHandle(
	    _in KOVL_HANDLE OverlappedK);

//! Waits for overlapped I/O completion, and performs actions specified in \c WaitFlags.
	/*!
	*
	* \param[in] OverlappedK
	* The overlappedK to wait on.
	*
	* \param[in] TimeoutMS
	* Number of milliseconds to wait for overlapped completion.
	*
	* \param[in] WaitFlags
	* See /ref KOVL_WAIT_FLAG
	*
	* \param[out] TransferredLength
	* On success, returns the number of bytes transferred by this overlappedK.
	*
	* \returns On success, TRUE. Otherwise FALSE. Use \c GetLastError() to get extended error information. See
	* the remarks section below for details on relevant error codes.
	*
	* \c OvlK_Wait waits the time interval specified in \c TimeoutMS for the overlapped I/O operation to
	* complete. Different actions can then taken depending on the flags specified in \c WaitFlags.
	*
	* \WINERRORTABLE
	*
	* \WINERROR{ERROR_CANCELLED,1223}
	* - The I/O was cancelled by the user. The transfer complete event was not signalled within the alotted
	*   transfer timeout time and the OvlK_Wait function issued a CancelIoEx/CancelIo request because the
	*   \ref KOVL_WAIT_FLAG_CANCEL_ON_TIMEOUT flag bit was set.
	* \ENDWINERROR
	*
	* \WINERROR{ERROR_OPERATION_ABORTED,995}
	* - The transfer complete event is signalled but the overlapped result was allready cancelled. The
	*   overlapped I/O may have bee cancelled for one of the following reasons:
	*   - Driver cancelled because of pipe timeout policy expiration.
	*   - The device was disconnected.
	*   - A \ref UsbK_AbortPipe request was issued.
	* \ENDWINERROR
	*
	* \ENDWINERRORTABLE
	*
	*/
	KUSB_EXP BOOL KUSB_API OvlK_Wait(
	    _in KOVL_HANDLE OverlappedK,
	    _inopt INT TimeoutMS,
	    _inopt KOVL_WAIT_FLAG WaitFlags,
	    _out PUINT TransferredLength);

//! Waits for overlapped I/O completion on the oldest acquired OverlappedK handle and performs actions specified in \c WaitFlags.
	/*!
	*
	* \param[in] PoolHandle
	* The pool handle containing one or more acuired OverlappedKs.
	*
	* \param[out] OverlappedK
	* On success, set to the oldest overlappedK in the acquired list.
	*
	* \param[in] TimeoutMS
	* See /ref OvlK_Wait
	*
	* \param[in] WaitFlags
	* See /ref KOVL_WAIT_FLAG
	*
	* \param[out] TransferredLength
	* See /ref OvlK_Wait
	*
	* \returns On success, TRUE. Otherwise FALSE. Use \c GetLastError() to get extended error information. See
	* See /ref OvlK_Wait
	*/
	KUSB_EXP BOOL KUSB_API OvlK_WaitOldest(
	    _in KOVL_POOL_HANDLE PoolHandle,
	    _outopt KOVL_HANDLE* OverlappedK,
	    _inopt INT TimeoutMS,
	    _inopt KOVL_WAIT_FLAG WaitFlags,
	    _out PUINT TransferredLength);

//! Waits for overlapped I/O completion, cancels on a timeout error.
	/*!
	*
	* \param[in] OverlappedK
	* The overlappedK to wait on.
	*
	* \param[in] TimeoutMS
	* Number of milliseconds to wait for overlapped completion.
	*
	* \param[out] TransferredLength
	* On success, returns the number of bytes transferred by this overlappedK.
	*
	* \returns On success, TRUE. Otherwise FALSE. Use \c GetLastError() to get extended error information. See
	* \ref OvlK_Wait for details on relevant win32 error codes.
	*
	* \note This convenience function calls \ref OvlK_Wait with \ref KOVL_WAIT_FLAG_CANCEL_ON_TIMEOUT.
	*
	* \c OvlK_WaitOrCancel waits the the time interval specified by \c TimeoutMS for an overlapped result. If
	* the \c TimeoutMS interval expires the I/O operation is cancelled. The \c OverlappedK is not released back
	* to its pool.
	*
	*/
	KUSB_EXP BOOL KUSB_API OvlK_WaitOrCancel(
	    _in KOVL_HANDLE OverlappedK,
	    _inopt INT TimeoutMS,
	    _out PUINT TransferredLength);

//! Waits for overlapped I/O completion, cancels on a timeout error and always releases the OvlK handle back to its pool.
	/*!
	*
	* \param[in] OverlappedK
	* The overlappedK to wait on.
	*
	* \param[in] TimeoutMS
	* Number of milliseconds to wait for overlapped completion.
	*
	* \param[out] TransferredLength
	* On success, returns the number of bytes transferred by this overlappedK.
	*
	* \returns On success, TRUE. Otherwise FALSE. Use \c GetLastError() to get extended error information. See
	* \ref OvlK_Wait for details on relevant win32 error codes.
	*
	* \note This convenience function calls \ref OvlK_Wait with \ref KOVL_WAIT_FLAG_RELEASE_ALWAYS.
	*
	* \c OvlK_WaitAndRelease waits the the time interval specified by \c TimeoutMS for an overlapped result.
	* When \c OvlK_WaitOrCancel returns, the I/O operation has either been completed or cancelled. The
	* \c OverlappedK is always released back to its pool where it can be re-acquired with \ref OvlK_Acquire.
	*
	*/
	KUSB_EXP BOOL KUSB_API OvlK_WaitAndRelease(
	    _in KOVL_HANDLE OverlappedK,
	    _inopt INT TimeoutMS,
	    _out PUINT TransferredLength);

//! Checks for i/o completion; returns immediately. (polling)
	/*!
	*
	* \param[in] OverlappedK
	* The overlappedK to check for completion.
	*
	* \warning \ref OvlK_IsComplete does \b no validation on the OverlappedK. It's purpose is to check the event
	* signal state as fast as possible.
	*
	* \returns TRUE if the \c OverlappedK has completed, otherwise FALSE.
	*
	* \c OvlK_IsComplete quickly checks if the \c OverlappedK i/o operation has completed.
	*/
	KUSB_EXP BOOL KUSB_API OvlK_IsComplete(
	    _in KOVL_HANDLE OverlappedK);

//! Initializes an overlappedK for re-use. The overlappedK is not return to its pool.
	/*!
	*
	* \param[in] OverlappedK
	* The overlappedK to re-use.
	*
	* \returns On success, TRUE. Otherwise FALSE. Use \c GetLastError() to get extended error information.
	*
	*  This function performs the following actions:
	*  - Resets the overlapped event to non-signaled via ResetEvent().
	*  - Clears the internal overlapped information.
	*  - Clears the 'Internal' and 'InternalHigh' members of the windows overlapped structure.
	*
	* \note
	* Re-using OverlappedKs is the most efficient means of OverlappedK management. When an OverlappedK is
	* "re-used" it is not returned to the pool. Instead, the application retains ownership for use in another
	* i/o operation.
	*
	*/
	KUSB_EXP BOOL KUSB_API OvlK_ReUse(
	    _in KOVL_HANDLE OverlappedK);

	/**@}*/

#endif

#ifndef _LIBUSBK_STMK_FUNCTIONS

	/*! \addtogroup stmk
	*  @{
	*/

//! Initializes a new uni-directional pipe stream.
	/*!
	*
	* \param[out] StreamHandle
	* On success, receives the new stream handle.
	*
	* \param[in] UsbHandle
	* Usb handle to associate with this stream.
	*
	* \param[in] PipeID
	* Endpoint address of USB pipe to associate with this stream.
	*
	* \param[in] MaxTransferSize
	* Maximum number of bytes transferred at once. Larger transfers committed with the stream read/write
	* functions are automatically split into multiple smaller chunks.
	*
	* \param[in] MaxPendingTransfers
	* Maximum number of transfers allowed to be outstanding and the total number of transfer contexts that are
	* allocated to the stream.
	*
	* \param[in] MaxPendingIO
	* Maximum number of I/O requests the internal stream thread is allowed to have submit at any given time.
	* (Pending I/O)
	*
	* \param[in] Callbacks
	* Optional user callback functions. If specified, these callback functions will be executed in real time
	* (from within the context of the internal stream thread) as transfers go through the various states.
	*
	* \param[in] Flags
	* Additional stream flags.
	*
	* \returns On success, TRUE. Otherwise FALSE. Use \c GetLastError() to get extended error information.
	*
	* \par
	* When a stream is initialized, it validates input parameters and allocates the required memory for the
	* transfer context array and transfer lists from a private memory heap. The stream is not started and no I/O
	* requests are sent to the device until \ref StmK_Start is executed.
	*
	*/
	KUSB_EXP BOOL KUSB_API StmK_Init(
	    _out KSTM_HANDLE* StreamHandle,
	    _in KUSB_HANDLE UsbHandle,
	    _in UCHAR PipeID,
	    _in INT MaxTransferSize,
	    _in INT MaxPendingTransfers,
	    _in INT MaxPendingIO,
	    _inopt PKSTM_CALLBACK Callbacks,
	    _inopt KSTM_FLAG Flags);

//! Frees resources allocated by a stream handle.
	/*!
	*
	* \param[in] StreamHandle
	* The stream handle to free.
	*
	* \returns TRUE.
	*
	* If the stream is currently started it is automatically stopped before its resources are freed.
	*
	*/
	KUSB_EXP BOOL KUSB_API StmK_Free(
	    _in KSTM_HANDLE StreamHandle);

//! Starts the internal stream thread.
	/*!
	*
	* \param[in] StreamHandle
	* The stream to start. A stream handle is created with \ref StmK_Init.
	*
	* \returns On success, TRUE. Otherwise FALSE. Use \c GetLastError() to get extended error information.
	*
	* \par
	* When a stream is started, an internal thread is created for managing pipe I/O operations. If a
	* \ref KSTM_CALLBACK::Started callback function is assgined, it is executed \b for each transfer context.
	* (\b MaxPendingTransfers) See \ref StmK_Init.
	*
	*/
	KUSB_EXP BOOL KUSB_API StmK_Start(
	    _in KSTM_HANDLE StreamHandle);

//! Stops the internal stream thread.
	/*!
	*
	* \param[in] StreamHandle
	* The stream to stop.
	*
	* \param[in] TimeoutCancelMS
	* Number of milliseconds the internal stream thread should wait for pending I/O to complete before
	* cancelling all pending requests.
	*
	* \returns On success, TRUE. Otherwise FALSE. Use \c GetLastError() to get extended error information.
	*
	*/
	KUSB_EXP BOOL KUSB_API StmK_Stop(
	    _in KSTM_HANDLE StreamHandle,
	    _in INT TimeoutCancelMS);

//! Reads data from the stream buffer.
	/*!
	*
	* \param[in] StreamHandle
	* The stream to read.
	*
	* \param[out] Buffer
	* A caller-allocated buffer that receives the data that is read.
	*
	* \param[in] Offset
	* Read start offset of \c Buffer.
	*
	* \param[in] Length
	* Size of \c Buffer.
	*
	* \param[out] TransferredLength
	* On success, receives the actual number of bytes that were copied into \c Buffer.
	*
	* \returns On success, TRUE. Otherwise FALSE. Use \c GetLastError() to get extended error information.
	*
	* - Read Stream Operations:
	*   -# The internal stream thread will always try and keep reads pending as specified by \b MaxPendingIO in
	*      \ref StmK_Init.
	*	-# As the stream submits transfers, it increments the \b PendingIO and \b PendingTransfer counts. As it
	*	   completes transfers, it decrements the \b PendingIO count. As the user processes transfers with
	*	   \ref StmK_Read, it decrements the \b PendingTransfer count and release control of the transfer context
	*	   back to the stream where it is re-used.
	*   -# When the pending I/O count reaches \c MaxPendingIO, the stream completes the oldest
	*      \b PendingTransfer and moves it into a FIFO complete where it awaits user processing via the
	*      \ref StmK_Read function.
	*	-# If the stream has not exhausted its MaxPendingTransfers count, another read request is submitted
	*	   immediately to satisfy \b MaxPendingIO.
	*
	*/
	KUSB_EXP BOOL KUSB_API StmK_Read(
	    _in KSTM_HANDLE StreamHandle,
	    _out PUCHAR Buffer,
	    _in INT Offset,
	    _in INT Length,
	    _out PUINT TransferredLength);

//! Writes data to the stream buffer.
	/*!
	*
	* \param[in] StreamHandle
	* The stream to write.
	*
	* \param[in] Buffer
	* A caller-allocated buffer the data is written from.
	*
	* \param[in] Offset
	* Write start offset of \c Buffer.
	*
	* \param[in] Length
	* Number of bytes to copy into the stream buffer.
	*
	* \param[out] TransferredLength
	* On success, receives the actual number of bytes that were copied into the stream buffer.
	*
	* \returns On success, TRUE. Otherwise FALSE. Use \c GetLastError() to get extended error information.
	*
	* - Write Stream Operations:
	*   -# The internal stream thread will always try and exhaust all pending transfers submitted by the user
	*      via \ref StmK_Write.
	*	-# As the user submits transfers via \ref StmK_Write, the \b PendingTransfer count is inceremented and
	*	   transfers are added to a queued FIFO list where they await processing by the internal stream thread.
	*	-# While the queued FIFO list is not empty and \b PendingIO count is less than \b MaxPendingIO, The
	*	   \b PendingIO count increments and the request is sent to the device.
	*   -# When a transfer completes, the internal pending I/O count is decremented and the transfers is moved
	*      back into the idle list where it can be reused again by subsequent \ref StmK_Write requests.
	*
	*/
	KUSB_EXP BOOL KUSB_API StmK_Write(
	    _in KSTM_HANDLE StreamHandle,
	    _in PUCHAR Buffer,
	    _in INT Offset,
	    _in INT Length,
	    _out PUINT TransferredLength);
	/**@}*/

#endif

#ifndef _LIBUSBK_ISOK_FUNCTIONS
	/*! \addtogroup isok
	*  @{
	*/

//! Creates a new isochronous transfer context.
	/*!
	*
	* \param[out] IsoContext
	* Receives a new isochronous transfer context.
	*
	* \param[in] NumberOfPackets
	* The number of \ref KISO_PACKET structures allocated to \c IsoContext. Assigned to
	* \ref KISO_CONTEXT::NumberOfPackets. The \ref KISO_CONTEXT::NumberOfPackets field is assignable by
	* \c IsoK_Init only and must not be changed by the user.
	*
	* \param[in] StartFrame
	* The USB frame number this request must start on (or \b 0 for ASAP) and assigned to
	* \ref KISO_CONTEXT::StartFrame. The \ref KISO_CONTEXT::StartFrame may be chamged by the user in subsequent
	* request. For more information, see \ref KISO_CONTEXT::StartFrame.
	*
	* \returns On success, TRUE. Otherwise FALSE. Use \c GetLastError() to get extended error information.
	*
	* \c IsoK_Init is performs the following tasks in order:
	* -# Allocates the \c IsoContext and the required \ref KISO_PACKET structures.
	* -# Zero-initializes all ISO context memory.
	* -# Assigns \b NumberOfPackets, \b PipeID, and \b StartFrame to \c IsoContext.
	*
	*/
	KUSB_EXP BOOL KUSB_API IsoK_Init(
	    _out PKISO_CONTEXT* IsoContext,
	    _in INT NumberOfPackets,
	    _inopt INT StartFrame);

//! Destroys an isochronous transfer context.
	/*!
	* \param[in] IsoContext
	* A pointer to an isochronous transfer context created with \ref IsoK_Init.
	*
	* \returns On success, TRUE. Otherwise FALSE. Use \c GetLastError() to get extended error information.
	*/
	KUSB_EXP BOOL KUSB_API IsoK_Free(
	    _in PKISO_CONTEXT IsoContext);

//! Convenience function for setting the offset of all ISO packets of an isochronous transfer context.
	/*!
	* \param[in] IsoContext
	* A pointer to an isochronous transfer context.
	*
	* \param[in] PacketSize
	* The packet size used to calculate and assign the absolute data offset for each \ref KISO_PACKET in
	* \c IsoContext.
	*
	* \returns On success, TRUE. Otherwise FALSE. Use \c GetLastError() to get extended error information.
	*
	* \c IsoK_SetPackets updates all \ref KISO_PACKET::Offset fields in a \ref KISO_CONTEXT so all offset are
	* \c PacketSize apart. For example:
	* - The offset of the first (0-index) packet is 0.
	* - The offset of the second (1-index) packet is PacketSize.
	* - The offset of the third (2-index) packet is PacketSize*2.
	*
	* \code
	* for (packetIndex = 0; packetIndex < IsoContext->NumberOfPackets; packetIndex++)
	* 	IsoContext->IsoPackets[packetIndex].Offset = packetIndex * PacketSize;
	* \endcode
	*
	*/
	KUSB_EXP BOOL KUSB_API IsoK_SetPackets(
	    _in PKISO_CONTEXT IsoContext,
	    _in INT PacketSize);

//! Convenience function for setting all fields of a \ref KISO_PACKET.
	/*!
	* \param[in] IsoContext
	* A pointer to an isochronous transfer context.
	*
	* \param[in] PacketIndex
	* The packet index to set.
	*
	* \param[in] IsoPacket
	* Pointer to a user allocated \c KISO_PACKET which is copied into the PKISO_CONTEXT::IsoPackets array at the
	* specified index.
	*
	* \returns On success, TRUE. Otherwise FALSE. Use \c GetLastError() to get extended error information.
	*/
	KUSB_EXP BOOL KUSB_API IsoK_SetPacket(
	    _in PKISO_CONTEXT IsoContext,
	    _in INT PacketIndex,
	    _in PKISO_PACKET IsoPacket);

//! Convenience function for getting all fields of a \ref KISO_PACKET.
	/*!
	* \param[in] IsoContext
	* A pointer to an isochronous transfer context.
	*
	* \param[in] PacketIndex
	* The packet index to get.
	*
	* \param[out] IsoPacket
	* Pointer to a user allocated \c KISO_PACKET which receives a copy of the ISO packet in the
	* PKISO_CONTEXT::IsoPackets array at the specified index.
	*
	* \returns On success, TRUE. Otherwise FALSE. Use \c GetLastError() to get extended error information.
	*/
	KUSB_EXP BOOL KUSB_API IsoK_GetPacket(
	    _in PKISO_CONTEXT IsoContext,
	    _in INT PacketIndex,
	    _out PKISO_PACKET IsoPacket);

//! Convenience function for enumerating ISO packets of an isochronous transfer context.
	/*!
	* \param[in] IsoContext
	* A pointer to an isochronous transfer context.
	*
	* \param[in] EnumPackets
	* Pointer to a user supplied callback function which is executed for all ISO packets in \c IsoContext or
	* until the user supplied callback function returns \c FALSE.
	*
	* \param[in] StartPacketIndex
	* The zero-based ISO packet index to begin enumeration at.
	*
	* \param[in] UserState
	* A user defined value which is passed as a parameter to the user supplied callback function.
	*
	* \returns On success, TRUE. Otherwise FALSE. Use \c GetLastError() to get extended error information.
	*/
	KUSB_EXP BOOL KUSB_API IsoK_EnumPackets(
	    _in PKISO_CONTEXT IsoContext,
	    _in KISO_ENUM_PACKETS_CB* EnumPackets,
	    _inopt INT StartPacketIndex,
	    _inopt PVOID UserState);

//! Convenience function for re-using an isochronous transfer context in a subsequent request.
	/*!
	* \param[in,out] IsoContext
	* A pointer to an isochronous transfer context.
	*
	* \returns On success, TRUE. Otherwise FALSE. Use \c GetLastError() to get extended error information.
	*
	* \c IsoK_ReUse does the following:
	* -# Zero-initializes the \b Length and \b Status fields of all \ref KISO_PACKET structures.
	* -# Zero-initializes the \b StartFrame and \b ErrorCount of the \ref KISO_CONTEXT.
	*
	*/
	KUSB_EXP BOOL KUSB_API IsoK_ReUse(
	    _ref PKISO_CONTEXT IsoContext);

	/*! @} */

#endif

#ifdef __cplusplus
}
#endif

#endif // _LIBUSBK_H__
