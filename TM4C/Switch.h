/* =======================Switch.h======================================
Created by: Benjamin Fa, Faiyaz Mostofa, Melissa Yang, and Yongye Zhu
EE445L Fall 2020 for McDermott, Mark 

The interface for input 2 switches (PF3 and PF4).
===================================================================== */

void Switch_Init(void(*PortF3_Event)(void), void(*PortF4_Event)(void));
