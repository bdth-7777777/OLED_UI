#ifndef __OLED_UI_MENUDATA_H
#define __OLED_UI_MENUDATA_H
// 检测是否是C++编译器
#ifdef __cplusplus
extern "C" {
#endif
#include "OLED_UI.h"

//进行前置声明
extern MenuItem MainMenuItems[],SettingsMenuItems[],GamesMenuItems[],AboutThisDeviceMenuItems[],
AboutOLED_UIMenuItems[],MoreMenuItems[],Font8MenuItems[] ,Font12MenuItems[] ,
Font16MenuItems[] ,Font20MenuItems[],LongMenuItems[],SpringMenuItems[],LongListMenuItems[],SmallAreaMenuItems[],
RGBEffectMenuItems[],BootAnimMenuItems[],MenuStyleMenuItems[],MenuStyleDemoItems[];
extern MenuPage MainMenuPage,SettingsMenuPage,GamesMenuPage,AboutThisDeviceMenuPage,
AboutOLED_UIMenuPage,MoreMenuPage,Font8MenuPage,Font12MenuPage,Font16MenuPage
,Font20MenuPage,LongMenuPage,SpringMenuPage,LongListMenuPage,SmallAreaMenuPage,
RGBEffectMenuPage,BootAnimMenuPage,MenuStyleMenuPage,
MenuStyleListPage,MenuStyleTilesPage,MenuStyleDepthPage,MenuStyleHopePage,MenuStyleArcPage;

// 窗口函数声明
void BrightnessWindow(void);
void BuzzerVolumeWindow(void);
void GyroscopeWindow(void);
void UartMonitorStart(void);
void SavedataWindow(void);
void EmptyWindow(void);
void ShowTextWindow(void);
void ShowFloatDataWindow(void);
void ShowIntDataWindow(void);
void DinoGameStart(void);
void FlappyBirdStart(void);
void PlaneGameStart(void);
void RGBRedWindow(void);
void RGBGreenWindow(void);
void RGBBlueWindow(void);
void RGBLedNumWindow(void);
void RGBSpeedWindow(void);
void LightModeWindow(void);
void BootAnimDecodeDemo(void);
void BootAnimParticleDemo(void);
void BootAnimCircleDemo(void);
void BootAnimGlitchDemo(void);
void RobotFaceStart(void);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif
