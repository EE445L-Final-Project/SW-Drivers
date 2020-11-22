/* =======================app_handler.h================================
Created by: Benjamin Fa, Faiyaz Mostofa, Melissa Yang, and Yongye Zhu
EE445L Fall 2020 for McDermott, Mark 

The high level interface for sending data to the mobile app that's
developed alongside this system.
===================================================================== */

#ifndef APP_HANDLER_H
#define APP_HANDLER_H

#include "../inc/user.h"

/** Initialize the code for connecting and sending data to the app. */
void App_Init(void);

// Connections =======================================================

/** I have no idea how the connections to the phone are going to be handled.
If they're using bluetooth, this class will probably merge with the ble_handler class. */
//void App_Connect(void);
//void App_Disconnect(void);
//void App_Pair(void);
//void App_Find(void);

// Send Data =========================================================
/* With this implementation, we are letting the app log all the interactions while
this system is simply responsible for connecting and collecting data on other devices. 
These methods will only send information about the most recent friend we made. */

/** Send information about the device owner to the app. */
void AppSend_UserProfile(profile_t);

///** Send the profile for the newest friend to the app. */
//void AppSend_FriendProfile(profile_t);

///** Send how long this device was connected with the newest friend. */
//void AppSend_FriendContactTime(int);

///** Send the closest distance between this and our friend's device to the app. */
//void AppSend_FriendContactDistance(int);

#endif // APP_HANDLER_H
