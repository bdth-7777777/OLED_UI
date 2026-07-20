#include "OLED_UI.h"
#include "..\OLED_UI_Launcher.h"


#ifdef OLED_UI
/**
 * @brief 本项目开源地址：
 * @param "https://github.com/bdth-7777777/OLED_UI"
 */
/*OLED_UI全局变量定义 */
OLED_UI_Counter OLED_FPS = {0,0,0};									//用于存储帧率的结构体
OLED_Key OLED_UI_Key = {1,1,1,1};   								//用于存储按键状态的结构体,默认没有按下，都为1
OLED_Key OLED_UI_LastKey = {1,1,1,1};								//用于存储上一轮按键状态的结构体,默认没有按下，都为1
MenuPage*  CurrentMenuPage = NULL;									//全局结构体指针，当前页面的指针
MenuWindow *CurrentWindow = NULL;									//全局结构体指针，当前窗口的指针
MutexFlag KeyEnterFlag = FLAGEND;									//全局enter按键的互斥锁，互斥锁为FLAGSTART时表示正在执行回调函数
MutexFlag FadeOutFlag = FLAGEND;									//渐隐效果的互斥锁，互斥锁为FLAGSTART时表示正在执行渐隐效果
bool ColorMode = DARKMODE;											//全局布尔型数据，存储当前显示模式，true为深色模式，false为浅色模式
bool OLED_UI_ShowFps = false;										//全局布尔型数据，用于控制是否显示帧率
int16_t OLED_UI_Brightness = 100;									//全局变量，存储当前屏幕亮度
OLED_UI_WindowSustainCounter OLED_SustainCounter = {0,false};		//用于存储窗口持续时间的结构体
int tmpi=0;                                                       	//Gif动画缓速播放temp值

/***********************************************************************************************/
/***************************这些变量用于存储需要绑定动画的控件的参数*******************************/

// OLED_UI_Cursor是用于存储光标的结构体，控制光标的移动
OLED_ChangeArea OLED_UI_Cursor;
// OLED_UI_MenuFrame是用于存储菜单边框的结构体，控制菜单边框与内部组件的移动
OLED_ChangeArea OLED_UI_MenuFrame;
// OLED_UI_Window是用于存储窗口的结构体，控制窗口与内部组件的移动
OLED_ChangeArea OLED_UI_Window;
// OLED_UI_ScrollBarHeight是用于存储滚动条高度的结构体，控制滚动条高度的变化
OLED_ChangeDistance OLED_UI_ScrollBarHeight;
// OLED_UI_ProbWidth是用于存储进度条宽度的结构体，控制进度条宽度的变化
OLED_ChangeDistance OLED_UI_ProbWidth;
// OLED_UI_PageStartPoint是用于存储页面起始点的结构体，控制页面整体的移动
OLED_ChangePoint OLED_UI_PageStartPoint ;
// OLED_UI_LineStep是用于存储行间距的结构体，控制行间距的变化
OLED_ChangeDistance OLED_UI_LineStep;



/**
 * @brief 获取当前屏幕刷新率，结果存储在全局变量OLED_FPS.value中
 * @param 无
 * @note 该函数需要放在20ms周期内调用，否则会导致计数错误
 * @return 无
 */
void GetFPS(void){
	if(OLED_FPS.step<49){
		OLED_FPS.step++;
	}else{
		OLED_FPS.step=0;
		OLED_FPS.value = OLED_FPS.count;
		OLED_FPS.count=0;
	}
}

/**
 * @brief 显示当前屏幕刷新率
 * @param 无
 * @note 需将此函数放在主循环当中，每循环一次记为一次刷新。
 * @return 无
 */
void OLED_UI_ShowFPS(void){
    OLED_FPS.count ++;
	if (OLED_UI_ShowFps){
		OLED_Printf(110,0,OLED_6X8_HALF,"%3d",OLED_FPS.value);
	}
}
/**
 * @brief 获取当前页面的字体宽度
 * @param style CHINESE【中文】或 ASCII【ASCII】
 * @return 当前页面的字体宽度
 */
OLED_Font GetOLED_Font(OLED_Font fontsize,bool style){
	//根据当前页面的字体大小设置字体
	OLED_Font ChineseFont,ASCIIFont;
	switch(fontsize){
		case OLED_UI_FONT_8:
			ChineseFont = OLED_8X8_FULL,ASCIIFont = OLED_6X8_HALF;break;
		case OLED_UI_FONT_12:
			ChineseFont = OLED_12X12_FULL,ASCIIFont = OLED_7X12_HALF;break;
		case OLED_UI_FONT_16:
			ChineseFont = OLED_16X16_FULL,ASCIIFont = OLED_8X16_HALF;break;
		case OLED_UI_FONT_20:
			ChineseFont = OLED_20X20_FULL,ASCIIFont = OLED_10X20_HALF;break;
		default:
			ChineseFont = OLED_12X12_FULL,ASCIIFont = OLED_7X12_HALF;break;
	}
	if(style == CHINESE){
		return ChineseFont;
	}else{
		return ASCIIFont;
	}

}
/**
 * @brief 反转显示光标
 * @param X 光标X坐标
 * @param Y 光标Y坐标
 * @param Width 光标宽度
 * @param Height 光标高度
 * @param Style 光标样式
 * @note 该函数用于反转显示光标，使其显示为反色
 * @return 无
 */
void ReverseCoordinate(int16_t X, int16_t Y, int16_t Width, int16_t Height,uint8_t Style){
	switch(Style){
		
		case REVERSE_RECTANGLE://矩形反色
			OLED_ReverseArea(X, Y, Width, Height);
			break;
		case REVERSE_ROUNDRECTANGLE://圆角矩形反色
			if(Width >= 3 && Height >= 3){
				OLED_ReverseArea(X, Y, 1, 1);
				OLED_ReverseArea(X+Width-1, Y, 1, 1);
				OLED_ReverseArea(X, Y+Height-1, 1, 1);
				OLED_ReverseArea(X+Width-1, Y+Height-1, 1, 1);
			}
			OLED_ReverseArea(X, Y, Width, Height);
			break;

		case HOLLOW_RECTANGLE://空心矩形
			OLED_ReverseArea(X, Y, Width, Height);
			OLED_ReverseArea(X+1, Y+1, Width-2, Height-2);
			break;
		case HOLLOW_ROUNDRECTANGLE://空心矩形
			OLED_ReverseArea(X, Y, Width, Height);
			OLED_ReverseArea(X+1, Y+1, Width-2, Height-2);
			if(Width >= 3){
				OLED_ReverseArea(X, Y, 1, 1);
				OLED_ReverseArea(X+Width-1, Y, 1, 1);
			}
			if(Height >= 3){
				OLED_ReverseArea(X, Y+Height-1, 1, 1);
				OLED_ReverseArea(X+Width-1, Y+Height-1, 1, 1);
			}
			break;
		case REVERSE_BLOCK://小方块
			
			OLED_ReverseArea(X, Y + 1, GetOLED_Font(CurrentMenuPage->General_FontSize,ASCII), Height);
			
			break;
		case NOT_SHOW:
			break;
		default:
			OLED_ReverseArea(X, Y, Width, Height);
		break;
	}
}

/** 
 * @brief：在指定区域应用模式化渐隐效果(蒙版颗粒化)
 * @param x0 区域起始X坐标
 * @param y0 区域起始Y坐标
 * @param width 区域宽度
 * @param height 区域高度
 * @param fadeLevel 渐隐档位，1到5之间的值，表示不同的像素熄灭模式
 * @note 该函数用于在指定区域应用模式化渐隐效果，使得该区域的像素逐渐变暗，效果类似蒙版颗粒化。
*/
void OLED_UI_FadeOut_Masking(int16_t x0, int16_t y0, int16_t width, int16_t height, int8_t fadeLevel) {
    // 检查并调整区域范围
    if (x0 < 0) {
        width += x0;
        x0 = 0;
    }
    if (y0 < 0) {
        height += y0;
        y0 = 0;
    }
    if (x0 + width > OLED_WIDTH) {
        width = OLED_WIDTH - x0;
    }
    if (y0 + height > OLED_HEIGHT) {
        height = OLED_HEIGHT - y0;
    }
    if (width <= 0 || height <= 0) {
        return;
    }

    // 确保渐隐档位在有效范围内
    if (fadeLevel < 1 || fadeLevel > 5) {
        return;
    }

    // 定义2x2网格的渐隐模式
    // 每个数组表示一个2x2网格中哪些像素需要熄灭
    // 0表示保持亮，1表示熄灭
    const uint8_t patterns[5][2][2] = {
        {{0, 0},  // Level 1: 全亮
         {0, 0}},
        
        {{1, 0},  // Level 2: 左上角熄灭
         {0, 0}},
        
        {{1, 0},  // Level 3: 左上角和右下角熄灭
         {0, 1}},
        
        {{1, 0},  // Level 4: 只保留右上角
         {1, 1}},
        
        {{1, 1},  // Level 5: 全暗
         {1, 1}}
    };

    // 计算边界
    int16_t xEnd = x0 + width;
    int16_t yEnd = y0 + height;

    // 应用渐隐效果
    for (int16_t y = y0; y < yEnd; y++) {
        int page = y / 8;
        int bit_pos = y % 8;
        uint8_t pixel_mask = 1 << bit_pos;
        
        for (int16_t x = x0; x < xEnd; x++) {
            // 计算在2x2网格中的相对位置
            int grid_x = (x - x0) % 2;
            int grid_y = (y - y0) % 2;
            
            // 根据渐隐级别和网格位置决定是否熄灭像素
            if (patterns[fadeLevel - 1][grid_y][grid_x]) {
                OLED_DisplayBuf[page][x] &= ~pixel_mask;
            }
        }
    }
}


/**
 * @brief 获取菜单项结构体数组的最后一个元素的ID
 * @param items 结构体数组MenuItem的指针
 * @return 菜单项结构体数组的元素数量
 */
MenuID GetMenuItemNum(MenuItem * items){
	MenuID num = 0;
	while(items[num].General_item_text != NULL){
		num+=1;
	}
	return num;
}


/**
 * @brief 根据当前所选的动画方式，改变浮点数参数
 * @param CurrentNum 当前值的指针
 * @param TargetNum 目标值指针
 * @param StartNum 起始值指针
 * @param CurrentStepNum 当前步数指针
 * @return 无
 */
