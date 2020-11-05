/* =======================main.c=====================================
Created by: Benjamin Fa, Faiyaz Mostofa, Melissa Yang, and Yongye Zhu
EE445L Fall 2020 for McDermott, Mark 

The main method for the entire system. 
===================================================================== */

#include <stdint.h>
#include "../inc/user.h"
#include "../inc/PLL.h"
#include "../inc/CortexM.h"
#include "Switch.h"
#include "BLEHandler.h"
#include "AppHandler.h"
#include "Display.h"
#include "../inc/UART1int.h"

void FakeMessage() {
	char fakeContact[] = "ID:1234c0de:11:3";
	char fakeMsg[32];
	int i = 0;
	while(fakeContact[i]) {
		fakeMsg[i] = fakeContact[i];
		i++;
	}
	fakeMsg[i++] = 0xff;
	fakeMsg[i] = '\0';
	UART1_OutString(fakeMsg);
}

int main(void) 
{
	DisableInterrupts();
	char input[256];
	PLL_Init(Bus80MHz);
	Display_Init();
	Switch_Init(&BLESwitch_Advertisement,&FakeMessage);
	BLEHandler_Init();
  EnableInterrupts();
	
	while (1) {
		BLEGet_Input(input);
	  DisplaySend_String(input);
	}
}
