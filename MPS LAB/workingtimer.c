/******************************************************************************
* Project name:  Experiment 9 - Timers and Time Base Generation
* File name:     timer_expt9.c
* Author:        DEV + GPT helper
* Date:          3/12/25
* Description:   Timer0A in 16-bit periodic mode to generate interrupts
*                every 0.5 s and toggle Red LED (PF1).
******************************************************************************/

#include "TM4C123.h"  // Device header: defines core registers, SCB, etc.

/******************************************************************************
* Macros for Register Addresses and Values (APB bus)
* (All base addresses and offsets are taken from TM4C123GH6PM datasheet)
******************************************************************************/

/* GPIO registers (Port F) */
#define SYSCTL_RCGCGPIO_R      (*((volatile unsigned long*)0x400FE608)) // RCGCGPIO
#define GPIO_PORTF_DATA_R      (*((volatile unsigned long*)0x400253FC)) // GPIODATA (masked)
#define GPIO_PORTF_DIR_R       (*((volatile unsigned long*)0x40025400)) // GPIODIR
#define GPIO_PORTF_DEN_R       (*((volatile unsigned long*)0x4002551C)) // GPIODEN

/* Timer0 registers (16/32-bit General-Purpose Timer 0, APB base 0x4003.0000) */
#define SYSCTL_RCGCTIMER_R     (*((volatile unsigned long*)0x400FE604)) // RCGCTIMER
#define TIMER0_CTL_R           (*((volatile unsigned long*)0x4003000C)) // GPTMCTL
#define TIMER0_CFG_R           (*((volatile unsigned long*)0x40030000)) // GPTMCFG
#define TIMER0_TAMR_R          (*((volatile unsigned long*)0x40030004)) // GPTMTAMR
#define TIMER0_TAILR_R         (*((volatile unsigned long*)0x40030028)) // GPTMTAILR
#define TIMER0_TAPR_R          (*((volatile unsigned long*)0x40030038)) // GPTMTAPR
#define TIMER0_ICR_R           (*((volatile unsigned long*)0x40030024)) // GPTMICR
#define TIMER0_IMR_R           (*((volatile unsigned long*)0x40030018)) // GPTMIMR

/* NVIC registers used for Timer0A interrupt (interrupt #19) */
#define NVIC_EN__R             (*((volatile unsigned long*)0xE000E100)) // EN0
#define NVIC_PRI__R            (*((volatile unsigned long*)0xE000E410)) // PRI4

/* Constant values / bit masks */
#define TIM0_CLK_EN            0x01        // RCGCTIMER bit 0 → Timer0
#define TIM0_EN                0x01        // GPTMCTL TAEN bit
#define TIM_16_BIT_EN          0x00000004  // GPTMCFG = 0x4 → 16-bit mode
#define TIM_TAMR_PERIODIC_EN   0x00000002  // GPTMTAMR TAMR[1:0] = 0b10 (Periodic)
#define TIM_FREQ_10usec        159         // Prescaler → divide by 160
#define TIM0_INT_CLR           0x01        // GPTMICR TATOCINT = 1 (clear)
#define EN__INT                0x00080000  // EN0 bit 19 → Timer0A interrupt
#define PORTF_CLK_EN           0x20        // RCGCGPIO bit 5 → Port F
#define TOGGLE_PF1             GPIO_PORTF_DATA_R ^= 0x02 // Toggle PF1
#define LED_RED                0x02        // PF1 mask

/* Function prototypes (assembly functions provided by startup code) */
void GPIO_Init(void);
void Timer_Init(unsigned long period);
void DisableInterrupts(void);
void EnableInterrupts(void);
void WaitForInterrupt(void);

/* System Initialization for FPU (from CMSIS template) */
void SystemInit (void)
{
#if (__FPU_USED == 1)
  SCB->CPACR |= ((3UL << 10*2) | (3UL << 11*2));  // Enable CP10, CP11 full access
#endif
}

/* Timer 0A Configuration */
void Timer_Init(unsigned long period)
{
    volatile unsigned long delay;  // used for dummy read after clock enable

    /* 1) Enable clock for 16/32-bit Timer0 in Run mode */
    SYSCTL_RCGCTIMER_R |= TIM0_CLK_EN;

    /* Allow time for the clock to start by doing a dummy read */
    delay = SYSCTL_RCGCTIMER_R;
    (void)delay;

    /* 2) Disable Timer0A before configuration (TAEN = 0) */
    TIMER0_CTL_R &= ~TIM0_EN;

    /* 3) Configure Timer0 for 16-bit operation (Timer A only) */
    TIMER0_CFG_R = TIM_16_BIT_EN;

    /* 4) Configure Timer A in periodic down-count mode */
    TIMER0_TAMR_R = TIM_TAMR_PERIODIC_EN;

    /* 5) Load interval value (number of timer ticks) */
    TIMER0_TAILR_R = period - 1;

    /* 6) Set prescaler so that each tick = 10 µs (100 kHz) */
    TIMER0_TAPR_R = TIM_FREQ_10usec;

    /* ------------ Interrupt configuration ------------ */
    DisableInterrupts();           // Globally mask interrupts (PRIMASK = 1)

    /* 7) Enable Timer0A timeout interrupt at module level */
    TIMER0_IMR_R |= 0x01;         // TATOIM = 1

    /* 8) Clear any previous timeout flag */
    TIMER0_ICR_R = TIM0_INT_CLR;  // write 1 to TATOCINT

    /* 9) Set NVIC priority for Timer0A (interrupt 19) */
    NVIC_PRI__R = (NVIC_PRI__R & 0x00FFFFFF) | 0x40000000;  // priority level 2

    /* 10) Enable Timer0A interrupt in NVIC EN0 register */
    NVIC_EN__R |= EN__INT;

    /* 11) Enable Timer0A (start counting) */
    TIMER0_CTL_R |= TIM0_EN;

    EnableInterrupts();           // Globally enable interrupts (PRIMASK = 0)
}

/* Configure PF1 (red LED) as digital output */
void GPIO_Init(void)
{
    volatile unsigned long delay;

    /* 1) Enable clock to GPIO Port F */
    SYSCTL_RCGCGPIO_R |= PORTF_CLK_EN;

    /* Allow time for clock to stabilize */
    delay = SYSCTL_RCGCGPIO_R;
    (void)delay;

    /* 2) Configure PF1 direction as output */
    GPIO_PORTF_DIR_R |= LED_RED;

    /* 3) Enable digital function for PF1 */
    GPIO_PORTF_DEN_R |= LED_RED;
}

/* Timer0A ISR: runs on every timeout event */
void TIMER0A_Handler(void)
{
    /* Clear timeout flag so next interrupt can occur */
    TIMER0_ICR_R = TIM0_INT_CLR;

    /* Toggle Red LED (PF1) */
    TOGGLE_PF1;
}

int main(void)
{
    /* Configure GPIO first so LED pin is ready */
    GPIO_Init();

    /* For 0.5 s period: 50,000 ticks × 10 µs = 0.5 s */
    Timer_Init(50000);

    while(1)
    {
        /* Put CPU in sleep until any interrupt occurs (WFI) */
        WaitForInterrupt();
    }
}
