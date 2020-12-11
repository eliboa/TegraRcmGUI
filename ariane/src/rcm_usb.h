#ifndef _RCM_USB_H_
#define _RCM_USB_H_

#include <stdbool.h>

bool rcm_usb_device_ready();
int rcm_usb_device_read_ep1_out_sync(unsigned char* buf, unsigned int len, unsigned int* outTransferred);
int rcm_usb_device_write_ep1_in_sync(const unsigned char* buf, unsigned int len, unsigned int* outTransferred);
void rcm_usb_device_reset_ep1(void);
void rcm_ready_notice();

#endif
