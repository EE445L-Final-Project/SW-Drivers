/* =======================display_handler.h==========================
Created by: Benjamin Fa, Faiyaz Mostofa, Melissa Yang, and Yongye Zhu
EE445L Fall 2020 for McDermott, Mark 

The high level interface for sending debugging data to the ST7735 LCD display.
The LCD is only used for debugging purposes because it requires a lot of current (and
thus a lot of power).
===================================================================== */

#ifndef DISPLAY_INTERFACE_H
#define DISPLAY_INTERFACE_H

/** Initialize Port A for the display. */
void Display_Init(void);

// Send data ============================================

/** Send a string to display. */
void DisplaySend_String(char*);

/** Display the number of the display. */
void DisplaySend_Integer(int);

#endif // DISPLAY_INTERFACE_H
