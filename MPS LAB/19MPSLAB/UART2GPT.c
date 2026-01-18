/******************************************************************************
*	Project name:
*	File name:
*	Author:
*	Date:
*	Description: This is a template file for UART2 configuration. It echoes back
*				 the character sent.
 ******************************************************************************/


/******************************************************************************
*Macros for Register Addresses and Values
*******************************************************************************/
#include "TM4C123.h"

//Register definitions for Clock Enable
#define 	SYSCTL_RCGC_UART_R				(*((volatile unsigned long*)0x400FE618))
#define 	SYSCTL_RCGC_GPIO_R				(*((volatile unsigned long*)0x400FE608))

//Register definitions for GPIO Port D
#define 	GPIO_PORTD_AFSEL_R				(*((volatile unsigned long*)0x40007420))
#define 	GPIO_PORTD_PCTL_R				(*((volatile unsigned long*)0x4000752C))
#define 	GPIO_PORTD_DEN_R				(*((volatile unsigned long*)0x4000751C))
#define 	GPIO_PORTD_DIR_R				(*((volatile unsigned long*)0x40007400))
#define   GPIO_PORTD_LOCK_R              (*((volatile unsigned long*)0x40007520))
#define   GPIO_PORTD_CR_R                (*((volatile unsigned long*)0x40007524))

//Register definitions for UART2 module (base 0x4000E000)
#define 	UART2_CTL_R						(*((volatile unsigned long*)0x4000E030))
#define  	UART2_IBRD_R					(*((volatile unsigned long*)0x4000E024))
#define  	UART2_FBRD_R					(*((volatile unsigned long*)0x4000E028))
#define 	UART2_LCRH_R					(*((volatile unsigned long*)0x4000E02C))
#define 	UART2_CC_R						(*((volatile unsigned long*)0x4000EFC8))
#define 	UART2_FR_R						(*((volatile unsigned long*)0x4000E018))
#define 	UART2_DR_R						(*((volatile unsigned long*)0x4000E000))

//Macros
#define 	UART_FR_TX_FF				0x20		//UART Transmit FIFO Full
#define 	UART_FR_RX_FE				0x10		//UART Receive FIFO Empty

//Clock enable bits
#define     UART2_CLK_EN             0x04        // RCGCUART bit 2
#define     GPIOD_CLK_EN             0x08        // RCGCGPIO bit 3

//PD6 / PD7 masks
#define     PD6_RX                   0x40        // PD6 - U2RX
#define     PD7_TX                   0x80        // PD7 - U2TX
#define     PD67_MASK                (PD6_RX | PD7_TX)

//Function definitions
unsigned char UARTRx(void);
void UARTTx(unsigned char data);
void UARTTxString(char*pt);
void UARTRxString(char*bufPt, unsigned short max);

//System Initialization for Floating Point Unit
void SystemInit (void)
{
	  /* --------------------------FPU settings ----------------------------------*/
	#if (__FPU_USED == 1)
		SCB->CPACR |= ((3UL << 10*2) |                 /* set CP10 Full Access */
                  (3UL << 11*2)  );               /* set CP11 Full Access */
	#endif
}

