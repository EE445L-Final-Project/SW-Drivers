/* =======================main.c=====================================
Created by: Benjamin Fa, Faiyaz Mostofa, Melissa Yang, and Yongye Zhu
EE445L Fall 2020 for McDermott, Mark 

The main method for the entire system. 
===================================================================== */

#include <stdint.h>
#include "../inc/user.h"
#include "../inc/CortexM.h"
#include "Switch.h"
#include "BLEHandler.h"
#include "AppHandler.h"
#include "Display.h"

void Hello_World() {
}

int main(void) 
{
	Display_Init();
	Switch_Init(&BLESwitch_Advertisement,&Hello_World);
  EnableInterrupts();
	WaitForInterrupt(); // sleep until interrupt
	return 0;
}
