/**
 * OLED_UI_MenuData.c
 * Menu data definitions — all menu pages, items, window descriptors, and callback
 * functions that define the UI structure. This is the single place to edit menus.
 */
#include "OLED_UI_MenuData.h"
#include "OLED_UI.h"
#include "mid_ui_bridge.h"
#include "stdio.h"
/*此文件用于存放菜单数据。实际上菜单数据可以存放在任何地方，存放于此处是为了规范与代码模块化*/

// ColorMode 是一个在OLED_UI当中定义的bool类型变量，用于控制OLED显示的颜色模式， DARKMODE 为深色模式， LIGHTMOOD 为浅色模式。这里将其引出是为了创建单选框菜单项。
extern bool ColorMode;
extern bool OLED_UI_ShowFps;
// OLED_UI_Brightness 是一个在OLED_UI当中定义的int16_t类型变量，用于控制OLED显示的亮度。这里将其引出是为了创建调整亮度的滑动条窗口，范围0-255。
extern int16_t OLED_UI_Brightness;
float testfloatnum = 0.5;
int32_t testintnum = 1;
#define SPEED 10

//关于窗口的结构体
MenuWindow SetBrightnessWindow = {
	.General_Width = 80,								//窗口宽度
	.General_Height = 28, 							//窗口高度
	.Text_String = "屏幕亮度",					//窗口标题
	.Text_FontSize = OLED_UI_FONT_12,				//字高
	.Text_FontSideDistance = 4,							//字体距离左侧的距离
	.Text_FontTopDistance = 3,							//字体距离顶部的距离
	.General_WindowType = WINDOW_ROUNDRECTANGLE, 	//窗口类型
	.General_ContinueTime = 4.0,						//窗口持续时间

	.Prob_Data_Int_16 = &OLED_UI_Brightness,				//显示的变量地址
	.Prob_DataStep = 5,								//步长
	.Prob_MinData = 5,									//最小值
	.Prob_MaxData = 255, 								//最大值
	.Prob_BottomDistance = 3,							//底部间距
	.Prob_LineHeight = 8,								//进度条高度
	.Prob_SideDistance = 4,								//边距
};
MenuWindow SetBuzzerVolumeWindow = {
	.General_Width = 80,								//窗口宽度
	.General_Height = 28, 							//窗口高度
	.Text_String = "蜂鸣器音量",					//窗口标题
	.Text_FontSize = OLED_UI_FONT_12,				//字高
	.Text_FontSideDistance = 4,							//字体距离左侧的距离
	.Text_FontTopDistance = 3,							//字体距离顶部的距离
	.General_WindowType = WINDOW_ROUNDRECTANGLE, 	//窗口类型
	.General_ContinueTime = 4.0,						//窗口持续时间

	.Prob_Data_Int_16 = &mid_beeper0.sound_loud,				//显示的变量地址
	.Prob_DataStep = 5,								//步长
	.Prob_MinData = 5,									//最小值
	.Prob_MaxData = 100, 								//最大值
	.Prob_BottomDistance = 3,							//底部间距
	.Prob_LineHeight = 8,								//进度条高度
	.Prob_SideDistance = 4,								//边距
};

MenuWindow SetGyroscopeWindow = {
	.General_Width = 128,								//窗口宽度
	.General_Height = 64, 							//窗口高度 
	.Text_String = "Gyroscope",					//窗口标题
	.Text_FontSize = OLED_UI_FONT_12,				//字高
	.Text_FontSideDistance = (128/2)-27,							//字体距离左侧的距离
	.Text_FontTopDistance = 1,							//字体距离顶部的距离
	.General_WindowType = WINDOW_ROUNDRECTANGLE, 	//窗口类型
	.General_ContinueTime = 120.0,						//窗口持续时间
};

MenuWindow SetSavedataWindow = {
	.General_Width = 80,								//窗口宽度
	.General_Height = 20, 							//窗口高度
	.Text_String = "保存成功",					//窗口标题
	.Text_FontSize = OLED_UI_FONT_12,				//字高
	.Text_FontSideDistance = 80/2-24,							//字体距离左侧的距离
	.Text_FontTopDistance = 3,							//字体距离顶部的距离
	.General_WindowType = WINDOW_ROUNDRECTANGLE, 		//窗口类型
	.General_ContinueTime = 1.0,						//窗口持续时间
};

/**
 * @brief 创建显示亮度窗口
 */
void BrightnessWindow(void){
	OLED_UI_CreateWindow(&SetBrightnessWindow);
}
/**
 * @brief 创建蜂鸣器音量窗口
 */
void BuzzerVolumeWindow(void){
	OLED_UI_CreateWindow(&SetBuzzerVolumeWindow);
}
/**
 * @brief 启动陀螺仪3D显示
 */
static const AppTaskDef app_gyroscope_app = {
	.init = app_gyroscope_init,
	.tick = app_gyroscope_tick,
	.sample = app_gyroscope_sample,
	.should_exit = app_gyroscope_should_exit,
	.on_exit = app_gyroscope_on_exit,
	.fade_tick = app_gyroscope_fade_tick,
	.fade_steps = 5,
	.frame_interval_ms = 20,
};
void GyroscopeWindow(void){
	app_task_start(&app_gyroscope_app);
}

static const AppTaskDef app_uart_monitor_app = {
	.init = app_uart_monitor_init,
	.tick = app_uart_monitor_tick,
	.sample = NULL,
	.should_exit = app_uart_monitor_should_exit,
	.on_exit = app_uart_monitor_on_exit,
	.fade_tick = app_uart_monitor_fade_tick,
	.fade_steps = 5,
	.frame_interval_ms = 30,
};
void UartMonitorStart(void){
	app_task_start(&app_uart_monitor_app);
}
/**
 * @brief 创建保存数据窗口
 */
void SavedataWindow(void){
	uint8_t temp[13];
	temp[0] = (uint8_t)OLED_UI_Brightness;
	temp[1] = mid_beeper0.sound_loud;
	temp[2] = mid_beeper0.enable;
	temp[3] = (uint8_t)ColorMode;
	temp[4] = (uint8_t)OLED_UI_ShowFps;
	temp[5] = (uint8_t)ws2812_enable;
	temp[6] = (uint8_t)ws2812_r;
	temp[7] = (uint8_t)ws2812_g;
	temp[8] = (uint8_t)ws2812_b;
	temp[9] = (uint8_t)ws2812_led_num;
	temp[10] = (uint8_t)ws2812_light_mode;
	temp[11] = (uint8_t)effect_param.speed;
	temp[12] = (uint8_t)ws2812_brightness;
	settings_save(temp);
	OLED_UI_CreateWindow(&SetSavedataWindow);
}

