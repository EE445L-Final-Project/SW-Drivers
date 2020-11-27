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
#include "./BGLib/sl_bt_api.h"
#include "./BGLib/sl_bt_ncp_host.h"
#include "../inc/ST7735.h"
#include "Timer.h"

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

extern int time;
static int second;
uint8_t month;
uint8_t day;
uint8_t year;

void Timer_Task(){
	if(second == 59){
		second = 0;
		if(time % 100 == 59){
			if(time / 100 == 23){
				time = 0;
				switch(month){
					case 1:
					case 3:
					case 5:
					case 7:
					case 8:
					case 10:
						if(day == 31){
							day = 1;
							month++;
						}
						else{
							day++;
						}
						break;
					case 2:
						if(day == (year % 4 == 0 ? 29 : 28)){
								day = 1;
								month++;
							}
							else{
								day++;
							}
						break;
					case 4:
					case 6:
					case 9:
					case 11:
						if(day == 30){
							day = 1;
							month++;
						}
						else{
							day++;
						}
						break;
					case 12:
						if(day == 31){
							month = 1;
							day = 1;
							year = (year + 1) % 100;
						}
						else{
							day++;
						}
						break;
				}
			}
			else{
				time = (time / 100 + 1) * 100;
			}
		}
		else{
			time += 1;
		}
	}
	else{
		second += 1;
	}
}

int main(void) 
{
	DisableInterrupts();
	char input[256];
	PLL_Init(Bus80MHz);
	Display_Init();
	//Switch_Init(&BLESwitch_Advertisement,&FakeMessage);
	Timer0A_Init1HzInt(&Timer_Task);
	BLEHandler_Init();
  EnableInterrupts();
//	sl_status_t sc;
//	sc = sl_bt_system_hello();
//	if(sc != SL_STATUS_OK){
//		DisplaySend_String("Connection Failed");
//	}
	
	month = 11;
	day = 27;
	year = 20;
	
	ST7735_OutString("\nHello WOrld");
	while (1) {
		
		//BLEHandler_Main_Loop();
	}
}
