/**
 * app_dino_game.c
 * 小恐龙游戏主实现文件
 */

// 包含必要的头文件
#include "app_dino_game.h"
#include "OLED.h"
#include "OLED_UI_Driver.h"
#include "hw_delay.h"
#include "stdlib.h"

// =============================================================================
// 游戏常量定义
// =============================================================================
#define SCREEN_WIDTH    128    // 屏幕宽度（像素）
#define SCREEN_HEIGHT   64     // 屏幕高度（像素）
#define GROUND_Y        50     // 地面Y坐标位置

// 恐龙参数设置
#define DINO_WIDTH      14     // 恐龙宽度（像素）
#define DINO_HEIGHT     14     // 恐龙高度（像素）
#define DINO_JUMP_SPEED 4      // 恐龙跳跃初始速度
#define DINO_X          15     // 恐龙初始X坐标位置

// 障碍物参数设置
#define OBSTACLE_WIDTH   6     // 障碍物宽度（像素）
#define OBSTACLE_HEIGHT1 12    // 小仙人掌高度（像素）
#define OBSTACLE_HEIGHT2 18    // 大仙人掌高度（像素）
#define OBSTACLE_SPEED   2     // 障碍物基础移动速度

// 游戏状态定义
#define GAME_READY     0       // 游戏准备状态
#define GAME_RUNNING   1       // 游戏运行状态
#define GAME_OVER      2       // 游戏结束状态

// =============================================================================
// 全局变量定义
// =============================================================================
dino_state_t dino_state = DINO_RUNNING;
dino_game_object_t dino;
dino_game_object_t obstacles[3];
int game_status = GAME_READY;
int score = 0;
int score_counter = 0;
int speed_level = 1;
float dino_gravity = 0.3;
float jump_velocity = 0;
int animation_frame = 0;
int ground_offset = 0;
bool exit_requested = false;  // 退出请求标志，由外部设置

// =============================================================================
// 游戏输入接口定义
// =============================================================================

// 游戏输入接口
typedef struct {
    bool click;  // 单击标志
} game_input_t;

game_input_t game_input = {0};

// =============================================================================
// 绘制函数
// =============================================================================

