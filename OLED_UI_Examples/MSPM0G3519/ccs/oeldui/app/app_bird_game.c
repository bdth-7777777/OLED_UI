#include "app_bird_game.h"
#include "stdlib.h"
#include "OLED.h"
#include "OLED_UI_Driver.h"
#include "hw_delay.h"

/**
 * 水管鸟游戏实现
 * 
 * 移植说明：
 * 1. 对外接口函数（无static修饰）是移植时可能需要修改的部分
 * 2. 硬件相关函数主要在以下部分：
 *    - OLED显示函数（OLED_ShowString, OLED_DrawRectangle等）
 *    - 延时函数（delay_ms）
 *    - 随机数生成器（srand）
 * 3. 修改游戏物理参数可以通过修改app_bird_game.h中的宏定义实现
 */

/**
 * 游戏状态和对象定义
 * 这些全局变量控制游戏的核心状态和对象属性
 */
static game_state_t game_state = GAME_READY;       // 当前游戏状态：准备、运行或结束
static bird_game_object_t bird;                    // 小鸟对象，包含位置、大小和速度信息
static pipe_pair_t pipes[MAX_PIPES];               // 管道对数组
static unsigned int score = 0;                     // 当前游戏分数
static unsigned int animation_frame = 0;           // 动画帧计数，用于控制小鸟动画
static bool jump_requested = false;                // 跳跃请求标志，由外部输入设置
static bool exit_requested = false;                // 退出请求标志，由外部模块设置

/**
 * 游戏背景动画参数
 */
static unsigned int background_offset = 0;         // 背景滚动偏移量
static const unsigned int BACKGROUND_WIDTH = 128;  // 背景宽度（与屏幕宽度相同）

/**
 * 内部函数声明
 */
static void draw_bird(unsigned int x, unsigned int y, unsigned int frame);
static void draw_pipe(bird_game_object_t *pipe);
static void draw_background(void);
static void generate_pipe(unsigned int index);
static void generate_single_pipe(void);
static void pregenerate_initial_pipes(void);
static void game_handle_input(void);
static void game_update(void);
static void game_render(void);
static bool game_check_collision(void);
static void game_reset(void);

/**
 * 对外接口函数区域 - 这些函数可以被其他模块调用
 */
/**
 * 初始化游戏
 * 
 * 函数设置游戏的初始状态，包括随机数种子、游戏状态、分数、小鸟位置等
 * 初始化管道位置在屏幕外，并显示游戏标题和开始提示
 * 
 * 对外接口函数 - 可被其他模块调用
 */
void game_init(void)
{
    // 初始化随机数生成器和游戏状态变量
    srand(animation_frame);        // 设置随机数种子
    game_state = GAME_READY;       // 设置游戏状态为准备状态
    score = 0;                     // 重置分数
    animation_frame = 0;           // 重置动画帧计数
    jump_requested = false;        // 清除跳跃请求
    exit_requested = false;        // 清除退出请求
    background_offset = 0;         // 重置背景偏移
    
    // 设置小鸟的初始位置和属性
    bird.x = 30;                           // 小鸟初始X坐标（左侧）
    bird.y = SCREEN_HEIGHT / 2;            // 小鸟初始Y坐标（屏幕中央）
    bird.width = BIRD_WIDTH;               // 小鸟宽度
    bird.height = BIRD_HEIGHT;             // 小鸟高度
    bird.y_speed = 0;                      // 初始速度为0
    
    // 初始化所有管道在屏幕外（宽度为0表示不可见）
    for (unsigned int i = 0; i < MAX_PIPES; i++) {
        pipes[i].top.width = 0;
        pipes[i].bottom.width = 0;
        pipes[i].passed = false;
    }
    
    // 显示游戏标题和开始提示
    OLED_Clear();
    OLED_ShowString(35, 20, "FLAPPY BIRD", OLED_7X12_HALF);
    OLED_ShowString(15, 40, "Press UP to start", OLED_6X8_HALF);
    OLED_Update();
}

