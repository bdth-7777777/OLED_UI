#ifndef __OLED_H
#define __OLED_H

// 检测是否是C++编译器
#ifdef __cplusplus
extern "C" {
#endif

#include "OLED_driver.h"			//oled底层驱动头文件
#include "OLED_Fonts.h"				//oled字体库头文件
#include "stdbool.h"


//使用宏定义的方式确定oled的横向像素与竖向像素
#define OLED_WIDTH						(128)					
#define OLED_HEIGHT 					(64)
/*半角字符参数取值*/
/*此参数值不仅用于判断，而且用于计算横向字符偏移，默认值为字体像素宽度*/
#define OLED_10X20_HALF				(10)
#define OLED_8X16_HALF				(8)
#define OLED_7X12_HALF				(7)				
#define OLED_6X8_HALF 			    (6)

/*全角大小参数取值*/
#define OLED_8X8_FULL					(8)
#define OLED_12X12_FULL				    (12)
#define OLED_16X16_FULL			        (16)
#define OLED_20X20_FULL			        (20)

/*IsFilled参数数值*/
#define OLED_UNFILLED			        (0)
#define OLED_FILLED				        (1)

/**关于字符串最大长度的宏，用于格式化输出字符串*/
#define  MAX_STRING_LENGTH 128


//清空屏幕
void OLED_Clear(void);
void OLED_ClearArea(int16_t X, int16_t Y, int16_t Width, int16_t Height);
//屏幕反色
void OLED_Reverse(void);
void OLED_ReverseArea(int16_t X, int16_t Y, int16_t Width, int16_t Height);
//绘制位图
void OLED_ShowImage(int16_t X, int16_t Y, uint16_t Width, uint16_t Height, const uint8_t *Image);
//显示ASCII字符与数字
void OLED_ShowChar(int16_t X, int16_t Y, char Char, uint8_t FontSize);
void OLED_ShowNum(int16_t X, int16_t Y, uint32_t Number, uint8_t Length, uint8_t FontSize);
void OLED_ShowSignedNum(int16_t X, int16_t Y, int32_t Number, uint8_t Length, uint8_t FontSize);
void OLED_ShowHexNum(int16_t X, int16_t Y, uint32_t Number, uint8_t Length, uint8_t FontSize);
void OLED_ShowBinNum(int16_t X, int16_t Y, uint32_t Number, uint8_t Length, uint8_t FontSize);
void OLED_ShowFloatNum(int16_t X, int16_t Y, double Number, uint8_t IntLength, uint8_t FraLength, uint8_t FontSize);
//显示中英文字符串
void OLED_ShowString(int16_t X, int16_t Y, char *String, uint8_t FontSize);
void OLED_ShowMixString(int16_t X, int16_t Y, char *String, uint8_t ChineseFontSize, uint8_t ASCIIFontSize);
void OLED_ShowChinese(int16_t X, int16_t Y, char *Chinese, uint8_t FontSize);
void OLED_Printf(int16_t X, int16_t Y, uint8_t FontSize, char *format, ...);
void OLED_PrintfMix(int16_t X, int16_t Y, uint8_t ChineseFontSize,uint8_t ASCIIFontSize,const char *format, ...);

//Area系列显示函数，可以想象为将屏幕使用蒙版遮挡，并在上面挖出一个X2，Y2，AreaWidth，AreaHeight的透明区域，图片仅仅会在这个区域内显示
void OLED_ShowImageArea(int16_t X_Pic, int16_t Y_Pic, int16_t PictureWidth, int16_t PictureHeight, int16_t X_Area, int16_t Y_Area, int16_t AreaWidth, int16_t AreaHeight, const uint8_t *Image);
void OLED_ShowCharArea(int16_t RangeX,int16_t RangeY,int16_t RangeWidth,int16_t RangeHeight, int16_t X, int16_t Y, char Char, uint8_t FontSize);
void OLED_ShowStringArea(int16_t RangeX,int16_t RangeY,int16_t RangeWidth,int16_t RangeHeight, int16_t X, int16_t Y, char *String, uint8_t FontSize);
void OLED_ShowChineseArea(int16_t RangeX,int16_t RangeY,int16_t RangeWidth,int16_t RangeHeight, int16_t X, int16_t Y, char *Chinese, uint8_t FontSize);
void OLED_PrintfArea(int16_t RangeX,int16_t RangeY,int16_t RangeWidth,int16_t RangeHeight, int16_t X, int16_t Y,uint8_t FontSize, char *format, ...);
void OLED_ShowMixStringArea(int16_t RangeX,int16_t RangeY,int16_t RangeWidth,int16_t RangeHeight,int16_t X, int16_t Y, char *String, uint8_t ChineseFontSize,uint8_t ASCIIFontSize);
void OLED_PrintfMixArea(int16_t RangeX,int16_t RangeY,int16_t RangeWidth,int16_t RangeHeight,int16_t X, int16_t Y, uint8_t ChineseFontSize,uint8_t ASCIIFontSize, char *format, ...);

//绘制函数，绘制基础的ui
void OLED_DrawPoint(int16_t X, int16_t Y);
uint8_t OLED_GetPoint(uint8_t X, uint8_t Y);
void OLED_DrawLine(int16_t X0, int16_t Y0, int16_t X1, int16_t Y1);
void OLED_DrawRectangle(int16_t X, int16_t Y, int16_t Width, int16_t Height, uint8_t IsFilled);
void OLED_DrawTriangle(int16_t X0, int16_t Y0, int16_t X1, int16_t Y1, int16_t X2, int16_t Y2, uint8_t IsFilled);
void OLED_DrawCircle(int16_t X, int16_t Y, int16_t Radius, uint8_t IsFilled);
void OLED_DrawEllipse(int16_t X, int16_t Y, int16_t A, int16_t B, uint8_t IsFilled);
void OLED_DrawArc(int16_t X, int16_t Y, int16_t Radius, int16_t StartAngle, int16_t EndAngle, uint8_t IsFilled);
void OLED_DrawRoundedRectangle(int16_t X, int16_t Y, int16_t Width, int16_t Height, int16_t Radius, uint8_t IsFilled);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif

