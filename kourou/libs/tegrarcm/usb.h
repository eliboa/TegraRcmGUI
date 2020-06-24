/*
 * Copyright (c) 2011, NVIDIA CORPORATION
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *  * Neither the name of NVIDIA CORPORATION nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef USB_H
#define USB_H

#include <stdbool.h>
#include <libusb.h>

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

#if defined(LIBUSBX_API_VERSION) && (LIBUSBX_API_VERSION >= 0x01000102)
#define HAVE_USB_PORT_MATCH
#define PORT_MATCH_MAX_PORTS 7
#endif

#define USB_VENID_NVIDIA 0x955
#define USB_DEVID_NVIDIA_TEGRA20 0x20
#define USB_DEVID_NINTENDO_SWITCH 0x21
#define USB_DEVID_NVIDIA_TEGRA30 0x30
#define USB_DEVID_NVIDIA_TEGRA114 0x35
#define USB_DEVID_NVIDIA_TEGRA124 0x40

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

typedef struct {
	libusb_device_handle *handle;
	uint8_t iface_num;
	uint8_t endpt_in;
	uint8_t endpt_out;
	int initialized;
} usb_device_t;

usb_device_t *usb_open(uint16_t venid, uint16_t *devid
#ifdef HAVE_USB_PORT_MATCH
	,
	bool *match_port, uint8_t *match_bus, uint8_t *match_ports,
	int *match_ports_len
#endif
);
void usb_close(usb_device_t *usb);
int usb_write(usb_device_t *usb, uint8_t *buf, int len);
int usb_read(usb_device_t *usb, uint8_t *buf, int len, int *actual_len);


#endif // USB_H