// 绘制恐龙 - 仿Chrome T-Rex风格
void draw_dino(int x, int y, dino_state_t state) {
    // 清除之前的恐龙图像区域
    OLED_ClearArea(x - 2, y - 2, DINO_WIDTH + 4, DINO_HEIGHT + 6);

    if (state == DINO_DEAD) {
        // 死亡状态 - 眼睛变X
        // 头部
        OLED_DrawRectangle(x + 6, y, 8, 6, OLED_FILLED);
        // 眼睛X形
        OLED_ClearArea(x + 10, y + 1, 3, 3);
        OLED_DrawLine(x + 10, y + 1, x + 12, y + 3);
        OLED_DrawLine(x + 12, y + 1, x + 10, y + 3);
        // 嘴巴
        OLED_DrawRectangle(x + 10, y + 5, 4, 1, OLED_FILLED);
        // 身体
        OLED_DrawRectangle(x + 4, y + 5, 6, 7, OLED_FILLED);
        // 小手臂
        OLED_DrawRectangle(x + 9, y + 7, 2, 3, OLED_FILLED);
        // 尾巴
        OLED_DrawRectangle(x, y + 4, 2, 2, OLED_FILLED);
        OLED_DrawRectangle(x + 2, y + 5, 2, 2, OLED_FILLED);
        // 腿 - 站立
        OLED_DrawRectangle(x + 5, y + 12, 2, 3, OLED_FILLED);
        OLED_DrawRectangle(x + 8, y + 12, 2, 3, OLED_FILLED);
    } else if (state == DINO_DUCKING) {
        // 下蹲状态 - 身体压扁拉长
        int dy = 6; // 下蹲时整体下移
        // 头部
        OLED_DrawRectangle(x + 10, y + dy, 6, 4, OLED_FILLED);
        // 眼睛
        OLED_ClearArea(x + 13, y + dy + 1, 2, 2);
        OLED_DrawPoint(x + 14, y + dy + 1);
        // 身体（扁平）
        OLED_DrawRectangle(x, y + dy + 3, 12, 4, OLED_FILLED);
        // 小手臂
        OLED_DrawRectangle(x + 11, y + dy + 4, 2, 2, OLED_FILLED);
        // 腿 - 交替跑步
        if (animation_frame % 8 < 4) {
            OLED_DrawRectangle(x + 3, y + dy + 7, 2, 3, OLED_FILLED);
            OLED_DrawPoint(x + 7, y + dy + 7);
        } else {
            OLED_DrawPoint(x + 3, y + dy + 7);
            OLED_DrawRectangle(x + 7, y + dy + 7, 2, 3, OLED_FILLED);
        }
    } else {
        // 跑步/跳跃状态 - T-Rex侧面轮廓
        // 头部（较大的方块）
        OLED_DrawRectangle(x + 6, y, 8, 6, OLED_FILLED);
        // 眼睛（在头部右上方挖空）
        OLED_ClearArea(x + 10, y + 1, 2, 2);
        OLED_DrawPoint(x + 11, y + 1);
        // 嘴巴（头部下方凸出）
        OLED_DrawRectangle(x + 10, y + 5, 4, 1, OLED_FILLED);
        // 身体（连接头和尾）
        OLED_DrawRectangle(x + 4, y + 5, 6, 7, OLED_FILLED);
        // 小手臂
        OLED_DrawRectangle(x + 9, y + 7, 2, 3, OLED_FILLED);
        // 尾巴（向左上方延伸）
        OLED_DrawRectangle(x, y + 4, 2, 2, OLED_FILLED);
        OLED_DrawRectangle(x + 2, y + 5, 2, 2, OLED_FILLED);

        // 腿部动画
        if (state == DINO_JUMPING) {
            // 跳跃时双腿并拢伸直
            OLED_DrawRectangle(x + 5, y + 12, 2, 3, OLED_FILLED);
            OLED_DrawRectangle(x + 8, y + 12, 2, 3, OLED_FILLED);
        } else {
            // 跑步时腿部交替
            if (animation_frame % 8 < 4) {
                // 帧1：左腿前伸，右腿后蹬
                OLED_DrawRectangle(x + 5, y + 12, 2, 3, OLED_FILLED);
                OLED_DrawPoint(x + 5, y + 14);
                OLED_DrawRectangle(x + 8, y + 12, 2, 2, OLED_FILLED);
            } else {
                // 帧2：右腿前伸，左腿后蹬
                OLED_DrawRectangle(x + 5, y + 12, 2, 2, OLED_FILLED);
                OLED_DrawRectangle(x + 8, y + 12, 2, 3, OLED_FILLED);
                OLED_DrawPoint(x + 8, y + 14);
            }
        }
    }
}

// 绘制障碍物（仙人掌）- 仿Chrome风格
void draw_obstacle(dino_game_object_t *obs) {
    if (obs->width == 0 || obs->height == 0) return;

    // 清除之前的障碍物图像
    OLED_ClearArea(obs->x - 3, obs->y - 1, obs->width + 6, obs->height + 2);

    if (obs->height == OBSTACLE_HEIGHT1) {
        // 小仙人掌 - 单株
        int bx = obs->x;
        int by = obs->y;
        // 主干
        OLED_DrawRectangle(bx + 2, by, 2, 12, OLED_FILLED);
        // 左臂（从中间向左上伸出）
        OLED_DrawRectangle(bx, by + 3, 2, 4, OLED_FILLED);
        OLED_DrawPoint(bx, by + 3);
        // 右臂（从中间向右上伸出）
        OLED_DrawRectangle(bx + 4, by + 5, 2, 3, OLED_FILLED);
        OLED_DrawPoint(bx + 5, by + 5);
        // 顶部尖刺
        OLED_DrawPoint(bx + 2, by - 1);
        OLED_DrawPoint(bx + 3, by - 1);
    } else {
        // 大仙人掌 - 双株组合
        int bx = obs->x;
        int by = obs->y;
        // 左株主干
        OLED_DrawRectangle(bx, by + 2, 2, 16, OLED_FILLED);
        // 左株左臂
        OLED_DrawRectangle(bx - 2, by + 6, 2, 4, OLED_FILLED);
        // 左株右臂
        OLED_DrawRectangle(bx + 2, by + 9, 1, 3, OLED_FILLED);
        // 左株顶部
        OLED_DrawPoint(bx, by + 1);
        OLED_DrawPoint(bx + 1, by + 1);

        // 右株主干（稍矮）
        OLED_DrawRectangle(bx + 4, by + 5, 2, 13, OLED_FILLED);
        // 右株右臂
        OLED_DrawRectangle(bx + 6, by + 8, 2, 4, OLED_FILLED);
        // 右株顶部
        OLED_DrawPoint(bx + 4, by + 4);
        OLED_DrawPoint(bx + 5, by + 4);
    }
}