void ChangeFloatNum(float *CurrentNum, float *TargetNum, float *ErrorNum, float *LastErrorNum, float * IntegralNum, float *DerivativeNum)  {
	if(CurrentMenuPage->General_MoveStyle ==  UNLINEAR){
		if(*CurrentNum == *TargetNum){
			return;
		}
		//如果用户将速度设置为0，那么当前值直接等于目标值
		if(CurrentMenuPage->General_MovingSpeed <= 0){

			*ErrorNum = 0;
			*LastErrorNum = 0;
		    *CurrentNum = *TargetNum;
			return;
		}
		*LastErrorNum = *ErrorNum;
		//计算本轮误差值
		*ErrorNum = *TargetNum - *CurrentNum; 
		//计算当前值
		*CurrentNum += 0.02f*CurrentMenuPage->General_MovingSpeed * (*ErrorNum);
		//当目标值与当前值差距小于速度值的1/20时，认为已经到达目标值
		if(fabs(*CurrentNum - *TargetNum) < CurrentMenuPage->General_MovingSpeed/20.0f){

			*ErrorNum = 0;
			*LastErrorNum = 0;
		    *CurrentNum = *TargetNum;
			return;
		}
	}
	//如果当前的动画方式是PID_CURVE方式
	if(CurrentMenuPage->General_MoveStyle ==  PID_CURVE){
		/*这是一种奇特的方法，因为当当前值等于目标值的时候，其他项置零了，但是积分项并没有被置零。根据实际现象，这样的效果是最好的。 */
		//如果用户将速度设置为0，那么当前值直接等于目标值，其他所有中间值置零
		if(CurrentMenuPage->General_MovingSpeed <= 0){

			*ErrorNum = 0;
			*LastErrorNum = 0;
			*DerivativeNum = 0;
			*IntegralNum = 0;
		    *CurrentNum = *TargetNum;
			return;
		}
		// 定义PID参数
		//定义PID参数
		float Kp = 0.02f * CurrentMenuPage->General_MovingSpeed;
		float Ki = 0.005f * CurrentMenuPage->General_MovingSpeed;
		float Kd = 0.002f ;

		//记录上一轮误差值
		*LastErrorNum = *ErrorNum;
		//计算本轮误差值
		*ErrorNum = *TargetNum - *CurrentNum; 
		//计算积分值
		*IntegralNum += *ErrorNum;
		//计算微分值，并假设时间间隔为0.1s
		*DerivativeNum = (*ErrorNum - *LastErrorNum) / 0.1f;
		//计算当前值
		*CurrentNum += Kp * (*ErrorNum) + Ki *  (*IntegralNum) + Kd *(*DerivativeNum);
		//当目标值与当前值差距小于0.5时，将目标值强制等于当前值，除了积分项不置零，其他所有中间值置零
		if(fabs(*TargetNum - *CurrentNum) < 0.5f){

			*ErrorNum = 0;
			*LastErrorNum = 0;
			*DerivativeNum = 0;
		    *CurrentNum = *TargetNum;
			return;
		}	
	}
	
}

/**
 * @brief 非线性改变距离参数
 * @param CurrentNum 当前值的指针
 * @param TargetNum 目标值指针
 * @param StepNum 步长指针
 */
void ChangeDistance(OLED_ChangeDistance *distance){
	ChangeFloatNum(&distance->CurrentDistance,&distance->TargetDistance,&distance->Error,&distance->LastError,&distance->Integral,&distance->Derivative);
}

/**
 * @brief 非线性改变点坐标参数
 * @param OLED_MovingArea 结构体数组的指针
 * @return 无
 */
void ChangePoint(OLED_ChangePoint *point){
	ChangeFloatNum(&point->CurrentPoint.X,&point->TargetPoint.X,&point->Error.X,&point->LastError.X,&point->Integral.X,&point->Derivative.X);
	ChangeFloatNum(&point->CurrentPoint.Y,&point->TargetPoint.Y,&point->Error.Y,&point->LastError.Y,&point->Integral.Y,&point->Derivative.Y);
}
/**
 * @brief 非线性改变区域参数
 * @param OLED_MovingArea 结构体数组的指针
 * @return 无
 */
void ChangeArea(OLED_ChangeArea *area)	{
	ChangeFloatNum(&area->CurrentArea.X,&area->TargetArea.X,&area->Error.X,&area->LastError.X,&area->Integral.X,&area->Derivative.X);
	ChangeFloatNum(&area->CurrentArea.Y,&area->TargetArea.Y,&area->Error.Y,&area->LastError.Y,&area->Integral.Y,&area->Derivative.Y);
	ChangeFloatNum(&area->CurrentArea.Width,&area->TargetArea.Width,&area->Error.Width,&area->LastError.Width,&area->Integral.Width,&area->Derivative.Width);
	ChangeFloatNum(&area->CurrentArea.Height,&area->TargetArea.Height,&area->Error.Height,&area->LastError.Height,&area->Integral.Height,&area->Derivative.Height);
}




/**
 * @brief 对当前的菜单页面的参数进行检查与初始化
 * @param 无
 * @note 用于确认事件下进入子菜单时的操作
 * @return 无
 */
void CurrentMenuPageInit(void){
	//如果当前的菜单类型为LIST
	if(CurrentMenuPage->General_MenuType == MENU_TYPE_LIST){
		//设置全局页面当前起始点为菜单结构体的开始点+相对位置起始点
		OLED_UI_PageStartPoint.CurrentPoint.X = OLED_UI_MenuFrame.CurrentArea.X+CurrentMenuPage->List_StartPointX + OLED_WIDTH;
		OLED_UI_PageStartPoint.CurrentPoint.Y = OLED_UI_MenuFrame.CurrentArea.Y+CurrentMenuPage->List_StartPointY;
		//设置全局页面目标起始点为菜单结构体的开始点+相对位置起始点
		OLED_UI_PageStartPoint.TargetPoint.X = CurrentMenuPage->List_MenuArea.X+CurrentMenuPage->List_StartPointX;
		OLED_UI_PageStartPoint.TargetPoint.Y = CurrentMenuPage->List_MenuArea.Y+CurrentMenuPage->List_StartPointY;
		//设置全局页面目标区域为当前菜单结构体的区域
		OLED_UI_MenuFrame.TargetArea.X = CurrentMenuPage->List_MenuArea.X;
		OLED_UI_MenuFrame.TargetArea.Y = CurrentMenuPage->List_MenuArea.Y;
		OLED_UI_MenuFrame.TargetArea.Width = CurrentMenuPage->List_MenuArea.Width;
		OLED_UI_MenuFrame.TargetArea.Height = CurrentMenuPage->List_MenuArea.Height;

		//设置当前行间距为-字体大小以制造菜单项展开的动画效果
		OLED_UI_LineStep.CurrentDistance =-3;
		//设置目标行间距为用户设置的行距
		OLED_UI_LineStep.TargetDistance = CurrentMenuPage->General_LineSpace;
		//设置当前光标为当前菜单项的第一个
		CurrentMenuPage->_ActiveMenuID = 0;
		//设置当前槽位为0
		CurrentMenuPage->_Slot = 0;
	}
	//如果当前的菜单类型为TILES
	if (CurrentMenuPage->General_MenuType == MENU_TYPE_TILES)
	{
		//设置全局页面目标起始点为屏幕中央向左偏移半个磁贴宽度，使得当前菜单项居中
		OLED_UI_PageStartPoint.TargetPoint.X = CurrentMenuPage->Tiles_ScreenWidth/2-CurrentMenuPage->Tiles_TileWidth/2;
		//设置全局页面目标起始点为预设的起始点
		OLED_UI_PageStartPoint.TargetPoint.Y = TILES_STARTPOINT_Y;
		//设置全局页面当前起始点为一个负的位置，使得开始时有动画
		OLED_UI_PageStartPoint.CurrentPoint.X = -50;
		OLED_UI_PageStartPoint.CurrentPoint.Y = -CurrentMenuPage->Tiles_TileWidth;
		//设置当前磁贴间距为1以便产生动画效果
		OLED_UI_LineStep.CurrentDistance = 1;
		//设置目标磁贴间距为用户设置的行距
		OLED_UI_LineStep.TargetDistance = CurrentMenuPage->General_LineSpace;
		//设置当前光标为当前菜单项的第一个
		CurrentMenuPage->_ActiveMenuID = 0;
	}
	//将滚动的开始点归零，确保当确认或是返回操作时，滚动的菜单项都从新开始
	SetLineSplitZero();
}
/**
 * @brief 还原菜单参数到上一次的状态
 * @param 无
 * @note 用于返回上一级菜单时的操作
 * @return 无
 */
void CurrentMenuPageBackUp(void){
	//如果当前的菜单类型为LIST
	if(CurrentMenuPage->General_MenuType == MENU_TYPE_LIST){
		//设置全局页面当前起始点为菜单结构体的开始点+2
		OLED_UI_PageStartPoint.CurrentPoint.X = OLED_UI_MenuFrame.CurrentArea.X + CurrentMenuPage->List_StartPointX - OLED_WIDTH;
		OLED_UI_PageStartPoint.CurrentPoint.Y = CurrentMenuPage->_StartPoint.Y;
		//设置全局页面目标起始点为菜单结构体的开始点
		OLED_UI_PageStartPoint.TargetPoint.X = CurrentMenuPage->_StartPoint.X;
		OLED_UI_PageStartPoint.TargetPoint.Y = CurrentMenuPage->_StartPoint.Y;
		//设置当前行间距为-字体大小以制造动画效果
		OLED_UI_LineStep.CurrentDistance = CurrentMenuPage->General_LineSpace;
		//设置目标行间距
		OLED_UI_LineStep.TargetDistance = CurrentMenuPage->General_LineSpace;
	}
	//如果当前的菜单类型为TILES
	if (CurrentMenuPage->General_MenuType == MENU_TYPE_TILES)
	{
		//设置全局页面当前起始点为菜单结构体的开始点+2
		OLED_UI_PageStartPoint.CurrentPoint.X = CurrentMenuPage->_StartPoint.X + CurrentMenuPage->Tiles_TileWidth;
		OLED_UI_PageStartPoint.CurrentPoint.Y = -CurrentMenuPage->Tiles_TileHeight-1;
		//设置全局页面目标起始点为菜单结构体的开始点
		OLED_UI_PageStartPoint.TargetPoint.X = CurrentMenuPage->_StartPoint.X;
		OLED_UI_PageStartPoint.TargetPoint.Y = CurrentMenuPage->_StartPoint.Y;
		//设置当前行间距为-字体大小以制造动画效果
		OLED_UI_LineStep.CurrentDistance = CurrentMenuPage->General_LineSpace;
		//设置目标行间距
		OLED_UI_LineStep.TargetDistance = CurrentMenuPage->General_LineSpace;
	}
	//将滚动的开始点归零，确保当确认或是返回操作时，滚动的菜单项都从新开始
	SetLineSplitZero();
}

/**
 * @brief 初始化OLED_UI，设置当前页面的结构体指针，并初始化OLED显示屏
 * @param Page 菜单页面结构体
 * @return 无
 */
void OLED_UI_Init(MenuPage* Page){
	//初始化OLED显示屏
	OLED_Init();

	//启动定时器
	HAL_TIM_Base_Start_IT(&htim1);
	Timer_Init();
	Key_Init();
	Encoder_Init();

	//设置当前页面的结构体指针
	CurrentMenuPage = Page;	//设置当前页面的结构体指针
	//初始化菜单页面参数
	CurrentMenuPageInit();
	
}



/**
 * @brief 获取enter事件状态，用于判断是否正在执行回调函数
 * @param void
 * @return 无
 */
