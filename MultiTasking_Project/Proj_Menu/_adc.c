
#include <avr/io.h>
#define F_CPU 14745600UL
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "_main.h"
#include "_adc.h"

#define ADC_VREF_TYPE 0x00      // A/D 컨버터 사용 기준 전압  REFS 설정
#define ADC_AVCC_TYPE 0x40      // A/D 컨버터 사용 기준 전압  AVCC 설정
#define ADC_RES_TYPE  0x80      // A/D 컨버터 사용 기준 전압  RES 설정
#define ADC_2_56_TYPE 0xC0      // A/D 컨버터 사용 기준 전압  2.56 설정

//ADC initialize
void Adc_init(void)
{
	ADCSRA = 0x00;	// disable adc
	ADMUX  = 0x00; 	// select adc input 0
	
	ACSR   = 0x80;	//
	ADCSRA = 0xC3;	// Conversion time: 6uS
}

unsigned int Read_Adc_Data(unsigned char adc_input)   // ADC  읽어 들이기
{
	unsigned int adc = 0;
	
	ADCSRA = 0xC3;    // Conversion time: 6uS
	//CLI();
	
	ADMUX = adc_input | ADC_AVCC_TYPE ;      // adc  사용 핀 설정
	//ADMUX.3 = 1;
	
	ADCSRA |= 0x40;                            // Wait for the AD conversion to complete
	
	while((ADCSRA & 0x10) != 0x10);
	adc += ADCL + ( ADCH * 256 );
	
	
	//SEI();
	ADCSRA = 0x00;
	return adc;
}