/* =======================Display.c===================================
Created by: Benjamin Fa, Faiyaz Mostofa, Melissa Yang, and Yongye Zhu
EE445L Fall 2020 for McDermott, Mark 

Display interface
===================================================================== */

#include <stdint.h>
#include "Display.h"
#include "../inc/tm4c123gh6pm.h"
#include "../inc/ST7735.h"

static uint32_t CurX, CurY;

static void moveCursor() {
	CurX = 0;
	if (CurY + 1 > 15) {
		ST7735_FillScreen(ST7735_BLACK);
	}
	CurY = (CurY + 1) % 16;
	ST7735_SetCursor(CurX, CurY);
}

void DisplaySend_String(char *string) {
	ST7735_OutString(string);
	moveCursor();
}

void DisplaySend_Integer(int number) {
	if (number < 0)
		ST7735_OutString("-");
	ST7735_OutUDec((uint32_t)number);
	moveCursor();
}

void Display_Init() {
	CurX = 0;
	CurY = 0;
	Output_Init();
	ST7735_SetCursor(0, 0);
	ST7735_SetTextColor(ST7735_GREEN);
	DisplaySend_String("UART Log:");
}
