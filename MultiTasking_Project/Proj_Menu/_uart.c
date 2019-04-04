
#include <avr/io.h>
#define F_CPU 14745600UL
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "_main.h"
#include "_uart.h"

// 시리얼 통신 1의 변수
unsigned char command = 0;			// 시리얼 통신 내용의 그에 따르는 내용을 할수 있도록 하는 변수
unsigned char InputSirialData = 0;  // UDR버퍼의 내용을 잠시 보관하는 변수
unsigned char Uart1_State = 0;  	// Key 값의 마지막을 기록 합니다.

char Enter[]={"\r\n"};          	// 엔터 값
char Tap[]={"\t"};              	// 탭 값


void Uart1_init(void)	// 시리얼 통신 초기화
{
	
	UCSR1A = 0x00;	// 1배속 전송모드
	UCSR1C = 0x06;	// 비동기 모드의 선택
	
	UCSR1B = 0x98;
	// RXCIE1=1(수신 인터럽트 설정),
	// RXEN1=1(수신 허가), // TXEN1 = 1(송신 허가)
	
	UBRR1L = 0x07; //set baud rate lo
	UBRR1H = 0x00; //set baud rate hi

	//UART1 initialize
	// desired baud rate:115200
	// actual baud rate:115200 (0.0%)
	// char size: 8 bit
	// parity: Disabled

}

void putch_USART1(char data)			// 1byte 데이터 uart1으로 전송
{
	while( !(UCSR1A & (1<<UDRE1))); 	// 전송 인터럽트가 걸릴 때까지 while문 반복
	UDR1 = data; 						// 데이터 값을 UDR1에 넣는다. = 전송
}

void puts_USART1(char *str)				// 문자열 출력 루틴
{
	while( *str != 0)		 			// 문자의 마지막에는 ‘\0’이 들어가 있으므로
	{
		putch_USART1(*str);				// ‘\0’이 나올 때까지 출력한다.
		str++;
	}
}

