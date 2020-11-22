#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
#include "../inc/CortexM.h"

#include "../inc/UART1int.h"
#define FIFOSIZE   256       // size of the FIFOs (must be power of 2)
#define FIFOSUCCESS 1        // return value on success
#define FIFOFAIL    0        // return value on failure
uint32_t RxPutI;      // should be 0 to SIZE-1
uint32_t RxGetI;      // should be 0 to SIZE-1 
uint32_t RxFifoLost;  // should be 0 
char RxFIFO[FIFOSIZE];
void RxFifo_Init(void){
  RxPutI = RxGetI = 0;                      // empty
  RxFifoLost = 0; // occurs on overflow
}
int RxFifo_Put(char data){
  if(((RxPutI+1)&(FIFOSIZE-1)) == RxGetI){
    RxFifoLost++;
    return FIFOFAIL; // fail if full  
  }    
  RxFIFO[RxPutI] = data;                    // save in FIFO
  RxPutI = (RxPutI+1)&(FIFOSIZE-1);         // next place to put
  return FIFOSUCCESS;
}
int RxFifo_Get(char *datapt){ 
  if(RxPutI == RxGetI) return 0;            // fail if empty
  *datapt = RxFIFO[RxGetI];                 // retrieve data
  RxGetI = (RxGetI+1)&(FIFOSIZE-1);         // next place to get
  return FIFOSUCCESS; 
}


// Initialize UART1
// Baud rate is 115200 bits/sec
// Lab 9
void UART1_Init(void){
  // write this
	SYSCTL_RCGCUART_R |= 0x0002;		// activate UART1
	SYSCTL_RCGCGPIO_R |= 0x0004;		// activate PortC
	UART1_CTL_R &= ~0x0001;					// disable UART
	UART1_IBRD_R = 43;							// 80MHz, was 27
	UART1_FBRD_R = 26; 							// 80MHz, was 8
	UART1_LCRH_R = 0x0070;
	
	//Init for Rx
	UART1_IM_R |= 0x10;
	UART1_IFLS_R &= ~0x38; 
	UART1_IFLS_R |= 0x10;
	NVIC_PRI1_R = (NVIC_PRI1_R&~0x70000)+0x70000;
	NVIC_EN0_R |= 0x40;

	
	UART1_CTL_R = 0x0301;
	GPIO_PORTC_AFSEL_R |= 0x30; 		//alt func 
	GPIO_PORTC_PCTL_R |= (GPIO_PORTC_PCTL_R&0xFF00FFFF)+0x00220000;
	GPIO_PORTC_DEN_R |= 0x30;				// digital I/O on PC5-4
	GPIO_PORTC_AMSEL_R &= ~0x30; 		// no analog on PC5-4
}


// input ASCII character from UART
// spin if RxFifo is empty
// Lab 9
char UART1_InChar(void){
  // write this
	while((UART1_FR_R&0x0010) != 0);
	// wait until RXFE is 0
	return ((uint8_t)(UART1_DR_R&0xFF));
}

// output ASCII character to UART
// busy-wait spin if hardware not ready
// Lab 9
// in Lab 9 this will never wait
void UART_OutChar(char data){
  // write this
	while((UART1_FR_R&0x0020) != 0);
	// wait until TXFF is 0
	UART1_DR_R = data;
}

// one thing has happened:
// hardware RX FIFO goes from 7 to 8 items
// Lab 9
void UART1_Handler(void){
	uint32_t RXCounter = 0;
 // write this
	for(uint8_t i = 0; i < 8; i++) {
		char data = UART1_InChar();
	}
	RXCounter++;
	UART1_ICR_R = 0x10; //this clears bit 4 (RXRIS) in the RIS register
}

//------------UART1_OutString------------
// Output String (NULL termination)
// Input: pointer to a NULL-terminated string to be transferred
// Output: none
void UART1_OutString(char *pt){
  while(*pt){
    UART_OutChar(*pt);
    pt++;
  }
}


//-----------------------UART_OutUDec-----------------------
// Output a 32-bit number in unsigned decimal format
// Input: 32-bit number to be transferred
// Output: none
// Variable format 1-10 digits with no space before or after
void UART_OutUDec(uint32_t n){
// This function uses recursion to convert decimal number
//   of unspecified length as an ASCII string
  if(n >= 10){
    UART_OutUDec(n/10);
    n = n%10;
  }
  UART_OutChar(n+'0'); /* n is between 0 and 9 */
}

