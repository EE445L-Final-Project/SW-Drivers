/* =======================Switch.h======================================
Created by: Benjamin Fa, Faiyaz Mostofa, Melissa Yang, and Yongye Zhu
EE445L Fall 2020 for McDermott, Mark 

The interface for input 2 switches (PF3 and PF4).
===================================================================== */

#ifndef SWITCH_H
#define SWITCH_H

#define SW1       (*((volatile uint32_t *)0x40025020)) //PF3
#define SW2       (*((volatile uint32_t *)0x40025040))  //PF4
	
void Switch_Init(void(*PortF3_Event)(void), void(*PortF4_Event)(void));

#endif // SWITCH_H
