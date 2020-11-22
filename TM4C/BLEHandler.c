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
#include "./BGLib/sl_bt_api.h"
#include "./BGLib/sl_bt_ncp_host.h"
#include "../inc/ST7735.h"

SL_BT_API_DEFINE();
static void sl_bt_on_event(sl_bt_msg_t* evt);
static void uart_tx_wrapper(uint32_t len, uint8_t* data);
static int32_t uartRx(uint32_t len, uint8_t* data);
static int32_t uartRxPeek(void);

#define CONTACT_LIST_SIZE 256

profile_t Contacts[CONTACT_LIST_SIZE];
uint16_t CurContactIdx;

static char message[100];

void BLEHandler_Init(void) {
	SL_BT_API_INITIALIZE_NONBLOCK(uart_tx_wrapper, uartRx, uartRxPeek);
	UART1_Init();
	ST7735_OutString("EE445L Final\nInitializing BLE...");
	CurContactIdx = 0;
	
	//sl_bt_system_reset(0);
}

void BLEHandler_Main_Loop(void){
	sl_bt_msg_t evt;
	
	sl_status_t status = sl_bt_pop_event(&evt);
	if(status != SL_STATUS_OK) return;
	sl_bt_on_event(&evt);
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


//****************************************//
//        Private Function                //
//****************************************//

//****************************************//
//        Event Handler                   //
//****************************************//
static void sl_bt_on_event(sl_bt_msg_t* evt){
	sl_status_t sc;
	bd_addr address;
	uint8_t address_type;
	uint8_t system_id[8];
	
	switch(SL_BT_MSG_ID(evt->header)){
		case sl_bt_evt_system_boot_id:{
			ST7735_OutString("Connection Success");
			sprintf(message, "Bluttooth Stack \nBooted:\n v%d.%d.%d-b%d\n", evt->data.evt_system_boot.major, evt->data.evt_system_boot.minor, 
																											evt->data.evt_system_boot.patch, evt->data.evt_system_boot.build);
			ST7735_OutString(message);
			sc = sl_bt_system_hello();
			if(sc != SL_STATUS_OK){
				ST7735_OutString("Connection Failed");
			}
			sc = sl_bt_system_get_identity_address(&address, &address_type);
			if(sc != SL_STATUS_OK){
				ST7735_OutString("Failed to get address");
			}
			sprintf(message, "%s Address:\n %02X:%02X:%02X:%02X:%02X:%02X", address_type? "static random": "public device", address.addr[5], address.addr[4], address.addr[3], address.addr[2], address.addr[1], address.addr[0]);
			ST7735_OutString(message);
			break;
		}
		case sl_bt_evt_connection_opened_id:{
			ST7735_OutString("New Connection Opened");
			break;
		}
		case sl_bt_evt_connection_closed_id:{
			ST7735_OutString("Connection Closed");
			break;
		}
		default:
			break;
	}
	
}



//****************************************//
//        UART_TX_WRAPPER                 //
//****************************************//
static void uart_tx_wrapper(uint32_t len, uint8_t* data){
	for(int i = 0; i < len; i++){
		UART1_OutChar(data[i]);
	}
}

//****************************************//
//        UART_RX                         //
//****************************************//
static int32_t uartRx(uint32_t len, uint8_t* data){
		int32_t total = 0;
		for(int i = 0; i < len; i++){
			data[i] = UART1_InChar();
			total++;
		}
		return total;
}
//****************************************//
//        UART_RX_PEEK                    //
//****************************************//
static int32_t uartRxPeek(void){
	return (int32_t)UART1_InStatus();
}



