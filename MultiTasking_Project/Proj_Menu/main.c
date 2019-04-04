/*
 * Proj_Menu.c
 *
 * Created: 2016-11-27 오후 6:51:45
 * Author : Netpi
 */ 

#include <avr/io.h>
#define F_CPU 14745600UL
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "_main.h"
#include "_buzzer.h"
#include "_adc.h"
#include "_init.h"
#include "_port.h"
#include "_timer2.h"

#define ADC_VREF_TYPE 0x00
#define ADC_AVCC_TYPE 0x40
#define ADC_RES_TYPE 0x80
#define ADC_2_56_TYPE 0xC0

/* 전역변수 초기화 */
unsigned int menu_choice = 0; // 메뉴 선택
unsigned int page_value = 1; //  페이지
int rank[5] = {0,}; // 랭크 리스트
int score = 0; // 점수
int count = 0; // Polling 방식의 타이머
int endflag = 0; // 게임 Defeat flag
int jflag = 0; // JumpGame : Jump Flag 점프 속도 결정
int jumpIng = 0; // JumpGame : Jump-Ing Flag
int move = 0; // BalanceGame : move
int mode = 0; // 난이도

//Ship Game
// Ship Structure
struct Ship{ 
	int startX;
	int startY;
	int endX;
	int endY;
};

// Ship이 시간내에 도착해야할 Destination
struct Destination{
	int time;
	int startX;
	int startY;
	int endX;
	int endY;
};

// Structure 초기화
struct Ship ship = {31, 94, 33, 96};
struct Destination d = {0, 0, 0, 0, 0};

// Destination 생성 함수
void createDest(){
	int x = rand()%7; // line 0~7
	int y = rand()%47+63; // Y-axis 63~110

	/*
		Destination 정보
		가로 : 15 세로 : 15
		line별로 등장
	*/
	d.startX = x*8;
	d.endX = d.startX+15;
	d.startY = y;
	d.endY = y+15;
}

// 시간안에 Destination에 도달했는지 확인해주는 함수
int clearDest(){
	//ship이 Destination 안에 들어가면 성공
	if(ship.startX >= d.startX && ship.endX <= d.endX){
		if(ship.endY >= d.startY && ship.startY <= d.endY){
			return 1;
		}

		if(ship.startY >= d.endY && ship.endY <= d.startY){
			return 1;
		}
	}

	return 0;
}

// Ship 동작 함수
void moveShip(){
	// 조이스틱 ADC 입력
	int RLswitch = Read_Adc_Data(3)/14;
	int UDswitch = Read_Adc_Data(4)/8;
	int x, y;
	unsigned char ud;

	// Ship 속도 : Easy - 4 / Normal - 3 / Hard - 2 
	if(RLswitch<20 && ship.startX > 0){
		ship.startX -= 4 + mode;
		ship.endX -= 4 + mode;
		ud = 1;
	}
	else if(RLswitch>50 && ship.endX < 64){
		ship.startX += 4 - mode;
		ship.endX += 4 - mode;
		ud = 0;
	}

	if(UDswitch>90 && ship.endY < 126){
		ship.startY += 4 - mode;;
		ship.endY += 4 - mode;;
	}
	else if(UDswitch<20 && ship.startY > 63){
		ship.startY -= 4 + mode;;
		ship.endY -= 4 + mode;;
	}
	
	// GLCD_Ship - 기존의 GLCD_Dot의 Overlapping 문제를 해결한 1x3 Dots 생성함수
	for(y = ship.startY; y <= ship.endY; y++) {
		GLCD_Ship(ship.startX, y);
	}
}