/**
 * 设置跳跃信号
 * 
 * 函数设置jump_requested标志为true，通知游戏主循环执行跳跃操作
 * 此函数由外部输入处理模块调用
 * 
 * 对外接口函数 - 可被其他模块调用
 */
void game_set_jump(void)
{
    jump_requested = true;  // 设置跳跃请求标志
}

/**
 * 游戏启动函数
 * 
 * 函数启动游戏主循环
 * 提供一个简单的接口供主菜单或其他模块调用
 * 
 * 对外接口函数 - 可被其他模块调用
 */
void game_start(void)
{
    game_loop();  // 启动游戏主循环
}

/**
 * 游戏主循环
 * 
 * 函数实现游戏的主循环逻辑：
 * 1. 初始化游戏
 * 2. 进入无限循环，每一帧依次执行：
 *    - 处理输入
 *    - 更新游戏状态
 *    - 渲染游戏画面
 *    - 延时控制游戏帧率
 * 
 * 对外接口函数 - 可被其他模块调用
 */
void game_loop(void)
{
    game_init();  // 初始化游戏
    
    // 游戏主循环
    while (1) {
        game_handle_input();  // 处理游戏输入
        game_update();        // 更新游戏状态
        game_render();        // 渲染游戏画面
        
        // 检查是否有退出请求
        if (game_should_exit()) {
            OLED_Clear();    // 清除屏幕
            break;          // 跳出游戏主循环，返回上一级菜单
        }
        
        delay_ms(30);         // 延时30ms，控制游戏速度（约33帧/秒）
    }
}

/**
 * 设置游戏退出标志
 * 外部模块可以调用此函数来请求游戏退出
 * 
 * 对外接口函数 - 可被其他模块调用
 */
void game_request_exit(void)
{
    exit_requested = true;
}

/**
 * 检查是否有退出请求
 * @return 如果有退出请求返回true，否则返回false
 * 
 * 对外接口函数 - 可被其他模块调用
 */
bool game_should_exit(void)
{
    return exit_requested;
}

void game_tick(void)
{
    game_handle_input();
    game_update();
    game_render();
}

void game_on_exit(void)
{
    OLED_Clear();
    OLED_Update();
}

/**
 * 内部函数区域 - 这些函数仅在模块内部调用，不对外暴露
 */

/**
 * 绘制小鸟
 * 
 * @param x 小鸟中心X坐标
 * @param y 小鸟中心Y坐标
 * @param frame 当前动画帧，用于控制翅膀动画
 * 
 * 函数首先清除小鸟之前的位置，然后根据当前帧绘制小鸟的身体、头部、眼睛和翅膀
 * 翅膀会根据动画帧在上下位置之间切换，产生扇动效果
 * 
 * 内部函数 - 仅在模块内部调用
 */
static void draw_bird(unsigned int x, unsigned int y, unsigned int frame)
{
    // 清除之前的小鸟位置，确保没有残影
    OLED_ClearArea(x - BIRD_WIDTH/2 - 2, y - BIRD_HEIGHT/2 - 2,
                  BIRD_WIDTH + 4, BIRD_HEIGHT + 4);

    // 身体：圆润的椭圆
    OLED_DrawEllipse(x, y, 4, 3, OLED_UNFILLED);

    // 眼睛：在身体右上方留一个空白像素点
    OLED_ClearArea(x + 2, y - 2, 2, 2);
    OLED_DrawPoint(x + 2, y - 2);

    // 嘴巴：向右的小三角
    OLED_DrawTriangle(x + 4, y - 1, x + 4, y + 1, x + 7, y, OLED_UNFILLED);

    // 尾巴：向左的两条短线
    OLED_DrawLine(x - 4, y - 1, x - 6, y - 3);
    OLED_DrawLine(x - 4, y,     x - 6, y - 1);

    // 翅膀：根据动画帧上下扇动
    if (frame % 8 < 4) {
        // 翅膀上扬
        OLED_DrawTriangle(x - 2, y - 1, x + 1, y - 1, x - 1, y - 5, OLED_UNFILLED);
    } else {
        // 翅膀下垂
        OLED_DrawTriangle(x - 2, y + 1, x + 1, y + 1, x - 1, y + 4, OLED_UNFILLED);
    }
}

