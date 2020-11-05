/* =======================user.h=====================================
Created by: Benjamin Fa, Faiyaz Mostofa, Melissa Yang, and Yongye Zhu
EE445L Fall 2020 for McDermott, Mark 

A class to manage modification of the user profile of the current device owner. 
===================================================================== */

#ifndef USER_H
#define USER_H

#include <stdint.h>

/** User profile thru a single encounter*/
typedef struct user_profile 
{
	char profile[32];
//	uint32_t id; // unique ID for each user that is registered on a database
//	uint8_t month;
//	uint8_t day;
} 
profile_t;

///** Initialize a basic user profile with a name and an id. Must be initialized else
//you risk attempting to change data in a null profile_t. */
//void User_Init(char*, uint8_t);

//// Getters =======================================================================

///** Returns the current user profile of the device. */
//profile_t User_Profile(void);

//// Setters =======================================================================

///** Assign a new profile for the device. */
//void UserSet_Profile(profile_t);

///** Set the name of the user profile. */
//void UserSet_Name(char*);

///** Set the id of the user profile. */
//void UserSet_Id(uint8_t);

#endif // USER_H
