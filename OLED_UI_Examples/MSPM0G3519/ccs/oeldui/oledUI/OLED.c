/*
 * 这个头文件是oled库的 [软件层] 实现文件，移植的时候不需要更改这个文件的内容
 * 在此文件当中，部分是b站up江协科技的函数，在此诚挚感谢。
*/
#include "OLED.h"
#include "oled_ext_font.h"
/**
  * 声明OLED显存数组，此数组已经在OLED_Driver.c中定义
  * 所有的显示函数，都只是对此显存数组进行读写
  * 随后调用OLED_Update函数或OLED_UpdateArea函数
  * 才会将显存数组的数据发送到OLED硬件，进行显示
  */
extern uint8_t OLED_DisplayBuf[OLED_HEIGHT/8][OLED_WIDTH];


/*********************工具函数↓********************/
/**
  * 函    数：次方函数
  * 参    数：X 底数
  * 参    数：Y 指数
  * 返 回 值：等于X的Y次方
  */
uint32_t OLED_Pow(uint32_t X, uint32_t Y)
{
	uint32_t Result = 1;	//结果默认为1
	while (Y --)			//累乘Y次
	{
		Result *= X;		//每次把X累乘到结果上
	}
	return Result;
}

/**
  * 函    数：判断指定点是否在指定多边形内部
  * 参    数：nvert 多边形的顶点数
  * 参    数：vertx verty 包含多边形顶点的x和y坐标的数组
  * 参    数：testx testy 测试点的X和y坐标
  * 返 回 值：指定点是否在指定多边形内部，1：在内部，0：不在内部
  */
uint8_t OLED_pnpoly(uint8_t nvert, int16_t *vertx, int16_t *verty, int16_t testx, int16_t testy)
{
	int16_t i = 0, j = 0;
	uint8_t c = 0;
	/*此算法由W. Randolph Franklin提出*/
	/*参考链接：https://wrfranklin.org/Research/Short_Notes/pnpoly.html*/
	for (i = 0, j = nvert - 1; i < nvert; j = i++)
	{
		if (((verty[i] > testy) != (verty[j] > testy)) &&
			(testx < (vertx[j] - vertx[i]) * (testy - verty[i]) / (verty[j] - verty[i]) + vertx[i]))
		{
			c = !c;
		}
	}
	return c;
}
/*
 * 函数：比大小函数
 * 参数：
 * 四个传入值
 * 返回值：四个传入值当中的最大值
*/

int16_t max(int16_t a, int16_t b, int16_t c, int16_t d) {
    int16_t max_val = a; // 假设a是最大的

    if (b > max_val) {
        max_val = b; // 如果b大于当前最大值，则更新最大值为b
    }
    if (c > max_val) {
        max_val = c; // 如果c大于当前最大值，则更新最大值为c
    }
    if (d > max_val) {
        max_val = d; // 如果d大于当前最大值，则更新最大值为d
    }

    return max_val; // 返回最大值
}

/*
 * 函数：绝对值函数
 * 参数：
 * num
 * 返回值：num的绝对值
*/

float numabs(float num){
	if(num>0)
		return num;
	if(num<0)
		return -num;
	return 0;
}



/**
  * 函    数：判断指定点是否在指定角度内部
  * 参    数：X Y 指定点的坐标
  * 参    数：StartAngle EndAngle 起始角度和终止角度，范围：-180~180
  *           水平向右为0度，水平向左为180度或-180度，下方为正数，上方为负数，顺时针旋转
  * 返 回 值：指定点是否在指定角度内部，1：在内部，0：不在内部
  */
uint8_t OLED_IsInAngle(int16_t X, int16_t Y, int16_t StartAngle, int16_t EndAngle)
{
	int16_t PointAngle;
	PointAngle = atan2(Y, X) / 3.14 * 180;	//计算指定点的弧度，并转换为角度表示
	if (StartAngle < EndAngle)	//起始角度小于终止角度的情况
	{
		/*如果指定角度在起始终止角度之间，则判定指定点在指定角度*/
		if (PointAngle >= StartAngle && PointAngle <= EndAngle)
		{
			return 1;
		}
	}
	else			//起始角度大于于终止角度的情况
	{
		/*如果指定角度大于起始角度或者小于终止角度，则判定指定点在指定角度*/
		if (PointAngle >= StartAngle || PointAngle <= EndAngle)
		{
			return 1;
		}
	}
	return 0;		//不满足以上条件，则判断判定指定点不在指定角度
}

/*********************工具函数↑********************/

/*********************功能函数↓*********************/
/**
  * 函    数：将OLED显存数组全部清零
  * 参    数：无
  * 返 回 值：无
  * 说    明：调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数
  */
void OLED_Clear(void)
{
	int16_t i, j;
	for (j = 0; j < OLED_HEIGHT/8; j ++)				//遍历8页
	{
		for (i = 0; i < OLED_WIDTH; i ++)			//遍历OLED_WIDTH列
		{
			OLED_DisplayBuf[j][i] = 0x00;	//将显存数组数据全部清零
		}
	}
}
/**
  * 函    数：将OLED显存数组部分清零
  * 参    数：X 指定区域左上角的横坐标，范围：0~OLED_WIDTH-1
  * 参    数：Y 指定区域左上角的纵坐标，范围：0~OLED_HEIGHT-1
  * 参    数：Width 指定区域的宽度，范围：0~OLED_WIDTH
  * 参    数：Height 指定区域的高度，范围：0~OLED_HEIGHT
  * 返 回 值：无
  * 说    明：调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数
  */
 
  void OLED_ClearArea(int16_t X, int16_t Y, int16_t Width, int16_t Height)
  {
	  int16_t x_start, y_start, x_end, y_end;
	  int16_t i, j;
  
	  if (Width <= 0 || Height <= 0) return;
  
	  // 计算X方向的起始和结束位置
	  x_start = (X < 0) ? 0 : X;
	  x_end = X + Width;
	  if (x_end > OLED_WIDTH) x_end = OLED_WIDTH;
	  if (x_start >= x_end) return;
  
	  // 计算Y方向的起始和结束位置
	  y_start = (Y < 0) ? 0 : Y;
	  y_end = Y + Height;
	  if (y_end > OLED_HEIGHT) y_end = OLED_HEIGHT;
	  if (y_start >= y_end) return;
  
	  // 调整Width和Height为实际需要清除的区域
	  Width = x_end - x_start;
	  Height = y_end - y_start;
  
	  for (j = y_start; j < y_end; j++) {
		  for (i = x_start; i < x_end; i++) {
			  OLED_DisplayBuf[j / 8][i] &= ~(0x01 << (j % 8));
		  }
	  }
  }



/**
  * 函    数：将OLED显存数组全部取反
  * 参    数：无
  * 返 回 值：无
  * 说    明：调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数
  */
void OLED_Reverse(void)
{
	uint8_t i, j;
	for (j = 0; j < OLED_HEIGHT/8; j ++)				//遍历页
	{
		for (i = 0; i < OLED_WIDTH; i ++)			//遍历OLED_WIDTH列
		{
			OLED_DisplayBuf[j][i] ^= 0xFF;	//将显存数组数据全部取反
		}
	}
}



/**
  * 函    数：将OLED显存数组部分取反
  * 参    数：X 指定区域左上角的横坐标，范围：负数~OLED_WIDTH-1
  * 参    数：Y 指定区域左上角的纵坐标，范围：负数~OLED_HEIGHT
  * 参    数：Width 指定区域的宽度，范围：负数~OLED_WIDTH
  * 参    数：Height 指定区域的高度，范围：负数~OLED_HEIGHT
  * 返 回 值：无
  * 说    明：调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数
  */
void OLED_ReverseArea(int16_t X, int16_t Y, int16_t Width, int16_t Height)
{
	int16_t i, j, x, y;
	if(Width <= 0 || Height <= 0) {return; }
	/*参数检查，保证指定区域不会超出屏幕范围*/
	if (X > OLED_WIDTH-1) {return;}
	if (Y > OLED_HEIGHT-1) {return;}
	if (X + Width > OLED_WIDTH) {Width = OLED_WIDTH - X;}
	if (Y + Height > OLED_HEIGHT) {Height = OLED_HEIGHT - Y;}
	if (X + Width < 0) {return;}
	if (Y + Height < 0) {return;}
	if (X < 0) { x = 0;} else { x = X;}
	if (Y < 0) { y = 0;} else { y = Y;}
	
	for (j = y; j < Y + Height; j ++)		//遍历指定页
	{
		for (i = x; i < X + Width; i ++)	//遍历指定列
		{
			OLED_DisplayBuf[j / 8][i] ^= 0x01 << (j % 8);	//将显存数组指定数据取反
		}
	}
}
/**
  * 函    数：OLED显示图像 BY BILIBILI上nm网课呢 xy轴均可为负
  * 参    数：X 指定图像左上角的横坐标，范围：负值~OLED_WIDTH-1
  * 参    数：Y 指定图像左上角的纵坐标，范围：负值~OLED_HEIGHT-1
  * 参    数：Width 指定图像的宽度，范围：正数
  * 参    数：Height 指定图像的高度，范围：正数
  * 参    数：Image 指定要显示的图像
  * 返 回 值：无
  * 说    明：调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数
  */
