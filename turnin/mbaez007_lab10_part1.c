/*	Author: Marco Baez
 *  Partner(s) Name: NA
 *	Lab Section: 021
 *	Assignment: Lab #10  Exercise #1
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 *
 *	No Demo required 
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

unsigned char GetBit(unsigned char c, unsigned char n){
	return(c&(0x01<<n));
}

unsigned char GetKeypadKey(){
	PORTC=0xEF;
	asm("nop");
	if(GetBit(PINC,0)==0) { return('1'); } //possibly might need ~PORTC, CHECK!
	if(GetBit(PINC,1)==0) { return('4'); }
	if(GetBit(PINC,2)==0) { return('7'); }
	if(GetBit(PINC,3)==0) { return('*'); }

	PORTC=0xDF;
	asm("nop");
	if(GetBit(PINC,0)==0) { return('2'); }
	if(GetBit(PINC,1)==0) { return('5'); }
	if(GetBit(PINC,2)==0) { return('8'); }
	if(GetBit(PINC,3)==0) { return('0'); }

	PORTC=0xBF;
	asm("nop");
	if(GetBit(PINC,0)==0) { return('3'); }
	if(GetBit(PINC,1)==0) { return('6'); }
	if(GetBit(PINC,2)==0) { return('9'); }
	if(GetBit(PINC,3)==0) { return('#'); }

	PORTC=0x7F;
	asm("nop");
	if(GetBit(PINC,0)==0) { return('A'); }
	if(GetBit(PINC,1)==0) { return('B'); }
	if(GetBit(PINC,2)==0) { return('C'); }
	if(GetBit(PINC,3)==0) { return('D'); }

	return ('\0');
}

unsigned long int FindGCD(unsigned long int a, unsigned long int b){
	unsigned long int c;
	while(1){
		c=a%b;
		if(c==0){return b;}
		a=b;
		b=c;
	}
	return 0;
}

typedef struct task{
	int state;
	unsigned long period;
	unsigned long elapsedtime;
	int(*TickFct)(int);
} task;

task tasks[2];

volatile unsigned char TimerFlag = 0;

unsigned long _avr_timer_M = 1;
unsigned long _avr_timer_cntcurr = 0;

void TimerOn(){

        TCCR1B = 0x0B;

        OCR1A = 125;

        TIMSK1 = 0x02;

        TCNT1 = 0x02;

        _avr_timer_cntcurr = _avr_timer_M;

        SREG |= 0x80;
}

void TimerOff(){

        TCCR1B = 0x00;

}

void TimerISR(){
        //TimerFlag = 1;
        unsigned char i;
        for(i=0;i<2;i++){
                if(tasks[i].elapsedtime>=tasks[i].period){
                        tasks[i].state=tasks[i].TickFct(tasks[i].state);
                        tasks[i].elapsedtime=0;
                }
                tasks[i].elapsedtime+=100;
        }
}

ISR(TIMER1_COMPA_vect){
        _avr_timer_cntcurr--;
        if(_avr_timer_cntcurr == 0){
                TimerISR();
                _avr_timer_cntcurr = _avr_timer_M;
        }
}

void TimerSet(unsigned long M){
        _avr_timer_M = M;
        _avr_timer_cntcurr = _avr_timer_M;
}

unsigned char key;
enum LEDStates{Init, Wait, On};
int LEDSM(int state){
	unsigned char tempinput;
	tempinput=GetKeypadKey();
	switch(state){
		case Init:
			state=Wait;
			break;
		case Wait:
			if(tempinput=='\0'){state=Wait;}
			else{state=On;}
	//		key=0x00;
			break;
		case On:
			if(tempinput=='\0'){state=Wait;}
			else{state=On;}
	//		key=0x80;
			break;
		default:
			break;
	}
	switch(state){
		case Init:
			break;
		case Wait:
			key=0x00;
			break;
		case On:
			key=0x80;
			break;
		default: 
			break;
	}
 return state;
}

enum KEYSMStates{InitKey, OnKey};
int KEYSM(int state){
	switch(state){
		case InitKey:
			state=OnKey;
			break;
		case OnKey:
			state=OnKey;
		//	PORTB=key;
			break;
		default:
			break;
	}
	switch(state){
		case InitKey:
			break;
		case OnKey:
			PORTB=key;
			break;
		default:
			break;
	}
 return state;
}


int main(void) {
    unsigned char temp = 0x00;
    DDRB=0xFF; PORTB=0x00;
    DDRC=0xF0; PORTC=0x0F;
    tasks[temp].state=Init;
    tasks[temp].period=100;
    tasks[temp].elapsedtime=0;
    tasks[temp].TickFct=&LEDSM;
    temp++;
    tasks[temp].state=InitKey;
    tasks[temp].period=100;
    tasks[temp].elapsedtime=0;
    tasks[temp].TickFct=&KEYSM;
    TimerSet(100);
    TimerOn();
    while (1) {
    }
    return 1;
}