/* ========== RGB灯设置窗口 ========== */
MenuWindow SetRGBRedWindow = {
	.General_Width = 80, .General_Height = 28,
	.Text_String = "红色 R",
	.Text_FontSize = OLED_UI_FONT_12,
	.Text_FontSideDistance = 4, .Text_FontTopDistance = 3,
	.General_WindowType = WINDOW_ROUNDRECTANGLE,
	.General_ContinueTime = 4.0,
	.Prob_Data_Int_16 = &ws2812_r,
	.Prob_DataStep = 5, .Prob_MinData = 0, .Prob_MaxData = 255,
	.Prob_BottomDistance = 3, .Prob_LineHeight = 8, .Prob_SideDistance = 4,
};
MenuWindow SetRGBGreenWindow = {
	.General_Width = 80, .General_Height = 28,
	.Text_String = "绿色 G",
	.Text_FontSize = OLED_UI_FONT_12,
	.Text_FontSideDistance = 4, .Text_FontTopDistance = 3,
	.General_WindowType = WINDOW_ROUNDRECTANGLE,
	.General_ContinueTime = 4.0,
	.Prob_Data_Int_16 = &ws2812_g,
	.Prob_DataStep = 5, .Prob_MinData = 0, .Prob_MaxData = 255,
	.Prob_BottomDistance = 3, .Prob_LineHeight = 8, .Prob_SideDistance = 4,
};
MenuWindow SetRGBBlueWindow = {
	.General_Width = 80, .General_Height = 28,
	.Text_String = "蓝色 B",
	.Text_FontSize = OLED_UI_FONT_12,
	.Text_FontSideDistance = 4, .Text_FontTopDistance = 3,
	.General_WindowType = WINDOW_ROUNDRECTANGLE,
	.General_ContinueTime = 4.0,
	.Prob_Data_Int_16 = &ws2812_b,
	.Prob_DataStep = 5, .Prob_MinData = 0, .Prob_MaxData = 255,
	.Prob_BottomDistance = 3, .Prob_LineHeight = 8, .Prob_SideDistance = 4,
};
MenuWindow SetRGBLedNumWindow = {
	.General_Width = 80, .General_Height = 28,
	.Text_String = "灯珠数量",
	.Text_FontSize = OLED_UI_FONT_12,
	.Text_FontSideDistance = 4, .Text_FontTopDistance = 3,
	.General_WindowType = WINDOW_ROUNDRECTANGLE,
	.General_ContinueTime = 4.0,
	.Prob_Data_Int_16 = &ws2812_led_num,
	.Prob_DataStep = 1, .Prob_MinData = 0, .Prob_MaxData = 4,
	.Prob_BottomDistance = 3, .Prob_LineHeight = 8, .Prob_SideDistance = 4,
};

void RGBRedWindow(void)   { OLED_UI_CreateWindow(&SetRGBRedWindow); }
void RGBGreenWindow(void) { OLED_UI_CreateWindow(&SetRGBGreenWindow); }
void RGBBlueWindow(void)  { OLED_UI_CreateWindow(&SetRGBBlueWindow); }
void RGBLedNumWindow(void){ OLED_UI_CreateWindow(&SetRGBLedNumWindow); }

/* RGB效果速度窗口 */
MenuWindow SetRGBSpeedWindow = {
	.General_Width = 80, .General_Height = 28,
	.Text_String = "渐变速度",
	.Text_FontSize = OLED_UI_FONT_12,
	.Text_FontSideDistance = 4, .Text_FontTopDistance = 3,
	.General_WindowType = WINDOW_ROUNDRECTANGLE,
	.General_ContinueTime = 4.0,
	.Prob_Data_Int_16 = (int16_t*)&effect_param.speed,
	.Prob_DataStep = 5, .Prob_MinData = 1, .Prob_MaxData = 100,
	.Prob_BottomDistance = 3, .Prob_LineHeight = 8, .Prob_SideDistance = 4,
};

void RGBSpeedWindow(void) { OLED_UI_CreateWindow(&SetRGBSpeedWindow); }

/* RGB亮度窗口 */
MenuWindow SetRGBBrightnessWindow = {
	.General_Width = 80, .General_Height = 28,
	.Text_String = "灯光亮度",
	.Text_FontSize = OLED_UI_FONT_12,
	.Text_FontSideDistance = 4, .Text_FontTopDistance = 3,
	.General_WindowType = WINDOW_ROUNDRECTANGLE,
	.General_ContinueTime = 4.0,
	.Prob_Data_Int_16 = &ws2812_brightness,
	.Prob_DataStep = 5, .Prob_MinData = 0, .Prob_MaxData = 100,
	.Prob_BottomDistance = 3, .Prob_LineHeight = 8, .Prob_SideDistance = 4,
};

void RGBBrightnessWindow(void) { OLED_UI_CreateWindow(&SetRGBBrightnessWindow); }

/* 灯光模式窗口：0=关, 1=流水灯, 2=跑马灯 */
MenuWindow SetLightModeWindow = {
	.General_Width = 80, .General_Height = 28,
	.Text_String = "灯光模式",
	.Text_FontSize = OLED_UI_FONT_12,
	.Text_FontSideDistance = 4, .Text_FontTopDistance = 3,
	.General_WindowType = WINDOW_ROUNDRECTANGLE,
	.General_ContinueTime = 4.0,
	.Prob_Data_Int_16 = &ws2812_light_mode,
	.Prob_DataStep = 1, .Prob_MinData = 0, .Prob_MaxData = 2,
	.Prob_BottomDistance = 3, .Prob_LineHeight = 8, .Prob_SideDistance = 4,
};

void LightModeWindow(void) { OLED_UI_CreateWindow(&SetLightModeWindow); }

/* 开机动画展示回调 */
void BootAnimDecodeDemo(void) {
	int16_t len = strlen(BOOT_TEXT);
	int16_t tx = (128 - len * 7) / 2;
	boot_anim_decode(BOOT_TEXT, tx, 26);
}
void BootAnimParticleDemo(void) {
	int16_t len = strlen(BOOT_TEXT);
	int16_t tx = (128 - len * 7) / 2;
	boot_anim_particle(BOOT_TEXT, tx, 26);
}
void BootAnimCircleDemo(void) {
	int16_t len = strlen(BOOT_TEXT);
	int16_t tx = (128 - len * 7) / 2;
	boot_anim_circle(BOOT_TEXT, tx, 26);
}
void BootAnimGlitchDemo(void) {
	int16_t len = strlen(BOOT_TEXT);
	int16_t tx = (128 - len * 7) / 2;
	boot_anim_glitch(BOOT_TEXT, tx, 26);
}

static const AppTaskDef app_robot_face_app = {
	.init = app_robot_face_init,
	.tick = app_robot_face_tick,
	.sample = NULL,
	.should_exit = app_robot_face_should_exit,
	.on_exit = app_robot_face_on_exit,
	.fade_tick = NULL,
	.fade_steps = 0,
	.frame_interval_ms = 30,
};
void RobotFaceStart(void){
	app_task_start(&app_robot_face_app);
}

/**
 * @brief 启动小恐龙游戏
 */
static const AppTaskDef dino_game_app = {
	.init = dino_game_init,
	.tick = dino_game_tick,
	.sample = NULL,
	.should_exit = dino_game_should_exit,
	.on_exit = dino_game_on_exit,
	.fade_tick = NULL,
	.fade_steps = 0,
	.frame_interval_ms = 30,
};
void DinoGameStart(void){
	app_task_start(&dino_game_app);
}

/**
 * @brief 启动水管鸟游戏
 */
static const AppTaskDef bird_game_app = {
	.init = app_bird_game_init,
	.tick = app_bird_game_tick,
	.sample = NULL,
	.should_exit = app_bird_game_should_exit,
	.on_exit = app_bird_game_on_exit,
	.fade_tick = NULL,
	.fade_steps = 0,
	.frame_interval_ms = 30,
};
void FlappyBirdStart(void){
	app_task_start(&bird_game_app);
}

/**
 * @brief 启动飞机大战游戏
 */
static const AppTaskDef plane_game_app = {
	.init = plane_game_init,
	.tick = plane_game_tick,
	.sample = NULL,
	.should_exit = plane_game_should_exit,
	.on_exit = plane_game_on_exit,
	.fade_tick = NULL,
	.fade_steps = 0,
	.frame_interval_ms = 30,
};
void PlaneGameStart(void){
	app_task_start(&plane_game_app);
}
//关于窗口的结构体
MenuWindow NullWindow = {
	.General_Width = 80,							//窗口宽度
	.General_Height = 28, 							//窗口高度
	.General_WindowType = WINDOW_ROUNDRECTANGLE, 	//窗口类型
	.General_ContinueTime = 4.0,					//窗口持续时间
};
/**
 * @brief 创建空白窗口
 */
