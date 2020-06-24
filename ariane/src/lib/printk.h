#ifndef __PRINTK_H__
#define __PRINTK_H__

#include <stdarg.h>
 	 
void printk(char *fmt, ...);
void vprintk(char *fmt, va_list args);
void dbg_print(char* fmt, ...);
void dbg_vprint(char* fmt, va_list args);
int snprintfk(char *buf, unsigned int bufSize, const char* fmt, ...);

#endif
