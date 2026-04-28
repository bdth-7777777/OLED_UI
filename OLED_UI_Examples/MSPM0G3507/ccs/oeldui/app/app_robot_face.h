/**
 * app_robot_face.h
 * Robot face expression interface — expression types, parameter struct, and lifecycle callbacks.
 */
#ifndef __APP_ROBOT_FACE_H__
#define __APP_ROBOT_FACE_H__

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    int16_t eye_width;
    int16_t eye_height;
    int16_t eye_radius;
    int16_t eye_spacing;
    int16_t eye_offset_y;
    int16_t brow_offset_y;
    int16_t brow_angle_l;
    int16_t brow_angle_r;
    int16_t mouth_width;
    int16_t mouth_height;
    int16_t mouth_curve;
    int16_t left_eye_h_pct;
    int16_t right_eye_h_pct;
} RobotExpression;

typedef enum {
    EXPR_NORMAL = 0,     // 普通/平静
    EXPR_HAPPY,         // 开心/高兴
    EXPR_SAD,           // 伤心/悲伤
    EXPR_ANGRY,         // 生气/愤怒
    EXPR_SURPRISED,     // 惊讶/吃惊
    EXPR_SLEEPY,        // 困倦/想睡觉
    EXPR_SUSPICIOUS,    // 怀疑/疑惑
    EXPR_LOVE,          // 喜爱/爱慕
    EXPR_CAUTIOUS,      // 谨慎/小心
    EXPR_CRYING,        // 哭泣/流泪
    EXPR_SMIRK,         // 傻笑/坏笑
    EXPR_AWKWARD,       // 尴尬/窘迫
    EXPR_SHY,           // 害羞/羞涩
    EXPR_DIZZY,         // 晕眩/迷糊
    EXPR_PROUD,         // 得意/骄傲
    EXPR_CRAZY,         // 疯狂/抓狂
    EXPR_HELPLESS,      // 无奈/无语
    EXPR_SHOCKED,       // 吃惊/震惊
    EXPR_SLEEPING,      // 睡觉/熟睡
    EXPR_BORED,         // 无聊/打哈欠
    EXPR_EXPECT,        // 期待/期望
    EXPR_COZY,          // 安逸/舒适
    EXPR_SIGH,          // 叹气/唉
    EXPR_DISDAIN,       // 嫌弃/起鸡皮疙瘩
    EXPR_CHEWING,       // 吃东西/咀嚼
    EXPR_DISGUST,       // 讨厌/嫌恶
    EXPR_HESITANT,      // 犹豫/纠结
    EXPR_COUNT
} ExpressionType;

void app_robot_face_init(void);
void app_robot_face_tick(void);
bool app_robot_face_should_exit(void);
void app_robot_face_on_exit(void);

#endif
