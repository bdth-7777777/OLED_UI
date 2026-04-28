/**
 * app_bird_game.h
 * 
 * 水管鸟游戏的头文件，定义了游戏中使用的结构体、枚举、宏和函数声明
 */

#ifndef __APP_BIRD_GAME_H__
#define __APP_BIRD_GAME_H__

#include <stdbool.h>

/**
 * 游戏状态枚举
 * 定义了游戏的三种可能状态
 */
typedef enum {
    GAME_READY,    // 游戏准备状态，显示标题和开始提示
    GAME_RUNNING,  // 游戏运行状态，更新游戏逻辑和渲染画面
    GAME_OVER      // 游戏结束状态，显示结束画面和重新开始提示
} app_bird_game_state_t;

/**
 * 游戏对象结构体
 * 用于表示游戏中的移动对象，如小鸟和单个管道
 */
typedef struct {
    unsigned int x;      // 对象的X坐标
    unsigned int y;      // 对象的Y坐标
    unsigned int width;  // 对象的宽度
    unsigned int height; // 对象的高度
    float y_speed;       // Y方向的速度，正值向下，负值向上
} bird_app_bird_game_object_t;

/**
 * 管道对结构体
 * 组合了顶部和底部管道，以及通过状态
 */
typedef struct {
    bird_app_bird_game_object_t top;    // 顶部管道
    bird_app_bird_game_object_t bottom; // 底部管道
    bool passed;               // 是否已通过该管道（用于计分）
} pipe_pair_t;

/**
 * 游戏常量定义
 * 这些宏定义了游戏的基本参数和物理常量
 */
#define SCREEN_WIDTH    128     // 屏幕宽度（像素）
#define SCREEN_HEIGHT   64      // 屏幕高度（像素）
#define BIRD_WIDTH      10      // 小鸟宽度（像素）
#define BIRD_HEIGHT     8       // 小鸟高度（像素）
#define PIPE_WIDTH      12      // 管道宽度（像素）
#define PIPE_GAP_HEIGHT 24      // 上下管道之间的间隙高度（像素）
#define PIPE_SPEED      1       // 管道移动速度（像素/帧）
#define GRAVITY         0.2f    // 重力加速度（像素/帧²）
#define JUMP_FORCE      -1.5f   // 跳跃力度（像素/帧），负值表示向上
#define MAX_PIPES       3       // 最大管道数量（目前未使用）

/**
 * 游戏控制函数
 * 由外部输入模块调用，设置跳跃请求
 */
void app_bird_game_set_jump(void);

/**
 * 游戏初始化函数
 * 设置游戏的初始状态、位置和属性
 */
void app_bird_game_init(void);

/**
 * 游戏启动函数
 * 供主菜单或其他模块调用，启动游戏
 */
void app_bird_game_start(void);

/**
 * 游戏主循环函数
 * 实现游戏的主循环逻辑
 */
void app_bird_game_loop(void);

/**
 * 设置游戏退出标志
 * 外部模块可以调用此函数来请求游戏退出
 */
void app_bird_game_request_exit(void);

/**
 * 检查是否有退出请求
 * @return 如果有退出请求返回true，否则返回false
 */
bool app_bird_game_should_exit(void);

void app_bird_game_tick(void);
void app_bird_game_on_exit(void);

#endif // _APP_BIRD_GAME_H_
