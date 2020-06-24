/*
 * This file is part of the coreboot project.
 *
 * Copyright 2016 Google Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef _DECOMP_H_
#define _DECOMP_H_

#include <stddef.h>

/* Decompresses an LZ4F image (multiple LZ4 blocks with frame header) from src
 * to dst, ensuring that it doesn't read more than srcn bytes and doesn't write
 * more than dstn. Buffer sizes must stay below 2GB. Can decompress files loaded
 * to the end of a buffer in-place, as long as buffer is larger than the final
 * output size. (Usually just a few bytes, but may be up to (8 + dstn/255) in
 * worst case. Will reliably return an error if buffer was too small.)
 * Returns amount of decompressed bytes, or 0 on error.
 */
size_t ulz4fn(const void *src, size_t srcn, void *dst, size_t dstn);

/* Defined in src/lib/lzma.c. Returns decompressed size or 0 on error. */
size_t ulzman(const void *src, size_t srcn, void *dst, size_t dstn);
int LZ4_decompress_safe(const char* source, char* dest, int compressedSize, int maxDecompressedSize);                 

#endif	/* _DECOMP_H_ */