// Ship 게임 시작
void startShipGame(){
	moveShip();

	// count 20 = 1초 따라서, 100 = 5초
	if(count > 100){
		count = 0;
	}

	// delay = 50ms이므로 count 20 = 1초
	else{
		if(count%20==0){
			d.time--; // 5초부터 1초씩 깎임
			score++; // 1초에 1점씩
			if(d.time == 0){
				endflag = 1; // 만약 0초때까지 Destination에 도달하지 못하면 패배
			}
		}
		count++;
	}

	// 시간안에 Destination에 도달하면 Destination 정보 새로 입력
	if(clearDest()){
		createDest();
		d.time=5;
	}

	// 남은 시간 오른쪽 위 lcd에 출력
	lcd_xy(0, 16); GLCD_4DigitDecimal(d.time);
	// Destination 생성
	GLCD_Rectangle_black(d.startX, d.startY, d.endX, d.endY);
}

// Ship 게임 초기화
void initShipGame(){
	createDest(); // Destination 생성
	d.time=5; // Destination 생존시간 5초
}
//Ship Game End

//Balance Game
// Ball Structure
struct Ball{
	int x;
	int y;
	int r;
};

// Bar Structure
struct Bar{
	int startX;
	int startY;
	int endX;
	int endY;
};

// Structure 초기화
struct Ball ball = {13, 32, 2};
struct Bar bar = {17, 10, 17, 53};

// Bar의 Movement를 조절하는 함수
void moveBar(){
	// 가속도 센서 입력 (Y)
	int acc = Read_Adc_Data(6)-358+62;

	// Overlap 되는 이전 정보를 지워주는 함수
	GLCD_Remove_Line(bar.startX, bar.startY, bar.endX, bar.endY);
	GLCD_Remove_Circle(ball.x, ball.y, ball.r);
	
	// 왼쪽으로 기울면 '왼쪽' 오른쪽으로 기울면'오른쪽'으로 Ball과 Bar를 Control
	if(acc > 150){
		if(move < 12 - mode){ // 최대 경계 점 : Easy -  ±12 / Normal - ±11 / Hard - ±10
			move++;
		}
		else{
			endflag = 1;
		}
	}

	else{
		if(move > -12 + mode){
			move--;
		}
		else{
			endflag = 1;
		}
	}

	if(move >= 0){
		bar.startX = 17 + move;
		bar.startY = 10 + move;
		bar.endX = 17 - move;
		bar.endY = 53 - move;

		ball.x = 13 + move;
		ball.y = 32 - move;
	}

	else{
		bar.startX = 17 + move;
		bar.startY = 10 - move;
		bar.endX = 17 - move;
		bar.endY = 53 + move;

		ball.x = 13 - move;
		ball.y = 32 - move;
	}
}

// Balance 게임 시작함수
void startBalanceGame(){
	moveBar();

	// Object 생성
	GLCD_Line(bar.startX, bar.startY, bar.endX, bar.endY);
	GLCD_Circle(ball.x, ball.y, ball.r);
}
//Balance Game End

//Jump Game
// Jumper Structure
struct Jumper{
	int startX;
	int startY;
	int endX;
	int endY;
};

// Ground Structure
struct Ground{
	int start;
	int end;
};

// Structure 초기화
struct Jumper jumper = {57, 10, 59, 12};
struct Ground grd1 = {0, 0};
struct Ground grd2 = {0, 63};

// 바닥을 움직이는 함수
void moveGround(){
	// 25ms 마다 왼쪽으로 이동
	grd1.end--;
	grd2.start--;

	// 구멍 생성
	if(grd1.end < 0){
		grd1.end = 60 - mode; // 간격 : Easy - 3 / Normal - 4 / Hard - 5
		grd2.start = 63;
	}

	// 바닥을 그려줌
	GLCD_Line(60, grd1.start, 60, grd1.end);
	GLCD_Line(60, grd2.start, 60, grd2.end);
}

