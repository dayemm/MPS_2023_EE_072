/******************************************************************************
*   Project: UART0 + On-board Switches
*   Description:
*       SW1 (PF4) pressed  -> send 'a' on UART0
*       SW2 (PF0) pressed  -> send 'b' on UART0
*       UART0 is on PA0 (RX) and PA1 (TX), 9600 baud, 8N1
******************************************************************************/

#include "TM4C123.h"

/******************************************************************************
*   FPU SystemInit (same as lab templates)
******************************************************************************/
void SystemInit (void)
{
#if (__FPU_USED == 1)
    SCB->CPACR |= ((3UL << 10*2) | (3UL << 11*2));
#endif
}

/******************************************************************************
*   Macros for Register Addresses and Values
******************************************************************************/

/* System Control Registers */
#define SYSCTL_RCGCGPIO_R      (*((volatile unsigned long*)0x400FE608))
#define SYSCTL_RCGCUART_R      (*((volatile unsigned long*)0x400FE618))

/* GPIO Port A Registers (for UART0 pins PA0, PA1) */
#define GPIO_PORTA_DATA_R      (*((volatile unsigned long*)0x400043FC))
#define GPIO_PORTA_DIR_R       (*((volatile unsigned long*)0x40004400))
#define GPIO_PORTA_AFSEL_R     (*((volatile unsigned long*)0x40004420))
#define GPIO_PORTA_PCTL_R      (*((volatile unsigned long*)0x4000452C))
#define GPIO_PORTA_DEN_R       (*((volatile unsigned long*)0x4000451C))

/* GPIO Port F Registers (for switches) */
#define GPIO_PORTF_DATA_R      (*((volatile unsigned long*)0x400253FC))
#define GPIO_PORTF_DIR_R       (*((volatile unsigned long*)0x40025400))
#define GPIO_PORTF_PUR_R       (*((volatile unsigned long*)0x40025510))
#define GPIO_PORTF_DEN_R       (*((volatile unsigned long*)0x4002551C))
#define GPIO_PORTF_LOCK_R      (*((volatile unsigned long*)0x40025520))
#define GPIO_PORTF_CR_R        (*((volatile unsigned long*)0x40025524))

/* UART0 Registers */
#define UART0_DR_R             (*((volatile unsigned long*)0x4000C000))
#define UART0_FR_R             (*((volatile unsigned long*)0x4000C018))
#define UART0_IBRD_R           (*((volatile unsigned long*)0x4000C024))
#define UART0_FBRD_R           (*((volatile unsigned long*)0x4000C028))
#define UART0_LCRH_R           (*((volatile unsigned long*)0x4000C02C))
#define UART0_CTL_R            (*((volatile unsigned long*)0x4000C030))
#define UART0_CC_R             (*((volatile unsigned long*)0x4000CFC8))

/* Clock enable bits */
#define GPIO_PORTA_CLK_EN      0x01       // bit0
#define GPIO_PORTF_CLK_EN      0x20       // bit5
#define UART0_CLK_EN           0x01       // bit0 in RCGCUART

/* Port F pins */
#define PF0_SW2                0x01       // SW2
#define PF4_SW1                0x10       // SW1

/* GPIO Lock key */
#define GPIO_LOCK_KEY          0x4C4F434B

/* UART Flag Register bits */
#define UART_FR_TXFF           0x20       // bit5: Transmit FIFO Full

/******************************************************************************
*   Function Prototypes
******************************************************************************/
void UART0_Init(void);
void GPIOF_Switches_Init(void);
void delay(unsigned long value);

