/*
 * firmware.c
 *
 * Created: 12/2/2014 2:41:55 AM
 *  Author: Aman
 *************************************************************************
 *	 Notes:
 *	 Actual motor speed control range: 1550-1599
 *	 Left - Right turn control range : 1001-1200(Center position 1100)
 *	 LED2 intensity control Range 1205-1284
 *	 STOP: 2000
 *	 Forward: 2001
 *	 Backward: 2002
 *	 Left rotate: 2003
 *	 Right rotate: 2004
 *	 go straight: 3000
 *	 go straight with full speed: 3001
 *	 Buzzer Off : 4000
 *	 Buzzer On: 4001
 *	 Led 1 off: 5000
 *	 led 1 on: 5001
 *	 Led 2 (with intensity) off: 6000
 *	 Led 2 on: 6001
 *************************************************************************
 */ 

#define F_CPU 8000000UL
#include <avr/io.h>
#include <stdlib.h>
#include "myusart.h"
#include <util/delay.h>
#include <math.h>

void go_forward(void)
{
	PORTD |= (1<<PD4);
	PORTD &= ~(1<<PD5);
	PORTD |= (1<<PD6);
	PORTD &= ~(1<<PD7);
	
	TCCR1B |= (1<<1);
}

void go_backward(void)
{
	PORTD &= ~(1<<PD4);
	PORTD |= (1<<PD5);	
	PORTD &= ~(1<<PD6);
	PORTD |= (1<<PD7);
	
	TCCR1B |= (1<<1);
}

void right_rotate(void)
{
	PORTD |= (1<<PD4);
	PORTD &= ~(1<<PD5);
	PORTD &= ~(1<<PD6);
	PORTD |= (1<<PD7);
	
	TCCR1B |= (1<<1);
}

void left_rotate(void)
{
	PORTD &= ~(1<<PD4);
	PORTD |= (1<<PD5);
	PORTD |= (1<<PD6);
	PORTD &= ~(1<<PD7);
	
	TCCR1B |= (1<<1);
}

void stay_stop(void)
{
	PORTD &= ~(1<<PD4);
	PORTD &= ~(1<<PD5);
	PORTD &= ~(1<<PD6);
	PORTD &= ~(1<<PD7);
	
	TCCR1B &= ~(1<<1);
}

int main(void)
{
	char ch[10];
	unsigned long i=0;
	unsigned long speed=990, LS=100, RS=100;
	
	PORTB=0x00;
	DDRB=0x1F;
	
	PORTC=0x02;
	DDRC=0x00;
	
	PORTD=0x08;
	DDRD=0xF0;
	
	usart_init(9600); //USART Initialization
	
	//Timer Counter1 Initialization
	TCCR1A=0xA2;
	TCCR1B=0x18;
	TCNT1=0x0000;
	ICR1H=0x03;
	ICR1L=0xE8; //03E8=1000 in decimal 
	OCR1A=speed;
	OCR1B=speed;
	
	//Timer Counter2 Initialization
	ASSR=0x00;
	TCCR2=0x68;
	TCNT2=0x00;
	OCR2=250;
	
    while(1)
    {
		usart_getstr(ch);
		i=atoi(ch);
		
		if (i>=1001 && i<=1100) //Speed control (Maintaining original reference speed) logic and range for left portion
		{
			i=(i-1000);
			LS=i;
			OCR1A=speed;
			OCR1B = (speed*LS)/100;
			i=0;
		}
		else if (i>=1101 && i<=1200) // Speed control (Maintaining original reference speed) logic and range for right portion
		{
			i=100-(i-1100);
			RS=i;
			OCR1B=speed;
			OCR1A = (speed*RS)/100;
			i=0;
		}
		else if (i>=1205 && i<=1284)  //LED intensity control range >> Connected OC2 - PB3
		{
			i=i-1200;
			OCR2=i*3;
			i=0;
		}
		else if (i>=1550 && i<=1599) //Actual Speed Control and range ....
		{
			i=i-1500;
			speed=i*10;
			if (OCR1A>OCR1B)
			{
				OCR1A=speed;
				OCR1B = (speed*LS)/100;
				i=0;
			}
			else if (OCR1B>OCR1A)
			{
				OCR1B=speed;
				OCR1A = (speed*RS)/100;
				i=0;
			} 
			else
			{
				OCR1A=speed;
				OCR1B=speed;
				i=0;
			}
		}
		else if (i==2000)
		{
			stay_stop();
			i=0;
		}
		else if (i==2001)
		{
			go_forward();
			i=0;
		}
		else if (i==2002)
		{
			go_backward();
			i=0;
		}
		else if (i==2003)
		{
			left_rotate();
			i=0;
		}
		else if (i==2004)
		{
			right_rotate();
			i=0;
		}
		else if (i==3000)
		{
			OCR1A=speed;
			OCR1B=speed;
			LS=100;
			RS=100;
			i=0;
		}
		else if (i==3001)
		{
			speed=990;
			OCR1A=speed;
			OCR1B=speed;
			LS=100;
			RS=100;
			i=0;
		}
		else if (i==4000)  //Buzzer off
		{
			PORTB &= ~(1<<PB0);
			i=0;
		}
		else if (i==4001) // Buzzer On
		{
			PORTB |= (1<<PB0);
			i=0;
		}
		else if (i==5000) // LED 1 off (without intensity control)
		{
			PORTB &= ~(1<<PB4);
			i=0;
		}
		else if (i==5001) // LED 1 on
		{
			PORTB |= (1<<PB4);
			i=0;
		}
		else if (i==6000)  //LED 2 off With intensity control
		{
			TCCR2 &= ~(1<<1);
			DDRB &= ~(1<<PB3);
			i=0;
		}
		else if (i==6001) //LED 2 On
		{
			TCCR2 |= (1<<1);
			DDRB |= (1<<PB3);
			i=0;
		}
		else
		{
			i=0;
		}

    }
}