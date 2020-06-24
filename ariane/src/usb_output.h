#ifndef _USB_OUTPUT_H_
#define _USB_OUTPUT_H_

enum
{
    //commands
    USB_OUTPUT_COMMAND_SEND_SYNC = 0x01,
    USB_OUTPUT_COMMAND_HALT_BPMP = 0xFE,
    USB_OUTPUT_COMMAND_MASK = 0xFF,
    //status
    USB_OUTPUT_STATUS_OP_STARTED = 1u << 30,
    USB_OUTPUT_STATUS_OP_COMPLETE = 1u << 31
};

typedef struct usb_output_mbox
{
    unsigned int    statcmd;
    unsigned int    buf;
    unsigned int    len;
    int             retVal;
} usb_output_mbox_t;

inline volatile usb_output_mbox_t* get_usb_output_mbox_addr()
{
    const unsigned int IRAM_START = 0x40000000;
    const unsigned int IRAM_SIZE = 256*1024;

    return (volatile usb_output_mbox_t*)(IRAM_START + IRAM_SIZE - sizeof(usb_output_mbox_t));
}

#endif