// Jumper의 Jump속도를 조절하는 함수
void jump(int jflag){
	//0~4까지 상승 5~9까지 하강
	switch(jflag){
		case 0:
		jumper.startX -= 4;
		jumper.endX -= 4;
		break;
		case 1:
		jumper.startX -= 3;
		jumper.endX -= 3;
		break;
		case 2:
		jumper.startX -= 2;
		jumper.endX -= 2;
		break;
		case 3:
		jumper.startX -= 1;
		jumper.endX -= 1;
		break;
		case 4:
		jumper.startX -= 1;
		jumper.endX -= 1;
		break;
		case 5:
		jumper.startX += 1;
		jumper.endX -= 1;
		break;
		case 6:
		jumper.startX += 1;
		jumper.endX -= 1;
		break;
		case 7:
		jumper.startX += 2;
		jumper.endX -= 2;
		break;
		case 8:
		jumper.startX += 3;
		jumper.endX -= 3;
		break;
		case 9:
		jumper.startX += 4;
		jumper.endX -= 4;
		break;
	}
}

// Jumper가 Jump를 성공했는지 확인하는 함수
int clearJump(){
	if(jumper.startY >= grd1.end){
		if(jumper.endY <= grd2.start){
			if(jumpIng == 0){
				endflag = 1;
			}
		}
	}
}

// Jump Game 시작 함수
void startJumpGame(){
	int y;
	// 조도 센서를 입력
	int cds = Read_Adc_Data(2);

	// 조도 센서를 가릴때마다 점프
	if(cds > 20 && jflag == 0){
		jumpIng = 1;
	}

	// 점프 중에는 조작이 불가능함
	if(jumpIng == 1){
		jump(jflag);
		jflag++;

		if(jflag == 10){
			jflag = 0;
			jumpIng = 0;
		}
	}

	// GLCD_Ship을 이용해 Jumper를 생성
	for(y = jumper.startY; y <= jumper.endY; y++) {
		GLCD_Ship(jumper.startX, y);
	}

	// Ground 생성
	moveGround();

	// Jumper의 성공여부 확인
	clearJump();
}
//Jump Game End


void write_rank(void)
{
	int i, j;
	int buf[5] = {0,}; // 기존 rank값을 기억해 둘 배열

	for( i = 0 ; i < 5 ; i++ )
	buf[i] = rank[i]; // 기존에 존재하던 rank값을 buf에 임시저장

	for( i = 0 ; i < 5 ; i++ )
	{
		if(rank[i] < score)
		{
			for(j = i ; j < 4 ; j++)
			{
				rank[j+1] = buf[j]; // rank 의 순위를 1순위씩 아래로 내린다.
			}
			rank[i] = score; // 알맞은 자리에 score 값을 rank 시킨다.
			score = 0; // 스코어를 초기화한다.
			return; // rank가 갱신 되었으니 함수를 종료한다.
		}
	}
}

void start_game(void)
{
	while (1)
	{
		lcd_clear(); // lcd 클리어
		GLCD_Line(32, 0, 32, 63); // 게임판이 될 Border 설정
		GLCD_Line(0,63,64,63);    //
		startShipGame(); // ShipGame 시작
		startJumpGame(); // JumpGame 시작
		startBalanceGame(); // BalacneGame 시작
		// 하나라도 실패한다면 실패
		if(endflag == 1){
			SError(); // 패배 Sound 출력
			lcd_clear();
			lcd_string(4,2, "==== Defeat ====");
			break;
		}
		_delay_ms(25); // 25ms 간격으로 게임을 처리
	}
	_delay_ms(3000);

	resetGame(); // 게임 리셋
	write_rank(); // 랭킹 업데이트
	screen_display(); // 디스플레이 리셋
}

void resetGame(){
	jumper.startX = 57;
	jumper.startY = 10;
	jumper.endX = 59;
	jumper.endY = 12;
	grd1.start = 0;
	grd1.end = 0;
	grd2.start = 0;
	grd2.end = 63;
	d.time = 5;
	count = 0;
	endflag = 0;
	jflag = 0;
	jumpIng = 0;
	move = 0;
}

void screen_display(void)
{
	lcd_clear();
	lcd_string(1,2, "==MULTITASKING==");
	lcd_string(4,5,"Start Game");
	lcd_string(5,5,"Set Level");
	lcd_string(6,5,"Rank");
	menu_choice = 4;
	page_value = 1;
	lcd_string(menu_choice, 3, ">");
}