void OLED_ShowImage(int16_t X, int16_t Y, uint16_t Width, uint16_t Height, const uint8_t *Image)
{
      uint8_t i, j;
    
    /* 参数检查，保证指定图像不会超出屏幕范围 */
    if (Width == 0 || Height == 0) {
        return; // 如果宽度或高度为0，直接返回
    }
    
    if (X > OLED_WIDTH-1) {
        return; // X 超出右边界，直接返回
    }
    if (Y > OLED_HEIGHT-1) {
        return; // Y 超出下边界，直接返回
    }
    
    /* 将图像所在区域清空 */
    uint8_t startX = (X < 0) ? 0 : X; // 计算实际起始显示位置的 X 坐标
    uint8_t startY = (Y < 0) ? 0 : Y; // 计算实际起始显示位置的 Y 坐标
    uint8_t endX = (X + Width - 1 > OLED_WIDTH-1) ? OLED_WIDTH-1 : X + Width - 1; // 计算实际结束显示位置的 X 坐标
    uint8_t endY = (Y + Height - 1 > OLED_HEIGHT-1) ? OLED_HEIGHT-1 : Y + Height - 1; // 计算实际结束显示位置的 Y 坐标
    
    OLED_ClearArea(startX, startY, endX - startX + 1, endY - startY + 1);
    
    /* 遍历指定图像涉及的相关页 */
    for (j = 0; j < (Height - 1) / 8 + 1; j++)
    {
        /* 遍历指定图像涉及的相关列 */
        for (i = 0; i < Width; i++)
        {
            int16_t currX = X + i;
            int16_t currY = Y + j * 8;
            
            /* 超出边界，则跳过显示 */
            if (currX < 0 || currX > OLED_WIDTH-1 ||currY < 0 || currY > OLED_HEIGHT-1) {
                continue;
            }
			/* 显示图像在当前页的内容 */
			OLED_DisplayBuf[currY / 8][currX] |= Image[j * Width + i] << (currY % 8);
			/* 当前页下一页 */
			if (currY + 8 <= OLED_HEIGHT-1) {
				OLED_DisplayBuf[currY / 8 + 1][currX] |= Image[j * Width + i] >> (8 - currY % 8);
			}
        }
    }
	if(Y<0){
		for (i = 0; i < Width; i++)
        {
            int16_t currX = X + i;
			if (currX < 0 || currX > OLED_WIDTH-1) {
                continue;
            }
			OLED_DisplayBuf[0][currX] |= Image[ -Y/8*Width+i] >> -Y%8;
		}
	}
}
/**
  * 函    数：OLED显示一个字符
  * 参    数：X 指定字符左上角的横坐标，范围：负数~OLED_WIDTH-1
  * 参    数：Y 指定字符左上角的纵坐标，范围：负数~OLED_HEIGHT-1
  * 参    数：Char 指定要显示的字符，范围：ASCII码可见字符
  * 参    数：FontSize 指定字体大小
  *           范围：OLED_8X16_HALF		宽8像素，高16像素
  *                 OLED_6X8_HALF		宽6像素，高8像素
  *                  OLED_7X12_HALF		宽7像素，高12像素
  *                 OLED_10X20_HALF		宽10像素，高20像素
  * 返 回 值：无
  * 说    明：调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数
  */
void OLED_ShowChar(int16_t X, int16_t Y, char Char, uint8_t FontSize)
{
	if (FontSize == OLED_8X16_HALF)		//字体为宽8像素，高16像素
	{
		/*将ASCII字模库OLED_F8x16的指定数据以8*16的图像格式显示*/
		OLED_ShowImage(X, Y, 8, 16, OLED_F8x16[Char - ' ']);
	}
	else if(FontSize == OLED_6X8_HALF)	//字体为宽6像素，高8像素
	{
		/*将ASCII字模库OLED_F6x8的指定数据以6*8的图像格式显示*/
		OLED_ShowImage(X, Y, 6, 8, OLED_F6x8[Char - ' ']);
	}
	else if(FontSize == OLED_7X12_HALF)	//字体为宽6像素，高8像素
	{
		/*将ASCII字模库OLED_F6x8的指定数据以6*8的图像格式显示*/
		OLED_ShowImage(X, Y, 7, 12, OLED_F7x12[Char - ' ']);
	}else if(FontSize == OLED_10X20_HALF)
	{
		/*将ASCII字模库OLED_F10X20的指定数据以10x20的图像格式显示*/
		OLED_ShowImage(X, Y, 10, 20, OLED_F10x20[Char - ' ']);
	}
}

/**
  * 函    数：OLED显示数字（十进制，正整数）
  * 参    数：X 指定数字左上角的横坐标，范围：负数~OLED_WIDTH-1
  * 参    数：Y 指定数字左上角的纵坐标，范围：负数~OLED_HEIGHT-1
  * 参    数：Number 指定要显示的数字，范围：负数~4294967295
  * 参    数：Length 指定数字的长度，范围：负数~10
  * 参    数：FontSize 指定字体大小
  *           范围：OLED_8X16		宽8像素，高16像素
  *                 OLED_6X8		宽6像素，高8像素
  * 				 OLED_7X12		宽7像素，高12像素
  *                 OLED_10X20		宽10像素，高20像素
  * 返 回 值：无
  * 说    明：调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数
  */
void OLED_ShowNum(int16_t X, int16_t Y, uint32_t Number, uint8_t Length, uint8_t FontSize)
{
	uint8_t i;
	for (i = 0; i < Length; i++)		//遍历数字的每一位
	{
		/*调用OLED_ShowChar函数，依次显示每个数字*/
		/*Number / OLED_Pow(10, Length - i - 1) % 10 可以十进制提取数字的每一位*/
		/*+ '0' 可将数字转换为字符格式*/
		OLED_ShowChar(X + i * FontSize, Y, Number / OLED_Pow(10, Length - i - 1) % 10 + '0', FontSize);
	}
}

/**
  * 函    数：OLED显示有符号数字（十进制，整数）
  * 参    数：X 指定数字左上角的横坐标，范围：负数~OLED_WIDTH-1
  * 参    数：Y 指定数字左上角的纵坐标，范围：负数~OLED_HEIGHT-1
  * 参    数：Number 指定要显示的数字，范围：-2147483648~2147483647
  * 参    数：Length 指定数字的长度，范围：负数~10
  * 参    数：FontSize 指定字体大小
  *           范围：OLED_8X16		宽8像素，高16像素
  *                 OLED_6X8		宽6像素，高8像素
  * 				 OLED_7X12		宽7像素，高12像素
  *                 OLED_10X20		宽10像素，高20像素
  * 返 回 值：无
  * 说    明：调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数
  */
void OLED_ShowSignedNum(int16_t X, int16_t Y, int32_t Number, uint8_t Length, uint8_t FontSize)
{
	uint8_t i;
	uint32_t Number1;
	
	if (Number >= 0)						//数字大于等于0
	{
		OLED_ShowChar(X, Y, '+', FontSize);	//显示+号
		Number1 = Number;					//Number1直接等于Number
	}
	else									//数字小于0
	{
		OLED_ShowChar(X, Y, '-', FontSize);	//显示-号
		Number1 = -Number;					//Number1等于Number取负
	}
	
	for (i = 0; i < Length; i++)			//遍历数字的每一位
	{
		/*调用OLED_ShowChar函数，依次显示每个数字*/
		/*Number1 / OLED_Pow(10, Length - i - 1) % 10 可以十进制提取数字的每一位*/
		/*+ '0' 可将数字转换为字符格式*/
		OLED_ShowChar(X + (i + 1) * FontSize, Y, Number1 / OLED_Pow(10, Length - i - 1) % 10 + '0', FontSize);
	}
}

/**
  * 函    数：OLED显示十六进制数字（十六进制，正整数）
  * 参    数：X 指定数字左上角的横坐标，范围：负数~OLED_WIDTH-1
  * 参    数：Y 指定数字左上角的纵坐标，范围：负数~OLED_HEIGHT-1
  * 参    数：Number 指定要显示的数字，范围：0x00000000~0xFFFFFFFF
  * 参    数：Length 指定数字的长度，范围：负数~8
  * 参    数：FontSize 指定字体大小
  *           范围：OLED_8X16		宽8像素，高16像素
  *                 OLED_6X8		宽6像素，高8像素
  * 				 OLED_7X12		宽7像素，高12像素
  *                 OLED_10X20		宽10像素，高20像素
  * 返 回 值：无
  * 说    明：调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数
  */
