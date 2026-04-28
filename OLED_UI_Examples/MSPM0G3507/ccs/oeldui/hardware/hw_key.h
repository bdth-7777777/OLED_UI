/**
 * hw_key.h
 * Key GPIO scan interface — key status struct and level read function declarations.
 */
#ifndef __HW_KEY_H__
#define __HW_KEY_H__

#include "ti_msp_dl_config.h"

typedef struct {
    unsigned int enter : 1; 
    unsigned int back : 1;
    // unsigned int up : 1;
    // unsigned int down : 1;
} KEY_STATUS;

KEY_STATUS key_scan(void);

#endif
