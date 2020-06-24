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
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <errno.h>

#include "nv3p.h"
#include "usb.h"
#include "debug.h"

/* nv3p command packet format */
/*|------------32 bits--------------|*/
/*************************************/
/* NV3P_VERSION                      */
/*-----------------------------------*/
/* NV3P_PACKET_TYPE_CMD              */
/*-----------------------------------*/
/* sequence no                       */
/*-----------------------------------*/
/* length of command arguments       */
/*-----------------------------------*/
/* command, one of:                  */
/*   NV3P_CMD_GET_PLATFORM_INFO      */
/*   NV3P_CMD_DL_BCT                 */
/*   NV3P_CMD_DL_BL                  */
/*   NV3P_CMD_STATUS                 */
/*-----------------------------------*/
/* command arguments                 */
/*                 .                 */
/*                 .                 */
/*                 .                 */
/*-----------------------------------*/
/* checksum                          */
/*-----------------------------------*/

/* nv3p ack packet format */
/*|------------32 bits--------------|*/
/*************************************/
/* NV3P_VERSION                      */
/*-----------------------------------*/
/* NV3P_PACKET_TYPE_ACK              */
/*-----------------------------------*/
/* sequence no                       */
/*-----------------------------------*/
/* checksum                          */
/*-----------------------------------*/

/* nv3p nack packet format */
/*|------------32 bits--------------|*/
/*************************************/
/* NV3P_VERSION                      */
/*-----------------------------------*/
/* NV3P_PACKET_TYPE_NACK             */
/*-----------------------------------*/
/* sequence no                       */
/*-----------------------------------*/
/* nack code                         */
/*-----------------------------------*/
/* checksum                          */
/*-----------------------------------*/

/* nv3p data packet format */
/*|------------32 bits--------------|*/
/*************************************/
/* NV3P_VERSION                      */
/*-----------------------------------*/
/* NV3P_PACKET_TYPE_DATA             */
/*-----------------------------------*/
/* sequence no                       */
/*-----------------------------------*/
/* length of data                    */
/*-----------------------------------*/
/* data                              */
/*                 .                 */
/*                 .                 */
/*                 .                 */
/*-----------------------------------*/
/* checksum                          */
/*-----------------------------------*/

typedef struct nv3p_state {
	usb_device_t *usb;
	uint32_t sequence;
	uint32_t recv_sequence;

	// for partial reads
	uint32_t bytes_remaining;
	uint32_t recv_checksum;

	// the last nack code
	uint32_t last_nack;
} nv3p_state_t;

/*
 * double the currently-largest command size, just to have some wiggle-room
 * (updates to commands without fixing this on accident, etc.)
 */
#define NV3P_MAX_COMMAND_SIZE  (2 << 12)

#define NV3P_MAX_ACK_SIZE \
    NV3P_PACKET_SIZE_BASIC + \
    NV3P_PACKET_SIZE_NACK + \
    NV3P_PACKET_SIZE_FOOTER

unsigned char s_buffer[NV3P_MAX_COMMAND_SIZE];
unsigned char s_args[NV3P_MAX_COMMAND_SIZE]; // handed out to clients

#define WRITE64( packet, data ) \
    do { \
	    (packet)[0] = (uint8_t)((uint64_t)(data)) & 0xff;	 \
	    (packet)[1] = (uint8_t)(((uint64_t)(data)) >> 8) & 0xff;	\
	    (packet)[2] = (uint8_t)(((uint64_t)(data)) >> 16) & 0xff;	\
	    (packet)[3] = (uint8_t)(((uint64_t)(data)) >> 24) & 0xff;	\
	    (packet)[4] = (uint8_t)(((uint64_t)(data)) >> 32) & 0xff;	\
	    (packet)[5] = (uint8_t)(((uint64_t)(data)) >> 40) & 0xff;	\
	    (packet)[6] = (uint8_t)(((uint64_t)(data)) >> 48) & 0xff;	\
	    (packet)[7] = (uint8_t)(((uint64_t)(data)) >> 56) & 0xff;	\
	    (packet) += 8;						\
    } while( 0 )