void EmptyWindow(void){
	OLED_UI_CreateWindow(&NullWindow);
}
//关于窗口的结构体
MenuWindow TextWindow = {
	.General_Width = 75,								//窗口宽度
	.General_Height = 18, 								//窗口高度
	.General_WindowType = WINDOW_ROUNDRECTANGLE, 		//窗口类型
	.General_ContinueTime = 4.0,						//窗口持续时间

	.Text_String = "文字text",							//窗口标题
	.Text_FontSize = OLED_UI_FONT_12,					//字高
	.Text_FontSideDistance = 8,							//字体距离左侧的距离
	.Text_FontTopDistance = 2,							//字体距离顶部的距离
	

};
/**
 * @brief 创建显示文本窗口
 */
void ShowTextWindow(void){
	OLED_UI_CreateWindow(&TextWindow);
}
//关于窗口的结构体
MenuWindow FloatDataWindow = {
	.General_Width = 80,								//窗口宽度
	.General_Height = 28, 								//窗口高度
	.Text_String = "浮点数据测试",						//窗口标题
	.Text_FontSize = OLED_UI_FONT_12,					//字高
	.Text_FontSideDistance = 4,							//字体距离左侧的距离
	.Text_FontTopDistance = 3,							//字体距离顶部的距离
	.General_WindowType = WINDOW_ROUNDRECTANGLE, 		//窗口类型
	.General_ContinueTime = 4.0,						//窗口持续时间

	.Prob_Data_Float = &testfloatnum,					//显示的变量地址
	.Prob_DataStep = 0.1,								//步长
	.Prob_MinData = -100,								//最小值
	.Prob_MaxData = 100, 								//最大值
	.Prob_BottomDistance = 3,							//底部间距
	.Prob_LineHeight = 8,								//进度条高度
	.Prob_SideDistance = 4,	
};
/**
 * @brief 创建显示亮度窗口
 */
void ShowFloatDataWindow(void){
	OLED_UI_CreateWindow(&FloatDataWindow);
}
//关于窗口的结构体
MenuWindow IntDataWindow = {
	.General_Width = 80,								//窗口宽度
	.General_Height = 28, 							//窗口高度
	.Text_String = "整数数据测试",					//窗口标题
	.Text_FontSize = OLED_UI_FONT_12,				//字高
	.Text_FontSideDistance = 4,							//字体距离左侧的距离
	.Text_FontTopDistance = 3,							//字体距离顶部的距离
	.General_WindowType = WINDOW_ROUNDRECTANGLE, 	//窗口类型
	.General_ContinueTime = 4.0,						//窗口持续时间

	.Prob_Data_Int_32 = &testintnum,				//显示的变量地址
	.Prob_DataStep = 1,								//步长
	.Prob_MinData = -100,									//最小值
	.Prob_MaxData = 100, 								//最大值
	.Prob_BottomDistance = 3,							//底部间距
	.Prob_LineHeight = 8,								//进度条高度
	.Prob_SideDistance = 4,	
};
/**
 * @brief 创建显示亮度窗口
 */
void ShowIntDataWindow(void){
	OLED_UI_CreateWindow(&IntDataWindow);
}
//主LOGO移动的结构体
OLED_ChangePoint LogoMove;
//主LOGO文字移动的结构体
OLED_ChangePoint LogoTextMove;
//welcome文字移动的结构体
OLED_ChangePoint WelcomeTextMove;

extern OLED_ChangePoint OLED_UI_PageStartPoint ;



//设置菜单项的辅助显示函数
void SettingAuxFunc(void){
	//在规定位置显示LOGO
	if(fabs(OLED_UI_PageStartPoint.CurrentPoint.X - OLED_UI_PageStartPoint.TargetPoint.X) < 4){
		LogoMove.TargetPoint.X = 0;
		LogoMove.TargetPoint.Y = 0;
	}
	//将LOGOTEXT移动到屏幕右侧看不见的地方
	LogoTextMove.TargetPoint.X = 129;
	LogoTextMove.TargetPoint.Y = 0;
	//将Welcome文字移动到屏幕底部看不见的地方
	WelcomeTextMove.TargetPoint.X = 128;
	WelcomeTextMove.TargetPoint.Y = 0;
	ChangePoint(&LogoMove); 
	OLED_ShowImageArea(LogoMove.CurrentPoint.X,LogoMove.CurrentPoint.Y,32,64,0,0,128,128,OLED_UI_SettingsLogo);
	ChangePoint(&LogoTextMove);
	OLED_ShowImageArea(LogoTextMove.CurrentPoint.X,LogoTextMove.CurrentPoint.Y,26,64,0,0,128,128,OLED_UI_LOGOTEXT64);
	ChangePoint(&WelcomeTextMove);
	OLED_ShowImageArea(WelcomeTextMove.CurrentPoint.X,WelcomeTextMove.CurrentPoint.Y,16,64,0,0,128,128,OLED_UI_LOGOGithub);
}

//关于菜单的辅助显示函数
void AboutThisDeviceAuxFunc(void){
	//将LOGO移动到屏幕上方看不见的地方
	LogoMove.TargetPoint.X = 0;
	LogoMove.TargetPoint.Y = -80;
	ChangePoint(&LogoMove);
    OLED_ShowImageArea(LogoMove.CurrentPoint.X,LogoMove.CurrentPoint.Y,32,64,0,0,128,128,OLED_UI_SettingsLogo);
	//在屏幕右侧显示LOGO文字
	if(fabs(OLED_UI_PageStartPoint.CurrentPoint.X - OLED_UI_PageStartPoint.TargetPoint.X) < 4){
		LogoTextMove.TargetPoint.X = 102;
		LogoTextMove.TargetPoint.Y = 0;
	}
	ChangePoint(&LogoTextMove);
	OLED_ShowImageArea(LogoTextMove.CurrentPoint.X,LogoTextMove.CurrentPoint.Y,26,64,0,0,128,128,OLED_UI_LOGOTEXT64);
}
//关于OLED UI的辅助显示函数
void AboutOLED_UIAuxFunc(void){
	//将LOGO移动到屏幕上方看不见的地方
	LogoMove.TargetPoint.X = 0;
	LogoMove.TargetPoint.Y = -80;
	ChangePoint(&LogoMove);
	OLED_ShowImageArea(LogoMove.CurrentPoint.X,LogoMove.CurrentPoint.Y,32,64,0,0,128,128,OLED_UI_SettingsLogo);
	//在屏幕右测显示Welcome文字
	if(fabs(OLED_UI_PageStartPoint.CurrentPoint.X - OLED_UI_PageStartPoint.TargetPoint.X) < 4){
		WelcomeTextMove.TargetPoint.X = 110;
		WelcomeTextMove.TargetPoint.Y = 0;
	}
	ChangePoint(&WelcomeTextMove);
	OLED_ShowImageArea(WelcomeTextMove.CurrentPoint.X,WelcomeTextMove.CurrentPoint.Y,16,64,0,0,128,128,OLED_UI_LOGOGithub);

}
//主菜单的辅助显示函数
void MainAuxFunc(void){
	//不显示
	LogoMove.TargetPoint.X = -200;
	LogoMove.TargetPoint.Y = 0;
	LogoMove.CurrentPoint.X = -200;
	LogoMove.CurrentPoint.Y = 0;

	LogoTextMove.TargetPoint.X = 129;
	LogoTextMove.TargetPoint.Y = 0;
	LogoTextMove.CurrentPoint.X = 129;
	LogoTextMove.CurrentPoint.Y = 0;
	
	WelcomeTextMove.TargetPoint.X = 128;
	WelcomeTextMove.TargetPoint.Y = 0;
	WelcomeTextMove.CurrentPoint.X = 128;
	WelcomeTextMove.CurrentPoint.Y = 0;
}

