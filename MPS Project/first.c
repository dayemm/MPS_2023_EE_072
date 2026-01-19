/******************************************************************************
*	Project name: GPIO LED Interfacing
*	File name: led
*	Author:
*	Date:
*	Description: This is a example blinky file.
*
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
	
#define SYSCTL_RCGCGPIO_R (*((volatile unsigned long*) 0x400FE608))
#define GPIO_PORTF_DATA_R (*((volatile unsigned long*) 0x400253FC))
#define GPIO_PORTF_DIR_R (*((volatile unsigned long*) 0x40025400))
#define GPIO_PORTF_DEN_R (*((volatile unsigned long*) 0x4002551C))
#define GPIO_PORTF_CLK_EN 0x20
	/* Values to be entered in the registers */
#define GREEN 0x08
#define RED   0X02
#define BLUE   0X04
#define DELAY 200000
	
int main ( void )
{
	volatile unsigned long ulLoop ;
	
	/* Enable the GPIO port that is used for the on-board LED. */
	SYSCTL_RCGCGPIO_R |= GPIO_PORTF_CLK_EN ;
	/* Do a dummy read to insert a few cycles after enabling the peripheral. */
	ulLoop = SYSCTL_RCGCGPIO_R ;
	/* Enable the GPIO pin for the LED (PF3) for digital function.*/
	GPIO_PORTF_DEN_R |= GREEN;
	/* Set the direction as output.*/
	GPIO_PORTF_DIR_R |= GREEN;
	
	/* Loop forever */
	while (1)
	{
		/* Turn on the LED */
		GPIO_PORTF_DATA_R |= GREEN ;
		/* Delay for a bit. */
		for ( ulLoop = 0 ; ulLoop < DELAY ; ulLoop++) ;
		/* Turn off the LED. */
		GPIO_PORTF_DATA_R &= ~GREEN ;
		/* Delay for a bit. */
		for ( ulLoop = 0 ; ulLoop < DELAY ; ulLoop++ ) ;
		}
}
