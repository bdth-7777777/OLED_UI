/**
 * mid_music.c
 * Buzzer music driver — note frequency table, song playback, and timer-based note sequencing.
 */
# include "mid_music.h"
# include "hw_buzzer.h"

/*创建Beeper的handle*/
mid_beeper_t mid_beeper0;

/*C4-B8的音调对应的频率大小*/
const uint16_t music_note_frequency[] = {
    // rest_note
    0,
    // C    C#   D   Eb    E    F    F#   G    G#   A   Bb    B
    262,
    277,
    294,
    311,
    330,
    349,
    370,
    392,
    415,
    440,
    466,
    494,
    523,
    554,
    587,
    622,
    659,
    698,
    740,
    784,
    830,
    880,
    932,
    988,
    1047,
    1109,
    1175,
    1245,
    1319,
    1397,
    1480,
    1568,
    1661,
    1760,
    1865,
    1976,
    2093,
    2218,
    2349,
    2489,
    2637,
    2794,
    2960,
    3136,
    3322,
    3520,
    3729,
    3951,
    4186,
    4435,
    4699,
    4978,
    5274,
    5588,
    5920,
    6272,
    6645,
    7040,
    7459,
    7902,
    // check_note
    0,
};

/*全局tone_t结构体指针，用于定时器TIM4中断函数中*/
tone_t *current_sound;

/*乐曲*/
tone_t const song_keypress[] = {
    {NOTE_C6, 7},
    {CHECK_NOTE, 0}, // 检查位
};

tone_t const song_tritone[] = {
    {NOTE_B5, 7},
    {REST_NOTE, 2},
    {NOTE_D6, 6},
    {REST_NOTE, 2},
    {NOTE_F6, 6},
    {CHECK_NOTE, 0}, // 检查位
};

tone_t const song_warning[] = {
    {NOTE_F4, 5},
    {REST_NOTE, 2},
    {NOTE_F4, 5},
    {CHECK_NOTE, 0}, // 检查位
};

tone_t const song_boot[] = {
    {NOTE_C5, 11},
    {REST_NOTE, 2}, // 休止符
    {NOTE_C5, 11},
    {REST_NOTE, 2},
    {NOTE_G5, 11},
    {REST_NOTE, 2},
    {NOTE_G5, 11},
    {REST_NOTE, 2},
    {NOTE_A5, 11},
    {REST_NOTE, 2},
    {NOTE_A5, 11},
    {REST_NOTE, 2},
    {NOTE_G5, 22},
    {REST_NOTE, 2},
    {NOTE_F6, 11},
    {REST_NOTE, 2},
    {NOTE_D8, 11},
    {REST_NOTE, 2},
    {NOTE_C5, 11},
    {CHECK_NOTE, 0}, // 检查位
};

tone_t const song_startup[] = {
    {NOTE_C5, 10},
    {NOTE_D5, 10},
    {NOTE_C5, 10},
    {NOTE_D5, 10},
    {NOTE_E5, 20},
    {NOTE_C5, 10},
    {NOTE_B4, 10},

    {NOTE_A4, 20},
    {NOTE_D5, 20},
    {NOTE_C5, 10},
    {NOTE_B4, 10},
    {REST_NOTE, 10},
    {NOTE_A4, 5},
    {NOTE_B4, 5},

    {NOTE_C5, 20},
    {NOTE_A4, 20},
    {NOTE_E5, 20},
    {NOTE_C5, 20},

    {NOTE_D5, 20},
    {NOTE_A5, 20},
    {NOTE_G5, 20},
    {NOTE_F5, 10},
    {NOTE_E5, 5},
    {NOTE_D5, 5},

    {NOTE_E5, 80},

    {CHECK_NOTE, 0}, // 检查位
};



void mid_beeper_init(void)
{
    //需设置PWM的定时器频率为1MHz
    buzzer_off();// 先关掉防止蜂鸣器怪叫
    /* BEEPER使能标志位 */
    mid_beeper0.enable = 1;
    mid_beeper0.continue_flag = 0;

    mid_beeper0.sound_loud = 20;
}

/*计算对应的预重装值 用 1000kHz / 音调频率 */
uint16_t set_musical_note(uint16_t frq)
{
    /*防止休止符时蜂鸣器怪叫*/
    if (frq == 0)
        return 0;
    float temp = 0;
    temp = 1000000.0f / (float)frq;
    return (uint16_t)temp;
}

/**
 * @brief Beeper的应用层函数
 * @param  tone_t *sound 传入结构体数组
 * @retval 无
 */
void mid_beeper_perform(const tone_t *sound)
{
    /*该变量用于计算结构体数组的长度*/
    uint16_t note_length;

    buzzer_off();

    /*让全局结构体指针指向传入的乐曲*/
    current_sound = (tone_t *)sound;

    /*通过寻找检查位CHECK_NOTE来计算传入的结构体长度//因为sizeof是在编译中完成的所以这里没法用*/
    for (note_length = 0; current_sound[note_length].note != CHECK_NOTE; note_length++)
        ;

    /*赋予长度大小*/
    mid_beeper0.sound_size = note_length;
    /*把音符表清零*/
    mid_beeper0.play_schedule = 0;

    /*开启蜂鸣器继续标志位*/
    mid_beeper0.continue_flag = 1;
    mid_beeper0.count = 0;
}

/* 用于10ms定时器中断进行循环 */
void mid_beeper_proc(void)
{
    /*判断是否继续*/
    if (mid_beeper0.continue_flag && mid_beeper0.enable)
    {
        /*判断音符表走完没*/
        if (mid_beeper0.play_schedule <= mid_beeper0.sound_size)
        {
            /*时间减短10ms*/
            mid_beeper0.count--;
            /*这个操作的意思是如果count = 65535时，意思就是延时结束了，这个音符演完了*/
            if (!(mid_beeper0.count < 65535))
            {
                // printf("Ps:%d ", mid_beeper0.play_schedule);
                /*给预重装载值赋值，改变音调*/
                uint16_t arr = (uint16_t)set_musical_note(music_note_frequency[current_sound[mid_beeper0.play_schedule].note]);
                buzzer_set_reload_value(arr);

                /*给PWM占空比赋值，改变音量*/
                buzzer_set_duty((uint16_t)arr / (100 - mid_beeper0.sound_loud));
                /*赋值新的延时长度给count*/
                mid_beeper0.count = current_sound[mid_beeper0.play_schedule].delay;
                /*音符表走到下一个音符*/
                mid_beeper0.play_schedule++;

                buzzer_on();
            }
        }
        /*失能蜂鸣器，清空标志位*/
        else
        {
            buzzer_off();
            mid_beeper0.continue_flag = 0;
        }
    }
    else
    {
        buzzer_off();
        mid_beeper0.continue_flag = 0;
    }
}