bool GetEnterFlag(void){
	if(KeyEnterFlag == FLAGEND){
		return true;
	}else{
		return false;
	}
}

/**
 * @brief 获取enter事件状态，用于判断是否正在执行回调函数
 * @param void
 * @return 无
 */
bool GetFadeoutFlag(void){
	if(FadeOutFlag == FLAGEND){
		return true;
	}else{
		return false;
	}
}

/**
 * @brief 计算字符串的宽度（可以是中英文字符串）
 * @param String 字符串指针
 * @param ChineseFont 中文字体宽度
 * @param ASCIIFont ASCII字体宽度
 * @return 字符串的宽度
 */
int16_t CalcStringWidth(int16_t ChineseFont, int16_t ASCIIFont, const char *format, ...) {
    int16_t StringLength = 0;
    char String[MAX_STRING_LENGTH];

    va_list args;
    va_start(args, format);
    vsnprintf(String, sizeof(String), format, args); // 使用vsnprintf
    va_end(args);

    char *ptr = String;
    while (*ptr != '\0') {
        if ((unsigned char)*ptr & 0x80) { // 处理中文字符
            StringLength += ChineseFont;
            if ((*ptr & 0xF0) == 0xF0) ptr += 4;      // 4字节字符
        	else if ((*ptr & 0xE0) == 0xE0) ptr += 3;  // 3字节中文 
        	else if ((*ptr & 0xC0) == 0xC0) ptr += 2;  // 2字节字符
        	else ptr += 2;  // fallback 
        } else {
            StringLength += ASCIIFont;
            ptr++;
        }
    }

    return StringLength;
}
/**
 * @brief 获取窗口数据情况
 * @param *int16_tdata 指针，用于存储int16_t数据
 * @param *float_tdata 指针，用于存储float数据
 * @return 浮点值为返回 WINDOW_DATA_STYLE_FLOAT ，int16_t值为返回 WINDOW_DATA_STYLE_INT ，空指针返回 WINDOW_DATA_STYLE_NONE
 */
int8_t GetWindowDataStyle(int16_t *int16_tdata,float *float_tdata){
	//保护避免访问非法内存
	if(CurrentWindow == NULL){
		return -1;
	}
	if(int16_tdata != NULL){
		return WINDOW_DATA_STYLE_INT;
	}else if(float_tdata != NULL){
		return WINDOW_DATA_STYLE_FLOAT;
	}else{
		return WINDOW_DATA_STYLE_NONE;
	}
}

/**
 * @brief 绘制窗口
 * @param 无
 * @return 无
 */
void OLED_DrawWindow(void){
	//如果窗口标志位没有被置位，则不进行窗口效果，将窗口的目标位置置位屏幕外部
	if(OLED_SustainCounter.SustainFlag == false){
		OLED_UI_Window.TargetArea.Height = 30;
		OLED_UI_Window.TargetArea.Width = 60;
		OLED_UI_Window.TargetArea.X = (OLED_WIDTH - 60)/2;	
		OLED_UI_Window.TargetArea.Y = -40;
	}
	if(CurrentWindow == NULL){
		return;
	}
	OLED_Font ChineseFont = GetOLED_Font(CurrentWindow->Text_FontSize,CHINESE),ASCIIFont = GetOLED_Font(CurrentWindow->Text_FontSize,ASCII);

	

	//如果窗口动画还没有退出屏幕，则绘制窗口
	if(OLED_SustainCounter.SustainFlag == true || (OLED_UI_Window.CurrentArea.Height != 30 && OLED_UI_Window.CurrentArea.Width != 60 && OLED_UI_Window.CurrentArea.Y != -40)){
		//通过宏判断是否需要绘制圆角矩形
		if(CurrentWindow->General_WindowType == WINDOW_RECTANGLE){
			OLED_DrawRectangle(OLED_UI_Window.CurrentArea.X - 1,OLED_UI_Window.CurrentArea.Y - 1,OLED_UI_Window.CurrentArea.Width + 2,OLED_UI_Window.CurrentArea.Height + 2,OLED_UNFILLED);
			OLED_ClearArea(OLED_UI_Window.CurrentArea.X,OLED_UI_Window.CurrentArea.Y,OLED_UI_Window.CurrentArea.Width,OLED_UI_Window.CurrentArea.Height);
		}else{
			OLED_DrawRoundedRectangle(OLED_UI_Window.CurrentArea.X - 1,OLED_UI_Window.CurrentArea.Y - 1,OLED_UI_Window.CurrentArea.Width + 2,OLED_UI_Window.CurrentArea.Height + 2,2,OLED_UNFILLED);
			OLED_ClearArea(OLED_UI_Window.CurrentArea.X,OLED_UI_Window.CurrentArea.Y,OLED_UI_Window.CurrentArea.Width,OLED_UI_Window.CurrentArea.Height);
		}
		//显示文字
		/**
		 * 限制显示的区域：
		 * X：当前窗口区域的起始点X + 此窗口设置的字体左侧间距
		 * Y： 当前窗口区域的起始点Y + 字体顶部间距
		 * Width：当前窗口宽度减去 2*文字边距 减去 数据的宽度 减去数据和文字的距离WINDOW_DATA_TEXT_DISTANCE
		 * Height：字符串高度
		 * 显示文字的起始点是：
		 * X： 当前窗口区域的起始点X + 此窗口设置的字体左侧间距
		 * Y： 当前窗口区域的起始点Y + 字体顶部间距
		 * Width：字符串宽度
		 * Height：字符串高度
		 * */
        // 先计算不包含数据宽度的字符串宽度作为默认宽度
		int16_t MaxLength  = OLED_UI_Window.CurrentArea.Width - 2*CurrentWindow->Text_FontSideDistance - WINDOW_DATA_TEXT_DISTANCE;
		int16_t DataLength = 0;
		//计算数据类型
		int8_t DataStyle = GetWindowDataStyle(CurrentWindow->Prob_Data_Int,CurrentWindow->Prob_Data_Float);
		//如果进度条数据不为空，则说明用户设置了进度条，则显示进度条
		//如果数据指针不为空
		if(DataStyle != WINDOW_DATA_STYLE_NONE ){
			//如果数据类型为int16_t
			if (DataStyle == WINDOW_DATA_STYLE_INT)
			{
				//计算字符串被限制的最大宽度
				MaxLength = OLED_UI_Window.CurrentArea.Width - 2*CurrentWindow->Text_FontSideDistance - CalcStringWidth(ChineseFont,ASCIIFont,"%3d",*CurrentWindow->Prob_Data_Int)- WINDOW_DATA_TEXT_DISTANCE;
				//计算数据所占的宽度
				DataLength = CalcStringWidth(ChineseFont,ASCIIFont,"%3d",*CurrentWindow->Prob_Data_Int);
				//显示数据
				OLED_PrintfMixArea(
					//区域限制
					OLED_UI_Window.CurrentArea.X,
					OLED_UI_Window.CurrentArea.Y,
					OLED_UI_Window.CurrentArea.Width,
					OLED_UI_Window.CurrentArea.Height,
					OLED_UI_Window.CurrentArea.X + CurrentWindow->General_Width - 1 - CurrentWindow->Text_FontSideDistance - DataLength,
					OLED_UI_Window.CurrentArea.Y + CurrentWindow->Text_FontTopDistance,
					ChineseFont,ASCIIFont,
					"%3d",*CurrentWindow->Prob_Data_Int);

			}else{//否则默认认为是浮点型数据
			
				//计算字符串被限制的最大宽度
				MaxLength = OLED_UI_Window.CurrentArea.Width - 2*CurrentWindow->Text_FontSideDistance - CalcStringWidth(ChineseFont,	ASCIIFont,	"%5.2f",*CurrentWindow->Prob_Data_Float) - WINDOW_DATA_TEXT_DISTANCE;
				//计算数据所占的宽度
				DataLength = CalcStringWidth(ChineseFont,ASCIIFont,"%5.2f",*CurrentWindow->Prob_Data_Float);
				//显示数据
				OLED_PrintfMixArea(
					//区域限制
					OLED_UI_Window.CurrentArea.X,
					OLED_UI_Window.CurrentArea.Y,
					OLED_UI_Window.CurrentArea.Width,
					OLED_UI_Window.CurrentArea.Height,
					OLED_UI_Window.CurrentArea.X + CurrentWindow->General_Width - 1 - CurrentWindow->Text_FontSideDistance - DataLength,
					OLED_UI_Window.CurrentArea.Y + CurrentWindow->Text_FontTopDistance,
					ChineseFont,ASCIIFont,
					"%5.2f",*CurrentWindow->Prob_Data_Float);
			}
			

			//显示进度条（外框）
			OLED_DrawRoundedRectangle(OLED_UI_Window.CurrentArea.X +CurrentWindow->Prob_SideDistance,
			OLED_UI_Window.CurrentArea.Y + OLED_UI_Window.CurrentArea.Height  - CurrentWindow->Prob_LineHeight - CurrentWindow->Prob_BottomDistance,
			OLED_UI_Window.CurrentArea.Width- 2*CurrentWindow->Prob_SideDistance,CurrentWindow->Prob_LineHeight,2,OLED_UNFILLED);
			//显示进度条（内部）
			OLED_DrawRectangle(OLED_UI_Window.CurrentArea.X +CurrentWindow->Prob_SideDistance + 2,
							OLED_UI_Window.CurrentArea.Y + OLED_UI_Window.CurrentArea.Height  - CurrentWindow->Prob_LineHeight - CurrentWindow->Prob_BottomDistance + 2,
							OLED_UI_ProbWidth.CurrentDistance>=OLED_UI_Window.CurrentArea.Width- 2*CurrentWindow->Prob_SideDistance - 4  ?OLED_UI_Window.CurrentArea.Width- 2*CurrentWindow->Prob_SideDistance - 4: OLED_UI_ProbWidth.CurrentDistance  ,CurrentWindow->Prob_LineHeight-4,OLED_FILLED);
		}
		if(CurrentWindow->Text_String != NULL){
			int16_t WindowTextStringLength = CalcStringWidth(ChineseFont,ASCIIFont,CurrentWindow->Text_String);
			//如果字符串的宽度超过了最大限定宽度
			if(WindowTextStringLength > MaxLength){
#if IF_WAIT_ANIMATION_FINISH
                if( OLED_UI_Window.CurrentArea.X == OLED_UI_Window.TargetArea.X &&
                        OLED_UI_Window.CurrentArea.Y == OLED_UI_Window.TargetArea.Y &&
                        OLED_UI_Window.CurrentArea.Width == OLED_UI_Window.TargetArea.Width &&
                        OLED_UI_Window.CurrentArea.Height == OLED_UI_Window.TargetArea.Height){
#endif
		    	CurrentWindow->_LineSlip-= (float)LINE_SLIP_SPEED;
#if IF_WAIT_ANIMATION_FINISH
                }
#endif
		}
			if(CurrentWindow->_LineSlip < -WindowTextStringLength){
		   	 	CurrentWindow->_LineSlip =  MaxLength + 1;
			}

			OLED_PrintfMixArea(
			        OLED_UI_Window.CurrentArea.X + CurrentWindow->Text_FontSideDistance,
			        OLED_UI_Window.CurrentArea.Y + CurrentWindow->Text_FontTopDistance,
			        MaxLength,
			        OLED_UI_Window.CurrentArea.Height,
					OLED_UI_Window.CurrentArea.X + CurrentWindow->Text_FontSideDistance + CurrentWindow->_LineSlip,
					OLED_UI_Window.CurrentArea.Y + CurrentWindow->Text_FontTopDistance,
					ChineseFont,ASCIIFont,CurrentWindow->Text_String);
			}

	}else{
		// 如果窗口动画已经退出屏幕，则将当前窗口指针指向NULL
		CurrentWindow = NULL;
	}
	
}