// 绘制地面 - 仿Chrome风格不规则地面
void draw_ground() {
    // 地面主线
    OLED_DrawLine(0, GROUND_Y, SCREEN_WIDTH - 1, GROUND_Y);

    // 地面碎石纹理 - 不规则的点和短线，随地面滚动
    int off = ground_offset % 128;
    // 第一层纹理：较密的小碎石
    for (int i = -off; i < SCREEN_WIDTH; i += 6) {
        int px = i + ((i * 7 + 13) % 5); // 伪随机偏移
        if (px >= 0 && px < SCREEN_WIDTH) {
            OLED_DrawPoint(px, GROUND_Y + 2);
        }
    }
    // 第二层纹理：稀疏的大碎石
    for (int i = -off; i < SCREEN_WIDTH; i += 11) {
        int px = i + ((i * 3 + 7) % 4);
        if (px >= 0 && px < SCREEN_WIDTH - 1) {
            OLED_DrawLine(px, GROUND_Y + 3, px + 1, GROUND_Y + 3);
        }
    }
    // 第三层纹理：零星小点
    for (int i = -off; i < SCREEN_WIDTH; i += 17) {
        int px = i + ((i * 11 + 3) % 7);
        if (px >= 0 && px < SCREEN_WIDTH) {
            OLED_DrawPoint(px, GROUND_Y + 5);
        }
    }
}

// 绘制分数 - Chrome风格 HI 00000 格式
void draw_score() {
    // 右上角显示分数，前面补零，5位数
    OLED_ShowNum(93, 2, score, 5, OLED_6X8_HALF);
}

// =============================================================================
// 游戏核心函数
// =============================================================================

// 初始化游戏
void dino_game_init() {
    // 初始化随机数生成器
    srand(animation_frame);
    
    // 初始化恐龙
    dino.x = DINO_X;
    dino.y = GROUND_Y - DINO_HEIGHT;
    dino.width = DINO_WIDTH;
    dino.height = DINO_HEIGHT;
    dino_state = DINO_RUNNING;
    jump_velocity = 0;
    
    // 初始化障碍物
    for (int i = 0; i < 3; i++) {
        obstacles[i].x = 0;
        obstacles[i].y = 0;
        obstacles[i].width = 0;  // 初始为无效状态
        obstacles[i].height = 0;
        obstacles[i].speed = OBSTACLE_SPEED;
    }
    
    // 初始化游戏状态
    game_status = GAME_READY;
    score = 0;
    score_counter = 0;
    speed_level = 1;
    animation_frame = 0;
    ground_offset = 0;
    exit_requested = false;  // 重置退出请求标志
    
    // 清空屏幕并显示游戏标题 - Chrome风格
    OLED_Clear();
    // 绘制地面线
    OLED_DrawLine(0, GROUND_Y, SCREEN_WIDTH - 1, GROUND_Y);
    // 在地面上画一只静止的恐龙
    draw_dino(DINO_X, GROUND_Y - DINO_HEIGHT, DINO_RUNNING);
    // 提示文字
    OLED_ShowString(40, 18, "DINO GAME", OLED_7X12_HALF);
    OLED_ShowString(35, 38, "Press to start", OLED_6X8_HALF);
    OLED_Update();
}

// 处理按键输入
void dino_game_handle_input(void) {
    if (Encoder_Get() != 0) {
        game_input.click = 1;
    }

    if (game_status == GAME_READY) {
        // 准备状态下，单击开始游戏
        if (game_input.click) {
            game_input.click = 0;
            game_status = GAME_RUNNING;
            OLED_Clear();
        }
    } else if (game_status == GAME_RUNNING) {
        // 运行状态下，单击让恐龙跳跃
        if (game_input.click && dino_state != DINO_JUMPING && dino.y >= GROUND_Y - DINO_HEIGHT) {
            game_input.click = 0;
            dino_state = DINO_JUMPING;
            jump_velocity = -DINO_JUMP_SPEED;
        }
    } else if (game_status == GAME_OVER) {
        // 游戏结束状态下，单击重新开始游戏
        if (game_input.click) {
            game_input.click = 0;
            dino_game_init();
        }
    }
}