void OLED_ShowHexNum(int16_t X, int16_t Y, uint32_t Number, uint8_t Length, uint8_t FontSize)
{
	uint8_t i, SingleNumber;
	for (i = 0; i < Length; i++)		//遍历数字的每一位
	{
		/*以十六进制提取数字的每一位*/
		SingleNumber = Number / OLED_Pow(16, Length - i - 1) % 16;
		
		if (SingleNumber < 10)			//单个数字小于10
		{
			/*调用OLED_ShowChar函数，显示此数字*/
			/*+ '0' 可将数字转换为字符格式*/
			OLED_ShowChar(X + i * FontSize, Y, SingleNumber + '0', FontSize);
		}
		else							//单个数字大于10
		{
			/*调用OLED_ShowChar函数，显示此数字*/
			/*+ 'A' 可将数字转换为从A开始的十六进制字符*/
			OLED_ShowChar(X + i * FontSize, Y, SingleNumber - 10 + 'A', FontSize);
		}
	}
}



/**
  * 函    数：OLED显示二进制数字（二进制，正整数）
  * 参    数：X 指定数字左上角的横坐标，范围：负数~OLED_WIDTH-1
  * 参    数：Y 指定数字左上角的纵坐标，范围：负数~OLED_HEIGHT-1
  * 参    数：Number 指定要显示的数字，范围：0x00000000~0xFFFFFFFF
  * 参    数：Length 指定数字的长度，范围：负数~16
  * 参    数：FontSize 指定字体大小
  *           范围：OLED_8X16		宽8像素，高16像素
  *                 OLED_6X8		宽6像素，高8像素
  * 				 OLED_7X12		宽7像素，高12像素
  *                 OLED_10X20		宽10像素，高20像素
  * 返 回 值：无
  * 说    明：调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数
  */
void OLED_ShowBinNum(int16_t X, int16_t Y, uint32_t Number, uint8_t Length, uint8_t FontSize)
{
	uint8_t i;
	for (i = 0; i < Length; i++)		//遍历数字的每一位
	{
		/*调用OLED_ShowChar函数，依次显示每个数字*/
		/*Number / OLED_Pow(2, Length - i - 1) % 2 可以二进制提取数字的每一位*/
		/*+ '0' 可将数字转换为字符格式*/
		OLED_ShowChar(X + i * FontSize, Y, Number / OLED_Pow(2, Length - i - 1) % 2 + '0', FontSize);
	}
}

/**
  * 函    数：OLED显示浮点数字（十进制，小数）
  * 参    数：X 指定数字左上角的横坐标，范围：负数~OLED_WIDTH-1
  * 参    数：Y 指定数字左上角的纵坐标，范围：负数~OLED_HEIGHT-1
  * 参    数：Number 指定要显示的数字，范围：-4294967295.0~4294967295.0
  * 参    数：IntLength 指定数字的整数位长度，范围：0~10
  * 参    数：FraLength 指定数字的小数位长度，范围：0~9，小数进行四舍五入显示
  * 参    数：FontSize 指定字体大小
  *           范围：OLED_8X16		宽8像素，高16像素
  *                 OLED_6X8		宽6像素，高8像素
  * 				 OLED_7X12		宽7像素，高12像素
  *                 OLED_10X20		宽10像素，高20像素
  * 返 回 值：无
  * 说    明：调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数
  */
void OLED_ShowFloatNum(int16_t X, int16_t Y, double Number, uint8_t IntLength, uint8_t FraLength, uint8_t FontSize)
{
	uint32_t PowNum, IntNum, FraNum;
	
	if (Number >= 0)						//数字大于等于0
	{
		OLED_ShowChar(X, Y, '+', FontSize);	//显示+号
	}
	else									//数字小于0
	{
		OLED_ShowChar(X, Y, '-', FontSize);	//显示-号
		Number = -Number;					//Number取负
	}
	
	/*提取整数部分和小数部分*/
	IntNum = Number;						//直接赋值给整型变量，提取整数
	Number -= IntNum;						//将Number的整数减掉，防止之后将小数乘到整数时因数过大造成错误
	PowNum = OLED_Pow(10, FraLength);		//根据指定小数的位数，确定乘数
	FraNum = round(Number * PowNum);		//将小数乘到整数，同时四舍五入，避免显示误差
	IntNum += FraNum / PowNum;				//若四舍五入造成了进位，则需要再加给整数
	
	/*显示整数部分*/
	OLED_ShowNum(X + FontSize, Y, IntNum, IntLength, FontSize);
	
	/*显示小数点*/
	OLED_ShowChar(X + (IntLength + 1) * FontSize, Y, '.', FontSize);
	
	/*显示小数部分*/
	OLED_ShowNum(X + (IntLength + 2) * FontSize, Y, FraNum, FraLength, FontSize);
}



/**
  * 函    数：OLED显示字符串
  * 参    数：X 指定数字左上角的横坐标，范围：0~OLED_WIDTH-1
  * 参    数：Y 指定数字左上角的纵坐标，范围：0~OLED_HEIGHT-1
  * 参    数：String 指定要显示的字符串，范围：ASCII码可见字符组成的字符串
  * 参    数：FontSize 指定字体大小
  *           范围：OLED_8X16		宽8像素，高16像素
  *                 OLED_6X8		宽6像素，高8像素
  * 				 OLED_7X12		宽7像素，高12像素
  *                 OLED_10X20		宽10像素，高20像素
  * 返 回 值：无
  * 说    明：调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数
  */
void OLED_ShowString(int16_t X, int16_t Y, char *String, uint8_t FontSize)
{
	uint8_t i;
	for (i = 0; String[i] != '\0'; i++)		//遍历字符串的每个字符
	{
		/*调用OLED_ShowChar函数，依次显示每个字符*/
		OLED_ShowChar(X + i * FontSize, Y, String[i], FontSize);
		
	}
}



/**
  * 函    数：OLED显示汉字串
  * 参    数：X 指定数字左上角的横坐标，范围：0~OLED_WIDTH-1
  * 参    数：Y 指定数字左上角的纵坐标，范围：0~OLED_HEIGHT-1
  * 参    数：Chinese 指定要显示的汉字串，范围：必须全部为汉字或者全角字符，不要加入任何半角字符
  *           显示的汉字需要在OLED_Data.c里的OLED_CF16x16数组定义
  *           未找到指定汉字时，会显示默认图形（一个方框，内部一个问号）
  * 参    数：FontSize 指定中文文字大小，OLED_8X8_FULL,OLED_12X12_FULL,OLED_16X16_FULL,OLED_20X20_FULL
  * 返 回 值：无
  * 说    明：调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数
  */
void OLED_ShowChinese(int16_t X, int16_t Y, char *Chinese, uint8_t FontSize)
{
    // 8x8 无外部字库，保留内部字模查找
    if (FontSize == OLED_8X8_FULL)
    {
        uint8_t pChinese = 0;
        uint8_t pIndex;
        uint8_t i;
        char SingleChinese[OLED_CHN_CHAR_WIDTH + 1] = {0};
        for (i = 0; Chinese[i] != '\0'; i++)
        {
            SingleChinese[pChinese] = Chinese[i];
            pChinese++;
            if (pChinese >= OLED_CHN_CHAR_WIDTH)
            {
                pChinese = 0;
                const ChineseCell8x8_t* fontArray = OLED_CF8x8;
                for (pIndex = 0; strcmp(fontArray[pIndex].Index, "") != 0; pIndex++)
                {
                    if (strcmp(fontArray[pIndex].Index, SingleChinese) == 0) break;
                }
                OLED_ShowImage(X + ((i + 1) / OLED_CHN_CHAR_WIDTH - 1) * OLED_8X8_FULL, Y,
                               OLED_8X8_FULL, OLED_8X8_FULL, fontArray[pIndex].Data);
            }
        }
        return;
    }
    // 12/16/20 走外部 Flash 字库
    OLED_ShowChineseExt(X, Y, Chinese, FontSize);
}

/**
  * 函    数：OLED使用printf函数打印格式化字符串
  * 参    数：X 指定格式化字符串左上角的横坐标，范围：0~OLED_WIDTH-1
  * 参    数：Y 指定格式化字符串左上角的纵坐标，范围：0~OLED_HEIGHT-1
  * 参    数：FontSize 指定字体大小
  *           范围：OLED_8X16		宽8像素，高16像素
  *                 OLED_6X8		宽6像素，高8像素
  * 				OLED_7X12		宽7像素，高12像素
  *                 OLED_10X20		宽10像素，高20像素
  * 参    数：format 指定要显示的格式化字符串，范围：ASCII码可见字符组成的字符串
  * 参    数：... 格式化字符串参数列表
  * 返 回 值：无
  * 说    明：调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数
  */
void OLED_Printf(int16_t X, int16_t Y, uint8_t FontSize, char *format, ...)
{
	char String[MAX_STRING_LENGTH];						//定义字符数组
	va_list arg;							//定义可变参数列表数据类型的变量arg
	va_start(arg, format);					//从format开始，接收参数列表到arg变量
	vsprintf(String, format, arg);			//使用vsprintf打印格式化字符串和参数列表到字符数组中
	va_end(arg);							//结束变量arg
	OLED_ShowString(X, Y, String, FontSize);//OLED显示字符数组（字符串）
}