/**
 * 绘制单个管道
 * 
 * @param pipe 指向要绘制的管道对象的指针
 * 
 * 函数根据管道的位置和类型（顶部或底部）绘制管道主体和边缘
 * 顶部管道有底部边缘，底部管道有顶部边缘，增加视觉层次感
 * 
 * 内部函数 - 仅在模块内部调用
 */
static void draw_pipe(bird_game_object_t *pipe)
{
    // 确保管道在屏幕范围内才绘制
    if (pipe->x + pipe->width <= 0 || pipe->x >= SCREEN_WIDTH || pipe->width == 0) {
        return;
    }
    
    // 绘制管道主体矩形
    OLED_DrawRectangle(pipe->x, pipe->y, pipe->width, pipe->height, OLED_FILLED);
    
    // 根据管道位置判断是顶部管道还是底部管道，并绘制相应的边缘
    if (pipe->y == 0) {
        // 顶部管道，绘制底部边缘（略微超出主体宽度）
        OLED_DrawRectangle(pipe->x-1, pipe->y + pipe->height - 1, 
                          pipe->width + 2, 2, OLED_FILLED);
    } else {
        // 底部管道，绘制顶部边缘（略微超出主体宽度）
        OLED_DrawRectangle(pipe->x-1, pipe->y, 
                          pipe->width + 2, 2, OLED_FILLED);
    }
}

/**
 * 绘制游戏背景
 * 
 * 函数清除整个屏幕，然后绘制地面和地面纹理
 * 通过不断更新background_offset实现地面滚动的效果
 * 
 * 内部函数 - 仅在模块内部调用
 */
static void draw_background(void)
{
    // 清除整个屏幕
    OLED_Clear();

    // 绘制地面双线
    OLED_DrawLine(0, SCREEN_HEIGHT - 5, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 5);
    OLED_DrawLine(0, SCREEN_HEIGHT - 4, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 4);

    // 绘制地面斜线纹理，模拟草地效果
    for (int i = -8 + (int)(background_offset % 8); i < (int)SCREEN_WIDTH; i += 8) {
        OLED_DrawLine(i, SCREEN_HEIGHT - 3, i + 3, SCREEN_HEIGHT - 1);
    }

    // 绘制远景云朵（随背景缓慢移动）
    int cloud_offset = (int)(background_offset / 3) % SCREEN_WIDTH;

    // 云朵1
    int cx1 = (40 - cloud_offset + SCREEN_WIDTH) % SCREEN_WIDTH;
    OLED_DrawEllipse(cx1, 8, 8, 3, OLED_FILLED);
    OLED_DrawEllipse(cx1 - 6, 9, 4, 2, OLED_FILLED);
    OLED_DrawEllipse(cx1 + 6, 9, 5, 2, OLED_FILLED);

    // 云朵2
    int cx2 = (110 - cloud_offset + SCREEN_WIDTH) % SCREEN_WIDTH;
    OLED_DrawEllipse(cx2, 14, 6, 2, OLED_FILLED);
    OLED_DrawEllipse(cx2 - 5, 15, 3, 2, OLED_FILLED);
    OLED_DrawEllipse(cx2 + 5, 15, 4, 2, OLED_FILLED);

    // 更新背景偏移
    background_offset = (background_offset + 1) % BACKGROUND_WIDTH;
}



/**
 * 生成指定索引的管道对
 * 
 * 函数为指定索引的管道对生成新的位置和属性
 * 管道间隙位置是随机的，但保证在合理范围内，确保游戏难度适中
 * 
 * @param index 要生成的管道索引
 * 
 * 内部函数 - 仅在模块内部调用
 */