// 更新游戏状态
void dino_game_update(void) {
    if (game_status != GAME_RUNNING) return;
    
    // 更新动画帧计数器
    animation_frame++;
    
    // 更新地面滚动 - 移到update函数中统一管理
    int ground_speed = 1 + (speed_level - 1) / 3;
    ground_offset += ground_speed;
    if (ground_offset >= SCREEN_WIDTH) {
        ground_offset = 0;
    }
    
    // 更新分数 - 每10帧增加1分
    score_counter++;
    if (score_counter >= 10) {
        score_counter = 0;
        score++;
        
        // 每100分增加一次难度，但最大难度为5
        if (score % 100 == 0 && speed_level < 5) {
            speed_level++;
            for (int i = 0; i < 3; i++) {
                obstacles[i].speed = OBSTACLE_SPEED + (speed_level - 1) / 2;
            }
        }
    }
    
    // 更新恐龙状态 - 处理跳跃物理
    if (dino_state == DINO_JUMPING) {
        // 应用重力加速度
        jump_velocity += dino_gravity;
        // 更新恐龙的Y坐标
        dino.y += jump_velocity;
        
        // 限制恐龙最大跳跃高度
        if (dino.y <= 5) {  // 距离屏幕顶部至少保留5个像素
            dino.y = 5;
            if (jump_velocity < 0) {
                jump_velocity = 0.1;  // 给一个小的正速度开始下落
            }
        }
        
        // 检测是否落地
        if (dino.y >= GROUND_Y - DINO_HEIGHT) {
            dino.y = GROUND_Y - DINO_HEIGHT;
            dino_state = DINO_RUNNING;
            jump_velocity = 0;
        }
    }
    
    // 障碍物生成计时器
    static int obstacle_spawn_timer = 0;
    
    // 更新障碍物位置
    for (int i = 0; i < 3; i++) {
        if (obstacles[i].x > 0) {  // 只更新激活的障碍物
            obstacles[i].x -= obstacles[i].speed;
            
            // 障碍物完全移出屏幕左侧后重置
            if (obstacles[i].x <= 0) {
                obstacles[i].x = 0;
                obstacles[i].width = 0;
                obstacles[i].height = 0;
            }
        }
    }
    
    // 更新障碍物生成计时器
    obstacle_spawn_timer++;
    
    // 计算当前活跃障碍物数量
    int active_obstacles = 0;
    for (int i = 0; i < 3; i++) {
        if (obstacles[i].x > 0) active_obstacles++;
    }
    
    // 优化障碍物生成逻辑：减少生成间隔，允许更多障碍物
    int spawn_interval = 100 - speed_level * 10;  // 减少初始间隔并增加难度影响
    spawn_interval = (spawn_interval < 50) ? 50 : spawn_interval;  // 最小间隔设为50帧（约1.5秒）
    
    // 修改生成条件：允许同时存在最多3个障碍物，且考虑前一个障碍物的距离
    bool should_spawn = (obstacle_spawn_timer >= spawn_interval && active_obstacles < 3);
    
    // 如果有障碍物，确保与前一个障碍物有足够的距离
    if (should_spawn && active_obstacles > 0) {
        // 查找最右侧的障碍物
        int rightmost_x = -1;
        for (int i = 0; i < 3; i++) {
            if (obstacles[i].x > 0 && obstacles[i].x > rightmost_x) {
                rightmost_x = obstacles[i].x;
            }
        }
        
        // 确保与前一个障碍物有足够的距离
        int min_distance = 40 + speed_level * 5;  // 距离随速度调整
        should_spawn = (rightmost_x < SCREEN_WIDTH - min_distance);
    }
    
    if (should_spawn) {
        // 找到一个空闲的障碍物槽并生成
        for (int i = 0; i < 3; i++) {
            if (obstacles[i].width == 0) {
                bool is_big = (rand() % 4 == 0);
                
                // 设置障碍物属性
                obstacles[i].x = SCREEN_WIDTH + 10;
                obstacles[i].y = GROUND_Y - (is_big ? OBSTACLE_HEIGHT2 : OBSTACLE_HEIGHT1);
                obstacles[i].width = OBSTACLE_WIDTH;
                obstacles[i].height = is_big ? OBSTACLE_HEIGHT2 : OBSTACLE_HEIGHT1;
                // 增加速度增长，使游戏更有挑战性
                obstacles[i].speed = OBSTACLE_SPEED + (speed_level - 1) / 2;
                
                // 重置计时器
                obstacle_spawn_timer = 0;
                break;
            }
        }
    }
    
    // 检查碰撞
    if (dino_game_check_collision()) {
        dino_state = DINO_DEAD;
        game_status = GAME_OVER;
    }
}

