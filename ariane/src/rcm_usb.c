#include "rcm_usb.h"

static const struct {
	char is_usb3;
	char init_hw_done;
	char init_proto_done;
	char unk0;

	int (*usb_device_init)(void); //returns 0 or 3
	int (*usb_init_proto)(void* tempBuffer); //returns 0 or 8

	int (*usb_device_read_ep1_out)(unsigned char* buf, unsigned int len); //returns 0 or some number of bytes in write buffer, len clamped at 4096
	int (*usb_device_ep1_get_bytes_readable)(unsigned int* outNumBytes, void* unusedPtr, void* tempBuffer); //returns 0 or 28 or 26
	int (*usb_device_read_ep1_out_sync)(unsigned char* buf, unsigned int len, unsigned int* outTransferred); //0 is success, len clamped at 4096
	// 3 when timeout 
	// 28 is USB_EP_STATUS_DISABLED
	// 26 when endpoint status is USB_EP_STATUS_IDLE
	int (*usb_device_write_ep1_in)(const unsigned char* buf, unsigned int len); //returns 0 or 3 or 28, len clamped at 4096
	int (*usb_device_ep1_get_bytes_writing)(unsigned int* outNumBytes, void* unusedPtr, void* tempBuffer);
	int (*usb_device_write_ep1_in_sync)(const unsigned char* buf, unsigned int len, unsigned int* outTransferred); //0 on success, len clamped at 4096

	int (*usb_device_reset_ep1)(void); //returns 0
} *rcm_transport = (void*)0x40003114;                          

bool rcm_usb_device_ready()
{
	return (rcm_transport->is_usb3 == 0) && (rcm_transport->init_hw_done == 1) && (rcm_transport->init_proto_done == 1);
}

int rcm_usb_device_read_ep1_out_sync(unsigned char* buf, unsigned int len, unsigned int* outTransferred)
{
	unsigned int dummyOut = 0;
	if (!outTransferred)
		outTransferred = &dummyOut;

	return rcm_transport->usb_device_read_ep1_out_sync(buf, len, outTransferred);
}

int rcm_usb_device_write_ep1_in_sync(const unsigned char* buf, unsigned int len, unsigned int* outTransferred)
{
	unsigned int dummyOut = 0;
	if (!outTransferred)
		outTransferred = &dummyOut;

	return rcm_transport->usb_device_write_ep1_in_sync(buf, len, outTransferred);
}

void rcm_usb_device_reset_ep1(void)
{
	rcm_transport->usb_device_reset_ep1();
}

//static const unsigned char READY_NOTICE[] = "READY.\n";
void rcm_ready_notice()
{
	unsigned int b;
    rcm_usb_device_write_ep1_in_sync((const unsigned char*)"READY.\n", 7, &b);
}