// 아스키 값을 출력 하기 위한 테이블 작성
unsigned char TABLE[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

void USART1_putchdecu(unsigned int dt)	// unsigned int 형 0~62536까지 표현 가능함 .
{
	unsigned int tmp;				// 만약 3245 값이 입력 되어 진다면 -> int형이기 때문에 03245가 됩니다.
	tmp = dt;
	putch_USART1(TABLE[tmp/10000]);	// 03245/10000=0.3256 -> 은 int형이기 때문에 0만 남게되며 TABLE의 첫번째 자리의 0이 출력 됩니다.
	tmp %= 10000;					// 03245/10000의 나머지 3256을 tmp에 저장합니다.
	putch_USART1(TABLE[tmp/1000]);	// 3245/1000=3.256 -> 3만 남게되며 TABLE의 두번째 자리의 3이 출력 됩니다.
	tmp %= 1000;					// 3245/1000의 나머지 245을 tmp에 저장합니다.
	putch_USART1(TABLE[tmp/100]);	// 245/100=2.45 -> 2만 남게되며 TABLE의 두번째 자리의 2가 출력 됩니다.
	tmp %= 100;						// 245/100의 나머지 45를 tmp에 저장합니다.
	putch_USART1(TABLE[tmp/10]);	// 45/10=4.5 -> 은 4만 남게되며 TABLE의 두번째 자리의 4가 출력 됩니다.
	putch_USART1(TABLE[tmp%10]);	// 나머지 5를 출력 합니다.
}

void USART1_putchuchar(unsigned char dt)	// unsigned char 형 0 ~ 255까지 표현 가능함 .
{
	unsigned char tmp;
	tmp = dt;
	putch_USART1(TABLE[tmp/100]);
	tmp %= 100;
	putch_USART1(TABLE[tmp/10]);
	putch_USART1(TABLE[tmp%10]);
}

unsigned int H2C(unsigned char ch) 		// 케릭터 를 HEX 로 변경 하는구문
{
	unsigned char high, low;
	unsigned int s;

	high = ch >> 4;
	low = ch & 0x0f;
	s = TABLE[high];
	s <<= 8;
	s |= TABLE[low];

	return s;
}


void USART1_putchdecs(signed int dt)	// unsigned int 형 -32768 ~ 32767까지 표현 가능함 .
{
	signed int tmp;
	tmp = dt;
	if(tmp >= 0) {
		putch_USART1('+');
		putch_USART1(TABLE[tmp/10000]);
		tmp %= 10000;
		putch_USART1(TABLE[tmp/1000]);
		tmp %= 1000;
		putch_USART1(TABLE[tmp/100]);
		tmp %= 100;
		putch_USART1(TABLE[tmp/10]);
		putch_USART1(TABLE[tmp%10]);
	}
	else {
		tmp = -tmp;
		putch_USART1('-');
		putch_USART1(TABLE[tmp/10000]);
		tmp %= 10000;
		putch_USART1(TABLE[tmp/1000]);
		tmp %= 1000;
		putch_USART1(TABLE[tmp/100]);
		tmp %= 100;
		putch_USART1(TABLE[tmp/10]);
		putch_USART1(TABLE[tmp%10]);
	}
}

void USART1_putchlongs(long dt)		// long 형 -2147483648 ~ 2147483647까지 표현 가능함 .
{
	long tmp;
	tmp = dt;
	if(tmp >= 0) {
		putch_USART1('+');
		putch_USART1(TABLE[tmp/10000000]);
		tmp %= 10000000;
		putch_USART1(TABLE[tmp/1000000]);
		tmp %= 1000000;
		putch_USART1(TABLE[tmp/100000]);
		tmp %= 100000;
		putch_USART1(TABLE[tmp/10000]);
		tmp %= 10000;
		putch_USART1(TABLE[tmp/1000]);
		tmp %= 1000;
		putch_USART1(TABLE[tmp/100]);
		tmp %= 100;
		putch_USART1(TABLE[tmp/10]);
		putch_USART1(TABLE[tmp%10]);
	}
	else {
		tmp = -tmp;
		putch_USART1('-');
		putch_USART1(TABLE[tmp/10000000]);
		tmp %= 10000000;
		putch_USART1(TABLE[tmp/1000000]);
		tmp %= 1000000;
		putch_USART1(TABLE[tmp/100000]);
		tmp %= 100000;
		putch_USART1(TABLE[tmp/10000]);
		tmp %= 10000;
		putch_USART1(TABLE[tmp/1000]);
		tmp %= 1000;
		putch_USART1(TABLE[tmp/100]);
		tmp %= 100;
		putch_USART1(TABLE[tmp/10]);
		putch_USART1(TABLE[tmp%10]);
	}
}


void USART1_puthex(unsigned char dt)		// 핵사 파일을 출력으로 나타 냅니다.
{
	unsigned int  tmp=0;
	tmp = H2C(dt);
	putch_USART1((unsigned char)(tmp>>8));
	putch_USART1((unsigned char)(tmp));
}

SIGNAL(USART0_RX_vect)	//시리얼 인터럽트 발생시 다음 구문으로 자동 들어옴 avrstudio
{
	cli(); //disable all interrupts
	
	InputSirialData = UDR1 ;	// UDR 레지스터로 data 수신

	if(InputSirialData == '1') 		{	command = '1';	}	// HELP 명령 수신시
	else if(InputSirialData == '2')	{	command = '2';	}	// Port Test
	
	sei(); //re-enable interrupts
}


void Serial_Main(void) // 시리얼 통신 메인 구문
{
	if(command=='1')	// Help 명령 수신시
	{	Uart1_State = '1';
		////////////////////////////////////////////////////////////////////
		// 키보드1 값을 입력 받았을 경우  동작을 만듭니다.
		PORTB = 0x00;
		//putch_USART1('a');
		Uart1_DataView = 0x00;
		
		//-----------------------------------------------------------------//
		command = 0; 		// 시리얼 통신 내용의 그에 따르는 내용을 할수 있도록 하는 변수를 초기화
		InputSirialData=0;	// UDR 버퍼의 내용을 잠시 보관하는 변수를 초기화

	}	/////////////////////////////////////////////////////////////////// end else

	else if(command=='2')	// Port Test	포트의 레지스터를 확인합니다.
	{
		Uart1_State = '2';
		////////////////////////////////////////////////////////////////////
		// 키보드2 값을 입력 받았을 경우  동작을 만듭니다.
		PORTB = 0xFF;
		//putch_USART1('b');
		Uart1_DataView = 0xFF;
		
		//-----------------------------------------------------------------//
		command = 0; 		// 시리얼 통신 내용의 그에 따르는 내용을 할수 있도록 하는 변수를 초기화
		InputSirialData=0;	// UDR 버퍼의 내용을 잠시 보관하는 변수를 초기화

	}	/////////////////////////////////////////////////////////////////// end else

}