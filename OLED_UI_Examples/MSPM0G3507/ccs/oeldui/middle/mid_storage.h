/**
 * mid_storage.h
 * External Flash storage bridge — wraps hw_w25qxx read for the oledUI font subsystem.
 */
#ifndef __MID_STORAGE_H__
#define __MID_STORAGE_H__

#include <stdint.h>

/**
 * @brief  Read data from external Flash (delegates to hardware/hw_w25qxx.c).
 * @note   Example:
 *           uint8_t buf[32];
 *           mid_storage_read(buf, 0x000000, 32);  // read 32 bytes from address 0 into buf
 * @param  buffer     destination buffer (caller-allocated)
 * @param  read_addr  24-bit Flash byte address (0x000000 ~ 0xFFFFFF)
 * @param  read_length number of bytes to read
 */
void mid_storage_read(uint8_t *buffer, uint32_t read_addr, uint16_t read_length);

#endif