//主菜单的菜单项
MenuItem MainMenuItems[] = {

	// {.General_item_text = "Settings",.General_callback = NULL,.General_SubMenuPage = &SettingsMenuPage,.Tiles_Icon = Image_setings},
	// {.General_item_text = "Gyroscope",.General_callback = GyroscopeWindow,.General_SubMenuPage = NULL,.Tiles_Icon = gImage_gyro},//Image_wechat},
	// // {.General_item_text = "Alipay",.General_callback = NULL,.General_SubMenuPage = NULL,.Tiles_Icon = Image_alipay},
	// // {.General_item_text = "计算器 Calc 长文本测试 LongText",.General_callback = NULL,.General_SubMenuPage = NULL,.Tiles_Icon = Image_calc},
	// {.General_item_text = "Games",.General_callback = NULL,.General_SubMenuPage = &GamesMenuPage,.Tiles_Icon = gImage_game_icon},
	// {.General_item_text = "Night",.General_callback = NULL,.General_SubMenuPage = NULL,.Tiles_Icon = Image_night},
	// {.General_item_text = "More",.General_callback = NULL,.General_SubMenuPage = &MoreMenuPage,.Tiles_Icon = Image_more},
	// {.General_item_text = NULL},/*最后一项的General_item_text置为NULL，表示该项为分割线*/
	{.General_item_text = "设置",.General_callback = NULL,.General_SubMenuPage = &SettingsMenuPage,.Tiles_Icon = Image_setings},
	{.General_item_text = "RGB灯",.General_callback = NULL,.General_SubMenuPage = &RGBEffectMenuPage,.Tiles_Icon = gImage_rgb_icon},
	{.General_item_text = "陀螺仪",.General_callback = GyroscopeWindow,.General_SubMenuPage = NULL,.Tiles_Icon = gImage_gyro},
	{.General_item_text = "串口",.General_callback = UartMonitorStart,.General_SubMenuPage = NULL,.Tiles_Icon = gImage_uart_icon},
	{.General_item_text = "机器人",.General_callback = RobotFaceStart,.General_SubMenuPage = NULL,.Tiles_Icon = gImage_robot_icon},
	{.General_item_text = "小游戏",.General_callback = NULL,.General_SubMenuPage = &GamesMenuPage,.Tiles_Icon = gImage_game_icon},
	{.General_item_text = "主题",.General_callback = NULL,.General_SubMenuPage = NULL,.Tiles_Icon = Image_night},
	{.General_item_text = "更多",.General_callback = NULL,.General_SubMenuPage = &MoreMenuPage,.Tiles_Icon = Image_more},
	{.General_item_text = NULL},/*最后一项的General_item_text置为NULL，表示该项为分割线*/

};
//设置菜单项内容数组
MenuItem SettingsMenuItems[] = {
	{.General_item_text = "亮度",.General_callback = BrightnessWindow,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "音量",.General_callback = BuzzerVolumeWindow,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "声音开关",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = &mid_beeper0.enable},
	{.General_item_text = "黑暗模式",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = &ColorMode},
	{.General_item_text = "显示帧率",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = &OLED_UI_ShowFps},
	{.General_item_text = "保存数据",.General_callback = SavedataWindow,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "此设备",.General_callback = NULL,.General_SubMenuPage = &AboutThisDeviceMenuPage,.List_BoolRadioBox = NULL},
	{.General_item_text = "关于OLED UI",.General_callback = NULL,.General_SubMenuPage = &AboutOLED_UIMenuPage,.List_BoolRadioBox = NULL},
	{.General_item_text = "感谢观看,一键三连! Thanks for watching, three clicks!",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "[返回]",.General_callback = OLED_UI_Back,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},

	{.General_item_text = NULL},/*最后一项的General_item_text置为NULL，表示该项为分割线*/
};

//游戏菜单项内容数组
MenuItem GamesMenuItems[] = {
	{.General_item_text = "小恐龙 ",.General_callback = DinoGameStart,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "水管鸟 ",.General_callback = FlappyBirdStart,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "飞机大战",.General_callback = PlaneGameStart,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "[返回]",.General_callback = OLED_UI_Back,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},

	{.General_item_text = NULL},/*最后一项的General_item_text置为NULL，表示该项为分割线*/
};

MenuItem AboutThisDeviceMenuItems[] = {
	{.General_item_text = "-[MCU:]",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = " MSPM0G3507",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = " RAM:32KB",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = " FLASH:128KB",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "-[Screen:]",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = " SSD1312 128x64 OLED",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "-[CP:]",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = " SoftWare I2C",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "[返回]",.General_callback = OLED_UI_Back,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},

	{.General_item_text = NULL},/*最后一项的General_item_text置为NULL，表示该项为分割线*/
};

MenuItem AboutOLED_UIMenuItems[] = {
	{.General_item_text = "-[Author:]",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = " bilibili @上nm网课呢",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "-[Adress:]",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = " https://github.com/bdth-7777777/OLED_UI",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "-[Secondary Developer:]",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = " LCKFB",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "[返回]",.General_callback = OLED_UI_Back,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},

	{.General_item_text = NULL},/*最后一项的General_item_text置为NULL，表示该项为分割线*/
};

//开机动画菜单项
MenuItem BootAnimMenuItems[] = {
	{.General_item_text = "解码效果",.General_callback = BootAnimDecodeDemo,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "粒子汇聚",.General_callback = BootAnimParticleDemo,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "圆形揭示",.General_callback = BootAnimCircleDemo,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "故障风",.General_callback = BootAnimGlitchDemo,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "[返回]",.General_callback = OLED_UI_Back,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = NULL},
};

//菜单类型样式选择菜单
MenuItem MenuStyleMenuItems[] = {
	{.General_item_text = "列表样式",.General_callback = NULL,.General_SubMenuPage = &MenuStyleListPage,.List_BoolRadioBox = NULL},
	{.General_item_text = "磁贴样式",.General_callback = NULL,.General_SubMenuPage = &MenuStyleTilesPage,.List_BoolRadioBox = NULL},
	{.General_item_text = "景深样式",.General_callback = NULL,.General_SubMenuPage = &MenuStyleDepthPage,.List_BoolRadioBox = NULL},
	{.General_item_text = "HOPE样式",.General_callback = NULL,.General_SubMenuPage = &MenuStyleHopePage,.List_BoolRadioBox = NULL},
	{.General_item_text = "弧形样式",.General_callback = NULL,.General_SubMenuPage = &MenuStyleArcPage,.List_BoolRadioBox = NULL},
	{.General_item_text = "[返回]",.General_callback = OLED_UI_Back,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = NULL},
};

//菜单类型演示用的菜单项（带图标，适用于所有类型）
MenuItem MenuStyleDemoItems[] = {
	{.General_item_text = "Item A",.General_callback = OLED_UI_Back,.General_SubMenuPage = NULL,.Tiles_Icon = Image_setings},
	{.General_item_text = "Item B",.General_callback = OLED_UI_Back,.General_SubMenuPage = NULL,.Tiles_Icon = gImage_gyro},
	{.General_item_text = "Item C",.General_callback = OLED_UI_Back,.General_SubMenuPage = NULL,.Tiles_Icon = gImage_game_icon},
	{.General_item_text = "Item D",.General_callback = OLED_UI_Back,.General_SubMenuPage = NULL,.Tiles_Icon = Image_night},
	{.General_item_text = "Item E",.General_callback = OLED_UI_Back,.General_SubMenuPage = NULL,.Tiles_Icon = Image_more},
	{.General_item_text = NULL},
};