#define WRITE32( packet, data ) \
    do { \
	    (packet)[0] = (data) & 0xff;    \
	    (packet)[1] = ((data) >> 8) & 0xff; \
	    (packet)[2] = ((data) >> 16) & 0xff;	\
	    (packet)[3] = ((data) >> 24) & 0xff;	\
	    (packet) += 4;				\
    } while( 0 )

#define WRITE8( packet, data ) \
    do { \
	    (packet)[0] = (data) & 0xff;	\
	    (packet) += 1;			\
    } while( 0 )

#define READ64( packet, data ) \
    do { \
	    (data) = (uint64_t)((packet)[0]	\
				| ((uint64_t)((packet)[1]) << 8)	\
				| ((uint64_t)((packet)[2]) << 16)	\
				| ((uint64_t)((packet)[3]) << 24)	\
				| ((uint64_t)((packet)[4]) << 32)	\
				| ((uint64_t)((packet)[5]) << 40)	\
				| ((uint64_t)((packet)[6]) << 48)	\
				| ((uint64_t)((packet)[7]) << 56));	\
	    (packet) += 8;						\
    } while( 0 )

#define READ32( packet, data ) \
    do { \
	    (data) = (uint32_t)((packet)[0]	\
				| (((packet)[1]) << 8)	\
				| (((packet)[2]) << 16) \
				| (((packet)[3]) << 24));	\
	    (packet) += 4;					\
    } while( 0 )


#define READ8( packet, data ) \
    do { \
	    (data) = (packet)[0];		\
	    (packet) += 1;			\
    } while( 0 )

// header structures for fun
typedef struct {
	uint32_t version;
	uint32_t packet_type;
	uint32_t sequence;
} nv3p_header_t;

static void nv3p_write_header(uint32_t type, uint32_t sequence, uint8_t *packet);
static void nv3p_write_footer(uint32_t checksum, uint8_t *packet);
static uint32_t nv3p_cksum(uint8_t *packet, uint32_t length);
static void nv3p_write_cmd(nv3p_handle_t h3p, uint32_t command, void *args,
			   uint32_t *length, uint8_t *packet );
static int nv3p_wait_ack(nv3p_handle_t h3p);
static int nv3p_get_cmd_return(nv3p_handle_t h3p, uint32_t command, void *args);

static int nv3p_recv_hdr(nv3p_handle_t h3p, nv3p_header_t *hdr,
			 uint32_t *checksum );
static int nv3p_drain_packet(nv3p_handle_t h3p, nv3p_header_t *hdr );
static void nv3p_nack(nv3p_handle_t h3p, uint32_t code);
static int nv3p_read(usb_device_t *usb, uint8_t *buf, int len);
static int nv3p_get_args(nv3p_handle_t h3p, uint32_t command, void **args,
			 uint8_t *packet );

static void nv3p_write_header(uint32_t type, uint32_t sequence, uint8_t *packet)
{
	WRITE32(packet, NV3P_VERSION);
	WRITE32(packet, type);
	WRITE32(packet, sequence);
}

static void nv3p_write_footer(uint32_t checksum, uint8_t *packet)
{
	WRITE32(packet, checksum);
}

/*
 * Just sum the bits. Don't get two's compliment
 */
static uint32_t nv3p_cksum(uint8_t *packet, uint32_t length)
{
	uint32_t i;
	uint32_t sum;

	sum = 0;
	for (i = 0; i < length; i++) {
		sum += *packet;
		packet++;
	}

	return sum;
}

int nv3p_open(nv3p_handle_t *h3p, usb_device_t *usb)
{
	nv3p_state_t *state;

	state = (nv3p_state_t *)malloc(sizeof(nv3p_state_t));
	if (!state) {
		return ENOMEM;
	}
	memset(state, 0, sizeof(nv3p_state_t));
	state->last_nack = NV3P_NACK_SUCCESS;

	state->usb = usb;

	*h3p = state;
	return 0;
}