static void generate_pipe(unsigned int index)
{
    if (index >= MAX_PIPES) return; // 索引检查
    
    // 计算管道间隙的最小和最大Y坐标，确保间隙在屏幕范围内且不会太小
    unsigned int min_gap = PIPE_GAP_HEIGHT / 2 + 10;
    unsigned int max_gap = SCREEN_HEIGHT - 4 - PIPE_GAP_HEIGHT / 2 - 10;
    
    // 生成随机的间隙中心位置
    unsigned int gap_center = min_gap + (rand() % (max_gap - min_gap + 1));
    
    // 计算新管道的起始X位置 - 根据索引错开一定距离
    // 第一个管道在屏幕右侧，后续管道依次向右排列，保证合适的间距
    unsigned int start_x = SCREEN_WIDTH + 10 + (index * (SCREEN_WIDTH / 2));
    
    // 设置顶部管道的属性
    pipes[index].top.x = start_x;              // 初始位置在屏幕右侧外
    pipes[index].top.y = 0;                    // 顶部管道从屏幕顶部开始
    pipes[index].top.width = PIPE_WIDTH;       // 设置管道宽度
    pipes[index].top.height = gap_center - PIPE_GAP_HEIGHT / 2; // 顶部管道高度
    pipes[index].top.y_speed = PIPE_SPEED;     // 管道移动速度
    
    // 设置底部管道的属性
    pipes[index].bottom.x = start_x;           // 初始位置与顶部管道相同
    pipes[index].bottom.y = gap_center + PIPE_GAP_HEIGHT / 2; // 底部管道起始Y坐标
    pipes[index].bottom.width = PIPE_WIDTH;    // 设置管道宽度
    // 底部管道高度，从起始位置到底部（减去地面高度）
    pipes[index].bottom.height = SCREEN_HEIGHT - pipes[index].bottom.y - 4;
    pipes[index].bottom.y_speed = PIPE_SPEED;  // 管道移动速度
    
    // 重置通过标志，准备记录分数
    pipes[index].passed = false;
}

/**
 * 生成单个管道对
 * 
 * 函数在屏幕右侧外生成一对新的管道（顶部和底部）
 * 管道间隙位置是随机的，但保证在合理范围内，确保游戏难度适中
 * 设置管道的位置、大小和移动速度
 * 
 * 内部函数 - 仅在模块内部调用
 */
static void generate_single_pipe(void)
{
    // 为了兼容性，调用generate_pipe生成第一个管道
    generate_pipe(0);
}

/**
 * 初始化游戏管道
 * 
 * 函数在游戏开始时生成第一个管道对
 * 重置管道通过标志，并调用generate_single_pipe生成新管道
 * 
 * 内部函数 - 仅在模块内部调用
 */
static void pregenerate_initial_pipes(void)
{
    // 为所有管道生成初始位置，错开一定距离
    for (unsigned int i = 0; i < MAX_PIPES; i++) {
        generate_pipe(i);
    }
}

/**
 * 处理游戏输入
 * 
 * 函数检查jump_requested标志，并根据当前游戏状态执行不同操作：
 * - 准备状态：开始游戏，给小鸟向上的初速度，生成第一个管道
 * - 运行状态：给小鸟向上的速度（跳跃）
 * - 结束状态：重新初始化游戏
 * 
 * 内部函数 - 仅在模块内部调用
 */
static void game_handle_input(void)
{
    if (Encoder_Get() != 0) {
        jump_requested = true;
    }

    // 检查是否有跳跃请求
    if (jump_requested) {
        jump_requested = false; // 清除请求标志
        
        // 根据当前游戏状态执行不同操作
        switch (game_state) {
            case GAME_READY:  // 游戏准备状态
                game_state = GAME_RUNNING;        // 切换到运行状态
                bird.y_speed = JUMP_FORCE;        // 给小鸟向上的初速度
                pregenerate_initial_pipes();      // 生成第一个管道
                break;
            case GAME_RUNNING: // 游戏运行中
                bird.y_speed = JUMP_FORCE;        // 给小鸟向上的速度（跳跃）
                break;
            case GAME_OVER:    // 游戏结束状态
                game_init();                      // 重新初始化游戏
                break;
        }
    }
}

