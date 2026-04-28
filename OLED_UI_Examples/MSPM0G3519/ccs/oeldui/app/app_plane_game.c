/**
 * app_plane_game.c
 * 飞机大战游戏主实现文件
 */

// 包含必要的头文件
#include "app_plane_game.h"
#include "OLED.h"
#include "OLED_UI_Driver.h"
#include "hw_delay.h"
#include "stdlib.h"

// =============================================================================
// 全局变量定义
// =============================================================================
static int game_status = PLANE_GAME_READY;       // 当前游戏状态
static plane_game_object_t player;               // 玩家飞机
static plane_game_object_t enemies[MAX_ENEMIES]; // 敌机数组
static plane_game_object_t bullets[MAX_BULLETS]; // 子弹数组
static unsigned int score = 0;                   // 当前分数
static unsigned int score_counter = 0;           // 分数计数器
static unsigned int fire_cooldown = 0;           // 发射冷却计数器
static bool exit_requested = false;              // 退出请求标志

// 游戏输入状态
static plane_game_input_t game_input = {
    .up = false,
    .down = false,
    .left = false,
    .right = false,
    .click = false
};

// =============================================================================
// 内部函数声明
// =============================================================================
static void draw_player(void);
static void draw_enemy(plane_game_object_t *enemy);
static void draw_bullet(plane_game_object_t *bullet);
static void draw_score(void);

static void draw_player(void) {
    if (!player.active) return;

    int x = player.x;
    int y = player.y;

    // 机身主体
    OLED_DrawRectangle(x + 3, y + 3, 7, 4, OLED_FILLED);
    // 机头尖端
    OLED_DrawTriangle(x + 10, y + 3, x + 10, y + 6, x + 13, y + 5, OLED_FILLED);
    // 上机翼
    OLED_DrawTriangle(x + 4, y + 3, x + 7, y + 3, x + 4, y, OLED_FILLED);
    OLED_DrawLine(x + 4, y, x + 7, y + 2);
    // 下机翼
    OLED_DrawTriangle(x + 4, y + 6, x + 7, y + 6, x + 4, y + 9, OLED_FILLED);
    OLED_DrawLine(x + 4, y + 9, x + 7, y + 7);
    // 尾翼
    OLED_DrawLine(x + 2, y + 1, x + 3, y + 3);
    OLED_DrawLine(x + 2, y + 8, x + 3, y + 6);
    OLED_DrawPoint(x + 2, y + 1);
    OLED_DrawPoint(x + 2, y + 8);
    // 驾驶舱
    OLED_ClearArea(x + 8, y + 4, 2, 2);
    OLED_DrawPoint(x + 9, y + 4);
}

/**
 * 绘制敌机 - 面朝左的小飞机轮廓
 */
static void draw_enemy(plane_game_object_t *enemy) {
    if (!enemy->active) return;

    int x = enemy->x;
    int y = enemy->y;

    // 机身
    OLED_DrawRectangle(x + 2, y + 2, 6, 4, OLED_FILLED);
    // 机头（朝左的三角）
    OLED_DrawTriangle(x + 2, y + 2, x + 2, y + 5, x, y + 4, OLED_FILLED);
    // 上翼
    OLED_DrawLine(x + 5, y + 2, x + 7, y);
    OLED_DrawLine(x + 7, y, x + 8, y);
    OLED_DrawLine(x + 8, y, x + 6, y + 2);
    // 下翼
    OLED_DrawLine(x + 5, y + 5, x + 7, y + 7);
    OLED_DrawLine(x + 7, y + 7, x + 8, y + 7);
    OLED_DrawLine(x + 8, y + 7, x + 6, y + 5);
    // 尾翼
    OLED_DrawPoint(x + 8, y + 3);
    OLED_DrawPoint(x + 8, y + 4);
}

/**
 * 绘制子弹 - 小菱形弹头
 */
static void draw_bullet(plane_game_object_t *bullet) {
    if (!bullet->active) return;

    int x = bullet->x;
    int y = bullet->y;

    // 菱形子弹：中间一条横线 + 上下各一个点
    OLED_DrawLine(x, y + 1, x + 3, y + 1);
    OLED_DrawPoint(x + 1, y);
    OLED_DrawPoint(x + 1, y + 2);
}

/**
 * 绘制分数
 */
static void draw_score(void) {
    OLED_ShowNum(98, 2, score, 4, OLED_6X8_HALF);
}

// =============================================================================
// 公共接口函数实现
// =============================================================================

/**
 * 设置单击信号函数
 */
void plane_game_set_click(void) {
    game_input.click = true;
}

/**
 * 设置方向键信号函数
 */
// 向上移动（竖屏模式）
void plane_game_set_up(void) {
    game_input.up = true;
    if (player.active && player.y > 0) {
        player.y -= player.speed;
        if (player.y < 0) player.y = 0;
    }
}

