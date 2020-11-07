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
	/* Keep for now: may want to divide up contact info more in future iteration */
//	uint32_t id; // unique ID for each user that is registered on a database
//	uint8_t month;
//	uint8_t day;
} 
profile_t;

#endif // USER_H
