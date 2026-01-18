#include "TM4C123.h"

#define SYSCTL_RCGCGPIO_R  (*((volatile unsigned long*)0x400FE608))
#define GPIO_PORTF_DATA_R  (*((volatile unsigned long*)0x400253FC))
#define GPIO_PORTF_DIR_R   (*((volatile unsigned long*)0x40025400))
#define GPIO_PORTF_DEN_R   (*((volatile unsigned long*)0x4002551C))
#define GPIO_PORTF_AFSEL_R (*((volatile unsigned long*)0x40025420))
#define GPIO_PORTF_CLK_EN  0x20      // Clock enable for Port F
#define BLUE               0x04      // PF2 = Blue LED
#define DELAY              200000

void SystemInit(void)
{
#if (__FPU_USED == 1)
  SCB->CPACR |= ((3UL << 10*2) | (3UL << 11*2)); // Enable FPU
#endif
}

int main(void)
{
  volatile unsigned long ulLoop;

  // 1. Enable clock for Port F
  SYSCTL_RCGCGPIO_R |= GPIO_PORTF_CLK_EN;
  ulLoop = SYSCTL_RCGCGPIO_R; // Dummy delay after enabling clock

  // 2. Disable alternate functions on PF2 and enable digital function
  GPIO_PORTF_AFSEL_R &= ~BLUE;
  GPIO_PORTF_DEN_R   |= BLUE;
  GPIO_PORTF_DIR_R   |= BLUE; // Set PF2 as output

  // 3. Blink BLUE LED
  while(1)
  {
    GPIO_PORTF_DATA_R |= BLUE;   // Turn ON Blue LED
    for(ulLoop = 0; ulLoop < DELAY; ulLoop++); // Delay
    GPIO_PORTF_DATA_R &= ~BLUE;  // Turn OFF Blue LED
    for(ulLoop = 0; ulLoop < DELAY; ulLoop++); // Delay
  }
}