// 向下移动（竖屏模式）
void plane_game_set_down(void) {
    game_input.down = true;
    if (player.active && player.y < SCREEN_HEIGHT - player.height) {
        player.y += player.speed;
        if (player.y > SCREEN_HEIGHT - player.height) {
            player.y = SCREEN_HEIGHT - player.height;
        }
    }
}
/**
 * 清除所有方向键信号
 */
void plane_game_clear_controls(void) {
    game_input.up = false;
    game_input.down = false;
    game_input.left = false;
    game_input.right = false;
    game_input.click = false;
}

/**
 * 游戏参数初始化
 */
void plane_game_init(void) {
    // 初始化游戏状态
    game_status = PLANE_GAME_READY;
    score = 0;
    score_counter = 0;
    fire_cooldown = 0;
    exit_requested = false;
    plane_game_clear_controls();
    
    // 初始化玩家飞机（标准128x64屏幕，左侧居中）
    player.x = PLAYER_INIT_X;  // 左侧位置
    player.y = SCREEN_HEIGHT / 2 - PLAYER_HEIGHT / 2;    // 垂直居中位置（0-63范围内）
    player.width = PLAYER_WIDTH;
    player.height = PLAYER_HEIGHT;
    player.speed = PLAYER_SPEED; // 使用常量定义的速度值
    player.active = true;
    
    // 初始化敌机数组
    for (int i = 0; i < MAX_ENEMIES; i++) {
        enemies[i].active = false;
    }
    
    // 初始化子弹数组
    for (int i = 0; i < MAX_BULLETS; i++) {
        bullets[i].active = false;
    }
    
    // 清除屏幕并显示游戏标题
    OLED_Clear();
    // 画一架静止的玩家飞机
    draw_player();
    OLED_ShowString(40, 15, "PLANE WAR", OLED_7X12_HALF);
    OLED_ShowString(35, 35, "Press to start", OLED_6X8_HALF);
    OLED_ShowString(35, 48, "UD to move", OLED_6X8_HALF);
    OLED_Update();
    plane_game_clear_controls();
}

/**
 * 处理按键输入
 */
void plane_game_handle_input(void) {
    int16_t enc_delta = Encoder_Get();
    if (enc_delta < 0) plane_game_set_up();
    if (enc_delta > 0) plane_game_set_down();

    // 处理游戏状态转换
    if (game_status == PLANE_GAME_READY) {
        // 准备状态：按任意键开始游戏
        if (game_input.up || game_input.down) {
            game_status = PLANE_GAME_RUNNING;
        }
    } else if (game_status == PLANE_GAME_RUNNING) {
        // 运行状态：处理移动和射击
        // 移动逻辑在update函数中处理

        // 自动发射子弹（不需要手动控制）
    } else if (game_status == PLANE_GAME_OVER) {
        // 游戏结束状态：按任意键重新开始
        if (game_input.up || game_input.down) {
            plane_game_init();
        }
    }

    // 使用统一的函数清除所有输入状态
        plane_game_clear_controls();
}

/**
 * 更新游戏状态
 */
void plane_game_update(void) {
    if (game_status != PLANE_GAME_RUNNING) return;
    
    // 更新分数
    score_counter++;
    
    // 更新分数（每30帧增加1分）
    if (score_counter >= 30) {
        score++;
        score_counter = 0;
    }
    
    // 更新发射冷却
    if (fire_cooldown > 0) {
        fire_cooldown--;
    }
    
    // 自动发射子弹
    if (fire_cooldown == 0 && player.active) {
        plane_game_fire_bullet();
        fire_cooldown = FIRE_INTERVAL;
    }
    
    // 生成敌机（从右侧生成）- 降低生成频率
    if (rand() % 3 == 0) {
        plane_game_generate_enemy();
    }
    
    // 更新敌机位置 - 标准128x64屏幕：敌机从右向左移动（x从127向0移动）
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].active) {
            enemies[i].x -= enemies[i].speed;  // 敌机向左移动
            
            // 检查敌机是否移出屏幕左侧
            if ((enemies[i].x <= 0) || (enemies[i].x>=60000) ) {
                enemies[i].active = false;
            }
        }
    }
    
    // 更新子弹位置 - 标准128x64屏幕：子弹从左向右移动（x从0向127移动）
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (bullets[i].active) {
            bullets[i].x += bullets[i].speed;  // 子弹向右移动
            
            // 检查子弹是否移出屏幕右侧
            if (bullets[i].x > SCREEN_WIDTH) {
                bullets[i].active = false;
            }
        }
    }
    
    // 碰撞检测（游戏逻辑保持不变）
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (bullets[i].active) {
            for (int j = 0; j < MAX_ENEMIES; j++) {
                if (enemies[j].active) {
                    if (plane_game_check_collision(&bullets[i], &enemies[j])) {
                        // 子弹和敌机碰撞，两者都失效
                        bullets[i].active = false;
                        enemies[j].active = false;
                        score += 10;  // 击中敌机加分
                    }
                }
            }
        }
    }
    
    // 检测玩家和敌机的碰撞
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].active) {
            if (plane_game_check_collision(&player, &enemies[i])) {
                plane_game_over();
                return;
            }
        }
    }
}

