#ifndef _HW_ENCODER_H_
#define _HW_ENCODER_H_

#include "ti_msp_dl_config.h"
#include <stdint.h>

/*--------------------------------------------------------------
 * 编译时后端选择：
 *   USE_QEI_ENCODER = 1  → 硬件 QEI（TIMG8）
 *   USE_QEI_ENCODER = 0  → 软件 GPIO 中断解码
 *--------------------------------------------------------------*/
#ifndef USE_QEI_ENCODER
#define USE_QEI_ENCODER  1
#endif

void HW_Encoder_Init(void);
void HW_Encoder_Enable(void);
void HW_Encoder_Disable(void);
int16_t HW_Encoder_GetDelta(void);

#endif /* _HW_ENCODER_H_ */