MenuItem MoreMenuItems[] = {
	{.General_item_text = "[返回]",.General_callback = OLED_UI_Back,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "开机动画样式",.General_callback = NULL,.General_SubMenuPage = &BootAnimMenuPage,.List_BoolRadioBox = NULL},
	{.General_item_text = "菜单类型样式",.General_callback = NULL,.General_SubMenuPage = &MenuStyleMenuPage,.List_BoolRadioBox = NULL},
	{.General_item_text = "字体高度8demo",.General_callback = NULL,.General_SubMenuPage = &Font8MenuPage,.List_BoolRadioBox = NULL},
	{.General_item_text = "字体高度12demo",.General_callback = NULL,.General_SubMenuPage = &Font12MenuPage,.List_BoolRadioBox = NULL},
	{.General_item_text = "字体高度16demo",.General_callback = NULL,.General_SubMenuPage = &Font16MenuPage,.List_BoolRadioBox = NULL},
	{.General_item_text = "字体高度20demo",.General_callback = NULL,.General_SubMenuPage = &Font20MenuPage,.List_BoolRadioBox = NULL},
	{.General_item_text = "超长文本demo",.General_callback = NULL,.General_SubMenuPage = &LongMenuPage,.List_BoolRadioBox = NULL},
	{.General_item_text = "回弹动画demo",.General_callback = NULL,.General_SubMenuPage = &SpringMenuPage,.List_BoolRadioBox = NULL},
	{.General_item_text = "超长菜单列表demo",.General_callback = NULL,.General_SubMenuPage = &LongListMenuPage,.List_BoolRadioBox = NULL},
	{.General_item_text = "超小区域菜单demo",.General_callback = NULL,.General_SubMenuPage = &SmallAreaMenuPage,.List_BoolRadioBox = NULL},
	{.General_item_text = "空窗口demo",.General_callback = EmptyWindow,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "文字窗口demo",.General_callback = ShowTextWindow,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "浮点数进度条窗口demo",.General_callback = ShowFloatDataWindow,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "整数进度条窗口demo",.General_callback = ShowIntDataWindow,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = NULL},/*最后一项的General_item_text置为NULL，表示该项为分割线*/
};
MenuItem Font8MenuItems[] = {
	{.General_item_text = "[返回]",.General_callback = OLED_UI_Back,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "中文文本",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "English Text",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "1234567890",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "ABCDEFGHIJKLMNOPQRSTUVWXYZ",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "abcdefghijklmnopqrstuvwxyz",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = ",.[]!@#$+-/^&*()",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "中文文本",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "English Text",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "1234567890",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "ABCDEFGHIJKLMNOPQRSTUVWXYZ",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "abcdefghijklmnopqrstuvwxyz",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = ",.[]!@#$+-/^&*()",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = NULL},/*最后一项的General_item_text置为NULL，表示该项为分割线*/
};

MenuItem Font12MenuItems[] = {
	{.General_item_text = "[返回]",.General_callback = OLED_UI_Back,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "中文文本",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "English Text",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "1234567890",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "ABCDEFGHIJKLMNOPQRSTUVWXYZ",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "abcdefghijklmnopqrstuvwxyz",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = ",.[]!@#$+-/^&*()",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "中文文本",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "English Text",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "1234567890",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "ABCDEFGHIJKLMNOPQRSTUVWXYZ",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "abcdefghijklmnopqrstuvwxyz",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = ",.[]!@#$+-/^&*()",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = NULL},/*最后一项的General_item_text置为NULL，表示该项为分割线*/
};

MenuItem Font16MenuItems[] = {
	{.General_item_text = "[返回]",.General_callback = OLED_UI_Back,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "中文文本",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "English Text",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "1234567890",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "ABCDEFGHIJKLMNOPQRSTUVWXYZ",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "abcdefghijklmnopqrstuvwxyz",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = ",.[]!@#$+-/^&*()",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "中文文本",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "English Text",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "1234567890",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "ABCDEFGHIJKLMNOPQRSTUVWXYZ",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "abcdefghijklmnopqrstuvwxyz",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = ",.[]!@#$+-/^&*()",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = NULL},/*最后一项的General_item_text置为NULL，表示该项为分割线*/
};

MenuItem Font20MenuItems[] = {
	{.General_item_text = "[返回]",.General_callback = OLED_UI_Back,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "中文文本",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "English Text",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "1234567890",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "ABCDEFGHIJKLMNOPQRSTUVWXYZ",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "abcdefghijklmnopqrstuvwxyz",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = ",.[]!@#$+-/^&*()",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "中文文本",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "English Text",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "1234567890",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "ABCDEFGHIJKLMNOPQRSTUVWXYZ",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "abcdefghijklmnopqrstuvwxyz",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = ",.[]!@#$+-/^&*()",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = NULL},/*最后一项的General_item_text置为NULL，表示该项为分割线*/
};

