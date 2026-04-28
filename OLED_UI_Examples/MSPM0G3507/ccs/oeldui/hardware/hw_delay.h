/**
 * hw_delay.h
 * Microsecond/millisecond/second delay functions based on delay_cycles.
 */
#ifndef __HW_DELAY_H__
#define __HW_DELAY_H__

#include "ti_msp_dl_config.h"

#define delay_us(X) delay_cycles((((CPUCLK_FREQ)/1000000)*(X))) 
#define delay_ms(X) delay_cycles((((CPUCLK_FREQ)/1000)*(X))) 

#endif