/**
 * @brief 将菜单整体向上移动一行
 * @param 无
 * @return 无
 */
void MenuItemsMoveUp(void){
	OLED_UI_PageStartPoint.TargetPoint.Y -= (OLED_UI_LineStep.TargetDistance + CurrentMenuPage->General_FontSize);
}
/**
 * @brief 将菜单整体向下移动一行
 * @param 无
 * @return 无
 */
void MenuItemsMoveDown(void){
	OLED_UI_PageStartPoint.TargetPoint.Y += (OLED_UI_LineStep.TargetDistance + CurrentMenuPage->General_FontSize);
}
/**
 * @brief 将菜单整体向左移动一列
 * @param 无
 * @return 无
 */
void MenuItemsMoveLeft(void){
	OLED_UI_PageStartPoint.TargetPoint.X -= (OLED_UI_LineStep.TargetDistance + CurrentMenuPage->Tiles_TileWidth);
}
/**
 * @brief 将菜单整体向右移动一列
 * @param 无
 * @return 无
 */
void MenuItemsMoveRight(void){
	OLED_UI_PageStartPoint.TargetPoint.X += (OLED_UI_LineStep.TargetDistance + CurrentMenuPage->Tiles_TileWidth);
}
/**
 * @brief 获取当前菜单页面的槽位数量
 * @param 无
 * @return 当前菜单页面的槽位数量
 */
int16_t GetCurrentMenuPageMaxSlotNum(void){
	return (int16_t)(CurrentMenuPage->List_MenuArea.Height - CurrentMenuPage->List_StartPointY+OLED_UI_LineStep.TargetDistance-1) / (OLED_UI_LineStep.TargetDistance + CurrentMenuPage->General_FontSize );
}



/**
 * @brief 设置目标光标区域
 * @param 无
 * @return 无
 */
void SetTargetCursor(void){
	//如果当前页面的类型为List类
	if(CurrentMenuPage->General_MenuType == MENU_TYPE_LIST){
		//目标光标x坐标等于 开始打印页面起始点的x坐标减1【减去1是为了确保光标覆盖到菜单项的文字】
		OLED_UI_Cursor.TargetArea.X = OLED_UI_PageStartPoint.TargetPoint.X -1;

		//目标光标y坐标等于 开始打印页面起始点的y坐标  加上  (字体高度 加 行距)乘 当前页面的ID号 减1【减去1是为了确保光标覆盖到菜单项的文字】
		OLED_UI_Cursor.TargetArea.Y = OLED_UI_PageStartPoint.TargetPoint.Y + 
		CurrentMenuPage->_ActiveMenuID * ( CurrentMenuPage->General_LineSpace + CurrentMenuPage->General_FontSize) -1 ;
		//目标光标高度等于 字体高度 加2【加2是为了确保光标覆盖到菜单项的文字】
		OLED_UI_Cursor.TargetArea.Height = CurrentMenuPage->General_FontSize + 2;
		//目标光标宽度等于 当前字符串的宽度	【加2是为了确保光标覆盖到菜单项的文字】
		//如果有行前缀
		int8_t LinePerfixWidth = 0;
		if(CurrentMenuPage->List_IfDrawLinePerfix == true){
			LinePerfixWidth = GetOLED_Font(CurrentMenuPage->General_FontSize,ASCII) + LINEPERFIX_DISTANCE;
		}
		////单选框宽度
		int8_t RadioCompensationWidth = 0;
		//如果需要绘制单选框(即BoolRadioBox不为空)
		if(CurrentMenuPage->General_MenuItems[CurrentMenuPage->_ActiveMenuID].List_BoolRadioBox != NULL){
				RadioCompensationWidth = (GetOLED_Font(CurrentMenuPage->General_FontSize,CHINESE) + 2);
		}
		else if(CurrentMenuPage->General_MenuItems[CurrentMenuPage->_ActiveMenuID].List_IntBox != NULL){
				char IntBoxValue[5]; // 用于存储IntBox的值转换后的字符串
				// 将IntBox的值转换为字符串
				sprintf(IntBoxValue, "%d", *CurrentMenuPage->General_MenuItems[CurrentMenuPage->_ActiveMenuID].List_IntBox);
				RadioCompensationWidth = (GetOLED_Font(CurrentMenuPage->General_FontSize,CHINESE) + (strlen(IntBoxValue) * 4));
		}
		else if(CurrentMenuPage->General_MenuItems[CurrentMenuPage->_ActiveMenuID].List_FloatBox != NULL){
				char FloatBoxValue[6]; // 用于存储IntBox的值转换后的字符串
				// 将IntBox的值转换为字符串
				sprintf(FloatBoxValue, "%.2f", *CurrentMenuPage->General_MenuItems[CurrentMenuPage->_ActiveMenuID].List_FloatBox);
				RadioCompensationWidth = (GetOLED_Font(CurrentMenuPage->General_FontSize,CHINESE) + (strlen(FloatBoxValue) * 4));
		}
		else{
			RadioCompensationWidth = 0;
		}
		OLED_UI_Cursor.TargetArea.Width = 
		fmin((float)CalcStringWidth(
			//字符串长度
			GetOLED_Font(CurrentMenuPage->General_FontSize,CHINESE),GetOLED_Font(CurrentMenuPage->General_FontSize,ASCII),CurrentMenuPage->General_MenuItems[CurrentMenuPage->_ActiveMenuID].General_item_text) + 2 + LinePerfixWidth ,
			//当前页面的宽度加当前页面的起始坐标减去开始打印页面起始点的坐标减去6（是滚动条宽度加一）加上行前缀的宽度
			OLED_UI_MenuFrame.CurrentArea.Width + OLED_UI_MenuFrame.CurrentArea.X - OLED_UI_PageStartPoint.CurrentPoint.X - 6 - LinePerfixWidth + LinePerfixWidth - RadioCompensationWidth) ;
	}
	//如果当前页面的类型为Tiles类
	if(CurrentMenuPage->General_MenuType == MENU_TYPE_TILES){
		//磁贴类不需要光标的显示，所以设置为0.
		// SetCursorZero();
		OLED_UI_Cursor.TargetArea.X = CurrentMenuPage->Tiles_ScreenWidth/2 - CalcStringWidth(GetOLED_Font(CurrentMenuPage->General_FontSize,CHINESE),GetOLED_Font(CurrentMenuPage->General_FontSize,ASCII),CurrentMenuPage->General_MenuItems[CurrentMenuPage->_ActiveMenuID].General_item_text)/2 - 1;
		OLED_UI_Cursor.TargetArea.Y = CurrentMenuPage->Tiles_ScreenHeight - CurrentMenuPage->General_FontSize - TILES_BOTTOM_DISTANCE - 1;
		OLED_UI_Cursor.TargetArea.Height = CurrentMenuPage->General_FontSize + 2;
		OLED_UI_Cursor.TargetArea.Width = CalcStringWidth(GetOLED_Font(CurrentMenuPage->General_FontSize,CHINESE),GetOLED_Font(CurrentMenuPage->General_FontSize,ASCII),CurrentMenuPage->General_MenuItems[CurrentMenuPage->_ActiveMenuID].General_item_text) + 2;

	}
}
/**
 * @brief 设置目标菜单边框
 * @param 无
 * @return 无
 */
void SetTargetMenuFrame(void){
	OLED_UI_MenuFrame.TargetArea.X = CurrentMenuPage->List_MenuArea.X;
	OLED_UI_MenuFrame.TargetArea.Y = CurrentMenuPage->List_MenuArea.Y;
	OLED_UI_MenuFrame.TargetArea.Width = CurrentMenuPage->List_MenuArea.Width;
	OLED_UI_MenuFrame.TargetArea.Height = CurrentMenuPage->List_MenuArea.Height;

}
/**
 * @brief 将当前页面的LineSlip 设置为0
 * @param 无
 * @return 无
 */
void SetLineSplitZero(void){
	MenuPage* page = CurrentMenuPage;
	for(MenuID i = 0; i<GetMenuItemNum(page->General_MenuItems);i++){
		page->General_MenuItems[i]._LineSlip = 0;
	}
}

/**
 * @brief 设置目标滚动条高度
 * @param 无
 * @return 无
 */
void SetTargetScrollBarHeight(void){
	if(CurrentMenuPage->General_MenuType == MENU_TYPE_LIST){
		OLED_UI_ScrollBarHeight.TargetDistance = (float)CurrentMenuPage->List_MenuArea.Height*(CurrentMenuPage->_ActiveMenuID + 1)/GetMenuItemNum(CurrentMenuPage->General_MenuItems);
	}else if(CurrentMenuPage->General_MenuType == MENU_TYPE_TILES){
		OLED_UI_ScrollBarHeight.TargetDistance = (float)(CurrentMenuPage->Tiles_ScreenWidth *(CurrentMenuPage->_ActiveMenuID + 1)/GetMenuItemNum(CurrentMenuPage->General_MenuItems));
	}
}
/**
 * @brief 设置目标进度条长度
 * @param 无
 * @return 无
 */
void SetTargetProbWidth(void){
	
	//确认数据类型
	int8_t DataStyle = GetWindowDataStyle(CurrentWindow->Prob_Data_Int,CurrentWindow->Prob_Data_Float);
	if(DataStyle != WINDOW_DATA_STYLE_NONE){
		if(DataStyle == WINDOW_DATA_STYLE_INT){
			OLED_UI_ProbWidth.TargetDistance = ((*CurrentWindow->Prob_Data_Int - CurrentWindow->Prob_MinData)/((float)CurrentWindow->Prob_MaxData - CurrentWindow->Prob_MinData ))*	(OLED_UI_Window.CurrentArea.Width- 2*CurrentWindow->Prob_SideDistance - 4);
		}else{
			OLED_UI_ProbWidth.TargetDistance = ((*CurrentWindow->Prob_Data_Float - CurrentWindow->Prob_MinData)/((float)CurrentWindow->Prob_MaxData - CurrentWindow->Prob_MinData ))*	(OLED_UI_Window.CurrentArea.Width- 2*CurrentWindow->Prob_SideDistance - 4);
		}
	}
	
}