/**
 * 更新游戏状态
 * 
 * 函数在每一帧更新游戏的所有动态元素：
 * - 更新动画帧计数
 * - 应用重力和速度更新小鸟位置
 * - 处理管道的移动和重生
 * - 检测管道通过以增加分数
 * - 检查碰撞
 * 
 * 仅在游戏运行状态(GAME_RUNNING)下执行更新
 * 
 * 内部函数 - 仅在模块内部调用
 */
static void game_update(void)
{
    // 仅在游戏运行状态下更新
    if (game_state != GAME_RUNNING) return;
    
    // 更新动画帧计数，用于控制小鸟翅膀动画
    animation_frame++;
    
    // 更新小鸟位置和物理
    bird.y_speed += GRAVITY;    // 应用重力，增加向下的速度
    bird.y += bird.y_speed;     // 根据速度更新Y坐标
    
    // 限制小鸟位置并检查边界碰撞
    if (bird.y < BIRD_HEIGHT / 2) {  // 防止小鸟飞出屏幕顶部
        bird.y = BIRD_HEIGHT / 2;    // 限制在屏幕顶部
        bird.y_speed = 0;           // 重置速度
    } else if (bird.y > SCREEN_HEIGHT - 4 - BIRD_HEIGHT / 2) { // 检查是否撞到地面
        game_state = GAME_OVER;     // 设置游戏结束状态
        return;                     // 提前返回，不再执行后续更新
    }
    
    // 更新所有管道的位置和处理重生
    for (unsigned int i = 0; i < MAX_PIPES; i++) {
        // 确保即使初始宽度为0也能生成管道
        if (pipes[i].top.width <= 0) {
            generate_pipe(i);     // 如果没有有效管道，生成一个新的
        }
        
        // 移动管道（向左移动）
        pipes[i].top.x -= pipes[i].top.y_speed;
        pipes[i].bottom.x -= pipes[i].bottom.y_speed;
        
        // 检查是否通过管道并增加分数
        if (!pipes[i].passed && pipes[i].top.x + pipes[i].top.width < bird.x) {
            pipes[i].passed = true;  // 标记为已通过，避免重复计分
            score++;                 // 增加分数
        }

        // 管道完全移出屏幕后重生新管道
        // 使用更简单可靠的判断条件
        if (pipes[i].top.x <= 0) {
            // 生成新的管道对，并确保新管道在屏幕右侧足够远的位置
            // 为了避免与其他管道重叠，计算新位置时考虑其他管道的位置
            generate_pipe(i);
            
            // 确保新生成的管道在屏幕右侧有足够的距离，并且与其他管道保持合适间距
            unsigned int max_x = SCREEN_WIDTH + 10;
            for (unsigned int j = 0; j < MAX_PIPES; j++) {
                if (j != i && pipes[j].top.x > max_x) {
                    max_x = pipes[j].top.x;
                }
            }
            // 新管道位置至少在最右侧管道的右侧，且距离为屏幕宽度的三分之一
            pipes[i].top.x = max_x + (SCREEN_WIDTH / 3);
            pipes[i].bottom.x = pipes[i].top.x;
        }
    }

    
    // 检查小鸟是否与管道发生碰撞
    if (game_check_collision()) {
        game_state = GAME_OVER;     // 设置游戏结束状态
    }
}

/**
 * 渲染游戏画面
 * 
 * 函数根据当前游戏状态绘制不同的游戏画面：
 * - 准备状态：显示游戏标题和开始提示
 * - 运行状态：绘制背景、管道、小鸟和分数
 * - 结束状态：在运行状态画面上叠加游戏结束提示
 * 
 * 最后调用OLED_Update()将绘制内容显示到屏幕上
 * 
 * 内部函数 - 仅在模块内部调用
 */
