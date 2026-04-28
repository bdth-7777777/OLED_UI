#ifndef __OLED_FONT_H
#define __OLED_FONT_H

#include <stdint.h>



/*中文字符字节宽度*/
#define OLED_CHN_CHAR_WIDTH			(3)		//UTF-8编码格式给3，GB2312编码格式给2

/*字模基本单元*/
typedef struct 
{
	char Index[OLED_CHN_CHAR_WIDTH + 1];	//汉字索引
	uint8_t Data[60];						//字模数据
} ChineseCell20x20_t;
typedef struct 
{
	char Index[OLED_CHN_CHAR_WIDTH + 1];	//汉字索引
	uint8_t Data[32];						//字模数据
} ChineseCell16x16_t;
typedef struct 
{
    char Index[OLED_CHN_CHAR_WIDTH + 1];   // 汉字索引
    uint8_t Data[24];                      // 字模数据，12*12的汉字需要24字节
} ChineseCell12x12_t;

typedef struct 
{
    char Index[OLED_CHN_CHAR_WIDTH + 1];   // 汉字索引
    uint8_t Data[8];                      // 字模数据，8*8的汉字需要24字节
} ChineseCell8x8_t;

/*ASCII字模数据声明*/
extern const uint8_t OLED_F10x20[][30];
extern const uint8_t OLED_F8x16[][16];
extern const uint8_t OLED_F7x12[][14];
extern const uint8_t OLED_F6x8[][6];

/*汉字字模数据声明*/
extern const ChineseCell20x20_t OLED_CF20x20[];
extern const ChineseCell16x16_t OLED_CF16x16[];
extern const ChineseCell12x12_t OLED_CF12x12[];  // 声明12x12字模数组
extern const ChineseCell8x8_t OLED_CF8x8[];  // 声明12x12字模数组
/*图像数据声明*/
extern const uint8_t Arrow[];
extern const uint8_t UnKnown[];
extern const uint8_t Image_setings[];
extern const uint8_t Image_window[];
extern const uint8_t Image_wechat[];
extern const uint8_t Image_qq[];
extern const uint8_t Image_alipay[];
extern const uint8_t Image_more[];
extern const uint8_t Image_calc[];
extern const uint8_t Image_night[];
extern const uint8_t Image_sleep[];


extern const uint8_t Image_settings_64[];
extern const uint8_t Image_calc_64[];
extern const uint8_t Image_wechat_64[];
extern const uint8_t Image_alipay_64[];
extern const uint8_t Image_night_64[];
extern const uint8_t Image_more_64[];
extern const uint8_t OLED_UI_LOGO[];
extern const uint8_t OLED_UI_LOGOTEXT[];
extern const uint8_t OLED_UI_LOGOTEXT64[];
extern const uint8_t OLED_UI_LOGOGithub[];
extern const uint8_t Image_alipay_QR_Code[];
extern const uint8_t OLED_UI_SettingsLogo[];
/*按照上面的格式，在这个位置加入新的图像数据声明*/
//...
extern const unsigned char gImage_gyro[]; 
extern const unsigned char gImage_game_icon[]; 
extern const unsigned char gImage_rgb_icon[];
extern const unsigned char gImage_robot_icon[];
extern const unsigned char gImage_uart_icon[];
#endif

