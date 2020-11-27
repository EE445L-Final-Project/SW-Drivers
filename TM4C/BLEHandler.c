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

#define gattdb_device_name 11
#define gattdb_fake_device_name 31
#define gattdb_data_ready 27
#define gattdb_contact_user 21


SL_BT_API_DEFINE();
static void sl_bt_on_event(sl_bt_msg_t* evt);
static void uart_tx_wrapper(uint32_t len, uint8_t* data);
static int32_t uartRx(uint32_t len, uint8_t* data);
static int32_t uartRxPeek(void);

static struct sl_bt_evt_scanner_scan_report_s report;
static const int8_t MIN_RSSI = -60;

#define CONTACT_LIST_SIZE 256
profile_t Contacts[CONTACT_LIST_SIZE];
uint16_t CurContactIdx;

static char message[100];

void BLEHandler_Init(void) {
	SL_BT_API_INITIALIZE(uart_tx_wrapper, uartRx);
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

static void addContact(profile_t newContact) {
	strcpy(Contacts[CurContactIdx++].profile, newContact.profile);
	
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

//void BLEGet_Input(char *input) {
//	uint32_t inIdx = 0;
//	uint8_t inChar = 0;
//	inChar = UART1_InChar();
//	while (inChar != 0xff) {
//		input[inIdx++] =  inChar;
//		inChar = UART1_InChar();
//	}
//	input[inIdx] = 0;
//	
//	if (input[0] == 'I' && input[1] == 'D')
//		addContact(input);
//	else if (input[0] == 'A' && input[1] == 'P')
//		sendContacts();
//}


//****************************************//
//        Helper Functions                //
//****************************************//

typedef struct _user_profile{
	uint8_t name[7];
	uint8_t date[3];
	uint8_t time[2];
} user_profile_t;

user_profile_t dummy_profile[10] = {
{{0x44, 0x65, 0x76, 0x69, 0x63, 0x65, 0x30}, {9,  20, 20}, {12, 20}}, 
{{0x44, 0x65, 0x76, 0x69, 0x63, 0x65, 0x31}, {10, 20, 20}, {12, 20}}, 
{{0x44, 0x65, 0x76, 0x69, 0x63, 0x65, 0x32}, {11, 20, 20}, {12, 20}}, 
{{0x44, 0x65, 0x76, 0x69, 0x63, 0x65, 0x33}, {12, 20, 20}, {12, 20}},
{{0x44, 0x65, 0x76, 0x69, 0x63, 0x65, 0x34}, {1, 20, 20}, {12, 20}},
{{0x44, 0x65, 0x76, 0x69, 0x63, 0x65, 0x35}, {2, 20, 20}, {12, 20}},
{{0x44, 0x65, 0x76, 0x69, 0x63, 0x65, 0x36}, {3, 20, 20}, {12, 20}},
{{0x44, 0x65, 0x76, 0x69, 0x63, 0x65, 0x37}, {4, 20, 20}, {12, 20}},
{{0x44, 0x65, 0x76, 0x69, 0x63, 0x65, 0x38}, {5, 20, 20}, {12, 20}},
{{0x44, 0x65, 0x76, 0x69, 0x63, 0x65, 0x39}, {6, 20, 20}, {12, 20}}
};


static bool existingContact(char* name){
	for(int i = 0; i < CONTACT_LIST_SIZE; i++){
		if(strcmp(name, Contacts[i].profile) == 0){
			return true;
		}
	}
	return false;
}

static bool validBLE(struct sl_bt_evt_scanner_scan_report_s* report){
	if(report->rssi < MIN_RSSI){ return false; }
	uint8array data = report->data;
	if(data.len < 18){ return false; }
	return data.data[3] == 0x05 
			&& data.data[4] == 0xFF 
			&& data.data[5] == 0xFF 
			&& data.data[6] == 0x02 
			&& data.data[7] == 0x00 
			&& data.data[8] == 0xFF;
}

static void parseData(uint8array data, profile_t* profile){
	char name[7];
	sprintf(name, "%c%c%c%c%c%c%d", (char)data.data[11], (char)data.data[12], (char)data.data[13], (char)data.data[14], (char)data.data[15], (char)data.data[16], data.data[17]);
	strcpy(profile->profile, name);
	//profile.profile = "";
	//profile_t profile;
	//return profile;
}

//****************************************//
//        Event Handler                   //
//****************************************//
static void sl_bt_on_event(sl_bt_msg_t* evt){
	sl_status_t sc;
	
	bd_addr address;
	uint8_t address_type;
	uint8_t system_id[8];
	
	static uint8_t advertising_set_handle = 0xff;
	static uint8_t adv_data_len = 17;
	static uint8_t adv_data[31]={
		0x02, 0x01, 0x06, // flags
		0x05, 0xFF, 0xFF, 0x02, 0x00, 0xFF, // specific data (identifier: 0x00FF)
		0x07, 0x08, 0x44, 0x65, 0x76, 0x69, 0x63, 0x65 // shortened local name
	};
	static int profile_index = 0;
	 
	
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
				break;
			}
			sprintf(message, "%s Address:\n %02X:%02X:%02X:%02X:%02X:%02X\n", address_type? "static random": "public device", address.addr[5], address.addr[4], address.addr[3], address.addr[2], address.addr[1], address.addr[0]);
			ST7735_OutString(message);
			

			uint8_t device_name[] = {0x44, 0x65, 0x76, 0x69, 0x63, 0x65};
			sc = sl_bt_gatt_server_write_attribute_value(gattdb_device_name, 0, 6 , device_name);
			if(sc != SL_STATUS_OK){
				ST7735_OutString("Failed to set attribute\n");
			}
				
			// Create an advertising set.
      sc = sl_bt_advertiser_create_set(&advertising_set_handle);
			if (sc != SL_STATUS_OK){
				ST7735_OutString("Failed to create advertising set\n");
				break;
			}
			
			
		 // Set advertising data
			sc = sl_bt_advertiser_set_data(advertising_set_handle, 0, adv_data_len, adv_data);
			if (sc != SL_STATUS_OK){
				ST7735_OutString("Failed to set advertising data\n");
				break;
			}
			
      // Set advertising interval to 100ms.
      sc = sl_bt_advertiser_set_timing(
        advertising_set_handle,
        160, // min. adv. interval (milliseconds * 1.6)
        160, // max. adv. interval (milliseconds * 1.6)
        0,   // adv. duration
        0);  // max. num. adv. events
			if (sc != SL_STATUS_OK){
				ST7735_OutString("Failed to set \nadvertising timing\n");
				break;
			}
				
      // Start general advertising and enable connections.
      sc = sl_bt_advertiser_start(
        advertising_set_handle,
        advertiser_user_data,
        advertiser_connectable_scannable);
			if (sc != SL_STATUS_OK){
				ST7735_OutString("Failed to start advertising\n");
				break;
			}
			
//			// Start scanning
//			sc = sl_bt_scanner_start(1, 1);
//			if(sc != SL_STATUS_OK){
//				ST7735_OutString("Failed to start scanning\n");
//			}
			
			break;
		}
		case sl_bt_evt_connection_opened_id:{
			ST7735_OutString("New Connection Opened\n");
			profile_index = 0;
			break;
		}
		case sl_bt_evt_connection_closed_id:{
			ST7735_OutString("Connection Closed\n");
      // Start general advertising and enable connections.
      sc = sl_bt_advertiser_start(
        advertising_set_handle,
        advertiser_user_data,
        advertiser_connectable_scannable);
			if (sc != SL_STATUS_OK){
				ST7735_OutString("Failed to start advertising\n");
				break;
			}
			break;
		}
		
		case sl_bt_evt_gatt_server_attribute_value_id:{
			switch (evt->data.evt_gatt_server_attribute_value.attribute){
				case gattdb_fake_device_name: {
					ST7735_OutString("Device Name Changed\n");
					size_t value_len;
					const size_t max_length = 15;
					uint8_t value[16];
					sc = sl_bt_gatt_server_read_attribute_value(gattdb_fake_device_name, 0, max_length, &value_len, value);
					if(sc != SL_STATUS_OK){
						ST7735_OutString("Failed to get device name\n");
					}	
					sc = sl_bt_gatt_server_write_attribute_value(gattdb_device_name, 0, value_len , value);
					if(sc != SL_STATUS_OK){
						ST7735_OutString("Failed to set attribute\n");
					}
					adv_data_len = 11 + value_len;
					adv_data[9] = value_len + 1;
					memcpy(adv_data + 11, value, value_len);					
					sc = sl_bt_advertiser_set_data(advertising_set_handle, 0, adv_data_len, adv_data);
					if (sc != SL_STATUS_OK){
						ST7735_OutString("Failed to set advertising data\n");
						break;
					}				
					break;
				}
				case gattdb_data_ready: {
					uint8_t ready = 1;
					uint16_t sent_len;
					uint8_t user_profile[12];
					memcpy(user_profile, dummy_profile[profile_index].name, 7);
					memcpy(user_profile + 7, dummy_profile[profile_index].date, 3);
					memcpy(user_profile + 10, dummy_profile[profile_index].time, 2);
					sc = sl_bt_gatt_server_write_attribute_value(gattdb_contact_user, 0, 12, user_profile);
					if(sc != SL_STATUS_OK){
						ST7735_OutString("Fail to write user profile\n");	
					}
					if(profile_index == 10){
						return;
					}
					profile_index ++;
					sc = sl_bt_gatt_server_send_characteristic_notification(0xff, gattdb_data_ready, 1, &ready, &sent_len);
					if(sc != SL_STATUS_OK){
						ST7735_OutString("Fail to write user profile\n");	
					}

				}
				default:
					break;			
			}
			break;
		}
		
		case sl_bt_evt_scanner_scan_report_id:{
			report = evt->data.evt_scanner_scan_report;
			if (validBLE(&report)){
				profile_t profile;
				parseData(report.data, &profile);
				addContact(profile);
			}
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