/**
  * @brief OLED显示混合字符串（汉字与ASCII）
  * @param X 指定汉字串左上角的横坐标，范围：负值~OLED_WIDTH-1
  * @param Y 指定汉字串左上角的纵坐标，范围：负值~OLED_HEIGHT-1
  * @param String 指定要显示的混合字符串，范围：全角字符与半角字符都可以
  *        显示的汉字需要在OLED_Data.c里的OLED_CF16x16或OLED_CF12x12数组定义
  *        未找到指定汉字时，会显示默认图形（一个方框，内部一个问号）
  * @param ChineseFontSize 指定中文文字大小，OLED_8X8_FULL,OLED_12X12_FULL,OLED_16X16_FULL,OLED_20X20_FULL
  * @param ASCIIFontSize  指定ASCII文字大小,OLED_6X8_HALF,OLED_7X12_HALF,OLED_8X16_HALF,OLED_10X20_HALF
  * @return 无
  */
void OLED_ShowMixString(int16_t X, int16_t Y, char *String, uint8_t ChineseFontSize, uint8_t ASCIIFontSize)
{
    // 8x8 保留内部字模
    if (ChineseFontSize == OLED_8X8_FULL)
    {
        while (*String != '\0') {
            if (*String & 0x80) {
                char Chinese[OLED_CHN_CHAR_WIDTH+1];
                for (uint8_t i=0;i<OLED_CHN_CHAR_WIDTH;i++){
                    Chinese[i] = *(String+i);
                }
                Chinese[OLED_CHN_CHAR_WIDTH] = '\0';
                OLED_ShowChinese(X, Y, Chinese, ChineseFontSize);
                X += ChineseFontSize;
                String += OLED_CHN_CHAR_WIDTH;
            } else {
                OLED_ShowChar(X, Y, *String, ASCIIFontSize);
                X += ASCIIFontSize;
                String++;
            }
        }
        return;
    }
    // 12/16/20 走外部 Flash 字库
    OLED_ShowMixStringExt(X, Y, String, ChineseFontSize, ASCIIFontSize);
}

/**
  * 函    数：OLED使用printf函数打印格式化字符串,可以是中英文混杂的字符串。此函数由bilibili@上nm网课呢 添加
  * 参    数：X 指定汉字串左上角的横坐标，范围：负值~OLED_WIDTH-1
  * 参    数：Y 指定汉字串左上角的纵坐标，范围：负值~OLED_HEIGHT-1
  * 参    数：ChineseFontSize 指定汉字字体大小，OLED_8X8_FULL,OLED_12X12_FULL,OLED_16X16_FULL,OLED_20X20_FULL
  * 参    数：ASCIIFontSize 指定ASCII字体大小,OLED_6X8_HALF,OLED_7X12_HALF,OLED_8X16_HALF,OLED_10X20_HALF
  * 参    数：format 指定要显示的格式化字符串，范围：ASCII码可见字符组成的字符串
  * 参    数：... 格式化字符串参数列表
  * 返 回 值：无
  * 说    明：调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数
  */

void OLED_PrintfMix(int16_t X, int16_t Y, uint8_t ChineseFontSize,uint8_t ASCIIFontSize,const char *format, ...)
{
	char String[MAX_STRING_LENGTH];						//定义字符数组
	va_list arg;							//定义可变参数列表数据类型的变量arg
	va_start(arg, format);					//从format开始，接收参数列表到arg变量
	vsprintf(String, format, arg);			//使用vsprintf打印格式化字符串和参数列表到字符数组中
	va_end(arg);							//结束变量arg
	OLED_ShowMixString( X, Y, String, ChineseFontSize,ASCIIFontSize);//OLED显示字符数组（字符串）
}





/**
  * @brief 在指定区域内显示图片
  * @param X_Pic 图片左上角的横坐标
  * @param Y_Pic 图片左上角的纵坐标
  * @param PictureWidth 图片宽度
  * @param PictureHeight 图片高度
  * @param X_Area 显示区域的左上角的横坐标
  * @param Y_Area 显示区域的左上角的纵坐标
  * @param AreaWidth 显示区域的宽度
  * @param AreaHeight 显示区域的高度
  * @param Image 图片取模数组
  * @note 此函数至关重要，它可以将一个图片显示在指定的区域内，实现复杂的显示效果，为OLED_UI的诸多功能提供基础。
  * @retval 无
  */
 void OLED_ShowImageArea(int16_t X_Pic, int16_t Y_Pic, int16_t PictureWidth, int16_t PictureHeight, int16_t X_Area, int16_t Y_Area, int16_t AreaWidth, int16_t AreaHeight, const uint8_t *Image)
 {
	 if (PictureWidth == 0 || PictureHeight == 0 || AreaWidth == 0 || AreaHeight == 0 || X_Pic > OLED_WIDTH-1 || X_Area > OLED_WIDTH-1 || Y_Pic > OLED_HEIGHT-1 || Y_Area > OLED_HEIGHT-1) {return; }
		 int16_t startX = (X_Pic < X_Area) ? X_Area : X_Pic;
	 int16_t endX = ((X_Area + AreaWidth - 1) < (X_Pic + PictureWidth - 1)) ? (X_Area + AreaWidth - 1) : (X_Pic + PictureWidth - 1);
	 int16_t startY = (Y_Pic < Y_Area) ? Y_Area : Y_Pic;
	 int16_t endY = ((Y_Area + AreaHeight - 1) < (Y_Pic + PictureHeight - 1)) ? (Y_Area + AreaHeight - 1) : (Y_Pic + PictureHeight - 1);
	 endX = (endX > OLED_WIDTH-1) ? OLED_WIDTH-1 : endX;
	 endY = (endY > OLED_HEIGHT-1) ? OLED_HEIGHT-1 : endY;
		 if(startX > endX || startY > endY){return;}
		 //OLED_ClearArea(startX, startY, endX - startX + 1, endY - startY + 1);
		 for (uint8_t j = 0; j <= (PictureHeight - 1) / 8; j++) {
		 for (uint8_t i = 0; i < PictureWidth; i++) {
			 uint8_t currX = X_Pic + i;
			 if (currX < startX || currX > endX) {continue;};
			 for (uint8_t bit = 0; bit < 8; bit++) {
				 uint8_t currY = Y_Pic + j * 8 + bit;
				 if (currY < startY || currY > endY) {continue;};
				 uint8_t page = currY / 8;
				 uint8_t bit_pos = currY % 8;
				 uint8_t data = Image[j * PictureWidth + i];
				 if (data & (1 << bit)) {OLED_DisplayBuf[page][currX] |= (1 << bit_pos); }
			 }
		 }
	 }
 }



/**
  * @brief 在指定范围内显示一个字符
  * @param RangeX 指定字符可以显示范围的左上角的横坐标，范围：负值~OLED_WIDTH-1
  * @param RangeY 指定字符可以显示范围的左上角的纵坐标，范围：负值~OLED_HEIGHT-1
  * @param RangeWidth 指定范围宽度
  * @param RangeHeight 指定范围高度
  * @param X 指定字符左上角的横坐标，范围：负值~OLED_WIDTH-1
  * @param Y 指定字符左上角的纵坐标，范围：负值~OLED_HEIGHT-1
  * @param Char 指定要显示的字符，范围：ASCII码可见字符
  * @param FontSize 指定字体大小,OLED_6X8_HALF,OLED_7X12_HALF,OLED_8X16_HALF,OLED_10X20_HALF
  * @retval 无
  */
void OLED_ShowCharArea(int16_t RangeX,int16_t RangeY,int16_t RangeWidth,int16_t RangeHeight, int16_t X, int16_t Y, char Char, uint8_t FontSize)
{
	
	if (FontSize == OLED_8X16_HALF)		//字体为宽8像素，高16像素
	{
		/*将ASCII字模库OLED_F8x16的指定数据以8*16的图像格式显示*/
		OLED_ShowImageArea(X, Y, 8, 16, RangeX, RangeY, RangeWidth, RangeHeight, OLED_F8x16[Char - ' ']);
	}
	else if(FontSize == OLED_6X8_HALF)	//字体为宽6像素，高8像素
	{
		/*将ASCII字模库OLED_F6x8的指定数据以6*8的图像格式显示*/
		
		OLED_ShowImageArea(X, Y, 6, 8, RangeX, RangeY, RangeWidth, RangeHeight, OLED_F6x8[Char - ' ']);
	}
	else if(FontSize == OLED_7X12_HALF)	//字体为宽6像素，高8像素
	{
		/*将ASCII字模库OLED_F7X12的指定数据以6*8的图像格式显示*/
		OLED_ShowImageArea(X, Y, 7, 12, RangeX, RangeY, RangeWidth, RangeHeight, OLED_F7x12[Char - ' ']);
	}else if(FontSize == OLED_10X20_HALF)
	{
		/*将ASCII字模库OLED_F10x20的指定数据以6*8的图像格式显示*/
		OLED_ShowImageArea(X, Y, 10, 20, RangeX, RangeY, RangeWidth, RangeHeight, OLED_F10x20[Char - ' ']);
	}
}
/**
  * @brief 在指定范围内显示字符串在指定区域内显示字符串
  * @param RangeX 指定字符可以显示范围的左上角的横坐标，范围：负值~OLED_WIDTH-1
  * @param RangeY 指定字符可以显示范围的左上角的纵坐标，范围：负值~OLED_HEIGHT-1
  * @param RangeWidth 指定范围宽度
  * @param RangeHeight 指定范围高度
  * @param X 指定字符左上角的横坐标，范围：负值~OLED_WIDTH-1
  * @param Y 指定字符左上角的纵坐标，范围：负值~OLED_HEIGHT-1
  * @param String 指定要显示的字符串，范围：ASCII码可见字符
  * @param FontSize 指定字体大小,OLED_6X8_HALF,OLED_7X12_HALF,OLED_8X16_HALF,OLED_10X20_HALF
  * @retval 无
  */