static void game_render(void)
{
    // 游戏准备状态，只在初始帧绘制
    if (game_state == GAME_READY) {
        if (animation_frame == 0) {
            OLED_Clear();
            OLED_ShowString(30, 20, "FLAPPY BIRD", OLED_7X12_HALF);
            OLED_ShowString(15, 40, "Press UP to start", OLED_6X8_HALF);
            OLED_Update();
        }
        return;  // 准备状态下只显示静态画面
    }
    
    // 绘制游戏元素（运行状态和结束状态共用）
    draw_background();  // 绘制背景
    
    // 绘制所有有效管道
    for (unsigned int i = 0; i < MAX_PIPES; i++) {
        if (pipes[i].top.width > 0 && pipes[i].top.x < SCREEN_WIDTH) {
            draw_pipe(&pipes[i].top);     // 绘制顶部管道
            draw_pipe(&pipes[i].bottom);  // 绘制底部管道
        }
    }
    
    // 绘制小鸟
    draw_bird(bird.x, bird.y, animation_frame);
    
    // 显示当前分数在右上角
    // 计算适合右上角的位置，确保文本不会超出屏幕
    unsigned int score_str_width = 6 * 5; // "Score:"字符串宽度 (6像素宽字体 * 5个字符)
    unsigned int num_width = 6 * 3;       // 数字宽度 (6像素宽字体 * 3位数)
    unsigned int total_width = score_str_width + num_width + 2; // 加2个像素的间距
    unsigned int start_x = SCREEN_WIDTH - total_width; // 从右边缘计算起始位置
    
    OLED_ShowString(start_x, 0, "Score:", OLED_6X8_HALF);
    OLED_ShowNum(start_x + score_str_width + 2, 0, score, 3, OLED_6X8_HALF);
    
    // 游戏结束画面（在运行画面上叠加）
    if (game_state == GAME_OVER) {
        OLED_DrawRectangle(20, 20, 88, 24, OLED_UNFILLED);  // 绘制结束框
        OLED_ShowString(40, 24, "GAME OVER", OLED_6X8_HALF);
        OLED_ShowString(15, 38, "Press UP to retry", OLED_6X8_HALF);
    }
    
    OLED_Update();  // 将所有绘制内容更新到屏幕
}

/**
 * 检查碰撞
 * 
 * 函数检测小鸟是否与管道发生碰撞
 * 使用矩形碰撞检测算法，计算小鸟和管道的边界矩形是否相交
 * 
 * @return 如果发生碰撞返回true，否则返回false
 * 
 * 内部函数 - 仅在模块内部调用
 */
static bool game_check_collision(void)
{
    // 只有当管道有效时才检查碰撞
    for (unsigned int i = 0; i < MAX_PIPES; i++) {
        if (pipes[i].top.width > 0) {
            // 计算小鸟的边界矩形坐标
            int bird_left = bird.x - BIRD_WIDTH/2;
            int bird_right = bird.x + BIRD_WIDTH/2;
            int bird_top = bird.y - BIRD_HEIGHT/2;
            int bird_bottom = bird.y + BIRD_HEIGHT/2;
            
            // 检查与顶部管道的碰撞（矩形相交检测）
            if (bird_right > pipes[i].top.x && bird_left < pipes[i].top.x + pipes[i].top.width &&
                bird_bottom > pipes[i].top.y && bird_top < pipes[i].top.y + pipes[i].top.height) {
                return true;  // 发生碰撞
            }
            
            // 检查与底部管道的碰撞（矩形相交检测）
            if (bird_right > pipes[i].bottom.x && bird_left < pipes[i].bottom.x + pipes[i].bottom.width &&
                bird_bottom > pipes[i].bottom.y && bird_top < pipes[i].bottom.y + pipes[i].bottom.height) {
                return true;  // 发生碰撞
            }
        }
    }
    return false;  // 没有发生碰撞
}

/**
 * 重置游戏
 * 
 * 函数调用game_init()重新初始化游戏状态
 * 提供一个简单的接口供外部调用
 * 
 * 内部函数 - 仅在模块内部调用
 */
static void game_reset(void)
{
    game_init();  // 重新初始化游戏
}








