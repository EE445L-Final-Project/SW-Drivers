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

/** Main Event Loop */
void BLEHandler_Main_Loop(void);

// Receiving data ====================================================

void BLEGet_Input(char *input);

// Sending data ======================================================

/** Send the given user profile to the BLE chip to advertise. */
void BLESend_Profile(profile_t);

/** Tell the BLE module to switch modes from broadcast to single-connection */
void BLESwitch_Advertisement(void);

#endif //	BLE_HANDLER_H