MenuItem LongMenuItems[] = {
	{.General_item_text = "[返回]",.General_callback = OLED_UI_Back,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "非常非常非常长中文文本",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Very Very Very Long English Text",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "1 2 3 4 5 6 7 8 9 10 11 12 13 14 15",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = NULL},/*最后一项的General_item_text置为NULL，表示该项为分割线*/
};

MenuItem SpringMenuItems[] = {
	{.General_item_text = "[返回]",.General_callback = OLED_UI_Back,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "中文文本",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "English Text",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "1234567890",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "ABCDEFGHIJKLMNOPQRSTUVWXYZ",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "abcdefghijklmnopqrstuvwxyz",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = ",.[]!@#$+-/^&*()",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "中文文本",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "English Text",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "1234567890",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "ABCDEFGHIJKLMNOPQRSTUVWXYZ",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "abcdefghijklmnopqrstuvwxyz",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = ",.[]!@#$+-/^&*()",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = NULL},/*最后一项的General_item_text置为NULL，表示该项为分割线*/
};

MenuItem LongListMenuItems[] = {
	{.General_item_text = "[返回]",.General_callback = OLED_UI_Back,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item1",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item2",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item3",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item4",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item5",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item6",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item7",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item8",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item9",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item10",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item11",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item12",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item13",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item14",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item15",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item16",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item17",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item18",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item19",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item20",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item21",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item22",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item23",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item24",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item25",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item26",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item27",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item28",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item29",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item30",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item31",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item32",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item33",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item34",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item35",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item36",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item37",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item38",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item39",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item40",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item41",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item42",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item43",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item44",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item45",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item46",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item47",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item48",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item49",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item50",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item51",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item52",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item53",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item54",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item55",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item56",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item57",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item58",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item59",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item60",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item61",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item62",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item63",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item64",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item65",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item66",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item67",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item68",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item69",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item70",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item71",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item72",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item73",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item74",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item75",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item76",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item77",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item78",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item79",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item80",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item81",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item82",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item83",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item84",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item85",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},	
	{.General_item_text = "Item86",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item87",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item88",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item89",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item90",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item91",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item92",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item93",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item94",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item95",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item96",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item97",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item98",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item99",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item100",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item101",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item102",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item103",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item104",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item105",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item106",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item107",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item108",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item109",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item110",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item111",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item112",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item113",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item114",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item115",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item116",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item117",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item118",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item119",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item120",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item121",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item122",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item123",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item124",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item125",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item126",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item127",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item128",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Item129",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	
	{.General_item_text = NULL},/*最后一项的General_item_text置为NULL，表示该项为分割线*/
};

MenuItem SmallAreaMenuItems[] = {
	{.General_item_text = "[返回]",.General_callback = OLED_UI_Back,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "中文文本",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "English Text",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "1234567890",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "ABCDEFGHIJKLMNOPQRSTUVWXYZ",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "abcdefghijklmnopqrstuvwxyz",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = ",.[]!@#$+-/^&*()",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "中文文本",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "English Text",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "1234567890",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "ABCDEFGHIJKLMNOPQRSTUVWXYZ",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "abcdefghijklmnopqrstuvwxyz",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = ",.[]!@#$+-/^&*()",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = NULL},/*最后一项的General_item_text置为NULL，表示该项为分割线*/
};



//RGB效果菜单项内容数组
MenuItem RGBEffectMenuItems[] = {
	{.General_item_text = "灯光模式",.General_callback = LightModeWindow,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "渐变速度",.General_callback = RGBSpeedWindow,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "灯光亮度",.General_callback = RGBBrightnessWindow,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "RGB开关",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = &ws2812_enable},
	{.General_item_text = "红色 R",.General_callback = RGBRedWindow,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "绿色 G",.General_callback = RGBGreenWindow,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "蓝色 B",.General_callback = RGBBlueWindow,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "灯珠数量",.General_callback = RGBLedNumWindow,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "[返回]",.General_callback = OLED_UI_Back,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = NULL},
};

MenuPage MainMenuPage = {
	//通用属性，必填
	.General_MenuType = MENU_TYPE_TILES_ARC,  		 //弧形透视磁贴类型
	.General_CursorStyle = NOT_SHOW,			 //光标类型
	.General_FontSize = OLED_UI_FONT_20,			//字高
	.General_ParentMenuPage = NULL,				//由于这是根菜单，所以父菜单为NULL
	.General_LineSpace = 8,						//磁贴间距
	.General_MoveStyle = UNLINEAR,				//移动方式
	.General_MovingSpeed = SPEED,					//动画移动速度
	.General_ShowAuxiliaryFunction = MainAuxFunc,		 //显示辅助函数
	.General_MenuItems = MainMenuItems,			//菜单项内容数组

	//特殊属性，根据.General_MenuType的类型选择
	.Tiles_ScreenHeight = 64,					//屏幕高度
	.Tiles_ScreenWidth = 128,						//屏幕宽度
	.Tiles_TileWidth = 32,						 //磁贴宽度
	.Tiles_TileHeight = 32,						 //磁贴高度
};


MenuPage SettingsMenuPage = {
	//通用属性，必填
	.General_MenuType = MENU_TYPE_LIST,  		 //菜单类型为列表类型
	.General_CursorStyle = REVERSE_ROUNDRECTANGLE,	 //光标类型为线型
	.General_FontSize = OLED_UI_FONT_12,			//字高
	.General_ParentMenuPage = &MainMenuPage,		 //父菜单为主菜单
	.General_LineSpace = 4,						//行间距 单位：像素
	.General_MoveStyle = UNLINEAR,				//移动方式为非线性曲线动画
	.General_MovingSpeed = SPEED,					//动画移动速度(此值根据实际效果调整)
	.General_ShowAuxiliaryFunction = SettingAuxFunc,//显示辅助函数
	.General_MenuItems = SettingsMenuItems,		 //菜单项内容数组

	//特殊属性，根据.General_MenuType的类型选择
	.List_MenuArea = {32, 0, 95, 64},			 //列表显示区域
	.List_IfDrawFrame = false,					 //是否显示边框
	.List_IfDrawLinePerfix = true,				 //是否显示行前缀
	.List_StartPointX = 4,                        //列表起始点X坐标
	.List_StartPointY = 2,                        //列表起始点Y坐标
};

MenuPage RGBEffectMenuPage = {
	.General_MenuType = MENU_TYPE_LIST,
	.General_CursorStyle = REVERSE_ROUNDRECTANGLE,
	.General_FontSize = OLED_UI_FONT_12,
	.General_ParentMenuPage = &MainMenuPage,
	.General_LineSpace = 4,
	.General_MoveStyle = UNLINEAR,
	.General_MovingSpeed = SPEED,
	.General_ShowAuxiliaryFunction = NULL,
	.General_MenuItems = RGBEffectMenuItems,

	.List_MenuArea = {1, 1, 128, 64},
	.List_IfDrawFrame = false,
	.List_IfDrawLinePerfix = true,
	.List_StartPointX = 4,
	.List_StartPointY = 2,
};

MenuPage GamesMenuPage = {
	//通用属性，必填
	.General_MenuType = MENU_TYPE_LIST,  		 //菜单类型为列表类型
	.General_CursorStyle = REVERSE_ROUNDRECTANGLE,	 //光标类型
	.General_FontSize = OLED_UI_FONT_12,			//字高
	.General_ParentMenuPage = &MainMenuPage,		 //父菜单为主菜单
	.General_LineSpace = 4,						//行间距 单位：像素
	.General_MoveStyle = UNLINEAR,				//移动方式为非线性曲线动画
	.General_MovingSpeed = SPEED,					//动画移动速度(此值根据实际效果调整)
	.General_ShowAuxiliaryFunction = NULL,		 //显示辅助函数
	.General_MenuItems = GamesMenuItems,		 //菜单项内容数组

	//特殊属性，根据.General_MenuType的类型选择
	.List_MenuArea = {1, 1, 128, 64},			 //列表显示区域
	.List_IfDrawFrame = false,					 //是否显示边框
	.List_IfDrawLinePerfix = true,				 //是否显示行前缀
	.List_StartPointX = 4,                        //列表起始点X坐标
	.List_StartPointY = 2,                        //列表起始点Y坐标
};

MenuPage AboutThisDeviceMenuPage = {
	//通用属性，必填
	.General_MenuType = MENU_TYPE_LIST,  		 //菜单类型为列表类型
	.General_CursorStyle = REVERSE_BLOCK,	 //光标类型为圆角矩形
	.General_FontSize = OLED_UI_FONT_12,			//字高
	.General_ParentMenuPage = &SettingsMenuPage,		 //父菜单为主菜单
	.General_LineSpace = 4,						//行间距 单位：像素
	.General_MoveStyle = UNLINEAR,				//移动方式为非线性曲线动画
	.General_MovingSpeed = SPEED,					//动画移动速度(此值根据实际效果调整)
	.General_ShowAuxiliaryFunction = AboutThisDeviceAuxFunc,		 //显示辅助函数
	.General_MenuItems = AboutThisDeviceMenuItems,		 //菜单项内容数组

	//特殊属性，根据.General_MenuType的类型选择
	.List_MenuArea = {0, 0, 100, 64},			 //列表显示区域
	.List_IfDrawFrame = false,					 //是否显示边框
	.List_IfDrawLinePerfix = false,				 //是否显示行前缀
	.List_StartPointX = 4,                        //列表起始点X坐标
	.List_StartPointY = 2,                        //列表起始点Y坐标

};

MenuPage AboutOLED_UIMenuPage = {
	//通用属性，必填
	.General_MenuType = MENU_TYPE_LIST,  		 //菜单类型为列表类型
	.General_CursorStyle = REVERSE_BLOCK,	 //光标类型为圆角矩形
	.General_FontSize = OLED_UI_FONT_12,			//字高
	.General_ParentMenuPage = &SettingsMenuPage,		 //父菜单为主菜单
	.General_LineSpace = 4,						//行间距 单位：像素
	.General_MoveStyle = UNLINEAR,				//移动方式为非线性曲线动画
	.General_MovingSpeed = SPEED,					//动画移动速度(此值根据实际效果调整)
	.General_ShowAuxiliaryFunction = AboutOLED_UIAuxFunc,		 //显示辅助函数
	.General_MenuItems = AboutOLED_UIMenuItems,		 //菜单项内容数组

	//特殊属性，根据.General_MenuType的类型选择
	.List_MenuArea = {0, 0, 105, 64},			 //列表显示区域
	.List_IfDrawFrame = false,					 //是否显示边框
	.List_IfDrawLinePerfix = false,				 //是否显示行前缀
	.List_StartPointX = 4,                        //列表起始点X坐标
	.List_StartPointY = 2,                        //列表起始点Y坐标

};
MenuPage MoreMenuPage = {
	//通用属性，必填
	.General_MenuType = MENU_TYPE_LIST,  		 //菜单类型为列表类型
	.General_CursorStyle = REVERSE_ROUNDRECTANGLE,	 //光标类型为圆角矩形
	.General_FontSize = OLED_UI_FONT_12,			//字高
	.General_ParentMenuPage = &MainMenuPage,		 //父菜单为主菜单
	.General_LineSpace = 4,						//行间距 单位：像素
	.General_MoveStyle = UNLINEAR,				//移动方式为非线性曲线动画
	.General_MovingSpeed = SPEED,					//动画移动速度(此值根据实际效果调整)
	.General_ShowAuxiliaryFunction = NULL,		 //显示辅助函数
	.General_MenuItems = MoreMenuItems,		 //菜单项内容数组

	//特殊属性，根据.General_MenuType的类型选择
	.List_MenuArea = {1, 1, 128, 64},			 //列表显示区域
	.List_IfDrawFrame = false,					 //是否显示边框
	.List_IfDrawLinePerfix = true,				 //是否显示行前缀
	.List_StartPointX = 4,                        //列表起始点X坐标
	.List_StartPointY = 2,                        //列表起始点Y坐标

};

MenuPage BootAnimMenuPage = {
	.General_MenuType = MENU_TYPE_LIST,
	.General_CursorStyle = REVERSE_ROUNDRECTANGLE,
	.General_FontSize = OLED_UI_FONT_12,
	.General_ParentMenuPage = &MoreMenuPage,
	.General_LineSpace = 4,
	.General_MoveStyle = UNLINEAR,
	.General_MovingSpeed = SPEED,
	.General_ShowAuxiliaryFunction = NULL,
	.General_MenuItems = BootAnimMenuItems,

	.List_MenuArea = {1, 1, 128, 64},
	.List_IfDrawFrame = false,
	.List_IfDrawLinePerfix = true,
	.List_StartPointX = 4,
	.List_StartPointY = 2,
};

/* 菜单类型样式选择页 */
MenuPage MenuStyleMenuPage = {
	.General_MenuType = MENU_TYPE_LIST,
	.General_CursorStyle = REVERSE_ROUNDRECTANGLE,
	.General_FontSize = OLED_UI_FONT_12,
	.General_ParentMenuPage = &MoreMenuPage,
	.General_LineSpace = 4,
	.General_MoveStyle = UNLINEAR,
	.General_MovingSpeed = SPEED,
	.General_ShowAuxiliaryFunction = NULL,
	.General_MenuItems = MenuStyleMenuItems,

	.List_MenuArea = {1, 1, 128, 64},
	.List_IfDrawFrame = false,
	.List_IfDrawLinePerfix = true,
	.List_StartPointX = 4,
	.List_StartPointY = 2,
};

/* 列表样式演示 */
MenuPage MenuStyleListPage = {
	.General_MenuType = MENU_TYPE_LIST,
	.General_CursorStyle = REVERSE_ROUNDRECTANGLE,
	.General_FontSize = OLED_UI_FONT_12,
	.General_ParentMenuPage = &MenuStyleMenuPage,
	.General_LineSpace = 4,
	.General_MoveStyle = UNLINEAR,
	.General_MovingSpeed = SPEED,
	.General_ShowAuxiliaryFunction = NULL,
	.General_MenuItems = MenuStyleDemoItems,
	.List_MenuArea = {1, 1, 128, 64},
	.List_IfDrawFrame = false,
	.List_IfDrawLinePerfix = true,
	.List_StartPointX = 4,
	.List_StartPointY = 2,
};

/* 磁贴样式演示 */
MenuPage MenuStyleTilesPage = {
	.General_MenuType = MENU_TYPE_TILES,
	.General_CursorStyle = NOT_SHOW,
	.General_FontSize = OLED_UI_FONT_16,
	.General_ParentMenuPage = &MenuStyleMenuPage,
	.General_LineSpace = 8,
	.General_MoveStyle = UNLINEAR,
	.General_MovingSpeed = SPEED,
	.General_ShowAuxiliaryFunction = NULL,
	.General_MenuItems = MenuStyleDemoItems,
	.Tiles_ScreenHeight = 64,
	.Tiles_ScreenWidth = 128,
	.Tiles_TileWidth = 32,
	.Tiles_TileHeight = 32,
};

/* 景深样式演示 */
MenuPage MenuStyleDepthPage = {
	.General_MenuType = MENU_TYPE_TILES_DEPTH,
	.General_CursorStyle = NOT_SHOW,
	.General_FontSize = OLED_UI_FONT_16,
	.General_ParentMenuPage = &MenuStyleMenuPage,
	.General_LineSpace = 8,
	.General_MoveStyle = UNLINEAR,
	.General_MovingSpeed = SPEED,
	.General_ShowAuxiliaryFunction = NULL,
	.General_MenuItems = MenuStyleDemoItems,
	.Tiles_ScreenHeight = 64,
	.Tiles_ScreenWidth = 128,
	.Tiles_TileWidth = 32,
	.Tiles_TileHeight = 32,
};

/* HOPE样式演示 */
MenuPage MenuStyleHopePage = {
	.General_MenuType = MENU_TYPE_TILES_HOPE,
	.General_CursorStyle = NOT_SHOW,
	.General_FontSize = OLED_UI_FONT_16,
	.General_ParentMenuPage = &MenuStyleMenuPage,
	.General_LineSpace = 8,
	.General_MoveStyle = UNLINEAR,
	.General_MovingSpeed = SPEED,
	.General_ShowAuxiliaryFunction = NULL,
	.General_MenuItems = MenuStyleDemoItems,
	.Tiles_ScreenHeight = 64,
	.Tiles_ScreenWidth = 128,
	.Tiles_TileWidth = 32,
	.Tiles_TileHeight = 32,
};

/* 弧形样式演示 */
MenuPage MenuStyleArcPage = {
	.General_MenuType = MENU_TYPE_TILES_ARC,
	.General_CursorStyle = NOT_SHOW,
	.General_FontSize = OLED_UI_FONT_16,
	.General_ParentMenuPage = &MenuStyleMenuPage,
	.General_LineSpace = 8,
	.General_MoveStyle = UNLINEAR,
	.General_MovingSpeed = SPEED,
	.General_ShowAuxiliaryFunction = NULL,
	.General_MenuItems = MenuStyleDemoItems,
	.Tiles_ScreenHeight = 64,
	.Tiles_ScreenWidth = 128,
	.Tiles_TileWidth = 32,
	.Tiles_TileHeight = 32,
};

MenuPage Font8MenuPage = {
	//通用属性，必填
	.General_MenuType = MENU_TYPE_LIST,  		 //菜单类型为列表类型
	.General_CursorStyle = REVERSE_ROUNDRECTANGLE,	 //光标类型为圆角矩形
	.General_FontSize = OLED_UI_FONT_8,			//字高
	.General_ParentMenuPage = &MoreMenuPage,		 //父菜单为主菜单
	.General_LineSpace = 5,						//行间距 单位：像素
	.General_MoveStyle = UNLINEAR,				//移动方式为非线性曲线动画
	.General_MovingSpeed = SPEED,					//动画移动速度(此值根据实际效果调整)
	.General_ShowAuxiliaryFunction = NULL,		 //显示辅助函数
	.General_MenuItems = Font8MenuItems,		 //菜单项内容数组

	//特殊属性，根据.General_MenuType的类型选择
	.List_MenuArea = {0, 0, 128, 64},			 //列表显示区域
	.List_IfDrawFrame = false,					 //是否显示边框
	.List_IfDrawLinePerfix = true,				 //是否显示行前缀
	.List_StartPointX = 4,                        //列表起始点X坐标
	.List_StartPointY = 2,                        //列表起始点Y坐标

};

MenuPage Font12MenuPage = {
	//通用属性，必填
	.General_MenuType = MENU_TYPE_LIST,  		 //菜单类型为列表类型
	.General_CursorStyle = REVERSE_ROUNDRECTANGLE,	 //光标类型为圆角矩形
	.General_FontSize = OLED_UI_FONT_12,			//字高
	.General_ParentMenuPage = &MoreMenuPage,		 //父菜单为主菜单
	.General_LineSpace = 4,						//行间距 单位：像素
	.General_MoveStyle = UNLINEAR,				//移动方式为非线性曲线动画
	.General_MovingSpeed = SPEED,					//动画移动速度(此值根据实际效果调整)
	.General_ShowAuxiliaryFunction = NULL,		 //显示辅助函数
	.General_MenuItems = Font12MenuItems,		 //菜单项内容数组

	//特殊属性，根据.General_MenuType的类型选择
	.List_MenuArea = {0, 0, 128, 64},			 //列表显示区域
	.List_IfDrawFrame = false,					 //是否显示边框
	.List_IfDrawLinePerfix = true,				 //是否显示行前缀
	.List_StartPointX = 4,                        //列表起始点X坐标
	.List_StartPointY = 2,                        //列表起始点Y坐标

};

MenuPage Font16MenuPage = {
	//通用属性，必填
	.General_MenuType = MENU_TYPE_LIST,  		 //菜单类型为列表类型
	.General_CursorStyle = REVERSE_ROUNDRECTANGLE,	 //光标类型为圆角矩形
	.General_FontSize = OLED_UI_FONT_16,			//字高
	.General_ParentMenuPage = &MoreMenuPage,		 //父菜单为主菜单
	.General_LineSpace = 5,						//行间距 单位：像素
	.General_MoveStyle = UNLINEAR,				//移动方式为非线性曲线动画
	.General_MovingSpeed = SPEED,					//动画移动速度(此值根据实际效果调整)
	.General_ShowAuxiliaryFunction = NULL,		 //显示辅助函数
	.General_MenuItems = Font16MenuItems,		 //菜单项内容数组

	//特殊属性，根据.General_MenuType的类型选择
	.List_MenuArea = {1, 1, 126, 62},			 //列表显示区域
	.List_IfDrawFrame = false,					 //是否显示边框
	.List_IfDrawLinePerfix = true,				 //是否显示行前缀
	.List_StartPointX = 4,                        //列表起始点X坐标
	.List_StartPointY = 2,                        //列表起始点Y坐标

};

MenuPage Font20MenuPage = {
	//通用属性，必填
	.General_MenuType = MENU_TYPE_LIST,  		 //菜单类型为列表类型
	.General_CursorStyle = REVERSE_ROUNDRECTANGLE,	 //光标类型为圆角矩形
	.General_FontSize = OLED_UI_FONT_20,			//字高
	.General_ParentMenuPage = &MoreMenuPage,		 //父菜单为主菜单
	.General_LineSpace = 10,						//行间距 单位：像素
	.General_MoveStyle = UNLINEAR,				//移动方式为非线性曲线动画
	.General_MovingSpeed = SPEED,					//动画移动速度(此值根据实际效果调整)
	.General_ShowAuxiliaryFunction = NULL,		 //显示辅助函数
	.General_MenuItems = Font20MenuItems,		 //菜单项内容数组

	//特殊属性，根据.General_MenuType的类型选择
	.List_MenuArea = {0, 0, 128, 64},			 //列表显示区域
	.List_IfDrawFrame = false,					 //是否显示边框
	.List_IfDrawLinePerfix = true,				 //是否显示行前缀
	.List_StartPointX = 4,                        //列表起始点X坐标
	.List_StartPointY = 2,                        //列表起始点Y坐标

};

MenuPage LongMenuPage = {
	//通用属性，必填
	.General_MenuType = MENU_TYPE_LIST,  		 //菜单类型为列表类型
	.General_CursorStyle = REVERSE_ROUNDRECTANGLE,	 //光标类型为圆角矩形
	.General_FontSize = OLED_UI_FONT_12,			//字高
	.General_ParentMenuPage = &MoreMenuPage,		 //父菜单为主菜单
	.General_LineSpace = 4,						//行间距 单位：像素
	.General_MoveStyle = UNLINEAR,				//移动方式为非线性曲线动画
	.General_MovingSpeed = SPEED,					//动画移动速度(此值根据实际效果调整)
	.General_ShowAuxiliaryFunction = NULL,		 //显示辅助函数
	.General_MenuItems = LongMenuItems,		 //菜单项内容数组

	//特殊属性，根据.General_MenuType的类型选择
	.List_MenuArea = {0, 0, 128, 64},			 //列表显示区域
	.List_IfDrawFrame = false,					 //是否显示边框
	.List_IfDrawLinePerfix = true,				 //是否显示行前缀
	.List_StartPointX = 4,                        //列表起始点X坐标
	.List_StartPointY = 2,                        //列表起始点Y坐标

};

MenuPage SpringMenuPage = {
	//通用属性，必填
	.General_MenuType = MENU_TYPE_LIST,  		 //菜单类型为列表类型
	.General_CursorStyle = REVERSE_ROUNDRECTANGLE,	 //光标类型为圆角矩形
	.General_FontSize = OLED_UI_FONT_12,			//字高
	.General_ParentMenuPage = &MoreMenuPage,		 //父菜单为主菜单
	.General_LineSpace = 4,						//行间距 单位：像素
	.General_MoveStyle = PID_CURVE,				//移动方式为非线性曲线动画
	.General_MovingSpeed = SPEED,					//动画移动速度(此值根据实际效果调整)
	.General_ShowAuxiliaryFunction = NULL,		 //显示辅助函数
	.General_MenuItems = SpringMenuItems,		 //菜单项内容数组

	//特殊属性，根据.General_MenuType的类型选择
	.List_MenuArea = {0, 0, 128, 64},			 //列表显示区域
	.List_IfDrawFrame = false,					 //是否显示边框
	.List_IfDrawLinePerfix = true,				 //是否显示行前缀
	.List_StartPointX = 4,                        //列表起始点X坐标
	.List_StartPointY = 2,                        //列表起始点Y坐标

};

MenuPage LongListMenuPage = {
	//通用属性，必填
	.General_MenuType = MENU_TYPE_LIST,  		 //菜单类型为列表类型
	.General_CursorStyle = REVERSE_ROUNDRECTANGLE,	 //光标类型为圆角矩形
	.General_FontSize = OLED_UI_FONT_12,			//字高
	.General_ParentMenuPage = &MoreMenuPage,		 //父菜单为主菜单
	.General_LineSpace = 4,						//行间距 单位：像素
	.General_MoveStyle = UNLINEAR,				//移动方式为非线性曲线动画
	.General_MovingSpeed = SPEED,					//动画移动速度(此值根据实际效果调整)
	.General_ShowAuxiliaryFunction = NULL,		 //显示辅助函数
	.General_MenuItems = LongListMenuItems,		 //菜单项内容数组

	//特殊属性，根据.General_MenuType的类型选择
	.List_MenuArea = {0, 0, 128, 64},			 //列表显示区域
	.List_IfDrawFrame = false,					 //是否显示边框
	.List_IfDrawLinePerfix = true,				 //是否显示行前缀
	.List_StartPointX = 4,                        //列表起始点X坐标
	.List_StartPointY = 2,                        //列表起始点Y坐标

};

MenuPage SmallAreaMenuPage = {
	//通用属性，必填
	.General_MenuType = MENU_TYPE_LIST,  		 //菜单类型为列表类型
	.General_CursorStyle = REVERSE_ROUNDRECTANGLE,	 //光标类型为圆角矩形
	.General_FontSize = OLED_UI_FONT_12,			//字高
	.General_ParentMenuPage = &MoreMenuPage,		 //父菜单为主菜单
	.General_LineSpace = 6,						//行间距 单位：像素
	.General_MoveStyle = UNLINEAR,				//移动方式为非线性曲线动画
	.General_MovingSpeed = SPEED,					//动画移动速度(此值根据实际效果调整)
	.General_ShowAuxiliaryFunction = NULL,		 //显示辅助函数
	.General_MenuItems = SmallAreaMenuItems,		 //菜单项内容数组

	//特殊属性，根据.General_MenuType的类型选择
	.List_MenuArea = {10, 10, 60, 36},			 //列表显示区域
	.List_IfDrawFrame = true,					 //是否显示边框
	.List_IfDrawLinePerfix = true,				 //是否显示行前缀
	.List_StartPointX = 4,                        //列表起始点X坐标
	.List_StartPointY = 2,                        //列表起始点Y坐标

};

