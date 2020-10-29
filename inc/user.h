/* =======================user.h======================================
Created by: Benjamin Fa, Faiyaz Mostofa, Melissa Yang, and Yongye Zhu
EE445L Fall 2020 for McDermott, Mark 

A class to manage modification of the user profile of the current device owner. 
===================================================================== */

#ifndef USER_H
#define USER_H

#include <stdint.h>

/** The definition of the user profile for the owner of the device. */
typedef struct user_profile 
{
	char name[16];	// A 16 letter name
	uint8_t id;			// An 8 bit id number
} 
profile_t;

/** Initialize a basic user profile with a name and an id. Must be initialized else
you risk attempting to change data in a null profile_t. */
void User_Init(char*, uint8_t);

// Getters =======================================================================

/** Returns the current user profile of the device. */
profile_t User_Profile(void);

// Setters =======================================================================

/** Assign a new profile for the device. */
void UserSet_Profile(profile_t);

/** Set the name of the user profile. */
void UserSet_Name(char*);

/** Set the id of the user profile. */
void UserSet_Id(uint8_t);

#endif // USER_H