void nv3p_close(nv3p_handle_t h3p)
{
	if (h3p)
		free(h3p);
}

int nv3p_cmd_send(nv3p_handle_t h3p, uint32_t command, void *args)
{
	uint32_t checksum;
	uint32_t length = 0;
	uint8_t *packet;
	uint8_t *tmp;
	int ret = 0;

	packet = &s_buffer[0];

	nv3p_write_header(NV3P_PACKET_TYPE_CMD, h3p->sequence, packet);

	tmp = packet + NV3P_PACKET_SIZE_BASIC;
	nv3p_write_cmd(h3p, command, args, &length, tmp);

	length += NV3P_PACKET_SIZE_BASIC;
	length += NV3P_PACKET_SIZE_COMMAND;

	checksum = nv3p_cksum(packet, length);
	checksum = ~checksum + 1;
	tmp = packet + length;
	nv3p_write_footer(checksum, tmp);

	length += NV3P_PACKET_SIZE_FOOTER;

	// send the packet
	ret = usb_write(h3p->usb, packet, length);
	if (ret)
		return ret;

	h3p->sequence++;

	// wait for ack/nack
	ret = nv3p_wait_ack(h3p);
	if (ret)
		return ret;

	// some commands have return data
	ret = nv3p_get_cmd_return(h3p, command, args);
	if (ret)
		return ret;


	return 0;
}

static void nv3p_write_cmd(nv3p_handle_t h3p, uint32_t command, void *args,
			   uint32_t *length, uint8_t *packet)
{
	uint8_t *tmp;

	tmp = packet;

	switch(command) {
	case NV3P_CMD_GET_PLATFORM_INFO:
	case NV3P_CMD_GET_BCT:
		// no args or output only
		*length = 0;
		WRITE32(tmp, *length);
		WRITE32(tmp, command);
		break;
	case NV3P_CMD_DL_BCT:
	{
		nv3p_cmd_dl_bct_t *a = (nv3p_cmd_dl_bct_t *)args;
		*length = (1 * 4);
		WRITE32(tmp, *length);
		WRITE32(tmp, command);
		WRITE32(tmp, a->length);
		break;
	}
	case NV3P_CMD_DL_BL:
	{
		nv3p_cmd_dl_bl_t *a = (nv3p_cmd_dl_bl_t *)args;
		*length = (2 * 4) + (1 * 8);
		WRITE32(tmp, *length);
		WRITE32(tmp, command);
		WRITE64(tmp, a->length);
		WRITE32(tmp, a->address);
		WRITE32(tmp, a->entry);
		break;
	}
	default:
		dprintf("bad command: 0x%x\n", command);
		break;
	}
}


static int nv3p_wait_ack(nv3p_handle_t h3p)
{
	int ret;
	nv3p_header_t hdr = {0,0,0};
	uint32_t recv_checksum = 0, checksum;
	uint32_t length = 0;

	h3p->last_nack = NV3P_NACK_SUCCESS;

	ret = nv3p_recv_hdr(h3p, &hdr, &recv_checksum);
	if (ret)
		return ret;

	length = NV3P_PACKET_SIZE_BASIC;
	switch(hdr.packet_type) {
	case NV3P_PACKET_TYPE_ACK:
		length += NV3P_PACKET_SIZE_ACK;
		break;
	case NV3P_PACKET_TYPE_NACK:
		length += NV3P_PACKET_SIZE_NACK;
		break;
	default:
		dprintf("unknown packet type received: 0x%x\n", hdr.packet_type);
		return EINVAL;
	}

	if (hdr.packet_type == NV3P_PACKET_TYPE_NACK) {
		// read 4 more bytes to get the error code
		ret = nv3p_read(h3p->usb, (uint8_t *)&h3p->last_nack, 4);
		if (ret)
			return ret;

		recv_checksum += nv3p_cksum((uint8_t *)&h3p->last_nack, 4);
	}

	// get/verify the checksum
	ret = nv3p_read(h3p->usb, (uint8_t *)&checksum, 4);
	if (ret)
		return ret;

	if (recv_checksum + checksum != 0) {
		return EIO;
	}

	if (hdr.sequence != h3p->sequence - 1) {
		return EIO;
	}

	if (hdr.packet_type == NV3P_PACKET_TYPE_NACK) {
		return EIO;
	}

	return 0;
}


