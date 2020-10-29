/* =======================tm4c_handler.h================================
Created by: Benjamin Fa, Faiyaz Mostofa, Melissa Yang, and Yongye Zhu
EE445L Fall 2020 for McDermott, Mark 

This class handles the data receiving and sending to the TM4C. 
===================================================================== */

#ifndef TM4C_HANDLER_H
#define TM4C_HANDLER_H

#include <stdbool.h>
#include "../inc/user.h"

/** Initializes the UART needed to communicate with the TM4C. */
void TM4C_Init(void);

// Send Data =====================================================

/** We connected with someone, and got their profile! Send that 
data over to the TM4C for processing. */
void TM4CSend_Profile(profile_t);

/** Send the connection status to the TM4C so it can use it for logging
and calculations. */
void TM4CSend_ConnectionStatus(void);

// Receive Data ==================================================

/** The TM4C is updating the info on the user profile. Broadcast this
new data instead of our old data. */
profile_t TM4CGet_Profile(void);

#endif // TM4C_HANDLER_H
