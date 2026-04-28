/**
 * mid_font_burner.h
 * Font flash programmer interface — entry points for burning HZK12/16/20 and U2G map to Flash.
 */
#ifndef __MID_FONT_BURNER_H__
#define __MID_FONT_BURNER_H__

#include "ti_msp_dl_config.h"
#include "hw_w25qxx.h"

void font_burner_hzk16_run(void);   // 烧录 HZK16 (发送 HZK16.bin)
void font_burner_hzk12_run(void);   // 烧录 HZK12 (发送 HZK12.bin)
void font_burner_hzk20_run(void);   // 烧录 HZK20 (发送 HZK20.bin)
void font_burner_map_run(void);     // 烧录 Unicode→GB2312 映射表

#endif
