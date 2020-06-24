/**
 * Kernel print functions.
 */

#include "printk.h"

#include "vsprintf.h"
#include "../display/video_fb.h"
#ifdef DEBUG_UART_PORT
#include "../hwinit/uart.h"
#endif

/**
 * Temporary stand-in main printk.
 *
 * TODO: This should print via UART, console framebuffer, and to a ring for
 * consumption by Horizon
 */
void printk(char *fmt, ...)
{
	va_list list;
	va_start(list, fmt);
	vprintk(fmt, list);
	va_end(list);
}

void vprintk(char *fmt, va_list args)
{
	char buf[512];
	vsnprintf(buf, sizeof(buf), fmt, args);
	video_puts(buf);
}

void dbg_print(char *fmt, ...)
{
	va_list list;
	va_start(list, fmt);
	dbg_vprint(fmt, list);
	va_end(list);
}

void dbg_vprint(char *fmt, va_list args)
{
	char buf[512];
	int numChars = vsnprintf(buf, sizeof(buf), fmt, args);
#ifdef DEBUG_UART_PORT
	uart_print(DEBUG_UART_PORT, buf, numChars);
#else
	(void)numChars;
	video_puts(buf);
#endif
}

int snprintfk(char *buf, unsigned int bufSize, const char* fmt, ...)
{
	va_list list;
	va_start(list, fmt);
	int outVal = vsnprintf(buf, bufSize, fmt, list);
	va_end(list);

	return outVal;
}
