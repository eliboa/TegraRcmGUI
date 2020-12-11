/*! \file lusbk_shared.h
* \brief Types and defines shared with the driver.
*/

#ifndef __LUSBK_SHARED_H_
#define __LUSBK_SHARED_H_

#ifndef __USB_H__

//! Values used in the \c bmAttributes field of a \ref USB_ENDPOINT_DESCRIPTOR
typedef enum _USBD_PIPE_TYPE
{
    //! Indicates a control endpoint
    UsbdPipeTypeControl,

    //! Indicates an isochronous endpoint
    UsbdPipeTypeIsochronous,

    //! Indicates a bulk endpoint
    UsbdPipeTypeBulk,

    //! Indicates an interrupt endpoint
    UsbdPipeTypeInterrupt,
} USBD_PIPE_TYPE;

#endif

#if !defined(__WINUSB_COMPAT_IO_H__) && !defined(__WUSBIO_H__)

// pipe policy types ///////////////
#define SHORT_PACKET_TERMINATE  0x01
#define AUTO_CLEAR_STALL        0x02
#define PIPE_TRANSFER_TIMEOUT   0x03
#define IGNORE_SHORT_PACKETS    0x04
#define ALLOW_PARTIAL_READS     0x05
#define AUTO_FLUSH              0x06
#define RAW_IO                  0x07
#define MAXIMUM_TRANSFER_SIZE   0x08
#define RESET_PIPE_ON_RESUME    0x09

// libusbK ISO pipe policy types ///
#define ISO_START_LATENCY		0x20
#define ISO_ALWAYS_START_ASAP	0x21
#define ISO_NUM_FIXED_PACKETS	0x22

// http://msdn.microsoft.com/en-us/library/windows/hardware/ff552359%28v=vs.85%29.aspx
// Settings.Parallel.NumberOfPresentedRequests
// Maximum number of transfers that can be asynchronously delivered at a
// time. Available in version 1.9 and later versions of KMDF.
#define SIMUL_PARALLEL_REQUESTS	0x30

// Power policy types //////////////
#define AUTO_SUSPEND            0x81
#define SUSPEND_DELAY           0x83

// Device Information types ////////
#define DEVICE_SPEED            0x01

// Device Speeds
#define LowSpeed                0x01
#define FullSpeed               0x02
#define HighSpeed               0x03

//! The \c WINUSB_PIPE_INFORMATION structure contains pipe information that the \ref UsbK_QueryPipe routine retrieves.
typedef struct _WINUSB_PIPE_INFORMATION
{
	//! A \c USBD_PIPE_TYPE enumeration value that specifies the pipe type
	USBD_PIPE_TYPE	PipeType;

	//! The pipe identifier (ID)
	UCHAR PipeId;

	//! The maximum size, in bytes, of the packets that are transmitted on the pipe
	USHORT MaximumPacketSize;

	//! The pipe interval
	UCHAR Interval;

} WINUSB_PIPE_INFORMATION;
//! Pointer to a \ref WINUSB_PIPE_INFORMATION structure
typedef WINUSB_PIPE_INFORMATION* PWINUSB_PIPE_INFORMATION;
C_ASSERT(sizeof(WINUSB_PIPE_INFORMATION) == 12);

#include <pshpack1.h>

//! The \c WINUSB_SETUP_PACKET structure describes a USB setup packet.
/*!
* It is often more convient to use this structure in combination with a \ref KUSB_SETUP_PACKET.
* For example:
* \code

* \endcode
*/
typedef struct _WINUSB_SETUP_PACKET
{
	//! The request type. The values that are assigned to this member are defined in Table 9.2 of section 9.3 of the Universal Serial Bus (USB) specification (www.usb.org).
	UCHAR   RequestType;

	//! The device request. The values that are assigned to this member are defined in Table 9.3 of section 9.4 of the Universal Serial Bus (USB) specification.
	UCHAR   Request;

	//! The meaning of this member varies according to the request. For an explanation of this member, see the Universal Serial Bus (USB) specification.
	USHORT  Value;

	//! The meaning of this member varies according to the request. For an explanation of this member, see the Universal Serial Bus (USB) specification.
	USHORT  Index;

	//! The number of bytes to transfer. (not including the \c WINUSB_SETUP_PACKET itself)
	USHORT  Length;

} WINUSB_SETUP_PACKET;
//! pointer to a \c WINUSB_SETUP_PACKET structure
typedef WINUSB_SETUP_PACKET* PWINUSB_SETUP_PACKET;
C_ASSERT(sizeof(WINUSB_SETUP_PACKET) == 8);

