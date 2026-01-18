/******************************************************************************
*	Project name:
*	File name:
*	Author:
*	Date:
*	Description: This is a template file for GPIO interfacing for Green LED
*
 ******************************************************************************/

#include "TM4C123.h"

/******************************************************************************
*Macros for Register Addresses and Values
*******************************************************************************/	
#define SYSCTL_RCGCGPIO_R (*((volatile unsigned long*) 0x400FE608))//PB5
#define GPIO_PORTB_DATA_R (*((volatile unsigned long*) 0x400053FC))
#define GPIO_PORTB_AFSEL_R (*((volatile unsigned long *)0x40005420))
#define GPIO_PORTB_DIR_R (*((volatile unsigned long*) 0x40005400))
#define GPIO_PORTB_DEN_R (*((volatile unsigned long*) 0x4000551C))
#define GPIO_PORTB_DR8R_R (*((volatile unsigned long*)0x40005508))

/* Enable clock for PortB */
#define GPIO_PORTB_CLK_EN 0x02
/* PF3 GPIO configuration */
#define GPIO_PORTB_PIN5_EN 0x20     //00100000
/* Green LED on */
#define LED_ON 0x20
/* Green LED off */
#define LED_OFF ~(0x20)
/* Delay */
#define DELAY 100000

//System Initialization for Floating Point Unit
void SystemInit (void)
{
	  /* --------------------------FPU settings ----------------------------------*/
	#if (__FPU_USED == 1)
		SCB->CPACR |= ((3UL << 10*2) |                 /* set CP10 Full Access */
                  (3UL << 11*2)  );               /* set CP11 Full Access */
	#endif
}

int main ( void )
{
	volatile unsigned long ulLoop ;

	 /* 1. Enable the GPIO port that is used for the on-board LED. */
	SYSCTL_RCGCGPIO_R |= GPIO_PORTB_CLK_EN ;
	 /* Do a dummy read to insert a few cycles after enabling the peripheral. */
	ulLoop = SYSCTL_RCGCGPIO_R ;
	
	/* 2. Enable the GPIO pin for the LED (PB5) for digital function.
	Set the direction as output.*/
	GPIO_PORTB_AFSEL_R &=~GPIO_PORTB_PIN5_EN;
	GPIO_PORTB_DEN_R |= GPIO_PORTB_PIN5_EN ;
	GPIO_PORTB_DIR_R |= GPIO_PORTB_PIN5_EN ;
	GPIO_PORTB_DR8R_R  |=  GPIO_PORTB_PIN5_EN;
	
	/* Loop forever */
	while (1)
	{
		 /* Turn on the LED */
		GPIO_PORTB_DATA_R |= LED_ON ;
		/* Delay for a bit. */
		for ( ulLoop = 0 ; ulLoop < DELAY ; ulLoop++) ;
		/* Turn off the LED. */
		GPIO_PORTB_DATA_R &= LED_OFF ;
		/* Delay for a bit. */
		for ( ulLoop = 0 ; ulLoop < DELAY ; ulLoop++ ) ;
	}
}
