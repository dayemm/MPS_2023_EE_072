#include "TM4C123.h"

/* Memory-mapped registers (must be volatile POINTERS, dereferenced) */
#define SYSCTL_RCGCGPIO_R  (*((volatile unsigned long*)0x400FE608))
#define GPIO_PORTF_DATA_R  (*((volatile unsigned long*)0x400253FC))
#define GPIO_PORTF_DIR_R   (*((volatile unsigned long*)0x40025400))
#define GPIO_PORTF_DEN_R   (*((volatile unsigned long*)0x4002551C))
#define GPIO_PORTF_AFSEL_R (*((volatile unsigned long*)0x40025420))

#define GPIO_PORTF_CLK_EN   0x20   // Port F clock
#define GPIO_PORTF_PIN3_EN  0x08   // PF3 (Green LED)
#define LED_ON              0x08
#define LED_OFF            ~(0x08)
#define DELAY               200000

void SystemInit(void){
#if (__FPU_USED == 1)
  SCB->CPACR |= ((3UL << 10*2) | (3UL << 11*2));
#endif
}

int main(void){
  volatile unsigned long ulLoop;

  // 1) Enable clock to Port F
  SYSCTL_RCGCGPIO_R |= GPIO_PORTF_CLK_EN;
  ulLoop = SYSCTL_RCGCGPIO_R; // dummy read

  // 2) Configure PF3 as GPIO digital output
  GPIO_PORTF_AFSEL_R &= ~GPIO_PORTF_PIN3_EN; // GPIO, not alternate
  GPIO_PORTF_DEN_R   |=  GPIO_PORTF_PIN3_EN; // digital enable
  GPIO_PORTF_DIR_R   |=  GPIO_PORTF_PIN3_EN; // output

  // 3) Blink
  while(1){
    GPIO_PORTF_DATA_R |= LED_ON;              // ON
    for(ulLoop=0; ulLoop<DELAY; ulLoop++);
    GPIO_PORTF_DATA_R &= LED_OFF;             // OFF
    for(ulLoop=0; ulLoop<DELAY; ulLoop++);
  }
}