#include <poppack.h>

#endif // __WUSBIO_H__ __WINUSB_COMPAT_IO_H__

#include <pshpack1.h>

/*! \addtogroup isok
*  @{
*/

//! Structure describing an isochronous transfer packet.
typedef struct _KISO_PACKET
{
	//! Specifies the offset, in bytes, of the buffer for this packet from the beginning of the entire isochronous transfer data buffer.
	/*!
	* \c Offset represents an absolute data offset from the start of the \c Buffer parameter \ref UsbK_IsoReadPipe or \ref UsbK_IsoWritePipe.
	*
	* \note This field is assigned by the user application only and used by the driver upon transfer submission and completion.
	*/
	UINT Offset;

	//! Set by the host controller to indicate the actual number of bytes received by the device for isochronous IN transfers. Length not used for isochronous OUT transfers.
	/*!
	* \note This field is is not user assignable and is updated by the driver upon transfer completion.
	*/
	USHORT Length;

	//! Contains the 16 least significant USBD status bits, on return from the host controller driver, of this transfer packet.
	/*!
	* See MSDN for USBD status codes: <A href="http://msdn.microsoft.com/en-us/library/ff539136%28VS.85%29.aspx">USBD status code reference</A>
	*
	* \note This field is is not user assignable and is updated by the driver upon transfer completion.
	*/
	USHORT Status;

} KISO_PACKET;
//! pointer to a \c KISO_PACKET structure
typedef KISO_PACKET* PKISO_PACKET;

#pragma warning(disable:4200)

//! Additional ISO transfer flags.
typedef enum _KISO_FLAG
{
    KISO_FLAG_NONE = 0,

    //! Do not start the transfer immediately, instead use \ref KISO_CONTEXT::StartFrame.
    /*!
    * By default, isochronous transfers start on the next frame and \ref KISO_CONTEXT::StartFrame is
    * ignored.  If this flag is specified, the transfer is postponed until the current usb frame number
    * equals that specified by \ref KISO_CONTEXT::StartFrame.
    *
    * Under certain circumstances, the driver can specify 0 for \ref KISO_CONTEXT::StartFrame, and the bus
    * driver will begin the transaction in the next available frame.
    *
    * Specifing \b 0 for \ref KISO_CONTEXT::StartFrame (start transfer ASAP) is restricted to the first
    * transaction on a newly opened or reset pipe. Furthermore, the USB stack contains a bug in Microsoft
    * Windows Server 2003 and Windows XP that limits the use of this to an isochronous context with 255 or fewer
    * packets.
    *
    * For more information about resetting pipes, see \ref UsbK_ResetPipe.
    */
    KISO_FLAG_SET_START_FRAME = 0x00000001,
} KISO_FLAG;

