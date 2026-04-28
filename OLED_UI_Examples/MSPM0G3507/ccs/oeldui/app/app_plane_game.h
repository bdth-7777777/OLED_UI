/**
 * app_plane_game.h
 * 飞机大战游戏头文件
 * 
 * 此文件定义了飞机大战游戏的核心数据结构、枚举类型和函数接口
 * 为游戏实现提供必要的声明
 */

#ifndef __APP_PLANE_GAME_H__
#define __APP_PLANE_GAME_H__

// 包含必要的标准库头文件
#include <stdbool.h>  // 提供bool类型定义

// =============================================================================
// 游戏常量定义
// =============================================================================
#define SCREEN_WIDTH    128    // 屏幕宽度（像素）- 竖屏模式
#define SCREEN_HEIGHT   64     // 屏幕高度（像素）- 竖屏模式

// 玩家飞机参数
#define PLAYER_WIDTH    12     // 玩家飞机宽度（竖屏模式调整）
#define PLAYER_HEIGHT   10     // 玩家飞机高度（竖屏模式调整）
#define PLAYER_SPEED    6      // 玩家飞机移动速度（竖屏模式调整）
#define PLAYER_INIT_X   2      // 玩家飞机初始X坐标（左侧）
#define PLAYER_INIT_Y   60     // 玩家飞机初始Y坐标（居中）

// 敌机参数
#define ENEMY_WIDTH     10     // 敌机宽度
#define ENEMY_HEIGHT    8      // 敌机高度
#define ENEMY_SPEED_MIN 1      // 敌机最小速度
#define ENEMY_SPEED_MAX 3      // 敌机最大速度
#define MAX_ENEMIES     3      // 最大敌机数量

// 子弹参数
#define BULLET_WIDTH    4      // 子弹宽度
#define BULLET_HEIGHT   2      // 子弹高度
#define BULLET_SPEED    5      // 子弹速度
#define MAX_BULLETS     4      // 最大子弹数量
#define FIRE_INTERVAL   15     // 发射间隔（帧）

// 游戏状态定义
#define PLANE_GAME_READY     0  // 游戏准备状态
#define PLANE_GAME_RUNNING   1  // 游戏运行状态
#define PLANE_GAME_OVER      2  // 游戏结束状态

// =============================================================================
// 数据结构定义
// =============================================================================

/**
 * 游戏对象结构体
 * 用于表示游戏中的物理对象（飞机、敌机、子弹）
 */
typedef struct {
    unsigned int x;        // 对象的X坐标（水平位置）
    unsigned int y;        // 对象的Y坐标（垂直位置）
    unsigned int width;    // 对象的宽度（像素）
    unsigned int height;   // 对象的高度（像素）
    unsigned int speed;    // 对象的移动速度
    bool active;           // 对象是否激活
} plane_game_object_t;

/**
 * 游戏输入结构体
 * 用于跟踪游戏输入状态
 */
typedef struct {
    bool up;               // 上方向键状态
    bool down;             // 下方向键状态
    bool left;             // 左方向键状态
    bool right;            // 右方向键状态
    bool click;            // 单击状态（用于开始游戏）
} plane_game_input_t;

// =============================================================================
// 公共接口函数声明
// =============================================================================

/**
 * 设置单击信号函数
 * 
 * 这是游戏提供给外部调用的接口函数，
 * 用户需要在检测到单击事件时调用此函数
 * 用于控制游戏开始或重新开始游戏
 */
void plane_game_set_click(void);

/**
 * 设置方向键信号函数
 * 
 * 这是游戏提供给外部调用的接口函数，
 * 用户需要在检测到方向键事件时调用相应的函数
 */
void plane_game_set_up(void);
void plane_game_set_down(void);

/**
 * 清除所有方向键信号
 */
void plane_game_clear_controls(void);

/**
 * 游戏参数初始化
 * 
 * 初始化所有游戏变量、玩家状态和敌机数组
 * 显示游戏开始界面
 */
void plane_game_init(void);

/**
 * 游戏主循环
 * 
 * 游戏的入口函数，包含完整的游戏循环：
 * 初始化、输入处理、更新、渲染
 */
void plane_game_loop(void);

/**
 * 处理按键输入
 * 
 * 检查用户输入标志，并根据当前游戏状态执行相应操作：
 * - 游戏准备状态：开始游戏
 * - 游戏运行状态：移动玩家飞机、发射子弹
 * - 游戏结束状态：重新开始游戏
 */
void plane_game_handle_input(void);

/**
 * 更新游戏状态
 * 
 * 更新游戏的所有动态元素：
 * - 玩家飞机的位置
 * - 敌机的位置和生成
 * - 子弹的位置和生成
 * - 游戏分数
 * - 碰撞检测
 */
void plane_game_update(void);

/**
 * 渲染游戏画面
 * 
 * 绘制游戏的所有视觉元素：
 * - 玩家飞机
 * - 敌机
 * - 子弹
 * - 分数
 * - 游戏结束信息（如果游戏已结束）
 */
void plane_game_render(void);

/**
 * 碰撞检测
 * 
 * 检查两个游戏对象是否发生碰撞
 * 
 * @return 如果发生碰撞返回true，否则返回false
 */
bool plane_game_check_collision(plane_game_object_t *obj1, plane_game_object_t *obj2);

/**
 * 生成敌机
 */
void plane_game_generate_enemy(void);

/**
 * 发射子弹
 */
void plane_game_fire_bullet(void);

/**
 * 游戏结束处理
 */
void plane_game_over(void);

/**
 * 设置游戏退出标志
 * 外部模块可以调用此函数来请求游戏退出
 */
void plane_game_request_exit(void);

/**
 * 检查是否有退出请求
 * @return 如果有退出请求返回true，否则返回false
 */
bool plane_game_should_exit(void);

void plane_game_tick(void);
void plane_game_on_exit(void);

#endif  // _APP_PLANE_GAME_H_