//Initialize and configure UART2 (PD6 = RX, PD7 = TX, 115200 baud @16 MHz)
void UARTInit(void)
{
	volatile unsigned long delay_clk;
	
	//1. Enable clock for UART2 and GPIO Port D
	SYSCTL_RCGC_UART_R |= UART2_CLK_EN;
	SYSCTL_RCGC_GPIO_R |= GPIOD_CLK_EN;
	
	// dummy read to allow clocks to stabilize
	delay_clk = SYSCTL_RCGC_UART_R;
	(void)delay_clk;
	
	//2. Unlock PD7 (it is a locked pin) and commit changes on PD7
	GPIO_PORTD_LOCK_R = 0x4C4F434B;   // unlock Port D
	GPIO_PORTD_CR_R  |= PD67_MASK;    // allow changes on PD6, PD7
	
	//3. Configure GPIOD pins for UART (AFSEL, PCTL, DEN, DIR)
	//   PD6 = U2RX (input), PD7 = U2TX (output)
	GPIO_PORTD_AFSEL_R |= PD67_MASK;        // enable alternate function on PD6, PD7
	
	// set PCTL bits for PD6 & PD7 to 0x1 (UART2)
	// PCTL[31:28] = 0x1 for PD7, PCTL[27:24] = 0x1 for PD6
	GPIO_PORTD_PCTL_R &= ~0xFF000000;       // clear PD6/PD7 fields
	GPIO_PORTD_PCTL_R |=  0x11000000;       // set to U2RX/U2TX
	
	GPIO_PORTD_DEN_R   |= PD67_MASK;        // digital enable PD6, PD7
	
	GPIO_PORTD_DIR_R   &= ~PD6_RX;          // PD6 input
	GPIO_PORTD_DIR_R   |=  PD7_TX;          // PD7 output
	
	//4. Disable UART2 before configuration
	UART2_CTL_R &= ~0x01;                   // clear UARTEN
	
	//5. Select system clock as UART Baud clock
	UART2_CC_R = 0x0;                       // use system clock (16 MHz)
	
	//6. Set Baud Rate for 115200 baud @16MHz
	//   BRD = 16,000,000 / (16 * 115200) ≈ 8.6805
	//   IBRD = 8, FBRD ≈ 44
	UART2_IBRD_R = 8;
	UART2_FBRD_R = 44;
	
	//7. 8-bit word length, no parity, one stop bit, FIFOs enable
	//   WLEN = 0x3 (8 bits) -> bits[6:5] = 11b -> 0x60
	//   FEN = 1 -> bit4 -> 0x10
	UART2_LCRH_R = 0x70;                    // 0b0111_0000
	
	//8. Enable UART2: RXE, TXE, UARTEN
	UART2_CTL_R |= (0x01 | 0x100 | 0x200);  // UARTEN, TXE, RXE
}

//Wait for input and return its ASCII value
unsigned char UARTRx(void)
{
	while((UART2_FR_R & UART_FR_RX_FE)!=0);    // wait while RX FIFO empty
	return((unsigned char)(UART2_DR_R & 0xFF));
}

/*Accepts ASCII characters from the serial port and
adds them to a string. It echoes each character as it
is inputted. Terminates on 'Enter' (CR).*/
void UARTRxString (char *bufPt, unsigned short max)
{
	unsigned short i = 0;
	unsigned char ch;
	
	while(1)
	{
		ch = UARTRx();        // get character
		
		// if Enter is pressed, finish the string
		if(ch == '\r')
		{
			// echo CR+LF so hyper terminal goes to new line
			UARTTx('\r');
			UARTTx('\n');
			break;
		}
		
		// echo the character back
		UARTTx(ch);
		
		// store into buffer if space left
		if(i < max)
		{
			bufPt[i] = ch;
			i++;
		}
	}
	
	// null-terminate the string
	bufPt[i] = '\0';
	
	// Optionally, send the complete string back
	UARTTxString("You typed: ");
	UARTTxString(bufPt);
	UARTTx('\r');
	UARTTx('\n');
	
	// Prompt again
	UARTTxString("EnterText:");
}

//Output 8bit to serial port
void UARTTx(unsigned char data)
{
	while((UART2_FR_R & UART_FR_TX_FF)!=0);    // wait while TX FIFO full
	UART2_DR_R=data;
}


//Output a character string to serial port
void UARTTxString(char *pt)
{
	while(*pt)
	{
		UARTTx(*pt);
		pt++;
	}
}

int main(void)
{

	char string[17];

	UARTInit();

	//The input given using keyboard is displayed on hyper terminal
	//i.e., data is echoed
	UARTTxString("EnterText:");

	while(1)
	{
		UARTRxString(string,16);
	}
}
