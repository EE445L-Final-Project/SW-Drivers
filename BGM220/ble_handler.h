/* =======================ble_handler.h================================
Created by: Benjamin Fa, Faiyaz Mostofa, Melissa Yang, and Yongye Zhu
EE445L Fall 2020 for McDermott, Mark 

This class handles the data communication between ble devices.
===================================================================== */

#ifndef BLE_HANDLER_H
#define BLE_HANDLER_H

#include <stdint.h>
#include <stdbool.h>

void BLE_Init(void);

// Connect ===========================================

/** Idk how the connections between BLEs work lol */
void BLE_SearchForPair(void);
void BLE_Pair(void);

/** Returns true if we are still connected with the same friend. 
We will use this to time how long we are in contact with them. */
void BLE_isConnected(void);

// Send Data =========================================

void BLESend_char(char);
void BLESend_string(char*); // Maybe this is compatible????
void BLESend_uint8(uint8_t);

// Get Data ==========================================

bool BLE_Status(void);	// Returns true when new data was recevied
char BLEGet_char(void);
char* BLEGet_string(void);
uint8_t BLEGet_uint8(void);

#endif //BLE_HANDLER_H
