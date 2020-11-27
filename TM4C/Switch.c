/* =======================Switch.c===================================
Created by: Benjamin Fa, Faiyaz Mostofa, Melissa Yang, and Yongye Zhu
EE445L Fall 2020 for McDermott, Mark 

The interface for input 2 switches (PF3 and PF4).
===================================================================== */

#include <stdint.h>
#include "Switch.h"
#include "../inc/tm4c123gh6pm.h"
#include "../inc/CortexM.h"

void (*SW1_Event)(void);
void (*SW2_Event)(void);

static void GPIOArm(void){
  GPIO_PORTF_ICR_R = 0x18;      // clear flag4-3
  GPIO_PORTF_IM_R |= 0x18;      // arm interrupt on PF4-3 *** No IME bit as mentioned in Book ***
  NVIC_PRI7_R = (NVIC_PRI7_R&0xFF00FFFF)|0x00A00000; // priority 5
  NVIC_EN0_R = 0x40000000;      // enable interrupt 30 in NVIC  
}

// Debounce switch
static void Timer0Arm(void){
  TIMER0_CTL_R = 0x00000000;    // 1) disable TIMER0A during setup
  TIMER0_CFG_R = 0x00000000;    // 2) configure for 32-bit mode
  TIMER0_TAMR_R = 0x0000001;    // 3) 1-SHOT mode
  TIMER0_TAILR_R = 160000*10*10;      // 4) 10ms reload value
  TIMER0_TAPR_R = 0;            // 5) bus clock resolution
  TIMER0_ICR_R = 0x00000001;    // 6) clear TIMER0A timeout flag
  TIMER0_IMR_R = 0x00000001;    // 7) arm timeout interrupt
  NVIC_PRI4_R = (NVIC_PRI4_R&0x00FFFFFF)|0x80000000; // 8) priority 4
// interrupts enabled in the main program after all devices initialized
// vector number 35, interrupt number 19
  NVIC_EN0_R = 1<<19;           // 9) enable IRQ 19 in NVIC
  TIMER0_CTL_R = 0x00000001;    // 10) enable TIMER0A
}

void Switch_Init(void(*task1)(void), void(*task2)(void)){
  SYSCTL_RCGCGPIO_R |= 0x00000020;    // activate clock for Port F
  while((SYSCTL_PRGPIO_R & 0x00000020) == 0){};
  GPIO_PORTF_AMSEL_R &= ~0x18;        // disable analog on PF4-3
  GPIO_PORTF_PCTL_R &= ~0x18;         // PCTL GPIO on PF4-3
  GPIO_PORTF_DIR_R &= ~0x18;          // PF4-3
  GPIO_PORTF_AFSEL_R &= ~0x18;        // disable alt funct on PF7-0
  GPIO_PORTF_DEN_R |= 0x18;           // enable digital I/O on PF4-3
	GPIO_PORTF_IS_R &= ~0x18;     			// PF4-3 is edge-sensitive
	GPIO_PORTF_IBE_R &= ~0x18;					// PF4-3 set to trigger only on one edge
	GPIO_PORTF_IEV_R |= 0x18;						// PF4-3 set to trigger on rising edge
	GPIOArm();
		
	SW1_Event = task1;
	SW2_Event = task2;
		
	SYSCTL_RCGCTIMER_R |= 0x01;   // activate TIMER0
}

void GPIOPortF_Handler(void){
  GPIO_PORTF_IM_R &= ~0x18;     // disarm interrupt on PF4-3 
  if((GPIO_PORTF_RIS_R & 0x10) == 0x10){
		(*SW1_Event)();
  }
  else{
		(*SW2_Event)();
  }
	Timer0Arm(); // debounce
}

//// Interrupt 10ms after PortF handler
//void Timer0A_Handler(void){
//  TIMER0_IMR_R = 0x00000000;    // disarm timeout interrupt
//  GPIOArm();   // start GPIO
//}