/**
 * @brief 根据当前页面情况决定是否绘制行前缀
 * @param page 菜单页面结构体指针
 * @param id 菜单项ID号
 * @param CursorPoint 光标的坐标
 * @return 无
 */
void DrawLinePermix(MenuPage* page,MenuID i,OLED_Point* CursorPoint,OLED_Font ChineseFont,OLED_Font ASCIIFont){
	//如果需要打印行前缀
	if(page->List_IfDrawLinePerfix == true){
		char* LinePerfixSymb = "";
		//如果当前的菜单项有子菜单且没有回调函数，那么打印相应的符号
		if(CurrentMenuPage->General_MenuItems[i].General_SubMenuPage != NULL && page->General_MenuItems[i].General_callback == NULL && page->General_MenuItems[i].List_BoolRadioBox == NULL){
			LinePerfixSymb = SUBMENU_PREFIX;
		}else//如果有回调函数
		if(CurrentMenuPage->General_MenuItems[i].General_SubMenuPage == NULL && page->General_MenuItems[i].General_callback != NULL && page->General_MenuItems[i].List_BoolRadioBox == NULL){
			LinePerfixSymb = FUNCTION_PREFIX;
		}else
		if(CurrentMenuPage->General_MenuItems[i].General_SubMenuPage == NULL && page->General_MenuItems[i].General_callback == NULL && page->General_MenuItems[i].List_BoolRadioBox != NULL)
		{
			LinePerfixSymb = RADIO_PREFIX;
		}else{
			LinePerfixSymb = NONE_PREFIX;
		}

		OLED_PrintfMixArea(//在限制的区域内打印文字
					   //光标的起始x坐标
				       OLED_UI_MenuFrame.CurrentArea.X ,
					   OLED_UI_MenuFrame.CurrentArea.Y,
					   OLED_UI_MenuFrame.CurrentArea.Width - 6,
					   OLED_UI_MenuFrame.CurrentArea.Height,
					   //打印文字的坐标
					   CursorPoint->X,
					   CursorPoint->Y,
					   //打印文字的大小
					   ChineseFont,ASCIIFont,
					   //打印文字的内容
					   LinePerfixSymb);
		
	}

}

/**
 * @brief 根据传入的结构体数组指针打印菜单元素
 * @param 无
 * @return 无
 */
void PrintMenuElements(void){
	//【踩坑】将当前页面的结构体指针取出，如果不这样做，若是在for循环当中改变结构体指针，则会导致for循环出错，访问到NULL指针，发生硬件错误造成卡死
	MenuPage* page = CurrentMenuPage;
	OLED_ChangePoint TempTargetPoint = OLED_UI_PageStartPoint;
	OLED_ChangeArea TempTargetArea = OLED_UI_MenuFrame;
	
	//获取当前页面的菜单项数量
	MenuID num = GetMenuItemNum(page->General_MenuItems);
	//获取文本大小，从当前页面的字体宽度推算出ASCII字体宽度与中文字体宽度
	OLED_Font ChineseFont = GetOLED_Font(page->General_FontSize,CHINESE),ASCIIFont = GetOLED_Font(page->General_FontSize,ASCII);
	//绘制所需的坐标光标,初始化为当前菜单结构体的开始点
	OLED_Point CursorPoint = TempTargetPoint.CurrentPoint;
    
	//如果当前页面的类型为列表类
	if(page->General_MenuType == MENU_TYPE_LIST){
		//行前缀宽度
		int16_t LinePerfixWidth = 0;
		////单选框宽度
		int16_t RadioCompensationWidth = 0;

		//绘制边框【坐标减一宽度加二是为了使得外框不会挡住菜单区域，所以实际上外框不属于菜单区域】
		if(page->List_IfDrawFrame == true){
			OLED_DrawRectangle(TempTargetArea.CurrentArea.X-1,TempTargetArea.CurrentArea.Y-1,	TempTargetArea.CurrentArea.Width+2,TempTargetArea.CurrentArea.Height+2,OLED_UNFILLED);
		}
		//如果需要绘制行前缀
		if(page->List_IfDrawLinePerfix == true){
			//将行前缀宽度设置为ASCII字体宽度加LINEPERFIX_DISTANCE（与后方字符串的距离）
			LinePerfixWidth = ASCIIFont + LINEPERFIX_DISTANCE;
		}


		//打印菜单项
		for(MenuID i = 0; i<num;i++){
			
			if(CursorPoint.Y + ChineseFont <0 || CursorPoint.Y > OLED_HEIGHT){
				CursorPoint.Y += (page->General_FontSize + OLED_UI_LineStep.CurrentDistance);
				continue;
			}

			//记录此轮循环的字符串宽度
			int16_t StringLength = CalcStringWidth(ChineseFont,ASCIIFont,page->General_MenuItems[i].General_item_text);

			//根据情况绘制行前缀
			DrawLinePermix(page,i,&CursorPoint,ChineseFont,ASCIIFont);

			//如果需要绘制单选框(即BoolRadioBox不为空)
			if(page->General_MenuItems[i].List_BoolRadioBox != NULL){
				RadioCompensationWidth = (ChineseFont + 2);
				char* RadioBoxSymb = "";
				if(*page->General_MenuItems[i].List_BoolRadioBox == true){
					RadioBoxSymb = "■";
				}else{
					RadioBoxSymb = "□";
				}


				OLED_PrintfMixArea(//在限制的区域内打印文字
							   //光标的起始x坐标加入行前缀宽度，这样可以自动留出打印行前缀的空间
						       TempTargetArea.CurrentArea.X ,
							   TempTargetArea.CurrentArea.Y,
							   TempTargetArea.CurrentArea.Width - 6,
							   TempTargetArea.CurrentArea.Height,

							   //打印文字的坐标
							   CursorPoint.X + TempTargetArea.CurrentArea.Width - RadioCompensationWidth -9 ,
							   CursorPoint.Y,
							   //打印文字的大小
							   ChineseFont,ASCIIFont,
							   //打印文字的内容
							   RadioBoxSymb);
			}	
			// 如果需要显示IntBox的值(即IntBox不为空)
			else if(page->General_MenuItems[i].List_IntBox != NULL){
				char IntBoxValue[5]; // 用于存储IntBox的值转换后的字符串
				// 将IntBox的值转换为字符串
				sprintf(IntBoxValue, "%d", *page->General_MenuItems[i].List_IntBox);
				
				RadioCompensationWidth = (ChineseFont + (strlen(IntBoxValue) * 4));
							
					// 在限制的区域内打印IntBox的值
				OLED_PrintfMixArea(
						TempTargetArea.CurrentArea.X, // 光标的起始x坐标加入行前缀宽度
						TempTargetArea.CurrentArea.Y,
						TempTargetArea.CurrentArea.Width - 6,
						TempTargetArea.CurrentArea.Height,
						CursorPoint.X + TempTargetArea.CurrentArea.Width - RadioCompensationWidth -9, // 打印文字的坐标
						CursorPoint.Y,
						ChineseFont, ASCIIFont, // 打印文字的大小
						IntBoxValue // 打印IntBox的值
				);
			}
			// 如果需要显示FloatBox的值(即FloatBox不为空)
			else if (page->General_MenuItems[i].List_FloatBox != NULL) {
					char FloatBoxValue[5]; // 用于存储FloatBox的值转换后的字符串
					// 将FloatBox的值转换为字符串，保留两位小数
					sprintf(FloatBoxValue, "%.2f", *page->General_MenuItems[i].List_FloatBox);

					// 计算补偿宽度
					RadioCompensationWidth = (ChineseFont + (strlen(FloatBoxValue) * 4));

					// 在限制的区域内打印FloatBox的值
					OLED_PrintfMixArea(
							TempTargetArea.CurrentArea.X, // 光标的起始x坐标加入行前缀宽度
							TempTargetArea.CurrentArea.Y,
							TempTargetArea.CurrentArea.Width - 6,
							TempTargetArea.CurrentArea.Height,
							CursorPoint.X + TempTargetArea.CurrentArea.Width - RadioCompensationWidth - 9, // 打印文字的坐标
							CursorPoint.Y,
							ChineseFont, ASCIIFont, // 打印文字的大小
							FloatBoxValue // 打印FloatBox的值
					);
			}
			else{
					RadioCompensationWidth = 0;
			}
			

			//如果字符串的宽度超过了最大限定宽度
			if(StringLength > (TempTargetArea.TargetArea.X + TempTargetArea.TargetArea.Width - 6 - 2) - TempTargetPoint.TargetPoint.X - LinePerfixWidth - RadioCompensationWidth ){
#if IF_WAIT_ANIMATION_FINISH
			    if(TempTargetArea.TargetArea.Width == TempTargetArea.CurrentArea.Width &&
					TempTargetArea.TargetArea.X == TempTargetArea.CurrentArea.X &&
					TempTargetArea.TargetArea.Height == TempTargetArea.CurrentArea.Height &&
					TempTargetArea.TargetArea.Y == TempTargetArea.CurrentArea.Y){
#endif
						page->General_MenuItems[i]._LineSlip -= (float)LINE_SLIP_SPEED;
#if IF_WAIT_ANIMATION_FINISH
					}
#endif
				
				
				//如果page->MenuItems[i]._LineSlip小于负的字符串宽度，说明字符串已经左移到看不见
				if(page->General_MenuItems[i]._LineSlip < -StringLength){
					//将位置设在最大限定宽度的右侧刚刚好看不见的地方
					page->General_MenuItems[i]._LineSlip = (TempTargetArea.TargetArea.X + TempTargetArea.TargetArea.Width - 6) - CursorPoint.X - LinePerfixWidth;
				}


			}

			OLED_PrintfMixArea(TempTargetArea.CurrentArea.X + LinePerfixWidth + CurrentMenuPage->List_StartPointX ,
			 					TempTargetArea.CurrentArea.Y,
			 					TempTargetArea.CurrentArea.Width - 6 - LinePerfixWidth - CurrentMenuPage->List_StartPointX  -2 - RadioCompensationWidth,
			 					TempTargetArea.CurrentArea.Height,
								//坐标加上LinePerfixWidth是为了给行前缀留下空间
							   	CursorPoint.X + LinePerfixWidth + page->General_MenuItems[i]._LineSlip,
							   	CursorPoint.Y,
							   	ChineseFont,ASCIIFont,page->General_MenuItems[i].General_item_text);

			// 打印光标下移
			CursorPoint.Y += (page->General_FontSize + OLED_UI_LineStep.CurrentDistance);

		}

		//绘制滚动条与其中心线
		OLED_DrawRectangle(TempTargetArea.CurrentArea.X + TempTargetArea.CurrentArea.Width - 5,TempTargetArea.CurrentArea.Y,5,(OLED_UI_ScrollBarHeight.CurrentDistance > page->List_MenuArea.Height) ?page->List_MenuArea.Height:OLED_UI_ScrollBarHeight.CurrentDistance,OLED_FILLED);
		OLED_DrawLine(TempTargetArea.CurrentArea.X + TempTargetArea.CurrentArea.Width - 3,TempTargetArea.CurrentArea.Y,TempTargetArea.CurrentArea.X + TempTargetArea.CurrentArea.Width-3,TempTargetArea.CurrentArea.Height + TempTargetArea.CurrentArea.Y-1);
	}
	//如果当前的页面为磁贴类型
	if(page->General_MenuType == MENU_TYPE_TILES){
		
		//打印磁贴项
		for(MenuID i = 0; i<num;i++){
			if(CursorPoint.X + page->Tiles_TileWidth < 0 || CursorPoint.X > OLED_WIDTH){
				CursorPoint.X += (page->Tiles_TileWidth + OLED_UI_LineStep.CurrentDistance);
				continue;
			}
			//动画或静态icon
			if (page->_ActiveMenuID == i && page->General_MenuItems[i].Tiles_GifIcon != NULL) {
        OLED_ShowImageArea(ceil(CursorPoint.X),CursorPoint.Y,page->Tiles_TileWidth,page->Tiles_TileHeight,0,0,page->Tiles_ScreenWidth,page->Tiles_ScreenHeight,page->General_MenuItems[i].Tiles_GifIcon == NULL?UnKnown:page->General_MenuItems[i].Tiles_GifIcon[page->General_MenuItems[i].Gif_index]);
				if(tmpi==GIFICON_SLIP_SPEED) page->General_MenuItems[i].Gif_index++,tmpi=0;//播放速度
				else tmpi++;
				if(page->General_MenuItems[i].Gif_index>31) page->General_MenuItems[i].Gif_index=0;
      }else{
				//显示磁贴图标
				OLED_ShowImageArea(ceil(CursorPoint.X),CursorPoint.Y,page->Tiles_TileWidth,page->Tiles_TileHeight,0,0,page->Tiles_ScreenWidth,page->Tiles_ScreenHeight,page->General_MenuItems[i].Tiles_Icon == NULL?UnKnown:page->General_MenuItems[i].Tiles_Icon);
			}
			// 打印光标右移
			CursorPoint.X += (page->Tiles_TileWidth + OLED_UI_LineStep.CurrentDistance);
			//显示菜单项文字
			
		}
		//绘制指示框Arrow1
		OLED_ShowImageArea(page->Tiles_ScreenWidth/2 -16-2,4,36,36,0,0,page->Tiles_ScreenWidth,page->Tiles_ScreenHeight,Arrow1);
		//绘制指示箭头Arrow
		//OLED_ShowImageArea(page->Tiles_ScreenWidth/2 - 3,0,6,5,0,0,page->Tiles_ScreenWidth,page->Tiles_ScreenHeight,Arrow);


		//记录此轮循环的字符串宽度
		int16_t StringLength = CalcStringWidth(ChineseFont,ASCIIFont,page->General_MenuItems[page->_ActiveMenuID].General_item_text);
		//如果字符串的宽度大于用户所设置的屏幕宽度
		if(StringLength > page->Tiles_ScreenWidth){
#if IF_WAIT_ANIMATION_FINISH
		    if(OLED_UI_PageStartPoint.CurrentPoint.X == OLED_UI_PageStartPoint.TargetPoint.X &&
		        OLED_UI_PageStartPoint.CurrentPoint.Y == OLED_UI_PageStartPoint.TargetPoint.Y ){
#endif
		        page->General_MenuItems[page->_ActiveMenuID]._LineSlip -= (float)LINE_SLIP_SPEED;
#if IF_WAIT_ANIMATION_FINISH
		    }
#endif
		}else{
		    SetLineSplitZero();
		}
		//如果page->MenuItems[i]._LineSlip小于负的字符串宽度，说明字符串已经左移到看不见
		if(page->General_MenuItems[page->_ActiveMenuID]._LineSlip < -StringLength){
		     //将位置设在最大限定宽度的右侧刚刚好看不见的地方
		     page->General_MenuItems[page->_ActiveMenuID]._LineSlip = page->Tiles_ScreenWidth + 1;
		}

		OLED_PrintfMixArea(0,0,page->Tiles_ScreenWidth,page->Tiles_ScreenHeight,
		        StringLength > page->Tiles_ScreenWidth? 0 + page->General_MenuItems[page->_ActiveMenuID]._LineSlip : page->Tiles_ScreenWidth/2 - CalcStringWidth(ChineseFont,ASCIIFont,page->General_MenuItems[page->_ActiveMenuID].General_item_text)/2 + page->General_MenuItems[page->_ActiveMenuID]._LineSlip,
							   page->Tiles_ScreenHeight - page->General_FontSize - TILES_BOTTOM_DISTANCE,
							   ChineseFont,ASCIIFont,
							   page->General_MenuItems[page->_ActiveMenuID].General_item_text);
		//绘制滚动条与其中心线
		int16_t ScrollBarHeight = (page->Tiles_ScreenHeight >= 128? 5:3);
		OLED_DrawRectangle(0,TILES_STARTPOINT_Y + page->Tiles_TileHeight + TILES_SCROLLBAR_Y,
						OLED_UI_ScrollBarHeight.CurrentDistance,ScrollBarHeight,OLED_FILLED);
		OLED_DrawLine(0,TILES_STARTPOINT_Y + page->Tiles_TileHeight + TILES_SCROLLBAR_Y + ScrollBarHeight/2,page->Tiles_ScreenWidth-1,TILES_STARTPOINT_Y + page->Tiles_TileHeight + TILES_SCROLLBAR_Y + ScrollBarHeight/2);

		
		
	}
	if(page->General_ShowAuxiliaryFunction != NULL){
		//绘制辅助功能
		page->General_ShowAuxiliaryFunction();
	}
	
}



