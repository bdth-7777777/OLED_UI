#ifndef _HW_DELAY_H_
#define _HW_DELAY_H_

#include "ti_msp_dl_config.h"

#define delay_us(X) delay_cycles((80*(X))) 
#define delay_ms(X) delay_cycles((80000*(X))) 

#endif