void OLED_ShowStringArea(int16_t RangeX,int16_t RangeY,int16_t RangeWidth,int16_t RangeHeight, int16_t X, int16_t Y, char *String, uint8_t FontSize)
{
	/*由于有可能显示极长的字符串，所以uint16_t*/
	uint16_t i;
	for (i = 0; String[i] != '\0'; i++)		//遍历字符串的每个字符
	{
		/*调用OLED_ShowCharArea函数，依次显示每个字符*/
		OLED_ShowCharArea(RangeX,RangeY,RangeWidth,RangeHeight,X + i * FontSize,Y, String[i],FontSize);
	}
}
/**
  * @brief 在指定区域范围内OLED显示汉字串
  * @param RangeX 指定字符可以显示范围的左上角的横坐标，范围：负值~OLED_WIDTH-1
  * @param RangeY 指定字符可以显示范围的左上角的纵坐标，范围：负值~OLED_HEIGHT-1
  * @param RangeWidth 指定范围宽度
  * @param RangeHeight 指定范围高度
  * @param X 指定字符左上角的横坐标，范围：负值~OLED_WIDTH-1
  * @param Y 指定字符左上角的纵坐标，范围：负值~OLED_HEIGHT-1
  * @param Chinese 指定要显示的汉字串，范围：必须全部为汉字或者全角字符，不要加入任何半角字符
  *           显示的汉字需要在OLED_Data.c里的OLED_CF16x16或OLED_CF12x12数组定义
  *           未找到指定汉字时，会显示默认图形（一个方框，内部一个问号）
  * @param FontSize 指定中文文字大小，，OLED_8X8_FULL,OLED_12X12_FULL,OLED_16X16_FULL,OLED_20X20_FULL
  * @retval 无
  */


void OLED_ShowChineseArea(int16_t RangeX,int16_t RangeY,int16_t RangeWidth,int16_t RangeHeight, int16_t X, int16_t Y, char *Chinese, uint8_t FontSize)
{
    // 8x8 保留内部字模
    if (FontSize == OLED_8X8_FULL)
    {
        uint8_t pChinese = 0;
        uint8_t pIndex;
        uint8_t i;
        char SingleChinese[OLED_CHN_CHAR_WIDTH + 1] = {0};
        for (i = 0; Chinese[i] != '\0'; i++)
        {
            SingleChinese[pChinese] = Chinese[i];
            pChinese++;
            if (pChinese >= OLED_CHN_CHAR_WIDTH)
            {
                pChinese = 0;
                const ChineseCell8x8_t* fontArray = OLED_CF8x8;
                for (pIndex = 0; strcmp(fontArray[pIndex].Index, "") != 0; pIndex++)
                {
                    if (strcmp(fontArray[pIndex].Index, SingleChinese) == 0) break;
                }
                OLED_ShowImageArea(X + ((i + 1) / OLED_CHN_CHAR_WIDTH - 1) * OLED_8X8_FULL, Y,
                                   OLED_8X8_FULL, OLED_8X8_FULL,
                                   RangeX, RangeY, RangeWidth, RangeHeight, fontArray[pIndex].Data);
            }
        }
        return;
    }
    // 12/16/20 走外部 Flash 字库
    OLED_ShowChineseAreaExt(RangeX, RangeY, RangeWidth, RangeHeight, X, Y, Chinese, FontSize);
}




/**
  * @brief 在指定区域范围内OLED显示汉字串
  * @param RangeX 指定字符可以显示范围的左上角的横坐标，范围：负值~OLED_WIDTH-1
  * @param RangeY 指定字符可以显示范围的左上角的纵坐标，范围：负值~OLED_HEIGHT-1
  * @param RangeWidth 指定范围宽度
  * @param RangeHeight 指定范围高度
  * @param X 指定字符左上角的横坐标，范围：负值~OLED_WIDTH-1
  * @param Y 指定字符左上角的纵坐标，范围：负值~OLED_HEIGHT-1
  * @param Chinese 指定要显示的汉字串，范围：必须全部为汉字或者全角字符，不要加入任何半角字符
  * @param format 指定要显示的格式化字符串，范围：ASCII码可见字符组成的字符串
  * @param ... 格式化字符串参数列表
  * @param FontSize 指定字体大小
  *           范围：OLED_8X16		宽8像素，高16像素
  *    							OLED_7X12		宽7像素，高12像素
  *                 OLED_6X8		宽6像素，高8像素
  * @retval 无
  */
 

void OLED_PrintfArea(int16_t RangeX,int16_t RangeY,int16_t RangeWidth,int16_t RangeHeight, int16_t X, int16_t Y,uint8_t FontSize, char *format, ...)
{
	//由于有可能显示极长的字符串，所以128
	char String[MAX_STRING_LENGTH];						//定义字符数组
	va_list arg;							//定义可变参数列表数据类型的变量arg
	va_start(arg, format);					//从format开始，接收参数列表到arg变量
	vsprintf(String, format, arg);			//使用vsprintf打印格式化字符串和参数列表到字符数组中
	va_end(arg);							//结束变量arg
	OLED_ShowStringArea(RangeX, RangeY, RangeWidth, RangeHeight, X, Y, String, FontSize);//OLED显示字符数组（字符串）
	
}


/**
  * @brief 在指定区域范围内OLED显示混合字符串（汉字与ASCII）
  * @param RangeX 指定字符可以显示范围的左上角的横坐标，范围：负值~OLED_WIDTH-1
  * @param RangeY 指定字符可以显示范围的左上角的纵坐标，范围：负值~OLED_HEIGHT-1
  * @param RangeWidth 指定范围宽度
  * @param RangeHeight 指定范围高度
  * @param X 指定字符左上角的横坐标，范围：负值~OLED_WIDTH-1
  * @param Y 指定字符左上角的纵坐标，范围：负值~OLED_HEIGHT-1
  * @param String 指定要显示的混合字符串，范围：全角字符与半角字符都可以
  *           显示的汉字需要在OLED_Data.c里的OLED_CF16x16或OLED_CF12x12数组定义
  *           未找到指定汉字时，会显示默认图形（一个方框，内部一个问号）
  * @param ChineseFontSize 指定中文文字大小，OLED_12X12或OLED_16X16或OLED_8X8
  * @param ASCIIFontSize  指定ASCII文字大小,OLED_6X8或OLED_7X12或OLED_8X16
  * @retval 无
  */

void OLED_ShowMixStringArea(int16_t RangeX, int16_t RangeY, int16_t RangeWidth, int16_t RangeHeight, int16_t X, int16_t Y, char *String, uint8_t ChineseFontSize, uint8_t ASCIIFontSize)
{
    // 8x8 保留内部字模
    if (ChineseFontSize == OLED_8X8_FULL)
    {
        while (*String != '\0') {
            if (*String & 0x80) {
                char Chinese[OLED_CHN_CHAR_WIDTH + 1];
                for (uint8_t i = 0; i < OLED_CHN_CHAR_WIDTH; i++) {
                    Chinese[i] = *(String + i);
                }
                Chinese[OLED_CHN_CHAR_WIDTH] = '\0';
                OLED_ShowChineseArea(RangeX, RangeY, RangeWidth, RangeHeight, X, Y, Chinese, ChineseFontSize);
                X += ChineseFontSize;
                String += OLED_CHN_CHAR_WIDTH;
            } else {
                OLED_ShowCharArea(RangeX, RangeY, RangeWidth, RangeHeight, X, Y, *String, ASCIIFontSize);
                X += ASCIIFontSize;
                String++;
            }
        }
        return;
    }
    // 12/16/20 走外部 Flash 字库
    OLED_ShowMixStringAreaExt(RangeX, RangeY, RangeWidth, RangeHeight, X, Y, String, ChineseFontSize, ASCIIFontSize);
}