/**
 * @brief 按键与编码器记录函数
 * @param 无
 * @return 上一轮与这一轮的_ActiveMenuID差值
 * @note 实际上_ActiveMenuID的值不变。
  */
MenuID_Type OLED_KeyAndEncoderRecord(void){
    static bool IfUpTapAndHold = false;
    static bool IfDownTapAndHold = false;
	//记录上一轮按键状态
    OLED_UI_LastKey.Up=OLED_UI_Key.Up;
	OLED_UI_LastKey.Down=OLED_UI_Key.Down;
	OLED_UI_LastKey.Enter=OLED_UI_Key.Enter;
	OLED_UI_LastKey.Back=OLED_UI_Key.Back;
    
    //记录当前按键状态
	OLED_UI_Key.Enter = Key_GetEnterStatus();
	OLED_UI_Key.Back = Key_GetBackStatus();
	OLED_UI_Key.Up = Key_GetUpStatus();
	OLED_UI_Key.Down = Key_GetDownStatus();
#if IF_START_UP_AND_DOWN_LONG_PRESS

	static int16_t UpPressCounter = 0;
	static int16_t UpPressedCounter  = 0;
	static int16_t DownPressCounter = 0;
	static int16_t DownPressedCounter = 0;
	//如果上一次和这一次都按下的状态持续
	if(OLED_UI_LastKey.Up == 0 && OLED_UI_Key.Up == 0){
	    UpPressCounter++;
	    if(UpPressCounter >= (float)PRESS_TIME * 50){
	        UpPressedCounter ++;
	        if(UpPressedCounter >= (UpPressCounter >= (PRESS_TIME + CONTINUE_PRESS_TIME) * 50? CONTINUE_PRESS_MOVE_SPEED : PRESS_MOVE_SPEED)){
	            IfUpTapAndHold = true;
	            UpPressedCounter = 0;
	            IfUpTapAndHold = true;
	        }else{
	            IfUpTapAndHold = false;
	        }
	    }

	}else{
	    UpPressCounter = 0;
	    UpPressedCounter = 0;
	    IfUpTapAndHold = false;
	}
	//如果上一次和这一次都按下的状态持续
	    if(OLED_UI_LastKey.Down == 0 && OLED_UI_Key.Down == 0){
	        DownPressCounter++;

	        if(DownPressCounter >= (float)PRESS_TIME * 50){
	            DownPressedCounter ++;
	            if(DownPressedCounter >= (DownPressCounter >= (PRESS_TIME + CONTINUE_PRESS_TIME) * 50? CONTINUE_PRESS_MOVE_SPEED : PRESS_MOVE_SPEED)){
	                IfDownTapAndHold = true;
	                DownPressedCounter = 0;
	                IfDownTapAndHold = true;
	            }else{
	                IfDownTapAndHold = false;
	            }
	        }

	    }else{
	        DownPressCounter = 0;
	        DownPressedCounter = 0;
	        IfDownTapAndHold = false;
	    }

#endif
	if(OLED_UI_Key.Up == OLED_UI_Key.Down){
	    IfUpTapAndHold = false;
	    IfDownTapAndHold = false;

	}

	//获取当前页面的菜单项数量
	int16_t MaxID = GetMenuItemNum(CurrentMenuPage->General_MenuItems);
	MenuID_Type IncreaseID = {0,0};
	int16_t ActiveMenuID = CurrentMenuPage->_ActiveMenuID;
	int16_t LastActiveID = ActiveMenuID;
	//获取编码器状态
	ActiveMenuID += Encoder_Get();

	//如果检测到【上】按键的上一状态与这次的状态不同，且这一状态是抬起状态，说明用户按下了【上】按键，并且刚刚才抬起
	if((OLED_UI_Key.Up != OLED_UI_LastKey.Up && OLED_UI_Key.Up == 1) || IfUpTapAndHold){
		ActiveMenuID--;
	}

	//如果检测到【下】按键的上一状态与这次的状态不同，且这一状态是抬起状态，说明用户按下了【下】按键，并且刚刚才抬起
	if((OLED_UI_Key.Down != OLED_UI_LastKey.Down && OLED_UI_Key.Down == 1) || IfDownTapAndHold){
		ActiveMenuID++;
	}
	
	IncreaseID.Unsafe = ActiveMenuID - LastActiveID;
	//如果当前菜单项ID号越界，则将其限制在0~MaxID-1之间
	if(ActiveMenuID > MaxID-1){
		//ActiveMenuID = MaxID-1;
		ActiveMenuID = 0;
	}
	if(ActiveMenuID < 0){
		//ActiveMenuID = 0;
		ActiveMenuID = MaxID-1;
	}

	IncreaseID.Safe = ActiveMenuID - LastActiveID;
	return IncreaseID;
}
/**
 * @brief 将KeyEnterFlag置FLAGSTART，用于标记回调函数正在执行
 * @param 无
 * @return 无
 */
void SetEnterFlag(void){
	KeyEnterFlag = FLAGSTART;
	Encoder_Disable();  // 失能编码器
}

/**
 * @brief 将KeyEnterFlag置FLAGEND，用于表示回调函数没有正在执行
 * @param void
 * @return 无
 */
void ResetEnterFlag(void){
	KeyEnterFlag = FLAGEND;
}


/**
 * @brief 将FadeOutFlag置FLAGSTART，用于标记渐隐效果正在执行
 * @param 无
 * @return 无
 */
void SetFadeOutFlag(MutexFlag action){
	FadeOutFlag = action;
	Encoder_Disable();  // 失能编码器
}

