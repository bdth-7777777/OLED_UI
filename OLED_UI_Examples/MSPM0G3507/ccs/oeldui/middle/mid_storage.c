/**
 * mid_storage.c
 * External Flash storage bridge — wraps hw_w25qxx read for use by the oledUI font subsystem.
 */
#include "mid_storage.h"
#include "hw_w25qxx.h"

void mid_storage_read(uint8_t *buffer, uint32_t read_addr, uint16_t read_length)
{
    w25q128_read(buffer, read_addr, read_length);
}