static int nv3p_get_cmd_return(nv3p_handle_t h3p, uint32_t command, void *args)
{
	int ret;
	uint32_t length = 0;

	switch (command) {
	case NV3P_CMD_GET_PLATFORM_INFO:
		length = sizeof(nv3p_platform_info_t);
		break;
	case NV3P_CMD_GET_BCT:
		length = sizeof(nv3p_bct_info_t);
		break;
	case NV3P_CMD_DL_BCT:
	case NV3P_CMD_DL_BL:
		break;
	default:
		dprintf("unknown command: 0x%x\n", command);
		return EINVAL;
	}


	if (length) {
		ret = nv3p_data_recv(h3p, args, length);
		if (ret)
			return ret;
	}

	return 0;
}

static int nv3p_recv_hdr(nv3p_handle_t h3p, nv3p_header_t *hdr,
			 uint32_t *checksum )
{
	int ret;
	uint32_t length;
	uint8_t *tmp;

	tmp = &s_buffer[0];
	length = NV3P_PACKET_SIZE_BASIC;
	ret = nv3p_read(h3p->usb, tmp, length);
	if (ret) {
		dprintf("error reading packet: %d\n", ret);
		return ret;
	}

	READ32(tmp, hdr->version);
	READ32(tmp, hdr->packet_type);
	READ32(tmp, hdr->sequence);

	if (hdr->version != NV3P_VERSION) {
		dprintf("version mismatch, expecting NV3P_VERSION(0x%x), got 0x%x\n",
			NV3P_VERSION, hdr->version);
		return EINVAL;
	}

	h3p->recv_sequence = hdr->sequence;

	*checksum = nv3p_cksum(&s_buffer[0], length);
	return 0;
}

int nv3p_data_recv(nv3p_handle_t h3p, uint8_t *data, uint32_t length)
{
	int ret;
	uint8_t *tmp;
	nv3p_header_t hdr = {0,0,0};
	uint32_t checksum;
	uint32_t recv_length;

	// check for left over stuff from a previous read
	if (h3p->bytes_remaining == 0) {
		// get the basic header, verify it's data
		ret = nv3p_recv_hdr(h3p, &hdr, &h3p->recv_checksum);
		if (ret)
			goto fail;

		if (hdr.packet_type != NV3P_PACKET_TYPE_DATA)
			return nv3p_drain_packet(h3p, &hdr);

		tmp = &s_buffer[0];

		// get length
		ret = nv3p_read(h3p->usb, tmp, (1 * 4));
		if (ret)
			goto fail;

		READ32(tmp, recv_length);

		if (!recv_length) {
			ret = EIO;
			goto fail;
		}

		h3p->recv_checksum += nv3p_cksum((uint8_t *)&recv_length, 4);

		// setup for partial reads
		h3p->bytes_remaining = recv_length;
		length = MIN(length, recv_length);
	}
	else {
		length = MIN(h3p->bytes_remaining, length);
	}

	// read the data
	ret = nv3p_read(h3p->usb, data, length);
	if (ret)
		goto fail;

	h3p->recv_checksum += nv3p_cksum(data, length);

	h3p->bytes_remaining -= length;
	if (h3p->bytes_remaining == 0) {
		// get/verify the checksum
		ret = nv3p_read(h3p->usb, (uint8_t *)&checksum, 4);
		if (ret)
			goto fail;

		if (h3p->recv_checksum + checksum != 0) {
			ret = EIO;
			goto fail;
		}

		nv3p_ack(h3p);
	}

	return 0;

fail:
	nv3p_nack(h3p, NV3P_NACK_BAD_DATA);
	return ret;
}