/**
 * @brief 将FadeOutFlag置FLAGEND，用于表示渐隐效果没有正在执行
 * @param void
 * @return 无
 */
void ResetFadeOutFlag(void){
	FadeOutFlag = FLAGEND;
}

/**
 * @brief 此函数处理按下Enter按键后的情况，分为以下几种情况：
 * 		1. 当前菜单项有回调函数，那么就将KeyEnterFlag置位。
 * 		2. 当前菜单项没有回调函数，但是有子菜单，那么就进入子菜单
 * 		3. 当前菜单项没有回调函数，也没有子菜单，那么就什么都不做
 * 		4. 当前菜单项既有回调函数，又有子菜单，那么就什么也不做
 * @param 无
 * @return 无
 */
void EnterEventMenuItem(void){
    //如果当前选中菜单项有回调函数并且没有子菜单，那么就将KeyEnterFlag置位
    if(CurrentMenuPage->General_MenuItems[CurrentMenuPage->_ActiveMenuID].General_callback != NULL && CurrentMenuPage->General_MenuItems[CurrentMenuPage->_ActiveMenuID].General_SubMenuPage == NULL){

        SetEnterFlag();
    }
	//如果当前选中菜单项没有回调函数，但是有子菜单，那么置确认标志位
    if(CurrentMenuPage->General_MenuItems[CurrentMenuPage->_ActiveMenuID].General_SubMenuPage != NULL 
	&& CurrentMenuPage->General_MenuItems[CurrentMenuPage->_ActiveMenuID].General_callback == NULL){
		SetFadeOutFlag(ENTER_FLAGSTART);
    }

}
/**
 * @brief 按下【返回】按键后的操作
 * @param 无
 * @return 无
 */
void BackEventMenuItem(void){
	//如果当前菜单的父菜单不为空
	if(CurrentMenuPage->General_ParentMenuPage != NULL){
		//置返回标置位
        SetFadeOutFlag(BACK_FLAGSTART);
    }
}
/**
 * @brief 返回函数
 * @param 无
 * @note 模拟按下【返回】按键
 * @return 无
 */
void OLED_UI_Back(void){
	BackEventMenuItem();
}

/**
 * @brief 进入并运行回调函数
 * @param 无
 * @return 无
 * @note 在OLED_UI的主循环函数中调用
 */
void RunCurrentCallBackFunction(void){
	//KeyEnterFlag == FLAGSTART这个条件表示enter被触发
	if(KeyEnterFlag == FLAGSTART){
		//检查回调函数指针是否为空，如果不为空，则执行回调函数
		if (CurrentMenuPage->General_MenuItems[CurrentMenuPage->_ActiveMenuID].General_callback != NULL){
			//执行回调函数
			CurrentMenuPage->General_MenuItems[CurrentMenuPage->_ActiveMenuID].General_callback();	
		}
		//在回调函数执行完毕之后，将KeyEnterFlag复位。
		ResetEnterFlag();
		Encoder_Enable();  // 使能编码器
	}
}
/**
 * @brief 将当前光标区域与目标光标区域都设置为0
 * @param 无
 * @return 无
 */
void SetCursorZero(void){
	OLED_UI_Cursor.TargetArea.X = 0;
	OLED_UI_Cursor.TargetArea.Y = 0;
	OLED_UI_Cursor.TargetArea.Width = 0;
	OLED_UI_Cursor.TargetArea.Height = 0;
	OLED_UI_Cursor.CurrentArea.X = 0;
	OLED_UI_Cursor.CurrentArea.Y = 0;
	OLED_UI_Cursor.CurrentArea.Width = 0;
	OLED_UI_Cursor.CurrentArea.Height = 0;
}
/**
 * @brief 设置窗口大小
 * @param Width 窗口宽度
 * @param Height 窗口高度
 * @return 无
 */
void OLED_UI_CreateWindow(MenuWindow* window){
	//将窗口标志位设置为true，表示开始窗口效果
	OLED_SustainCounter.SustainFlag = true;
	//将当前进度条长度置零
	OLED_UI_ProbWidth.CurrentDistance = 0;
	//重置窗口效果时间计数器
	OLED_SustainCounter.count = 0;
	//如果窗口标志位被置位，则设置目标窗口的位置到屏幕中央
	
	//设置目标窗口的位置为屏幕中央居中显示
	OLED_UI_Window.TargetArea.Width = window->General_Width;
	OLED_UI_Window.TargetArea.Height = window->General_Height;
	OLED_UI_Window.TargetArea.X = (OLED_WIDTH - window->General_Width)/2;
	OLED_UI_Window.TargetArea.Y = (OLED_HEIGHT - window->General_Height)/2;
	window->_LineSlip = 0;
	//将当前窗口指针指向window
	CurrentWindow = window;
	
}




/**
 * @brief 运行渐隐效果
 * @param 无
 * @return 无
 */
void RunFadeOut(void){

	static uint8_t FadeOut_Seq;
	static uint32_t FadeOut_Seq_StartTick;
	static int16_t FadeOut_x0, FadeOut_y0, FadeOut_width, FadeOut_height;

	/*如果当前的FadeOutFlag已经被置位，则说明正在运行渐隐效果。
	当前在运行渐隐效果的前提条件有2个：
	1.【在按下确认键的情况下】【如果当前选中菜单项没有回调函数，但是有子菜单】，此时 FadeOutFlag == ENTER_FLAGSTART
	2.【在按下返回键的情况下】【如果当前菜单的父菜单不为空】，此时 FadeOutFlag == BACK_FLAGSTART
	*/
	if(FadeOutFlag != FLAGEND){
		if (FadeOut_Seq != 0){	//如果当前不是步骤0
			if ((FadeOut_Seq_StartTick + FADEOUT_TIME) < HAL_GetTick()){	//计时FADEOUT_TIME毫秒
				FadeOut_Seq++;
				FadeOut_Seq_StartTick = HAL_GetTick();	//记录每一步的开始时间
			}
		}
		if (FadeOut_Seq == 0){	//步骤0：计算效果参数
			//如果当前菜单是列表类
			if(CurrentMenuPage->General_MenuType == MENU_TYPE_LIST){
				//当前菜单项的页面类型是列表类的情况下，按下了确认按键
				if(FadeOutFlag == ENTER_FLAGSTART){
					//（在有子菜单的情况下）如果当前页面的当前子菜单项的页面类型是列表类
					if(CurrentMenuPage->General_MenuItems[CurrentMenuPage->_ActiveMenuID].General_SubMenuPage->General_MenuType == MENU_TYPE_LIST){
						//只清除当前页面区域
						//OLED_UI_FadeoutCurrentArea(CurrentMenuPage->List_MenuArea.X,CurrentMenuPage->List_MenuArea.Y,CurrentMenuPage->List_MenuArea.Width-5,CurrentMenuPage->List_MenuArea.Height-2);
						FadeOut_x0 = CurrentMenuPage->List_MenuArea.X;
						FadeOut_y0 = CurrentMenuPage->List_MenuArea.Y;
						FadeOut_width = CurrentMenuPage->List_MenuArea.Width-5;
						FadeOut_height = CurrentMenuPage->List_MenuArea.Height-2;
					}else{
						//清除全部区域
						//OLED_UI_FadeoutAllArea();
						FadeOut_x0 = 0;
						FadeOut_y0 = 0;
						FadeOut_width = OLED_WIDTH;
						FadeOut_height = OLED_HEIGHT;
					}
				}
				//当前菜单项的页面类型是列表类的情况下，按下了取消按键
				if(FadeOutFlag == BACK_FLAGSTART) {
					//如果当前页面的父菜单项的页面类型是列表类
					if(CurrentMenuPage->General_ParentMenuPage->General_MenuType == MENU_TYPE_LIST){
						//只清除当前页面区域
						//OLED_UI_FadeoutCurrentArea(CurrentMenuPage->List_MenuArea.X,CurrentMenuPage->List_MenuArea.Y,CurrentMenuPage->List_MenuArea.Width-5,CurrentMenuPage->List_MenuArea.Height-2);
						FadeOut_x0 = CurrentMenuPage->List_MenuArea.X;
						FadeOut_y0 = CurrentMenuPage->List_MenuArea.Y;
						FadeOut_width = CurrentMenuPage->List_MenuArea.Width-5;
						FadeOut_height = CurrentMenuPage->List_MenuArea.Height-2;

					}else{
						//清除全部区域
						//OLED_UI_FadeoutAllArea();
						FadeOut_x0 = 0;
						FadeOut_y0 = 0;
						FadeOut_width = OLED_WIDTH;
						FadeOut_height = OLED_HEIGHT;
					}
				}

			}else //如果当前菜单类型是磁贴类
			if(CurrentMenuPage->General_MenuType == MENU_TYPE_TILES){
				//清除全部区域
				//OLED_UI_FadeoutAllArea();
				FadeOut_x0 = 0;
				FadeOut_y0 = 0;
				FadeOut_width = OLED_WIDTH;
				FadeOut_height = OLED_HEIGHT;

			}
			FadeOut_Seq++;
		}else
		if(FadeOut_Seq == 6){	//步骤6：渐隐完毕，复位变量
			OLED_UI_FadeOut_Masking(FadeOut_x0, FadeOut_y0, FadeOut_width, FadeOut_height, 5);	//这一帧应与步骤5一样显示全黑
			FadeOut_Seq = 0;
			//如果当前菜单是列表类
			if(CurrentMenuPage->General_MenuType == MENU_TYPE_LIST){
				//当前菜单项的页面类型是列表类的情况下，按下了确认按键
				if(FadeOutFlag == ENTER_FLAGSTART){
					//（在有子菜单的情况下）如果当前页面的当前子菜单项的页面类型是列表类
					if(CurrentMenuPage->General_MenuItems[CurrentMenuPage->_ActiveMenuID].General_SubMenuPage->General_MenuType == MENU_TYPE_LIST){
					}else{
						//将滚动条的当前高度设为0
						OLED_UI_ScrollBarHeight.CurrentDistance = 0;
					}

					//将当前菜单的位置保存，以便返回时恢复
					CurrentMenuPage->_StartPoint.X = OLED_UI_PageStartPoint.TargetPoint.X;
					CurrentMenuPage->_StartPoint.Y = OLED_UI_PageStartPoint.TargetPoint.Y;

					//将当前菜单的指针指向子菜单
					CurrentMenuPage = CurrentMenuPage->General_MenuItems[CurrentMenuPage->_ActiveMenuID].General_SubMenuPage;
					//对当前子菜单进行初始化
					CurrentMenuPageInit();
				}
				//当前菜单项的页面类型是列表类的情况下，按下了取消按键
				if(FadeOutFlag == BACK_FLAGSTART) {
					//如果当前页面的父菜单项的页面类型是列表类
					if(CurrentMenuPage->General_ParentMenuPage->General_MenuType == MENU_TYPE_LIST){
					}else{
						//将滚动条的当前高度设为0
						OLED_UI_ScrollBarHeight.CurrentDistance = 0;
					}
					//将当前菜单的指针指向父菜单
					CurrentMenuPage = CurrentMenuPage->General_ParentMenuPage;
					//将当前菜单的位置等参数恢复
					CurrentMenuPageBackUp();
				}

				//将FadeOutFlag复位
				ResetFadeOutFlag();
				//将当前光标区域与目标光标区域都设置为0
				SetCursorZero();

			}else //如果当前菜单类型是磁贴类
			if(CurrentMenuPage->General_MenuType == MENU_TYPE_TILES){
				//当前菜单项的页面类型是磁贴类的情况下，按下了确认操作
				if(FadeOutFlag == ENTER_FLAGSTART){
					//将当前菜单的位置保存，以便返回时恢复
					CurrentMenuPage->_StartPoint.X = OLED_UI_PageStartPoint.TargetPoint.X;
					CurrentMenuPage->_StartPoint.Y = OLED_UI_PageStartPoint.TargetPoint.Y;
					//将当前菜单的指针指向子菜单
					CurrentMenuPage = CurrentMenuPage->General_MenuItems[CurrentMenuPage->_ActiveMenuID].General_SubMenuPage;
					//对当前子菜单进行初始化
					CurrentMenuPageInit();

				}
				//当前菜单项的页面类型是磁贴类的情况下，按下了返回操作
				if(FadeOutFlag == BACK_FLAGSTART){
					//将当前菜单的指针指向父菜单
					CurrentMenuPage = CurrentMenuPage->General_ParentMenuPage;
					//将当前菜单的位置等参数恢复
					CurrentMenuPageBackUp();
				}
				//将滚动条的当前高度设为0
				OLED_UI_ScrollBarHeight.CurrentDistance = 0;
				//将当前光标区域与目标光标区域都设置为0
				SetCursorZero();
				//将FadeOutFlag复位
				ResetFadeOutFlag();
			}
			//将当前光标区域与目标光标区域都设置为0
			SetCursorZero();
			//将FadeOutFlag复位
			ResetFadeOutFlag();
			// 使能编码器
			Encoder_Enable();
		}
		else{					//步骤1-5：渐隐中
			OLED_UI_FadeOut_Masking(FadeOut_x0, FadeOut_y0, FadeOut_width, FadeOut_height, FadeOut_Seq);
		}
	}
}
/**
 * @brief 打印菜单元素并根据目标值改变元素的参数
 * @param 无
 * @return 无
 */