//! Structure describing a user defined isochronous transfer.
/*!
*
* \fixedstruct{16}
*
* The \ref KISO_CONTEXT::StartFrame member of the \ref KISO_CONTEXT specifies the starting USB frame number
* for the transaction. The driver can use \ref UsbK_GetCurrentFrameNumber to request the current frame
* number.
*
* In full-speed transmissions, the frame number for any particular packet will be the sum of the start frame
* number and the packet index. For instance, the fourth packet in the \ref KISO_CONTEXT has an index of 3, so
* its frame number will be StartFrame + 3. In a write transfer, the port driver loads this frame with the
* buffer data at the data buffer offset specified by IsoPacket[3].Offset.
*
* When the driver processes the \ref KISO_CONTEXT, it discards all packets in the \ref KISO_CONTEXT whose
* frame numbers are lower than the current frame number. The port driver sets the Status member of the packet
* descriptor for each discarded packet to USBD_STATUS_ISO_NA_LATE_USBPORT, USBD_STATUS_ISO_NOT_ACCESSED_BY_HW
* or USBD_STATUS_ISO_NOT_ACCESSED_LATE. Even if it discards some packets, the port driver attempts to
* transmit those packets in the \ref KISO_CONTEXT whose frame numbers are higher than the current frame
* number.
*
* The check for a valid StartFrame member is slightly more complicated in high-speed transmissions because
* the port driver loads each packet into a high-speed microframe; however, the value in StartFrame refers to
* the 1 millisecond (full-speed) frame number, not the microframe. For example, if the StartFrame value
* recorded in the \ref KISO_CONTEXT is one less than the current frame, the port driver will discard as many
* as eight packets. The exact number of packets that the port driver discards depends on the period
* associated with the isochronous pipe.
*
* High-speed isochronous pipes can have periods of 1, 2, 4, or 8. The period number specifies the frequency
* with which the port driver inserts packets into the data stream. If the period is 2, for example, the port
* driver will insert a packet into the data stream every two microframes. This means that it will only use
* four of the eight microframes available within each 1-millisecond frame for isochronous data transmission.
*
* In general, the higher the period, the fewer packets the port driver will discard when a \ref KISO_CONTEXT
* arrives late. Assume the period on an isochronous pipe is 2. With a period of 2, each 1-millisecond speed
* frame will carry four packets of isochronous data for that pipe. So, for example, if CurrentFrame -
* StartFrame = 3, the port driver will discard 3 * 4 = 12 packets. On the other hand, if the period is 4,
* each 1-millisecond frame carries only two packets of isochronous data for the pipe. Therefore, if the
* \ref KISO_CONTEXT arrives three 1-millisecond frames late, as in the previous example, the port driver will
* discard 3 * 2 = 6 packets, instead of 12 packets.
*
* For all types of isochronous pipe, the distance between the current frame and the StartFrame value
* specified in the \ref KISO_CONTEXT must be less than USBD_ISO_START_FRAME_RANGE. If StartFrame is not
* within the proper range, the driver sets the Status member of the \ref KISO_PACKET
* \c USBD_STATUS_BAD_START_FRAME and discards the entire \ref KISO_CONTEXT. The following code example shows
* the precise check that the port driver does on the \ref KISO_CONTEXT start frame:
* \code
* if (abs((CurrentFrame - StartFrame)) > USBD_ISO_START_FRAME_RANGE)
* {
* 	// discard the KISO_CONTEXT
* }
* \endcode
*
*/
typedef struct _KISO_CONTEXT
{
	//! Additional ISO transfer flags. See \ref KISO_FLAG.
	KISO_FLAG Flags;

	//! Specifies the frame number that the transfer should begin on (0 for ASAP).
	/*!
	* This variable must be within a system-defined range of the current frame. The range is specified by the
	* constant \ref USBD_ISO_START_FRAME_RANGE.
	*
	* If /ref KISO_FLAG_SET_START_FRAME was specified, this member contains the frame number that the transfer should begin on.
	* When the request is returned by the host controller driver, this member is updated to reflect the frame number this transfer
	* did begin on.
	*
	* \note This field may be assigned by the user application and is updated by the driver upon transfer
	* completion.
	*/
	UINT StartFrame;

	//! Contains the number of packets that completed with an error condition on return from the host controller driver.
	/*!
	* \note This field is is not user assignable and is updated by the driver upon transfer completion.
	*/
	SHORT ErrorCount;

	//! Specifies the number of packets that are described by the variable-length array member \c IsoPacket.
	/*
	* \note This field is assigned by the user application only and used by the driver upon transfer submission
	* and completion.
	*/
	SHORT NumberOfPackets;

	//! Contains the URB Hdr.Status value on return from the host controller driver.
	/*!
	* \note This field is is not user assignable and is updated by the driver upon transfer completion.
	*
	* The USB bus driver always returns a value of USBD_STATUS_SUCCESS in
	* Hdr.Status, unless every packet in the transfer generated an error or
	* the request was not well-formed and could not be executed at all. The
	* following table includes possible error codes returned in Hdr.Status:
	* - USBD_STATUS_ISOCH_REQUEST_FAILED
	*   Indicates that every packet of an isochronous request was completed with
	*   errors.
	* - USBD_STATUS_BAD_START_FRAME
	*   Indicates that the requested start frame is not within
	*   USBD_ISO_START_FRAME_RANGE of the current USB frame.
	* - USBD_ISO_NOT_ACCESSED_LATE
	*   Indicates that every packet was submitted too late for the packet to be
	*   sent, based on the requested start frame.
	* - USBD_STATUS_INVALID_PARAMETER
	*   Indicates that one of the URB parameters was incorrect.
	*/
	UINT UrbHdrStatus;

	//! Contains a variable-length array of \c KISO_PACKET structures that describe the isochronous transfer packets to be transferred on the USB bus.
	/*
	* \note This field is assigned by the user application, used by the driver upon transfer submission, and
	* updated by the driver upon transfer completion.
	*/
	KISO_PACKET IsoPackets[0];

} KISO_CONTEXT;
C_ASSERT(sizeof(KISO_CONTEXT) == 16);

//! pointer to a \c KISO_CONTEXT structure
typedef KISO_CONTEXT* PKISO_CONTEXT;

/*! @} */

#pragma warning(default:4200)

#include <poppack.h>

#endif // __LUSBK_SHARED_H_