/**
  * @brief OLED使用printf函数在指定区域内打印格式化字符串，此函数由bilibili@上nm网课呢 添加
  * @param RangeX 指定字符可以显示范围的左上角的横坐标，范围：负值~OLED_WIDTH-1
  * @param RangeY 指定字符可以显示范围的左上角的纵坐标，范围：负值~OLED_HEIGHT-1
  * @param RangeWidth 指定范围宽度
  * @param RangeHeight 指定范围高度
  * @param X 指定字符左上角的横坐标，范围：负值~OLED_WIDTH-1
  * @param Y 指定字符左上角的纵坐标，范围：负值~OLED_HEIGHT-1
  * @param ChineseFontSize 指定汉字字体大小,OLED_8X8_FULL,OLED_12X12_FULL,OLED_16X16_FULL,OLED_20X20_FULL
  * @param ASCIIFontSize 指定ASCII字体大小,OLED_6X8_HALF,OLED_7X12_HALF,OLED_8X16_HALF,OLED_10X20_HALF
			format 指定要显示的格式化字符串，范围：ASCII码可见字符组成的字符串
  * @param ... 格式化字符串参数列表
  * @return 无
  */

void OLED_PrintfMixArea(int16_t RangeX,int16_t RangeY,int16_t RangeWidth,int16_t RangeHeight,int16_t X, int16_t Y, uint8_t ChineseFontSize,uint8_t ASCIIFontSize, char *format, ...)
{
	//由于有可能显示极长的字符串，所以128
	char String[MAX_STRING_LENGTH];						//定义字符数组
	va_list arg;							//定义可变参数列表数据类型的变量arg
	va_start(arg, format);					//从format开始，接收参数列表到arg变量
	vsprintf(String, format, arg);			//使用vsprintf打印格式化字符串和参数列表到字符数组中
	va_end(arg);							//结束变量arg
	OLED_ShowMixStringArea(RangeX,RangeY,RangeWidth,RangeHeight,X, Y, String, ChineseFontSize,ASCIIFontSize);//OLED显示字符数组（字符串）
}





/**
  * 函    数：OLED在指定位置画一个点
  * 参    数：X 指定点的横坐标，范围：0~OLED_WIDTH-1
  * 参    数：Y 指定点的纵坐标，范围：0~OLED_HEIGHT-1
  * 返 回 值：无
  * 说    明：调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数
  */
void OLED_DrawPoint(int16_t X, int16_t Y)
{
	/*参数检查，保证指定位置不会超出屏幕范围*/
	if(X < 0 || Y < 0 || X > OLED_WIDTH-1 || Y > OLED_HEIGHT-1) {return;}
	
	/*将显存数组指定位置的一个Bit数据置1*/
	OLED_DisplayBuf[Y / 8][X] |= 0x01 << (Y % 8);
}

/**
  * 函    数：OLED获取指定位置点的值
  * 参    数：X 指定点的横坐标，范围：0~OLED_WIDTH-1
  * 参    数：Y 指定点的纵坐标，范围：0~OLED_HEIGHT-1
  * 返 回 值：指定位置点是否处于点亮状态，1：点亮，0：熄灭
  */
uint8_t OLED_GetPoint(uint8_t X, uint8_t Y)
{
	/*参数检查，保证指定位置不会超出屏幕范围*/
	if (X > OLED_WIDTH-1) {return 0;}
	if (Y > OLED_HEIGHT-1) {return 0;}
	
	/*判断指定位置的数据*/
	if (OLED_DisplayBuf[Y / 8][X] & 0x01 << (Y % 8))
	{
		return 1;	//为1，返回1
	}
	return 0;		//否则，返回0
}



/**
  * 函    数：OLED画线
  * 参    数：X0 指定一个端点的横坐标，范围：0~127
  * 参    数：Y0 指定一个端点的纵坐标，范围：0~63
  * 参    数：X1 指定另一个端点的横坐标，范围：0~127
  * 参    数：Y1 指定另一个端点的纵坐标，范围：0~63
  * 返 回 值：无
  * 说    明：调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数
  */
void OLED_DrawLine(int16_t X0, int16_t Y0, int16_t X1, int16_t Y1)
{
	int16_t x, y, dx, dy, d, incrE, incrNE, temp;
	int16_t x0 = X0, y0 = Y0, x1 = X1, y1 = Y1;
	uint8_t yflag = 0, xyflag = 0;
	
	if (y0 == y1)		//横线单独处理
	{
		/*0号点X坐标大于1号点X坐标，则交换两点X坐标*/
		if (x0 > x1) {temp = x0; x0 = x1; x1 = temp;}
		
		/*遍历X坐标*/
		for (x = x0; x <= x1; x ++)
		{
			OLED_DrawPoint(x, y0);	//依次画点
		}
	}
	else if (x0 == x1)	//竖线单独处理
	{
		/*0号点Y坐标大于1号点Y坐标，则交换两点Y坐标*/
		if (y0 > y1) {temp = y0; y0 = y1; y1 = temp;}
		
		/*遍历Y坐标*/
		for (y = y0; y <= y1; y ++)
		{
			OLED_DrawPoint(x0, y);	//依次画点
		}
	}
	else				//斜线
	{
		/*使用Bresenham算法画直线，可以避免耗时的浮点运算，效率更高*/
		/*参考文档：https://www.cs.montana.edu/courses/spring2009/425/dslectures/Bresenham.pdf*/
		/*参考教程：https://www.bilibili.com/video/BV1364y1d7Lo*/
		
		if (x0 > x1)	//0号点X坐标大于1号点X坐标
		{
			/*交换两点坐标*/
			/*交换后不影响画线，但是画线方向由第一、二、三、四象限变为第一、四象限*/
			temp = x0; x0 = x1; x1 = temp;
			temp = y0; y0 = y1; y1 = temp;
		}
		
		if (y0 > y1)	//0号点Y坐标大于1号点Y坐标
		{
			/*将Y坐标取负*/
			/*取负后影响画线，但是画线方向由第一、四象限变为第一象限*/
			y0 = -y0;
			y1 = -y1;
			
			/*置标志位yflag，记住当前变换，在后续实际画线时，再将坐标换回来*/
			yflag = 1;
		}
		
		if (y1 - y0 > x1 - x0)	//画线斜率大于1
		{
			/*将X坐标与Y坐标互换*/
			/*互换后影响画线，但是画线方向由第一象限0~90度范围变为第一象限0~45度范围*/
			temp = x0; x0 = y0; y0 = temp;
			temp = x1; x1 = y1; y1 = temp;
			
			/*置标志位xyflag，记住当前变换，在后续实际画线时，再将坐标换回来*/
			xyflag = 1;
		}
		
		/*以下为Bresenham算法画直线*/
		/*算法要求，画线方向必须为第一象限0~45度范围*/
		dx = x1 - x0;
		dy = y1 - y0;
		incrE = 2 * dy;
		incrNE = 2 * (dy - dx);
		d = 2 * dy - dx;
		x = x0;
		y = y0;
		
		/*画起始点，同时判断标志位，将坐标换回来*/
		if (yflag && xyflag){OLED_DrawPoint(y, -x);}
		else if (yflag)		{OLED_DrawPoint(x, -y);}
		else if (xyflag)	{OLED_DrawPoint(y, x);}
		else				{OLED_DrawPoint(x, y);}
		
		while (x < x1)		//遍历X轴的每个点
		{
			x ++;
			if (d < 0)		//下一个点在当前点东方
			{
				d += incrE;
			}
			else			//下一个点在当前点东北方
			{
				y ++;
				d += incrNE;
			}
			
			/*画每一个点，同时判断标志位，将坐标换回来*/
			if (yflag && xyflag){OLED_DrawPoint(y, -x);}
			else if (yflag)		{OLED_DrawPoint(x, -y);}
			else if (xyflag)	{OLED_DrawPoint(y, x);}
			else				{OLED_DrawPoint(x, y);}
		}	
	}
}

/**
  * 函    数：OLED矩形
  * 参    数：X 指定矩形左上角的横坐标，范围：0~127
  * 参    数：Y 指定矩形左上角的纵坐标，范围：0~63
  * 参    数：Width 指定矩形的宽度，范围：0~128
  * 参    数：Height 指定矩形的高度，范围：0~64
  * 参    数：IsFilled 指定矩形是否填充
  *           范围：OLED_UNFILLED		不填充
  *                 OLED_FILLED			填充
  * 返 回 值：无
  * 说    明：调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数。此函数经过更改，填充的时候效率更高
  */
