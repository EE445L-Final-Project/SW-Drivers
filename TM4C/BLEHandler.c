/* =======================BLEHandler.c===============================
Created by: Benjamin Fa, Faiyaz Mostofa, Melissa Yang, and Yongye Zhu
EE445L Fall 2020 for McDermott, Mark 

BLE handler
===================================================================== */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "BLEHandler.h"
#include "../inc/user.h"
#include "../inc/UART1int.h"
#include "../inc/tm4c123gh6pm.h"

#define CONTACT_LIST_SIZE 256

profile_t Contacts[CONTACT_LIST_SIZE];
uint16_t CurContactIdx;

void BLEHandler_Init(void) {
	UART1_Init();
	CurContactIdx = 0;
}

static void addContact(char *newContact) {
	strcpy(Contacts[CurContactIdx++].profile, newContact);
	
	/* Keep for now: may want to divide up contact info more in future iteration */
//	char cpContact[32];
//	strcpy(cpContact, newContact);
//	strtok(cpContact, ":");
//	Contacts[CurContactIdx].id = (uint32_t)strtol(strtok(cpContact, ":"), NULL, 16);
//	Contacts[CurContactIdx].month = (uint8_t)strtol(strtok(NULL, ":"), NULL, 10);
//	Contacts[CurContactIdx].day = (uint8_t)strtol(strtok(NULL, ":"), NULL, 10);
//	CurContactIdx = (CurContactIdx + 1) % CONTACT_LIST_SIZE; //replace old entries if overflow
}

static void sendContacts() {
	int i = 0;
	char contactMsg[32];
	for (i = 0; i < CurContactIdx; i++) {// fix for wraparound scenario
			/* Keep for now: may want to divide up contact info more in future iteration */
//		snprintf(contactMsg, 15, "%x:%d:%d", Contacts[i].id, Contacts[i].month, Contacts[i].day);
//		UART1_OutString(contactMsg);
	  int j = 0;
	  while(Contacts[i].profile[j]) {
		  contactMsg[j] = Contacts[i].profile[j];
		  j++;
	  }
	  contactMsg[j++] = 0xff;
	  contactMsg[j] = '\0';
		UART1_OutString(contactMsg);
	}
	CurContactIdx = 0;
}

void BLESwitch_Advertisement() {
	char switchMsg[14] = {'A', 'P',  0xff, '\0'}; 
	UART1_OutString(switchMsg);
}

void BLEGet_Input(char *input) {
	uint32_t inIdx = 0;
	uint8_t inChar = 0;
	inChar = UART1_InChar();
	while (inChar != 0xff) {
		input[inIdx++] =  inChar;
		inChar = UART1_InChar();
	}
	input[inIdx] = 0;
	
	if (input[0] == 'I' && input[1] == 'D')
		addContact(input);
	else if (input[0] == 'A' && input[1] == 'P')
		sendContacts();
}