/**
 * 渲染游戏画面
 */
void plane_game_render(void) {

    if (game_status == PLANE_GAME_RUNNING) {

        // 清空屏幕
        OLED_Clear();

        // 绘制游戏元素
        draw_player();
        
        // 绘制所有激活的敌机
        for (int i = 0; i < MAX_ENEMIES; i++) {
            if (enemies[i].active) {
                draw_enemy(&enemies[i]);
            }
        }
        
        // 绘制所有激活的子弹
        for (int i = 0; i < MAX_BULLETS; i++) {
            if (bullets[i].active) {
                draw_bullet(&bullets[i]);
            }
        }
        
        // 绘制分数
        draw_score();
    } else if (game_status == PLANE_GAME_OVER) {

        // 清空屏幕
        OLED_Clear();

        // 游戏结束画面
        OLED_ClearArea(18, 12, 92, 40);
        OLED_DrawRoundedRectangle(18, 12, 92, 40, 3, OLED_UNFILLED);
        OLED_ShowString(32, 16, "GAME  OVER", OLED_7X12_HALF);
        // 显示最终分数
        OLED_ShowString(36, 32, "Score:", OLED_6X8_HALF);
        OLED_ShowNum(72, 32, score, 4, OLED_6X8_HALF);
        OLED_ShowString(30, 44, "Press to retry", OLED_6X8_HALF);
    }
    
    // 更新OLED显示
    OLED_Update();
}

/**
 * 碰撞检测
 */
bool plane_game_check_collision(plane_game_object_t *obj1, plane_game_object_t *obj2) {
    // 矩形碰撞检测核心逻辑（游戏逻辑中的碰撞检测，不涉及显示坐标转换）
    bool x_overlap = (obj1->x < obj2->x + obj2->width) && 
                   (obj1->x + obj1->width > obj2->x);
    bool y_overlap = (obj1->y < obj2->y + obj2->height) && 
                   (obj1->y + obj1->height > obj2->y);
    
    return x_overlap && y_overlap;
}

/**
 * 生成敌机
 */
void plane_game_generate_enemy(void) {
    // 找到一个未激活的敌机
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].active) {
            // 初始化敌机属性 - 从右侧生成（x=128，y在0-63范围内）
            enemies[i].x = SCREEN_WIDTH;  // 屏幕外右侧（128）
            enemies[i].y = rand() % (SCREEN_HEIGHT - ENEMY_HEIGHT);  // 随机垂直位置（0-63范围内）
            enemies[i].width = ENEMY_WIDTH;
            enemies[i].height = ENEMY_HEIGHT;
            enemies[i].speed = ENEMY_SPEED_MIN + (rand() % (ENEMY_SPEED_MAX - ENEMY_SPEED_MIN + 1));
            enemies[i].active = true;
            break;
        }
    }
}

/**
 * 发射子弹
 */
void plane_game_fire_bullet(void) {
    // 找到一个未激活的子弹
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!bullets[i].active) {
            // 初始化子弹属性（从飞机右侧发射）
            bullets[i].x = player.x + player.width;
            bullets[i].y = player.y + player.height / 2 - BULLET_HEIGHT / 2;
            bullets[i].width = BULLET_WIDTH;
            bullets[i].height = BULLET_HEIGHT;
            bullets[i].speed = BULLET_SPEED;
            bullets[i].active = true;
            break;
        }
    }
}

/**
 * 游戏结束处理
 */
void plane_game_over(void) {
    game_status = PLANE_GAME_OVER;
}

/**
 * 游戏主循环
 */
void plane_game_loop(void) {
    plane_game_init();
    
    // 游戏主循环
    while (1) {
        plane_game_handle_input();  // 处理输入状态转换
        plane_game_update();       // 更新游戏状态（包括移动）
        plane_game_render();       // 渲染画面
        
        // 每个帧结束后清除方向键状态，这样需要持续按住才能保持移动
        plane_game_clear_controls();
        
        // 检查是否有退出请求
        if (plane_game_should_exit()) {
            OLED_Clear();    // 清除屏幕
            break;          // 跳出游戏主循环，返回上一级菜单
        }
        
        delay_ms(30);  // 控制游戏帧率约33FPS
    }
}

/**
 * 设置游戏退出标志
 */
void plane_game_request_exit(void) {
    exit_requested = true;
}

/**
 * 检查是否有退出请求
 */
bool plane_game_should_exit(void) {
    return exit_requested;
}

void plane_game_tick(void) {
    plane_game_handle_input();
    plane_game_update();
    plane_game_render();
    plane_game_clear_controls();
}

void plane_game_on_exit(void) {
    OLED_Clear();
    OLED_Update();
}