/******************************************************************************
*   UART0 Initialization (9600 baud, 8N1, PIOSC clock)
******************************************************************************/
void UART0_Init(void)
{
    volatile unsigned long dummy;

    /* Enable clocks for UART0 and Port A */
    SYSCTL_RCGCUART_R |= UART0_CLK_EN;       // UART0 clock
    SYSCTL_RCGCGPIO_R |= GPIO_PORTA_CLK_EN;  // Port A clock
    dummy = SYSCTL_RCGCGPIO_R;              // dummy read for delay

    /* Disable UART0 while configuring */
    UART0_CTL_R &= ~( (1<<0) | (1<<8) | (1<<9) );   // clear UARTEN, TXE, RXE

    /* Baud rate configuration for 9600 @ 16 MHz */
    UART0_IBRD_R = 104;      // integer part
    UART0_FBRD_R = 11;       // fractional part

    /* 8-bit, no parity, 1 stop bit, FIFO disabled (just word length bits) */
    UART0_LCRH_R = (0x3 << 5);   // WLEN = 11 -> 8 bits

    /* Clock source: PIOSC (16 MHz internal oscillator) */
    UART0_CC_R = 0x5;

    /* Configure PA0, PA1 for UART0 */
    GPIO_PORTA_AFSEL_R |= 0x03;             // enable alternate function on PA0, PA1
    GPIO_PORTA_PCTL_R  &= ~0x000000FF;      // clear PCTL for PA0, PA1
    GPIO_PORTA_PCTL_R  |=  0x00000011;      // PA0 = U0RX (1), PA1 = U0TX (1)
    GPIO_PORTA_DEN_R   |= 0x03;             // digital enable PA0, PA1
    GPIO_PORTA_DIR_R   |= 0x02;             // PA1 output (TX)
    GPIO_PORTA_DIR_R   &= ~0x01;            // PA0 input  (RX)

    /* Enable UART0, TX and RX */
    UART0_CTL_R |= (1<<0) | (1<<8) | (1<<9);
}

/******************************************************************************
*   GPIOF Initialization for SW1 (PF4) and SW2 (PF0)
******************************************************************************/
void GPIOF_Switches_Init(void)
{
    volatile unsigned long dummy;

    /* Enable clock for Port F */
    SYSCTL_RCGCGPIO_R |= GPIO_PORTF_CLK_EN;
    dummy = SYSCTL_RCGCGPIO_R;     // dummy read

    /* Unlock PF0 and commit */
    GPIO_PORTF_LOCK_R = GPIO_LOCK_KEY;
    GPIO_PORTF_CR_R  |= PF0_SW2;   // allow changes on PF0

    /* Direction: PF4, PF0 inputs */
    GPIO_PORTF_DIR_R &= ~(PF4_SW1 | PF0_SW2);

    /* Enable pull-ups on PF4 and PF0 (switches) */
    GPIO_PORTF_PUR_R |= (PF4_SW1 | PF0_SW2);

    /* Digital enable */
    GPIO_PORTF_DEN_R |= (PF4_SW1 | PF0_SW2);
}

/******************************************************************************
*   Simple Delay
******************************************************************************/
void delay(unsigned long value)
{
    unsigned long i;
    for(i = 0; i < value; i++);
}

/******************************************************************************
*   Main Function
******************************************************************************/
int main(void)
{
    unsigned char Button_State, Button_State1;

    UART0_Init();
    GPIOF_Switches_Init();

    while(1)
    {
        /* ---- Check SW1 on PF4 ---- */
        Button_State = (unsigned char)((GPIO_PORTF_DATA_R & PF4_SW1) >> 4);
        if(Button_State == 0)                 // SW1 pressed (pull-up logic)
        {
            /* Wait while transmit FIFO is full */
            while(UART0_FR_R & UART_FR_TXFF);
            UART0_DR_R = 'a';                 // send 'a'
            delay(6000000);
        }

        /* ---- Check SW2 on PF0 ---- */
        Button_State1 = (unsigned char)(GPIO_PORTF_DATA_R & PF0_SW2);
        if(Button_State1 == 0)                // SW2 pressed
        {
            while(UART0_FR_R & UART_FR_TXFF);
            UART0_DR_R = 'b';                 // send 'b'
            delay(6000000);
        }
    }
}