void OLED_DrawRectangle(int16_t X, int16_t Y, int16_t Width, int16_t Height, uint8_t IsFilled)
{
    if (Width == 0 || Height == 0) { return; } // 宽度或高度为0，不绘制

    int16_t X_start = X;
    int16_t X_end = X + Width - 1;
    int16_t Y_start = Y;
    int16_t Y_end = Y + Height - 1;

    // 限制坐标在屏幕范围内
    if (X_start < 0) X_start = 0;
    if (X_end >= OLED_WIDTH) X_end = OLED_WIDTH - 1;
    if (Y_start < 0) Y_start = 0;
    if (Y_end >= OLED_HEIGHT) Y_end = OLED_HEIGHT - 1;

    // 计算有效宽度和高度
    int16_t validWidth = X_end - X_start + 1;
    int16_t validHeight = Y_end - Y_start + 1;
    if (validWidth <= 0 || validHeight <= 0) { return; }

    
    // 计算起始和结束页
    int16_t start_page = Y_start / 8;
    int16_t end_page = Y_end / 8;

    // 计算每页的掩码
    uint8_t start_mask = 0xFF << (Y_start % 8);
    uint8_t end_mask = 0xFF >> (7 - (Y_end % 8));

    
		if(IsFilled){
			// 遍历每一列，应用掩码
			for (int16_t x = X_start; x <= X_end; x++) {
					for (int16_t page = start_page; page <= end_page; page++) {
							uint8_t mask = 0xFF;
							if (page == start_page) mask &= start_mask;
							if (page == end_page) mask &= end_mask;
							if (page >= 0 && page < OLED_HEIGHT / 8) { // 确保页数有效
									OLED_DisplayBuf[page][x] |= mask;
							}
					}
			}
		}else{
			// 绘制矩形边框
        for (int16_t i = X; i <= X + Width - 1; i++) {
            OLED_DrawPoint(i, Y);
            OLED_DrawPoint(i, Y + Height - 1);
        }
        for (int16_t i = Y; i <= Y + Height - 1; i++) {
            OLED_DrawPoint(X, i);
            OLED_DrawPoint(X + Width - 1, i);
        }
		}
		
		
   
}
/**
  * 函    数：OLED三角形
  * 参    数：X0 指定第一个端点的横坐标，范围：0~127
  * 参    数：Y0 指定第一个端点的纵坐标，范围：0~63
  * 参    数：X1 指定第二个端点的横坐标，范围：0~127
  * 参    数：Y1 指定第二个端点的纵坐标，范围：0~63
  * 参    数：X2 指定第三个端点的横坐标，范围：0~127
  * 参    数：Y2 指定第三个端点的纵坐标，范围：0~63
  * 参    数：IsFilled 指定三角形是否填充
  *           范围：OLED_UNFILLED		不填充
  *                 OLED_FILLED			填充
  * 返 回 值：无
  * 说    明：调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数
  */
void OLED_DrawTriangle(int16_t X0, int16_t Y0, int16_t X1, int16_t Y1, int16_t X2, int16_t Y2, uint8_t IsFilled)
{
	int16_t minx = X0, miny = Y0, maxx = X0, maxy = Y0;
	int16_t i, j;
	int16_t vx[] = {X0, X1, X2};
	int16_t vy[] = {Y0, Y1, Y2};
	
	if (!IsFilled)			//指定三角形不填充
	{
		/*调用画线函数，将三个点用直线连接*/
		OLED_DrawLine(X0, Y0, X1, Y1);
		OLED_DrawLine(X0, Y0, X2, Y2);
		OLED_DrawLine(X1, Y1, X2, Y2);
	}
	else					//指定三角形填充
	{
		/*找到三个点最小的X、Y坐标*/
		if (X1 < minx) {minx = X1;}
		if (X2 < minx) {minx = X2;}
		if (Y1 < miny) {miny = Y1;}
		if (Y2 < miny) {miny = Y2;}
		
		/*找到三个点最大的X、Y坐标*/
		if (X1 > maxx) {maxx = X1;}
		if (X2 > maxx) {maxx = X2;}
		if (Y1 > maxy) {maxy = Y1;}
		if (Y2 > maxy) {maxy = Y2;}
		
		/*最小最大坐标之间的矩形为可能需要填充的区域*/
		/*遍历此区域中所有的点*/
		/*遍历X坐标*/
		for (i = minx; i <= maxx; i ++)
		{
			/*遍历Y坐标*/
			for (j = miny; j <= maxy; j ++)
			{
				/*调用OLED_pnpoly，判断指定点是否在指定三角形之中*/
				/*如果在，则画点，如果不在，则不做处理*/
				if (OLED_pnpoly(3, vx, vy, i, j)) {OLED_DrawPoint(i, j);}
			}
		}
	}
}

/**
  * 函    数：OLED画圆
  * 参    数：X 指定圆的圆心横坐标，范围：0~127
  * 参    数：Y 指定圆的圆心纵坐标，范围：0~63
  * 参    数：Radius 指定圆的半径，范围：0~255
  * 参    数：IsFilled 指定圆是否填充
  *           范围：OLED_UNFILLED		不填充
  *                 OLED_FILLED			填充
  * 返 回 值：无
  * 说    明：调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数
  */
void OLED_DrawCircle(int16_t X, int16_t Y, int16_t Radius, uint8_t IsFilled)
{
	int16_t x, y, d, j;
	
	/*使用Bresenham算法画圆，可以避免耗时的浮点运算，效率更高*/
	/*参考文档：https://www.cs.montana.edu/courses/spring2009/425/dslectures/Bresenham.pdf*/
	/*参考教程：https://www.bilibili.com/video/BV1VM4y1u7wJ*/
	
	d = 1 - Radius;
	x = 0;
	y = Radius;
	
	/*画每个八分之一圆弧的起始点*/
	OLED_DrawPoint(X + x, Y + y);
	OLED_DrawPoint(X - x, Y - y);
	OLED_DrawPoint(X + y, Y + x);
	OLED_DrawPoint(X - y, Y - x);
	
	if (IsFilled)		//指定圆填充
	{
		/*遍历起始点Y坐标*/
		for (j = -y; j < y; j ++)
		{
			/*在指定区域画点，填充部分圆*/
			OLED_DrawPoint(X, Y + j);
		}
	}
	
	while (x < y)		//遍历X轴的每个点
	{
		x ++;
		if (d < 0)		//下一个点在当前点东方
		{
			d += 2 * x + 1;
		}
		else			//下一个点在当前点东南方
		{
			y --;
			d += 2 * (x - y) + 1;
		}
		
		/*画每个八分之一圆弧的点*/
		OLED_DrawPoint(X + x, Y + y);
		OLED_DrawPoint(X + y, Y + x);
		OLED_DrawPoint(X - x, Y - y);
		OLED_DrawPoint(X - y, Y - x);
		OLED_DrawPoint(X + x, Y - y);
		OLED_DrawPoint(X + y, Y - x);
		OLED_DrawPoint(X - x, Y + y);
		OLED_DrawPoint(X - y, Y + x);
		
		if (IsFilled)	//指定圆填充
		{
			/*遍历中间部分*/
			for (j = -y; j < y; j ++)
			{
				/*在指定区域画点，填充部分圆*/
				OLED_DrawPoint(X + x, Y + j);
				OLED_DrawPoint(X - x, Y + j);
			}
			
			/*遍历两侧部分*/
			for (j = -x; j < x; j ++)
			{
				/*在指定区域画点，填充部分圆*/
				OLED_DrawPoint(X - y, Y + j);
				OLED_DrawPoint(X + y, Y + j);
			}
		}
	}
}

/**
  * 函    数：OLED画椭圆
  * 参    数：X 指定椭圆的圆心横坐标，范围：0~127
  * 参    数：Y 指定椭圆的圆心纵坐标，范围：0~63
  * 参    数：A 指定椭圆的横向半轴长度，范围：0~255
  * 参    数：B 指定椭圆的纵向半轴长度，范围：0~255
  * 参    数：IsFilled 指定椭圆是否填充
  *           范围：OLED_UNFILLED		不填充
  *                 OLED_FILLED			填充
  * 返 回 值：无
  * 说    明：调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数
  */
void OLED_DrawEllipse(int16_t X, int16_t Y, int16_t A, int16_t B, uint8_t IsFilled)
{
	int16_t x, y, j;
	int16_t a = A, b = B;
	float d1, d2;
	
	/*使用Bresenham算法画椭圆，可以避免部分耗时的浮点运算，效率更高*/
	/*参考链接：https://blog.csdn.net/myf_666/article/details/128167392*/
	
	x = 0;
	y = b;
	d1 = b * b + a * a * (-b + 0.5);
	
	if (IsFilled)	//指定椭圆填充
	{
		/*遍历起始点Y坐标*/
		for (j = -y; j < y; j ++)
		{
			/*在指定区域画点，填充部分椭圆*/
			OLED_DrawPoint(X, Y + j);
			OLED_DrawPoint(X, Y + j);
		}
	}
	
	/*画椭圆弧的起始点*/
	OLED_DrawPoint(X + x, Y + y);
	OLED_DrawPoint(X - x, Y - y);
	OLED_DrawPoint(X - x, Y + y);
	OLED_DrawPoint(X + x, Y - y);
	
	/*画椭圆中间部分*/
	while (b * b * (x + 1) < a * a * (y - 0.5))
	{
		if (d1 <= 0)		//下一个点在当前点东方
		{
			d1 += b * b * (2 * x + 3);
		}
		else				//下一个点在当前点东南方
		{
			d1 += b * b * (2 * x + 3) + a * a * (-2 * y + 2);
			y --;
		}
		x ++;
		
		if (IsFilled)	//指定椭圆填充
		{
			/*遍历中间部分*/
			for (j = -y; j < y; j ++)
			{
				/*在指定区域画点，填充部分椭圆*/
				OLED_DrawPoint(X + x, Y + j);
				OLED_DrawPoint(X - x, Y + j);
			}
		}
		
		/*画椭圆中间部分圆弧*/
		OLED_DrawPoint(X + x, Y + y);
		OLED_DrawPoint(X - x, Y - y);
		OLED_DrawPoint(X - x, Y + y);
		OLED_DrawPoint(X + x, Y - y);
	}
	
	/*画椭圆两侧部分*/
	d2 = b * b * (x + 0.5) * (x + 0.5) + a * a * (y - 1) * (y - 1) - a * a * b * b;
	
	while (y > 0)
	{
		if (d2 <= 0)		//下一个点在当前点东方
		{
			d2 += b * b * (2 * x + 2) + a * a * (-2 * y + 3);
			x ++;
			
		}
		else				//下一个点在当前点东南方
		{
			d2 += a * a * (-2 * y + 3);
		}
		y --;
		
		if (IsFilled)	//指定椭圆填充
		{
			/*遍历两侧部分*/
			for (j = -y; j < y; j ++)
			{
				/*在指定区域画点，填充部分椭圆*/
				OLED_DrawPoint(X + x, Y + j);
				OLED_DrawPoint(X - x, Y + j);
			}
		}
		
		/*画椭圆两侧部分圆弧*/
		OLED_DrawPoint(X + x, Y + y);
		OLED_DrawPoint(X - x, Y - y);
		OLED_DrawPoint(X - x, Y + y);
		OLED_DrawPoint(X + x, Y - y);
	}
}

