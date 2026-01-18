/******************************************************************************
*	Project name: External LED Blinking
*	File name: main.c
*	Author: Muhammad Waleed Akram
*	Date: 24-10-2025
*	Description: This is a template file for GPIO interfacing for RED LED  connected to PA2 of tiva with led on breadboard
*
 ******************************************************************************/

#include "TM4C123.h"

/******************************************************************************
*Macros for Register Addresses and Values
*******************************************************************************/	
#define SYSCTL_RCGCGPIO_R (*((volatile unsigned long*) 0x400FE608))
#define GPIO_PORTA_DATA_R (*((volatile unsigned long*) 0x400043FC))
#define GPIO_PORTA_DIR_R (*((volatile unsigned long*) 0x40004400))
#define GPIO_PORTA_DEN_R (*((volatile unsigned long*) 0x4000451C))

/* Enable clock for PortA */
#define GPIO_PORTA_CLK_EN 0x01
/* PA2 GPIO configuration */
#define GPIO_PORTA_PIN2_EN 0x04
/* Green LED on */
#define LED_ON 0x04
/* Green LED off */
#define LED_OFF ~(0x04)
/* Delay */
#define DELAY 200000

//System Initialization for Floating Point Unit
void SystemInit (void)
{

}


int main ( void )
{
	volatile unsigned long ulLoop ;

	 /* Enable the GPIO port that is used for the on-board LED. */
	SYSCTL_RCGCGPIO_R |= GPIO_PORTA_CLK_EN ;
	 /* Do a dummy read to insert a few cycles after enabling the peripheral. */
	ulLoop = SYSCTL_RCGCGPIO_R ;
	
	/* Enable the GPIO pin for the LED (PA2) for digital function.
	Set the direction as output.*/
	GPIO_PORTA_DEN_R |= GPIO_PORTA_PIN2_EN ;
	GPIO_PORTA_DIR_R |= GPIO_PORTA_PIN2_EN ;
	
	
	
	
	/* Loop forever */
	while (1)
	{
		 /* Turn on the LED */
		GPIO_PORTA_DATA_R |= LED_ON ;
		/* Delay for a bit. */
		for ( ulLoop = 0 ; ulLoop < DELAY ; ulLoop++) ;
		/* Turn off the LED. */
		GPIO_PORTA_DATA_R &= LED_OFF ;
		/* Delay for a bit. */
		for ( ulLoop = 0 ; ulLoop < DELAY ; ulLoop++ ) ;
	}
}