static int nv3p_drain_packet(nv3p_handle_t h3p, nv3p_header_t *hdr)
{
	int ret = EIO;

	/*
	 * consume an ack or nack packet. the other packet types are not
	 * recoverable.
	 */
	if (hdr->packet_type == NV3P_PACKET_TYPE_ACK ||
	    hdr->packet_type == NV3P_PACKET_TYPE_NACK) {
		uint32_t checksum;

		if (hdr->packet_type == NV3P_PACKET_TYPE_NACK) {
			uint32_t code;

			// read 4 more bytes to get the error code
			ret = nv3p_read(h3p->usb, (uint8_t *)&code, 4);
			if (ret)
				return ret;

			h3p->last_nack = code;
		}

		// drain the checksum
		ret = nv3p_read(h3p->usb, (uint8_t *)&checksum, 4);
		if (ret)
			return ret;

		ret = EIO;
	}

	return ret;
}

void nv3p_ack(nv3p_handle_t h3p)
{
	uint32_t checksum;
	uint8_t packet[NV3P_MAX_ACK_SIZE];
	uint32_t length;

	nv3p_write_header(NV3P_PACKET_TYPE_ACK, h3p->recv_sequence, packet);

	length = NV3P_PACKET_SIZE_BASIC;
	length += NV3P_PACKET_SIZE_ACK;

	checksum = nv3p_cksum(packet, length);
	checksum = ~checksum + 1;
	nv3p_write_footer(checksum, &packet[length]);

	length += NV3P_PACKET_SIZE_FOOTER;

	// send the packet
	usb_write(h3p->usb, packet, length);
}

static void nv3p_nack(nv3p_handle_t h3p, uint32_t code)
{
	uint32_t checksum;
	uint8_t packet[NV3P_MAX_ACK_SIZE];
	uint8_t *tmp;
	uint32_t length;

	nv3p_write_header(NV3P_PACKET_TYPE_NACK, h3p->recv_sequence, packet);

	length = NV3P_PACKET_SIZE_BASIC;

	tmp = &packet[length];

	// write the nack code
	WRITE32(tmp, code);

	length += NV3P_PACKET_SIZE_NACK;
	checksum = nv3p_cksum(packet, length);
	checksum = ~checksum + 1;
	nv3p_write_footer(checksum, tmp);

	length += NV3P_PACKET_SIZE_FOOTER;

	// send the packet
	usb_write(h3p->usb, packet, length);
}

static int nv3p_get_args(nv3p_handle_t h3p, uint32_t command, void **args,
			 uint8_t *packet )
{
	uint8_t *tmp = packet;
	uint8_t *buf = &s_args[0];

	switch(command) {
	case NV3P_CMD_GET_PLATFORM_INFO:
	case NV3P_CMD_GET_BCT:
		// output only
		break;
	case NV3P_CMD_STATUS:
	{
		nv3p_cmd_status_t *a = (nv3p_cmd_status_t *)buf;
		memcpy(a->msg, tmp, NV3P_STRING_MAX);
		tmp += NV3P_STRING_MAX;

		READ32(tmp, a->code);
		READ32(tmp, a->flags);
		break;
	}
	case NV3P_CMD_DL_BCT:
	{
		nv3p_cmd_dl_bct_t *a = (nv3p_cmd_dl_bct_t *)buf;
		READ32(tmp, a->length);
		break;
	}
	case NV3P_CMD_DL_BL:
	{
		nv3p_cmd_dl_bl_t *a = (nv3p_cmd_dl_bl_t *)buf;
		READ64(tmp, a->length);
		READ32(tmp, a->address);
		READ32(tmp, a->entry);
		break;
	}
	default:
		dprintf("unknown command: 0x%x\n", command);
		return EINVAL;
	}

	*args = buf;
	return 0;
}

int nv3p_cmd_recv(nv3p_handle_t h3p, uint32_t *command, void **args)
{
	int ret;
	uint8_t *tmp;
	nv3p_header_t hdr = {0,0,0};
	uint32_t length;
	uint32_t checksum, recv_checksum = 0;

	// get the basic header, verify it's a command
	ret = nv3p_recv_hdr(h3p, &hdr, &recv_checksum);
	if (ret) {
		dprintf("nv3p_recv_hdr returned %d\n", ret);
		return ret;
	}

	if(hdr.packet_type != NV3P_PACKET_TYPE_CMD) {
		dprintf("expecting NV3P_PACKET_TYPE_CMD(0x%x), got 0x%x\n",
			NV3P_PACKET_TYPE_CMD, hdr.packet_type);
		return nv3p_drain_packet(h3p, &hdr);
	}

	tmp = &s_buffer[0];

	// get length and command number
	ret = nv3p_read(h3p->usb, tmp, (2 * 4));
	if (ret) {
		dprintf("nv3p_read() returned %d\n", ret);
		return ret;
	}

	READ32(tmp, length);
	READ32(tmp, *(uint32_t *)command);

	// read the args
	if (length) {
		ret = nv3p_read(h3p->usb, tmp, length);
		if (ret) {
			dprintf("nv3p_read returned %d\n", ret);
			return ret;
		}

		ret = nv3p_get_args(h3p, *command, args, tmp);
		if (ret) {
			dprintf("nv3p_get_args returned %d\n", ret);
			return ret;
		}
	} else {
		// command may be output only
		ret = nv3p_get_args(h3p, *command, args, 0);
		if (ret)
			*args = 0;
	}

	length += NV3P_PACKET_SIZE_COMMAND;
	recv_checksum += nv3p_cksum(&s_buffer[0], length);

	// get/verify the checksum
	ret = nv3p_read(h3p->usb, (uint8_t *)&checksum, 4);
	if (ret) {
		dprintf("nv3p_read returned %d reading checksum\n", ret);
		return ret;
	}

	if(recv_checksum + checksum != 0) {
		dprintf("checksum mismatch\n");
		return EIO;
	}

	return 0;
}

int nv3p_data_send(nv3p_handle_t h3p, uint8_t *data, uint32_t length)
{
	int ret;
	uint32_t checksum;
	uint8_t *packet;
	uint8_t *tmp;
	uint32_t hdrlen;

	packet = &s_buffer[0];

	nv3p_write_header(NV3P_PACKET_TYPE_DATA, h3p->sequence, packet);
	tmp = packet + NV3P_PACKET_SIZE_BASIC;

	// data header
	WRITE32(tmp, length);

	hdrlen = NV3P_PACKET_SIZE_BASIC + NV3P_PACKET_SIZE_DATA;

	// checksum
	checksum = nv3p_cksum(packet, hdrlen);
	checksum += nv3p_cksum(data, length);
	checksum = ~checksum + 1;

	// send the headers
	ret = usb_write(h3p->usb, packet, hdrlen);
	if (ret)
		return ret;

	// send the data
	ret = usb_write(h3p->usb, data, length);
	if (ret)
		return ret;

	// send checksum
	ret = usb_write(h3p->usb, (uint8_t *)&checksum, 4);
	if (ret)
		return ret;

	h3p->sequence++;

	// wait for ack/nack
	ret = nv3p_wait_ack(h3p);
	if (ret)
		return ret;

	return 0;
}

int bytesleft = 0;
uint8_t packet[NV3P_MAX_COMMAND_SIZE];
int offset = 0;

/*
 * nv3p_read() - buffered read, target sometimes pads extra bytes to the
 *               end of responses, and we don't handle short reads well,
 *               so buffer reads and copy out
 */
static int nv3p_read(usb_device_t *usb, uint8_t *buf, int len)
{
	int actual_len;
	int ret;

	if (len > sizeof(packet)) {
		dprintf("request for read too big (%d bytes)\n", len);
		return E2BIG;
	}

	if (len > bytesleft) {
		ret = usb_read(usb, packet, sizeof(packet), &actual_len);
		if (ret) {
			dprintf("USB read failed: %d\n", ret);
			return ret;
		}

		offset = 0;
		bytesleft = actual_len;

		len = MIN(len, actual_len);
	}

	memcpy(buf, packet + offset, len);

	offset += len;
	bytesleft -= len;

	return 0;
}