/**
  * 函    数：OLED画圆弧
  * 参    数：X 指定圆弧的圆心横坐标，范围：0~127
  * 参    数：Y 指定圆弧的圆心纵坐标，范围：0~63
  * 参    数：Radius 指定圆弧的半径，范围：0~255
  * 参    数：StartAngle 指定圆弧的起始角度，范围：-180~180
  *           水平向右为0度，水平向左为180度或-180度，下方为正数，上方为负数，顺时针旋转
  * 参    数：EndAngle 指定圆弧的终止角度，范围：-180~180
  *           水平向右为0度，水平向左为180度或-180度，下方为正数，上方为负数，顺时针旋转
  * 参    数：IsFilled 指定圆弧是否填充，填充后为扇形
  *           范围：OLED_UNFILLED		不填充
  *                 OLED_FILLED			填充
  * 返 回 值：无
  * 说    明：调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数
  */
void OLED_DrawArc(int16_t X, int16_t Y, int16_t Radius, int16_t StartAngle, int16_t EndAngle, uint8_t IsFilled)
{
	int16_t x, y, d, j;
	if(Radius <=0){return;} //半径为0，直接返回
	/*此函数借用Bresenham算法画圆的方法*/
	
	d = 1 - Radius;
	x = 0;
	y = Radius;
	
	/*在画圆的每个点时，判断指定点是否在指定角度内，在，则画点，不在，则不做处理*/
	if (OLED_IsInAngle(x, y, StartAngle, EndAngle))	{OLED_DrawPoint(X + x, Y + y);}
	if (OLED_IsInAngle(-x, -y, StartAngle, EndAngle)) {OLED_DrawPoint(X - x, Y - y);}
	if (OLED_IsInAngle(y, x, StartAngle, EndAngle)) {OLED_DrawPoint(X + y, Y + x);}
	if (OLED_IsInAngle(-y, -x, StartAngle, EndAngle)) {OLED_DrawPoint(X - y, Y - x);}
	
	if (IsFilled)	//指定圆弧填充
	{
		/*遍历起始点Y坐标*/
		for (j = -y; j < y; j ++)
		{
			/*在填充圆的每个点时，判断指定点是否在指定角度内，在，则画点，不在，则不做处理*/
			if (OLED_IsInAngle(0, j, StartAngle, EndAngle)) {OLED_DrawPoint(X, Y + j);}
		}
	}
	
	while (x < y)		//遍历X轴的每个点
	{
		x ++;
		if (d < 0)		//下一个点在当前点东方
		{
			d += 2 * x + 1;
		}
		else			//下一个点在当前点东南方
		{
			y --;
			d += 2 * (x - y) + 1;
		}
		
		/*在画圆的每个点时，判断指定点是否在指定角度内，在，则画点，不在，则不做处理*/
		if (OLED_IsInAngle(x, y, StartAngle, EndAngle)) {OLED_DrawPoint(X + x, Y + y);}
		if (OLED_IsInAngle(y, x, StartAngle, EndAngle)) {OLED_DrawPoint(X + y, Y + x);}
		if (OLED_IsInAngle(-x, -y, StartAngle, EndAngle)) {OLED_DrawPoint(X - x, Y - y);}
		if (OLED_IsInAngle(-y, -x, StartAngle, EndAngle)) {OLED_DrawPoint(X - y, Y - x);}
		if (OLED_IsInAngle(x, -y, StartAngle, EndAngle)) {OLED_DrawPoint(X + x, Y - y);}
		if (OLED_IsInAngle(y, -x, StartAngle, EndAngle)) {OLED_DrawPoint(X + y, Y - x);}
		if (OLED_IsInAngle(-x, y, StartAngle, EndAngle)) {OLED_DrawPoint(X - x, Y + y);}
		if (OLED_IsInAngle(-y, x, StartAngle, EndAngle)) {OLED_DrawPoint(X - y, Y + x);}
		
		if (IsFilled)	//指定圆弧填充
		{
			/*遍历中间部分*/
			for (j = -y; j < y; j ++)
			{
				/*在填充圆的每个点时，判断指定点是否在指定角度内，在，则画点，不在，则不做处理*/
				if (OLED_IsInAngle(x, j, StartAngle, EndAngle)) {OLED_DrawPoint(X + x, Y + j);}
				if (OLED_IsInAngle(-x, j, StartAngle, EndAngle)) {OLED_DrawPoint(X - x, Y + j);}
			}
			
			/*遍历两侧部分*/
			for (j = -x; j < x; j ++)
			{
				/*在填充圆的每个点时，判断指定点是否在指定角度内，在，则画点，不在，则不做处理*/
				if (OLED_IsInAngle(-y, j, StartAngle, EndAngle)) {OLED_DrawPoint(X - y, Y + j);}
				if (OLED_IsInAngle(y, j, StartAngle, EndAngle)) {OLED_DrawPoint(X + y, Y + j);}
			}
		}
	}
}



/**
  * 函    数：OLED圆角矩形
  * 参    数：X 指定矩形左上角的横坐标，范围：0~OLED_WIDTH-1
  * 参    数：Y 指定矩形左上角的纵坐标，范围：0~OLED_HEIGHT-1
  * 参    数：Width 指定矩形的宽度，范围：0~128
  * 参    数：Height 指定矩形的高度，范围：0~64
  * 参    数：Radius 圆角半径
  * 参    数：IsFilled 指定矩形是否填充
  *           范围：OLED_UNFILLED		不填充
  *                 OLED_FILLED			填充
  * 返 回 值：无
  * 说    明：调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数
  */
void OLED_DrawRoundedRectangle(int16_t X, int16_t Y, int16_t Width, int16_t Height, int16_t Radius, uint8_t IsFilled)
{
    // 安全检查
    if (Width == 0 || Height == 0) return;  // 宽度或高度为0,直接返回
    if (Radius > Width / 2 || Radius > Height / 2) {
        Radius = (Width < Height ? Width : Height) / 2;  // 限制圆角半径不超过宽高的一半
    }
    if (Radius <= 0) {
        OLED_DrawRectangle(X, Y, Width, Height, IsFilled);  // 如果半径为0,退化为普通矩形
        return;
    }
		// 绘制四个圆角
		OLED_DrawArc(X + Radius, Y + Radius, Radius,180,  -90, IsFilled);
		OLED_DrawArc(X + Width - Radius - 1, Y + Radius, Radius,-90,  0, IsFilled);
		OLED_DrawArc(X + Radius, Y + Height - Radius - 1, Radius,90,  180, IsFilled);
		OLED_DrawArc(X + Width - Radius - 1, Y + Height - Radius - 1, Radius,0,  90, IsFilled);


    // 填充或绘制矩形主体
    if (IsFilled) {
        OLED_DrawRectangle(X + Radius, Y, Width - 2 * Radius, Height, OLED_FILLED);
        OLED_DrawRectangle(X, Y + Radius, Width, Height - 2 * Radius, OLED_FILLED);
    } else {
        // 绘制顶部和底部的直线
        OLED_DrawLine(X + Radius, Y, X + Width - Radius - 1, Y);
        OLED_DrawLine(X + Radius, Y + Height - 1, X + Width - Radius - 1, Y + Height - 1);
        // 绘制左侧和右侧的直线
        OLED_DrawLine(X, Y + Radius, X, Y + Height - Radius - 1);
        OLED_DrawLine(X + Width - 1, Y + Radius, X + Width - 1, Y + Height - Radius - 1);
    }
}









