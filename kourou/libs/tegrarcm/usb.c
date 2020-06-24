/*
 * Copyright (c) 2011-2016, NVIDIA CORPORATION
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
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <libusb.h>
#include <sys/param.h>
#include "usb.h"
#include "debug.h"

// USB xfer timeout in ms
#define USB_TIMEOUT 1000

#define USB_XFER_MAX 4096

uint32_t usb_timeout = USB_TIMEOUT;
//
// returns 1 if the specified usb device matches the vendor id
//
static int usb_match(libusb_device *dev, uint16_t venid, uint16_t *devid
#ifdef HAVE_USB_PORT_MATCH
	,
	bool *match_port, uint8_t *match_bus, uint8_t *match_ports,
	int *match_ports_len
#endif
)
{
	struct libusb_device_descriptor desc;
#ifdef HAVE_USB_PORT_MATCH
	uint8_t dev_bus;
	uint8_t dev_ports[PORT_MATCH_MAX_PORTS];
	int dev_ports_len;
#ifdef DEBUG
	int i;
	char portstr[(PORT_MATCH_MAX_PORTS * 4)];
	char *portstrp;
	size_t portstr_lenleft;
	int printed;
#endif
#endif

	if (libusb_get_device_descriptor(dev, &desc)) {
		dprintf("libusb_get_device_descriptor\n");
		return 0;
	}
	if (desc.idVendor != venid) {
		dprintf("non-NVIDIA USB device: 0x%x:0x%x\n",
			desc.idVendor, desc.idProduct);
		return 0;
	}
	switch (desc.idProduct & 0xff) {
	case USB_DEVID_NVIDIA_TEGRA20:
	case USB_DEVID_NVIDIA_TEGRA30:
	case USB_DEVID_NVIDIA_TEGRA114:
	case USB_DEVID_NVIDIA_TEGRA124:
		break;
	default:
		dprintf("non-Tegra NVIDIA USB device: 0x%x:0x%x\n",
			desc.idVendor, desc.idProduct);
		return 0;
	}
#ifdef HAVE_USB_PORT_MATCH
	dev_bus = libusb_get_bus_number(dev);
	dev_ports_len = libusb_get_port_numbers(dev, dev_ports,
			PORT_MATCH_MAX_PORTS);
	if (dev_ports_len < 0) {
		dprintf("libusb_get_port_numbers failed: %d\n", dev_ports_len);
		return 0;
	}
	if (*match_port) {
		if (dev_bus != *match_bus) {
			dprintf("bus mismatch dev:%d match:%d\n", dev_bus,
				*match_bus);
			return 0;
		}
		if (memcmp(dev_ports, match_ports, dev_ports_len)) {
			dprintf("ports mismatch\n");
			return 0;
		}
	}
	if (!*match_port) {
		*match_port = true;
		*match_bus = dev_bus;
		memcpy(match_ports, dev_ports, dev_ports_len);
		*match_ports_len = dev_ports_len;
#ifdef DEBUG
		portstrp = portstr;
		portstr_lenleft = sizeof(portstr);
		printed = snprintf(portstrp, portstr_lenleft, "%d-%d",
			dev_bus, dev_ports[0]);
		portstrp += printed;
		portstr_lenleft -= printed;
		for (i = 1; i < dev_ports_len; i++) {
			printed = snprintf(portstrp, portstr_lenleft, ".%d",
				(int)dev_ports[i]);
			portstrp += printed;
			portstr_lenleft -= printed;
		}
		dprintf("Enabling future match %s\n", portstr);
#endif
	}
#endif

	dprintf("device matches\n");
	*devid = desc.idProduct;

	return 1;
}

static void usb_check_interface(const struct libusb_interface_descriptor *iface_desc,
				uint8_t *endpt_in,
				uint8_t *endpt_out,
				int *found_in,
				int *found_out)
{
	int endpt_num;
	const struct libusb_endpoint_descriptor *endpt_desc;

	for (endpt_num = 0;
	     endpt_num < iface_desc->bNumEndpoints && (!*found_in || !*found_out);
	     endpt_num++) {
		endpt_desc = &iface_desc->endpoint[endpt_num];
		// skip this endpoint if it's not bulk
		if ((endpt_desc->bmAttributes & LIBUSB_TRANSFER_TYPE_MASK) != LIBUSB_TRANSFER_TYPE_BULK)
			continue;
		if ((endpt_desc->bEndpointAddress & LIBUSB_ENDPOINT_DIR_MASK) == LIBUSB_ENDPOINT_IN) {
			// found input endpoint
			*endpt_in = endpt_desc->bEndpointAddress;
			*found_in = 1;
		} else {
			// found output endpoint
			*endpt_out = endpt_desc->bEndpointAddress;
			*found_out = 1;
		}
	}
}

static int usb_get_interface(libusb_device_handle *handle,
			     uint8_t *ifc,
			     uint8_t *endpt_in,
			     uint8_t *endpt_out)
{
	libusb_device *dev;
	struct libusb_config_descriptor *config = NULL;
	int iface_num, alt_iface;
	const struct libusb_interface *iface;
	const struct libusb_interface_descriptor *iface_desc;
	int found_in = 0, found_out = 0;

	dev = libusb_get_device(handle);
	if (!dev) {
		dprintf("libusb_get_device failed\n");
		return ENODEV;
	}
	if (libusb_get_active_config_descriptor(dev, &config)) {
		dprintf("libusb_get_active_config_descriptor failed");
		return EIO;
	}

	for (iface_num = 0; iface_num < config->bNumInterfaces; iface_num++) {
		iface = &config->interface[iface_num];
		for (alt_iface = 0;
		     alt_iface < iface->num_altsetting && (!found_in || !found_out);
		     alt_iface++) {
			iface_desc = &iface->altsetting[alt_iface];

			usb_check_interface(iface_desc, endpt_in, endpt_out,
					    &found_in, &found_out);

			if (found_in && found_out) {
				// save off the interface
				*ifc = iface_desc->bInterfaceNumber;
				break;
			}
		}
	}

	if (config)
		libusb_free_config_descriptor(config);

	if (!found_in || !found_out) {
		dprintf("failed to find input and output endpoints\n");
		return ENODEV;
	}
	return 0;
}

usb_device_t *usb_open(uint16_t venid, uint16_t *devid
#ifdef HAVE_USB_PORT_MATCH
	,
	bool *match_port, uint8_t *match_bus, uint8_t *match_ports,
	int *match_ports_len
#endif
)
{
	libusb_device **list = NULL;
	libusb_device *found = NULL;
	ssize_t cnt, i=0;
	usb_device_t *usb = NULL;

	if (libusb_init(NULL)) {
		dprintf("libusb_init\n");
		goto fail;
	}

	cnt = libusb_get_device_list(NULL, &list);
	if (cnt < 0) {
		dprintf("libusb_get_device_list\n");
		goto fail;
	}

	for (i = 0; i < cnt; i++) {
		libusb_device *device = list[i];

		if (usb_match(device, venid, devid
#ifdef HAVE_USB_PORT_MATCH
				, match_port, match_bus, match_ports,
				match_ports_len
#endif
		)) {
			found = device;
			break;
		}
	}

	if (!found) {
		dprintf("could't find device\n");
		goto fail;
	}

	usb = (usb_device_t *)malloc(sizeof(usb_device_t));
	if (!usb) {
		dprintf("out of mem\n");
		goto fail;
	}
	memset(usb, 0, sizeof(usb_device_t));

	if (libusb_open(found, &usb->handle)) {
		dprintf("libusb_open failed\n");
		goto fail;
	}
	if (usb_get_interface(usb->handle, &usb->iface_num,
			      &usb->endpt_in, &usb->endpt_out)) {
		dprintf("usb_get_interface failed\n");
		goto fail;
	}

	// claim the interface
	libusb_claim_interface(usb->handle, usb->iface_num);

	usb->initialized = 1;
	libusb_free_device_list(list, 1);

	return usb;

fail:
	if (usb)
		free(usb);
	if (list)
		libusb_free_device_list(list, 1);
	return NULL;
}

void usb_close(usb_device_t *usb)
{
	if (!usb)
		return;
	if (usb->initialized) {
		libusb_release_interface(usb->handle, usb->iface_num);
		if (usb->handle)
			libusb_close(usb->handle);
	}
	libusb_exit(NULL);
}

int usb_write(usb_device_t *usb, uint8_t *buf, int len)
{
	int ret;
	int chunk_size;
	int actual_chunk;

	while (len) {
		chunk_size = MIN(len, USB_XFER_MAX);
		ret = libusb_bulk_transfer(usb->handle, usb->endpt_out, buf,
					   chunk_size, &actual_chunk, usb_timeout);
		if (ret != LIBUSB_SUCCESS) {
			dprintf("libusb write failure: %d: %s\n", ret, libusb_error_name(ret));
			return EIO;
		}
		if (actual_chunk != chunk_size) {
			dprintf("write truncated");
			return EIO;
		}
		len -= actual_chunk;
		buf += actual_chunk;
	}

	return 0;
}

int usb_read(usb_device_t *usb, uint8_t *buf, int len, int *actual_len)
{
	int ret;
	int chunk_size;
	int actual_chunk;

	*actual_len = 0;

	while (len) {
		chunk_size = MIN(len, USB_XFER_MAX);
		ret = libusb_bulk_transfer(usb->handle, usb->endpt_in, buf,
					   chunk_size, &actual_chunk, usb_timeout);
		if (ret != LIBUSB_SUCCESS) {
			dprintf("libusb read failure: %d: %s\n", ret, libusb_error_name(ret));
			return EIO;
		}
		len -= chunk_size;
		buf += chunk_size;
		*actual_len += actual_chunk;

		if (actual_chunk < chunk_size)
			break;
	}

	return 0;
}
