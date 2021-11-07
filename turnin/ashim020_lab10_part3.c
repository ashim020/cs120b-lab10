/*	Author: Andrew Shim
 *  	Partner(s) Name: 
 *	Lab Section: 21
 *	Assignment: Lab # 10 Exercise # 3
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 *	
 * 	Demo: https://drive.google.com/file/d/1s036ZLr05XhsO6tLPRrJMP7prg02b_JQ/view?usp=sharing
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

enum BL_States {BL_SMStart, BL_LedOff, BL_LedOn} BL_State;
enum TL_States {TL_SMStart, TL_T0, TL_T1, TL_T2} TL_State;
enum CL_States {CL_SMStart, CL_Combine} CL_State;
enum Speaker_States {Speaker_Start, Speaker_Off, Speaker_On} Speaker_State;

volatile unsigned char TimerFlag = 0;

unsigned char tmp_TL = 0x00;
unsigned char tmp_BL = 0x00;
unsigned char tmp_Speaker = 0x00;
unsigned char counter = 0;

unsigned long _avr_timer_M = 1;
unsigned long _avr_timer_cntcurr = 0;

void TimerOn() {
	TCCR1B = 0x0B;
	OCR1A = 125;
	TIMSK1 = 0x02;
	TCNT1 = 0;
	_avr_timer_cntcurr = _avr_timer_M;
	SREG |= 0x80;
}

void TimerOff() {
	TCCR1B = 0x00;
}

void TimerISR(){
	TimerFlag = 1;
}

ISR(TIMER1_COMPA_vect) {
	_avr_timer_cntcurr--;
	if (_avr_timer_cntcurr == 0) {
		TimerISR();
		_avr_timer_cntcurr = _avr_timer_M;
	}
}

void TimerSet (unsigned long M) {
	_avr_timer_M = M;
	_avr_timer_cntcurr = _avr_timer_M;
}

void BlinkingLEDSM() {
    switch(BL_State) {
	case BL_SMStart:
	    BL_State = BL_LedOff;
	    break;
	case BL_LedOff:
	    BL_State = BL_LedOn;
	    break;
	case BL_LedOn:
	    BL_State = BL_LedOff;
	    break;
	default:
	    BL_State = BL_SMStart;
	    break;
    }
    switch(BL_State) {
	case BL_LedOff:
	    tmp_BL = 0x00;
	    break;
	case BL_LedOn:
	    tmp_BL = 0x08;
	    break;
	default:
	    break;
    }
}

void ThreeLEDsSM() {
    switch(TL_State) {
	case TL_SMStart:
	    TL_State = TL_T0;
	    break;
	case TL_T0:
	    TL_State = TL_T1;
	    break;
	case TL_T1:
	    TL_State = TL_T2;
	    break;
	case TL_T2:
	    TL_State = TL_T0;
	    break;
	default:
	    TL_State = TL_SMStart;
	    break;
    }
    switch(TL_State) {
	case TL_T0:
	    tmp_TL = 0x01;
	    break;
	case TL_T1:
	    tmp_TL = 0x02;
	    break;
	case TL_T2:
	    tmp_TL = 0x04;
	    break;
	default:
	    break;
    }
}

void CombineLEDsSM() {
    switch(CL_State) {
	case CL_SMStart:
	    CL_State = CL_Combine;
	    break;
	case CL_Combine:
	    CL_State = CL_Combine;
	    break;
	default:
	    CL_State = CL_SMStart;
	    break;
    }
    switch(CL_State) {
	case CL_Combine:
	    PORTB = tmp_BL | tmp_TL | tmp_Speaker;
	    break;
	default:
	    break;
    }
}

void SpeakerSM() {
    switch(Speaker_State) {
	case Speaker_Start:
	    Speaker_State = Speaker_Off;
	    break;
	case Speaker_Off:
	    if ((~PINA & 0x04) == 0x04) {
		Speaker_State = Speaker_On;
	    } else {
		Speaker_State = Speaker_Off;
	    }
	    break;
	case Speaker_On:
	    if ((~PINA & 0x04) == 0x00) {
		Speaker_State = Speaker_Off;
	    } else {
		Speaker_State = Speaker_On;
	    }
	    break;
	default:
	    Speaker_State = Speaker_Start;
	    break;
    }
    switch(Speaker_State) {
	case Speaker_Off:
	    tmp_Speaker = 0x00;
	    counter = 0;
	    break;
	case Speaker_On:
	    if (counter <= 2) {
		tmp_Speaker = 0x10;
	    }
	    else if (counter <= 4) {
		tmp_Speaker = 0x00;
	    }
	    else {
		counter = 0;
	    }
	    counter++;
	    break;
	default:
	    break;
    }
}

int main(void) {
    /* Insert DDR and PORT initializations */
    DDRA = 0x00; PINA = 0xFF;
    DDRB = 0xFF; PORTB = 0x00;
    TimerSet(1);
    TimerOn();
    unsigned long TL_count = 0;
    unsigned long BL_count = 0;
    const unsigned long timerPeriod = 1;
    BL_State = BL_SMStart;
    TL_State = TL_SMStart;
    CL_State = CL_SMStart;
    /* Insert your solution below */
    while (1) {
	if (TL_count >= 300) {
	    ThreeLEDsSM();
	    TL_count = 0;
	}
	if (BL_count >= 1000) {
	    BlinkingLEDSM();
	    BL_count = 0;
	}
	SpeakerSM();
	CombineLEDsSM();
	while (!TimerFlag) {};
	TimerFlag = 0;
	TL_count += timerPeriod;
	BL_count += timerPeriod;
    }
}