void MoveMenuElements(void){

	//设置目标光标区域
	SetTargetCursor();
	//设置目标菜单边框
	SetTargetMenuFrame();
	//设置目标进度条长度
	SetTargetProbWidth();
	//设置目标滚动条高度
	SetTargetScrollBarHeight();


	// 改变菜单起始元素的坐标
	ChangePoint(&OLED_UI_PageStartPoint);
	// 改变菜单项的行间距
	ChangeDistance(&OLED_UI_LineStep);
	// 改变滚动条高度
	ChangeDistance(&OLED_UI_ScrollBarHeight);
	
	

	// 改变菜单边框参数
	ChangeArea(&OLED_UI_MenuFrame);

	// 打印菜单页面的元素
	PrintMenuElements();

	//改变光标的参数
	ChangeArea(&OLED_UI_Cursor);
	//显示光标
	ReverseCoordinate(OLED_UI_Cursor.CurrentArea.X,OLED_UI_Cursor.CurrentArea.Y,OLED_UI_Cursor.CurrentArea.Width,OLED_UI_Cursor.CurrentArea.Height,CurrentMenuPage->General_CursorStyle);
	//设置颜色模式
	OLED_SetColorMode(ColorMode);

	OLED_Brightness(OLED_UI_Brightness);

	ChangeDistance(&OLED_UI_ProbWidth);
	// 改变窗口参数
	ChangeArea(&OLED_UI_Window);
	// 绘制窗口
	OLED_DrawWindow();


	
	
}

/**
 * @brief OLED_UI的主循环函数
 * @param 无
 * @note 该函数需要放在主循环中调用，以便实现UI的刷新
 * @return 无
 */
void OLED_UI_MainLoop(void){

	

	//清屏
	OLED_Clear();

	

	//移动菜单元素
	MoveMenuElements();

	
	//当互斥锁被置位时，运行当前菜单项的回调函数
	RunCurrentCallBackFunction();
	
	//当渐隐互斥锁被置位时，运行渐隐效果
	RunFadeOut();

	//显示FPS
	OLED_UI_ShowFPS();
	//刷屏
	OLED_Update();
	
}




/**
 * @brief  OLED_UI的中断函数，内部包含需在中断内处理的任务
 * @param  无
 * @return 无
 */
void OLED_UI_InterruptHandler(void){
	// 获取当前屏幕刷新率
    GetFPS();
    
	// 如果当前有正在执行的回调函数，则不处理中断内的任务
    if(GetEnterFlag() && GetFadeoutFlag()){
    	

		//获取_ActiveMenuID的变化值，_ActiveMenuID的值不变，并记录了按键的变化
		MenuID_Type IncreaseID = OLED_KeyAndEncoderRecord();


		//如果窗口停留的标志位为true，说明当前正在运行窗口
		if(OLED_SustainCounter.SustainFlag == true){
			//如果编码器或是按键的变化值不是0
			if(IncreaseID.Unsafe != 0){
				//窗口计数值清零
				OLED_SustainCounter.count = 0;
			}
			//如果窗口有数据
			int8_t DataStyle = GetWindowDataStyle(CurrentWindow->Prob_Data_Int,CurrentWindow->Prob_Data_Float);
			if(DataStyle != WINDOW_DATA_STYLE_NONE){
				if(DataStyle == WINDOW_DATA_STYLE_INT){
					*CurrentWindow->Prob_Data_Int += (IncreaseID.Unsafe * CurrentWindow->Prob_DataStep);
					if(*CurrentWindow->Prob_Data_Int < CurrentWindow->Prob_MinData) {*CurrentWindow->Prob_Data_Int = CurrentWindow->Prob_MinData;}
					if(*CurrentWindow->Prob_Data_Int > CurrentWindow->Prob_MaxData) {*CurrentWindow->Prob_Data_Int = CurrentWindow->Prob_MaxData;}
				}else{
					*CurrentWindow->Prob_Data_Float += (IncreaseID.Unsafe * CurrentWindow->Prob_DataStep);
					if(*CurrentWindow->Prob_Data_Float < CurrentWindow->Prob_MinData) {*CurrentWindow->Prob_Data_Float = CurrentWindow->Prob_MinData;}
					if(*CurrentWindow->Prob_Data_Float > CurrentWindow->Prob_MaxData) {*CurrentWindow->Prob_Data_Float = CurrentWindow->Prob_MaxData;}
				}
			}
			IncreaseID.Safe = 0;
		}
		 
		//如果变化值小于0，那么相当于按下IncreaseID.Safe次【上】按键
		if(IncreaseID.Safe < 0 ){
			for(int i = 0; i < -IncreaseID.Safe; i++){
				//如果当前菜单类型是列表类
				if(CurrentMenuPage->General_MenuType == MENU_TYPE_LIST){
					/*********************按下【上】按键进行的操作*************************/
					//如果当前菜单页面的光标已经到达最顶部的槽位,并且当前菜单项不是第一个菜单项，那么就向下移动菜单项的目标位置
           			if(CurrentMenuPage->_Slot == 0 && CurrentMenuPage->_ActiveMenuID !=0){
						MenuItemsMoveDown();

					}
					//如果光标还没有到达最顶部的槽位，那么就向上移动槽位
					if(CurrentMenuPage->_Slot > 0){
						CurrentMenuPage->_Slot--;
					}
					CurrentMenuPage->_ActiveMenuID--;
				}
				//如果当前菜单类型是列表类
				if(CurrentMenuPage->General_MenuType == MENU_TYPE_TILES){
					CurrentMenuPage->_ActiveMenuID--;
					MenuItemsMoveRight();
				}
			}
		}
		if(IncreaseID.Safe > 0){
			for(int i = 0; i < IncreaseID.Safe; i++){
				/*********************按下【下】按键进行的操作*************************/
					if(CurrentMenuPage->General_MenuType == MENU_TYPE_LIST){
					//如果当前菜单页面的光标已经到达最底部的槽位,并且当前菜单项不是最后一个菜单项，那么就向上移动菜单项的目标位置
					if(CurrentMenuPage->_Slot == GetCurrentMenuPageMaxSlotNum()-1 && CurrentMenuPage->_ActiveMenuID != GetMenuItemNum(CurrentMenuPage->General_MenuItems)){
						MenuItemsMoveUp();

					}
					//如果光标还没有到达最底部的槽位，那么就向下移动槽位
					if(CurrentMenuPage->_Slot < GetCurrentMenuPageMaxSlotNum()-1){
						CurrentMenuPage->_Slot++;
					}
					CurrentMenuPage->_ActiveMenuID++;
				}
				//如果当前菜单类型是列表类
				if(CurrentMenuPage->General_MenuType == MENU_TYPE_TILES){
					CurrentMenuPage->_ActiveMenuID++;
					MenuItemsMoveLeft();
				}

			}
		}
		
	
    	
		//如果检测到【返回】按键的上一状态与这次的状态不同，且这一状态是抬起状态，说明用户按下了【返回】按键，并且刚刚才抬起
		if(OLED_UI_Key.Back != OLED_UI_LastKey.Back && OLED_UI_Key.Back == 1){
			//如果当前没有运行窗口
			if(OLED_SustainCounter.SustainFlag == false){
				BackEventMenuItem();
			}else{
				OLED_SustainCounter.count = (int16_t)(CurrentWindow->General_ContinueTime * 50);
			}
			
		}
		//如果检测到【确认】按键的上一状态与这次的状态不同，且这一状态是抬起状态，说明用户按下了【确认】按键，并且刚刚才抬起
		if(OLED_UI_Key.Enter != OLED_UI_LastKey.Enter && OLED_UI_Key.Enter == 1){
			if(OLED_SustainCounter.SustainFlag == false){
				EnterEventMenuItem();
				if (CurrentMenuPage->General_MenuItems[CurrentMenuPage->_ActiveMenuID].List_BoolRadioBox != NULL) {
				    *CurrentMenuPage->General_MenuItems[CurrentMenuPage->_ActiveMenuID].List_BoolRadioBox = !(*CurrentMenuPage->General_MenuItems[CurrentMenuPage->_ActiveMenuID].List_BoolRadioBox);
				}
			}else{
				OLED_SustainCounter.count = 0;
			}
		}
		
	}

	//如果当前正在运行窗口，那么计数
	if(OLED_SustainCounter.SustainFlag == true){
		OLED_SustainCounter.count++;
	}
	if(CurrentWindow != NULL){
		if(OLED_SustainCounter.count >= (int16_t)(CurrentWindow->General_ContinueTime * 50)){
			OLED_SustainCounter.SustainFlag = false;
			OLED_SustainCounter.count = 0;
		}
	}
}
#endif


