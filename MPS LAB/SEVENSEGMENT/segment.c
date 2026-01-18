/******************************************************************************
*	Project name:
*	File name:
*	Author:
*	Date:
*	Description: This is a template file for GPIO parallel interfacing for
*				Seven Segment Display. This program is written for common anode
*				type seven segment display
* 			Seven segment digits pins:		PA2-PA5*
*				Seven segment data pins:		PB0-PB7*
*				Port B pins:					76543210*
*				Seven Segment LEDs:				pgfedcba*
 ******************************************************************************/

#include "TM4C123.h"

void SystemInit (void)
{
	  /* --------------------------FPU settings ----------------------------------*/
	#if (__FPU_USED == 1)
		SCB->CPACR |= ((3UL << 10*2) |                 /* set CP10 Full Access */
                  (3UL << 11*2)  );               /* set CP11 Full Access */
	#endif
}
#define	SYSCTL_RCGCGPIO_R		(*((volatile unsigned long*)0x400FE608))

#define	GPIO_PORTB_DATA_R		(*((volatile unsigned long*)0x400053FC))
#define	GPIO_PORTB_DIR_R		(*((volatile unsigned long*)0x40005400))
#define	GPIO_PORTB_DEN_R		(*((volatile unsigned long*)0x4000551C))
#define	GPIO_PORTB_AFSEL_R		(*((volatile unsigned long*)0x40005420))

#define	GPIO_PORTA_DATA_R		(*((volatile unsigned long*)0x400043FC))
#define	GPIO_PORTA_DIR_R		(*((volatile unsigned long*)0x40004400))
#define	GPIO_PORTA_DEN_R		(*((volatile unsigned long*)0x4000451C))
#define	GPIO_PORTA_AFSEL_R		(*((volatile unsigned long*)0x40004420))

#define	SEG_1		0xDF
#define	SEG_2		0xEF
#define	SEG_3		0xF7
#define	SEG_4		0xFB
#define	SEG_OFF	0xFF

void init_gpio(void);
void display_1(void);
void display_2(void);
void delay(unsigned long value);

const char lut_display1[4]={0xC1,
							0xC0,
							0x8E,
							0x92
							}; //UOFS
							
const char lut_display2[4]={0x89,
							0x86,
							0xC7,
							0xC0
							}; //HELO

const char seg_select[]={0xFB,
						 0xF7,
						 0xEF,
						 0xDF
						};
						
void init_gpio(void){
	volatile unsigned long delay_clk;
	SYSCTL_RCGCGPIO_R |= 0x03;
	delay_clk = SYSCTL_RCGCGPIO_R; 	// dummy read for delay for clock,must have 3sys clock delay
	
	GPIO_PORTB_DIR_R |= 0xFF;
	GPIO_PORTB_DEN_R |= 0xFF;
	GPIO_PORTB_AFSEL_R &= ~(0xFF);
	
	GPIO_PORTA_DIR_R |= 0x3C;	
	GPIO_PORTA_DEN_R |= 0x3C;
	GPIO_PORTA_AFSEL_R &= ~(0x3C);
}

/* display_1 on seven segments using Macros */
void display_1(void){
	int j;
	int i;
	for (j=0; j < 1000000; j++)
	{
		for (i=0; i < 4; i++)
		{
			GPIO_PORTB_DATA_R = lut_display1[i];
			GPIO_PORTA_DATA_R = seg_select[i];
			delay(1000);
			GPIO_PORTA_DATA_R |= ~seg_select[i];
			
		}
	}
}

/* display_2 on seven segments using for loop */
void display_2(void){
	int j;
	int i;
	for (j=0; j < 1000000; j++)
	{
		for (i=0; i < 4; i++)
		{
			GPIO_PORTB_DATA_R = lut_display2[i];
			GPIO_PORTA_DATA_R = seg_select[i];
			delay(1000);
			GPIO_PORTA_DATA_R |= ~seg_select[i];
			
		}
	}
}


void delay(unsigned long value){
	unsigned long i ;
	for(i=0;i<value;i++);
}

int main(void){
	int i;
	init_gpio();
	while(1){
		display_1();
		display_2();
	
}
}