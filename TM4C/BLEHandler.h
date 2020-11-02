/* =======================ble_handler.h================================
Created by: Benjamin Fa, Faiyaz Mostofa, Melissa Yang, and Yongye Zhu
EE445L Fall 2020 for McDermott, Mark 

Handles the data collection and sending between the TM4C and the BGM220P
bluetooth chip using UART1.
===================================================================== */

#ifndef BLE_HANDLER_H
#define BLE_HANDLER_H

#include <stdbool.h>
#include "../inc/user.h"

/** Initializes UART1. */
void BLEHandler_Init(void);

// Receiving data ====================================================

/** Returns true when new data was recieved from the BLE device. */
bool BLEGet_NewDataFound(void);

/** Returns the name of the owner of the device recieved by the */
profile_t BLEGet_Profile(void);

/** Returns the amount of time this device and the friend's device were in contact. 
This data is not recieved. It is collected by this device once a connection is established. */
int BLEGet_ContactTime(void);

/** Return the minimum distance between this device and the friend's device. 
This data is not recieved. It is collected by this device once a connection is established. */
int BLEGet_ContactDistance(void);

// Sending data ======================================================

/** Send the given user profile to the BLE chip to advertise. */
void BLESend_Profile(profile_t);

/** Tell the BLE module to switch modes from broadcast to single-connection */
void BLESwitch_Advertisement(void);

#endif //	BLE_HANDLER_H