// 渲染游戏画面
void dino_game_render(void) {
    if (game_status != GAME_RUNNING && game_status != GAME_OVER) return;
    
    // 清空屏幕
    OLED_Clear();
    
    // 绘制游戏元素
    draw_ground();
    draw_dino(dino.x, dino.y, dino_state);
    
    // 绘制所有激活的障碍物
    for (int i = 0; i < 3; i++) {
        if (obstacles[i].width > 0) {
            draw_obstacle(&obstacles[i]);
        }
    }
    
    // 绘制分数
    draw_score();
    
    // 如果游戏结束，显示结束信息 - Chrome风格
    if (game_status == GAME_OVER) {
        // 半透明遮罩效果：在中间区域画一个带边框的面板
        OLED_ClearArea(18, 14, 92, 28);
        OLED_DrawRoundedRectangle(18, 14, 92, 28, 3, OLED_UNFILLED);
        OLED_ShowString(32, 18, "GAME  OVER", OLED_7X12_HALF);
        OLED_ShowString(30, 34, "Press to retry", OLED_6X8_HALF);
    }
    
    // 更新OLED显示
    OLED_Update();
}

// 检测碰撞
bool dino_game_check_collision(void) {
    for (int i = 0; i < 3; i++) {
        if (obstacles[i].width > 0) {  // 只检查激活的障碍物
            // 矩形碰撞检测核心逻辑
            bool x_overlap = (dino.x < obstacles[i].x + obstacles[i].width) && 
                           (dino.x + dino.width > obstacles[i].x);
            bool y_overlap = (dino.y < obstacles[i].y + obstacles[i].height) && 
                           (dino.y + dino.height > obstacles[i].y);
            
            if (x_overlap && y_overlap) {
                return true;  // 检测到碰撞
            }
        }
    }
    return false;  // 没有检测到碰撞
}

// 实现生成障碍物函数，避免编译错误
void dino_game_generate_obstacle(void) {
    // 此功能已整合到dino_game_update中
    // 保留空实现以避免编译错误
}


// 游戏结束处理
void dino_game_over(void) {
    game_status = GAME_OVER;
    dino_state = DINO_DEAD;
}

/**
 * 设置游戏退出标志
 * 外部模块可以调用此函数来请求游戏退出
 * 
 * 此函数为外部硬件对接提供接口，当检测到长按事件时，
 * 可以调用此函数通知游戏主循环退出
 */
void dino_game_request_exit(void) {
    exit_requested = true;
}

/**
 * 检查是否有退出请求
 * @return 如果有退出请求返回true，否则返回false
 * 
 * 游戏主循环会定期调用此函数检查是否需要退出
 */
bool dino_game_should_exit(void) {
    return exit_requested;
}

void dino_game_tick(void) {
    dino_game_handle_input();
    dino_game_update();
    dino_game_render();
}

void dino_game_on_exit(void) {
    OLED_Clear();
    OLED_Update();
}

// =============================================================================
// 公共接口函数
// =============================================================================

// 设置单击信号函数
void dino_game_set_click(void) {
    game_input.click = 1;
}

// 游戏主循环
void dino_game_loop(void) {
    dino_game_init();
    
    // 游戏主循环
    while (1) {
        dino_game_handle_input();
        dino_game_update();
        dino_game_render();
        
        // 检查是否有退出请求
        if (dino_game_should_exit()) {
            OLED_Clear();  // 清除屏幕
            break;         // 跳出游戏主循环，返回上一级菜单
        }
        
        // 控制游戏速度，30ms延时控制帧率
        delay_ms(30);
    }
}