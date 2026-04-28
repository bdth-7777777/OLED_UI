#ifndef __HW_BUZZER_H__
#define __HW_BUZZER_H__

#ifdef __cplusplus
extern "C"
{
#endif
#include "ti_msp_dl_config.h"
#include <math.h>

	/*音调频率表对应的每一个音符名称*/
	typedef enum Musical_Note
	{
		// 休止符
		REST_NOTE,
		NOTE_C4,
		NOTE_D4b,
		NOTE_D4,
		NOTE_E4b,
		NOTE_E4,
		NOTE_F4,
		NOTE_G4b,
		NOTE_G4,
		NOTE_A4b,
		NOTE_A4,
		NOTE_B4b,
		NOTE_B4,
		NOTE_C5,
		NOTE_D5b,
		NOTE_D5,
		NOTE_E5b,
		NOTE_E5,
		NOTE_F5,
		NOTE_G5b,
		NOTE_G5,
		NOTE_A5b,
		NOTE_A5,
		NOTE_B5b,
		NOTE_B5,
		NOTE_C6,
		NOTE_D6b,
		NOTE_D6,
		NOTE_E6b,
		NOTE_E6,
		NOTE_F6,
		NOTE_G6b,
		NOTE_G6,
		NOTE_A6b,
		NOTE_A6,
		NOTE_B6b,
		NOTE_B6,
		NOTE_C7,
		NOTE_D7b,
		NOTE_D7,
		NOTE_E7b,
		NOTE_E7,
		NOTE_F7,
		NOTE_G7b,
		NOTE_G7,
		NOTE_A7b,
		NOTE_A7,
		NOTE_B7b,
		NOTE_B7,
		NOTE_C8,
		NOTE_D8b,
		NOTE_D8,
		NOTE_E8b,
		NOTE_E8,
		NOTE_F8,
		NOTE_G8b,
		NOTE_G8,
		NOTE_A8b,
		NOTE_A8,
		NOTE_B8b,
		NOTE_B8,
		// 检查位
		CHECK_NOTE,
	} note_t;

	/*音符的结构体*/
	typedef struct
	{
		note_t Note;	// musical_note
		uint16_t Delay; // ms
	} TONE;

	/*蜂鸣器的结构体*/
	typedef struct
	{
		uint8_t Beeper_Enable;		  // 开启标志位
		uint8_t Beeper_Continue_Flag; // 执行标志位
		uint16_t Beeper_Count;		  // 音符长度
		uint16_t Sound_Size;		  // 乐曲长度
		uint16_t Beep_Play_Schedule;  // 当前所在的乐曲位置
		uint8_t Sound_Loud;			  // 蜂鸣器响度
	} BEEPER_Tag;

	extern TONE *MySound;
	extern const TONE BEEPER_KEYPRESS[];
	extern const TONE BEEPER_TRITONE[];
	extern const TONE BEEPER_WARNING[];
	extern const TONE BEEP1[];
	extern const TONE BEEP2[];
	extern BEEPER_Tag Beeper0;

	/*音调频率表*/
	extern const uint16_t MusicNoteFrequency[];

	/*底层函数*/
	void TIM9_PWM_Init(uint16_t arr);
	void Beeper_Init(void);
	uint16_t Set_Musical_Note(uint16_t frq);
	void Beeper_Set_Musical_Tone(uint16_t frq);

	/*应用层函数*/
	void Beeper_Perform(const TONE *Sound);

	/* 定时器中断处理函数 */
	void Beeper_Proc(void);

#ifdef __cplusplus
}
#endif

#endif
