
#include <avr/io.h>
#define F_CPU 14745600UL
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "_main.h"
#include "_timer2.h"

// 타이머 2 관련 변수
unsigned int Count_Of_Timer2 = 0;	// 카운터를 더하는데 사용합니다.
unsigned int Tesk1_Of_Timer2 = 0;	// 일을 실행 해고자 할때 정확한 타이밍에 사용 합니다.
unsigned int Time_Of_Timer2 = 500;	// 실제 타이머의 시간을 만듭니다.

// TIMER2 initialize - prescale:64
// WGM: Normal
// desired value: 1mSec
// actual value:  0.998mSec (0.2%)  0.998246
void Timer2_init(void)	// 타이머 2 초기화
{
	TCCR2 = 0x00; //stop    // 타이머/카운터2 제어 레지스터/
	TCNT2 = 0x1A; //setup   // 타이머/카운터2 레지스터/
	TCCR2 = 0x03; //start   // 0b00000100 64분주비 사용
	TIMSK = 0x40;           // 오버플로우(TOV2) 인터럽트 허가
}

SIGNAL(TIMER2_OVF_vect)
{
	TCNT2 = 0x1A;                           // 26 부터 256까지 	230 X 4번의 카운터
	// setup (4.3402uS)
	Count_Of_Timer2++; 						// 카운터 값을 더합니다.

	if(Count_Of_Timer2 == Time_Of_Timer2)   // 1ms×500 = 500ms
	{
		Tesk1_Of_Timer2 = 1;				// 500ms 가 되면 Tesk1_Of_Timer2 의 변수가 1이 됩니다.
		Count_Of_Timer2 = 0;				// 카운터 값을 초기화 합니다.
	}
}