void menu_select(void)
{
	unsigned Data_ADC4=Read_Adc_Data(4) / 8;
	_delay_ms(30);
	
	if(Data_ADC4>90 && menu_choice < 6) // 메뉴를 아래로
	{
		menu_choice++; // 메뉴위치 한칸 증가
		lcd_string(menu_choice, 3, ">");
		lcd_string(menu_choice - 1, 3, " "); // 이전 위치 화살표 삭제

		_delay_ms(100);
	}
	else if(Data_ADC4<20 && menu_choice > 4) // 메뉴를 위로
	{
		menu_choice--; // 메뉴 위치 한칸 감소
		lcd_string(menu_choice, 3, ">");
		lcd_string(menu_choice + 1, 3, " "); // 이전 위치 화살표 삭제
		_delay_ms(100);
	}
}

void level_menu_lcd(void)
{
	lcd_clear();
	page_value = 2;
	menu_choice = 4;
	lcd_string(1,2, "==Set Level==");
	lcd_string(4,5,"Easy");
	lcd_string(5,5,"Normal");
	lcd_string(6,5,"Hard");
	lcd_string(menu_choice, 3, ">");
}

void rank_menu_lcd(void)
{
	lcd_clear();
	page_value = 3;
	menu_choice = 7;
	lcd_string(1,1,"Rank 1 : "); lcd_xy(1, 10); GLCD_4DigitDecimal(rank[0]);
	lcd_string(2,1,"Rank 2 : "); lcd_xy(2, 10); GLCD_4DigitDecimal(rank[1]);
	lcd_string(3,1,"Rank 3 : "); lcd_xy(3, 10); GLCD_4DigitDecimal(rank[2]);
	lcd_string(4,1,"Rank 4 : "); lcd_xy(4, 10); GLCD_4DigitDecimal(rank[3]);
	lcd_string(5,1,"Rank 5 : "); lcd_xy(5, 10); GLCD_4DigitDecimal(rank[4]);
	lcd_string(6,1,"<Exit=Button Press>");
}

void select_menu(void)
{
	byte key = 0;

	key = (PIND&0xff);
	                                                                         
	if((key == 254) && menu_choice == 4 && page_value == 1)
	{
		S_Start();
		start_game();
		_delay_ms(1000);
	}

	else if((key == 254) && menu_choice == 5 && page_value == 1)
	{
		S_Good();
		level_menu_lcd();
		_delay_ms(1000);
	}

	else if((key == 254) && menu_choice == 6 && page_value == 1)
	{
		S_Good();
		rank_menu_lcd();
		_delay_ms(1000);
	}

	else if((key == 254) && menu_choice == 4 && page_value == 2)
	{
		lcd_clear();
		lcd_string(3,3, "Easy Level");
		lcd_string(4,5, "Selected");
		// 난이도 값 설정
		mode = 0;
		_delay_ms(3000);
		screen_display();
	}

	else if((key == 254) && menu_choice == 5 && page_value == 2)
	{
		lcd_clear();
		lcd_string(3,3, "Normal Level");
		lcd_string(4,5, "Selected");

		// 난이도 값 설정
		mode = 1;
		_delay_ms(3000);
		screen_display();
	}

	else if((key == 254) && menu_choice == 6 && page_value == 2)
	{
		lcd_clear();
		lcd_string(3,3, "Hard Level");
		lcd_string(4,5, "Selected");
		// 난이도 값 설정
		mode = 2;
		_delay_ms(3000);
		screen_display();
	}

	else if((key == 254) && menu_choice == 7 && page_value == 3)
	{
		lcd_clear();
		screen_display();
		_delay_ms(1000);
	}
}

int main(void)
{
	init_devices();
	initShipGame();
	screen_display();

    /* Replace with your application code */
    while (1) 
    {
		menu_select(); // 조이스틱 조정
		select_menu(); // 메뉴 선택 화면
		if(page_value==3)
			menu_choice=7; // rank메뉴에서 menu_choice값을 7로 고정
    }
	
}

