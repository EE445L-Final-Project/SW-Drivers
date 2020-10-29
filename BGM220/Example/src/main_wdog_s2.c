/*****************************************************************************
 * @file main_wdog.c
 * @brief Watchdog Demo Application
 * @author Silicon Labs
 * @version  1.10

 ******************************************************************************
 * @section License
 * <b>(C) Copyright 2020 Silicon Labs, http://www.silabs.com</b>
 *******************************************************************************
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 * DISCLAIMER OF WARRANTY/LIMITATION OF REMEDIES: Silicon Labs has no
 * obligation to support this Software. Silicon Labs is providing the
 * Software "AS IS", with no express or implied warranties of any kind,
 * including, but not limited to, any implied warranties of merchantability
 * or fitness for any particular purpose or warranties against infringement
 * of any proprietary rights of a third party.
 *
 * Silicon Labs will not be liable for any consequential, incidental, or
 * special damages, or any other relief, or for any claim by any third party,
 * arising from your use of this Software.
 *
 ******************************************************************************/

 /*
 This project demonstrates many of the different configurations and uses
for the watchdog timer peripheral. The project includes 7 different "test"
modes illustrating the various watchdog uses:

Mode 0: Basic Watchdog - This test uses ULFRCO as watchdog clock source. It
  feeds watchdog every 50mS with usTimer driver. The onboard LED toggles and
  a '.' is printed to the VCOM during each feeding.

Mode 1: Watchdog Configuration Lock -  This test uses LFXO as watchdog clock
  source. It demonstrates the watchdog and CMU lock features. Once locked,
  the WDOG configuration CANNOT be disabled or modified. The CMU lock CAN be
  disabled and clock source changed.

Mode 2: Watchdog Interrupt -  This test uses LFRCO as watchdog clock source. 
  It tests the watchdog interrupt feature. When watchdog triggers, it generates
  an interrupt instead of resetting the MCU.

Mode 3: Watchdog PRS Clear (PRS Feeding) - This test uses ULFRCO as watchdog
  clock source. It tests watchdog PRS clear feature. The LETIMER0 is used to 
  clear the watchdog regularly (~0.5 second). The LETIMER is configured to also
  trigger an interrupt (though this is unnecessary) in order to toggle the onboard
  LED and print a '.' to the VCOM to visualize the feeding process.

Mode 4: Watchdog PRS Event Monitoring - This test uses ULFRCO as watchdog clock
  source. It feeds watchdog every 100ms seconds with usTimer driver. The LETIMER0
  trigger event is monitored on PRS channel 0 and is configured to trigger every
  50ms, twice during each feed cycle. The WDOG interrupt will trigger if the event
  does not happen within the feeding window. When the LETIMER0 is disabled, WDOG
  triggers interrupt on each WDOG feed. The onboard LED toggles and a '.' is printed
  to the VCOM during feeding process.

Mode 5: Watchdog Warning Interrupt -  This test uses ULFRCO as watchdog clock source.
  It tests the watchdog warning feature. The watchdog generates a warning every 1 seconds 
  (25% of watchdog interval; set in watchdog configuration). The watchdog is fed with the
  watchdog interrupt handler when a warning is generated. The onboard LED toggles and 
  a '.' is printed to the VCOM during feeding process.

Mode 6: Watchdog Window Interrupt - The test uses ULFRCO as watchdog clock source. 
  It tests the watchdog window feature. The watchdog generates a window interrupt when
  it is fed (1 second) before window period time has elapsed (2 seconds, 50% of watchdog
  interval period; set in watchdog configuration). The onboard LED toggles and a '.' is
  printed to the VCOM during feeding process.


How To Test:
1. Update the kit's firmware from the Simplicity Launcher (if necessary)
2. Build the project and download to the Starter Kit
3. Open a serial terminal (Putty, e.g.) using the COM port number associated with the 
   JLink CDC UART Port and set the baud rate to 115200
4. Run the application or disconnect the debugger to allow the program to run freely
5. Use PB1 on the WSTK to cycle through the different tests
6. Use PB2 to start a test
7. Observe any on screen instructions
8. Many of the tests will stop feeding the watchdog and let the system reset; press
   the reset button once completed to go back to the main test menu
9. The interrupt test will continue indefinitely; again, press the reset button to 
   go back to the main test menu  
 
Peripherals Used:
WDOG         - 32768 Hz LFXO
               32768 Hz LFRCO
               1000 Hz ULFRCO
PRS          - LETIMER0 signal to WDOG consumer			   
LETIMER0     - Load comp0 register into counter register when underflow
               Count until stopped by software
VCOM         - Virtual COM port at 115200 baud

Board:  Silicon Labs EFR32xG21 Radio Board (BRD4181A) + 
        Wireless Starter Kit Mainboard
Device: EFR32MG21A010F1024IM32
PB01 - GPIO Push/Pull output, Expansion Header Pin 13, WSTK Pin 10, LED1
PD02 - GPIO input pull filter, PB0
PD03 - GPIO input pull filter, PB1

Board:  Silicon Labs EFR32xG22 Radio Board (BRD4182A) + 
        Wireless Starter Kit Mainboard
Device: EFR32MG22C224F512IM40
PD03 - GPIO Push/Pull output, Expansion Header Pin 13, WSTK Pin 10, LED1
PB00 - GPIO input pull filter, PB0
PB01 - GPIO input pull filter, PB1
*/

#include "stdio.h"
#include "em_device.h"
#include "em_chip.h"
#include "em_cmu.h"
#include "em_emu.h"
#include "em_wdog.h"
#include "em_rmu.h"
#include "em_timer.h"
#include "em_letimer.h"
#include "bspconfig.h"
#include "retargetserial.h"
#include "ustimer.h"

/*******************************************************************************
 *******************************   DEFINES   ***********************************
 ******************************************************************************/

/* Defining various test modes for user selection */
#define  T_DOG     0
#define  T_LCK     1
#define  T_INT     2
#define  T_CLR     3
#define  T_EVT     4
#define  T_WRN     5
#define  T_WIN     6
#define  T_END     7

/*******************************************************************************
 ***************************   GLOBAL VARIABLES   ******************************
 ******************************************************************************/

volatile int tMode;            /* Selected test mode */
volatile int startTest;        /* Start test key pressed */

/* Main test function from wdog_mode_test.c */
void modeTest(unsigned int tMode);

/******************************************************************************
* @brief Setup GPIO interrupt to change test mode.
*****************************************************************************/
void initGPIO(void)
{
  /* Enable GPIO in CMU */
  CMU_ClockEnable(cmuClock_GPIO, true);

  /* Configure PB0 as input */
  GPIO_PinModeSet(BSP_GPIO_PB0_PORT, BSP_GPIO_PB0_PIN, gpioModeInputPullFilter, 1);
  GPIO_ExtIntConfig(BSP_GPIO_PB0_PORT, BSP_GPIO_PB0_PIN, BSP_GPIO_PB0_PIN, false, true, true);

  /* Configure PB1 as input */
  GPIO_PinModeSet(BSP_GPIO_PB1_PORT, BSP_GPIO_PB1_PIN, gpioModeInputPullFilter, 1);
  GPIO_ExtIntConfig(BSP_GPIO_PB1_PORT, BSP_GPIO_PB1_PIN, BSP_GPIO_PB1_PIN, false, true, true);

  /* Enable interrupt */
  NVIC_ClearPendingIRQ(GPIO_ODD_IRQn);
  NVIC_EnableIRQ(GPIO_ODD_IRQn);
  NVIC_ClearPendingIRQ(GPIO_EVEN_IRQn);
  NVIC_EnableIRQ(GPIO_EVEN_IRQn);

  /* Configure LED GPIO */
  GPIO_PinModeSet(BSP_GPIO_LED1_PORT, BSP_GPIO_LED1_PIN, gpioModePushPull, 0);
}

/******************************************************************************
* @brief GPIO Interrupt handler (Pushbutton for test start).
* Button 0 to start the test and button 1 to choose test mode
*****************************************************************************/
void GPIO_Unified_IRQ(void)
{
  /* Get and clear all pending GPIO interrupts */
  uint32_t interruptMask = GPIO_IntGet();
  GPIO_IntClear(interruptMask);

  /* Act on interrupts */
  if (interruptMask & (1 << BSP_GPIO_PB0_PIN))
  {
    /* BTN0: Start test */
    startTest = true;
  }

  if (interruptMask & (1 << BSP_GPIO_PB1_PIN))
  {
    /* BTN1: cycle through tests */
    tMode = (tMode + 1) % T_END;
  }
}

void GPIO_ODD_IRQHandler(void)
{
  GPIO_Unified_IRQ();
}

void GPIO_EVEN_IRQHandler(void)
{
  GPIO_Unified_IRQ();
}

/******************************************************************************
 * @brief TIMER1 Interrupt Handler. Clears interrupt flag.
 *        The interrupt table is in assembly startup file startup_efm32.s
 *****************************************************************************/
void TIMER1_IRQHandler(void)
{
  /* Clear flag for TIMER1 overflow interrupt */
  TIMER_IntClear(TIMER1, TIMER_IF_OF);
}

/******************************************************************************
 * @brief  TIMER1 initialization.
 *****************************************************************************/
void initTimer(void)
{
    uint32_t timerClkFreq = 0;
    uint32_t timerIntFreq = 20; // 20Hz; 50ms toggle
    TIMER_Init_TypeDef timerInit = TIMER_INIT_DEFAULT;
    uint32_t topValue = 0;

    /* Enable TIMER1 clock */
    CMU_ClockEnable(cmuClock_TIMER1, true);
    CMU_ClockSelectSet(cmuClock_EM01GRPACLK, cmuSelect_FSRCO);

    /* Choose the prescaler, and initialize the timer, without starting */
    timerInit.prescale = timerPrescale512;
    timerInit.enable = false;
    timerInit.mode = timerModeUp;

    /* configure timer */
    TIMER_Init(TIMER1, &timerInit);

    timerClkFreq = CMU_ClockFreqGet(cmuClock_TIMER1) / (timerInit.prescale + 1);
    topValue = (timerClkFreq / timerIntFreq);
    TIMER_TopSet(TIMER1, topValue);

    /* Enable TIMER1 interrupt from overflow */
    TIMER_IntEnable(TIMER1, TIMER_IF_OF);

    /* Enable TIMER1 interrupt vector in NVIC */
    NVIC_EnableIRQ(TIMER1_IRQn);

    /* Start the timer */
    TIMER_Enable(TIMER1, true);
}

/******************************************************************************
 * @brief LETIMER0 Interrupt Handler. Clears interrupt flag.
 *        The interrupt table is in assembly startup file startup_efm32.s
 *****************************************************************************/
void LETIMER0_IRQHandler(void)
{
  /* Clear LETIMER0 underflow interrupt flag */
  LETIMER_IntClear(LETIMER0, LETIMER_IF_UF);
}

/******************************************************************************
 * @brief  Main function
 *         Main is called from __iar_program_start, see assembly startup file
 *
 * The purpose of this example is to demonstrate some basic functionality of the
 * WDOG, and while doing so print feedback through the Virtual COM port.
 * After initialization and corresponding enabling of the WDOG, the program
 * enters a loop where feedback is written to the VCOM, while the WDOG timer is
 * cleared timely. After some time, the program stops feeding the WDOG, and the
 * WDOG triggers an WDOG event. For some test modes, this is handled by an interrupt,
 * for others the WDOG generates a system reset. The cause of the last reset is
 * detected during initialization, and the system enters an infinite loop without
 * enabling the WDOG. The user is asked to press the reset button to start another test.
 *
 *****************************************************************************/
int main(void)
{
  uint32_t resetCause;      /* Reset cause */

  /* Initialize chip */
  CHIP_Init();

  /* Initialize DC-DC */
  RMU_ResetControl(rmuResetWdog0, rmuResetModeEnabled);

  /* Store the cause of the last reset, and clear the reset cause register */
  resetCause = RMU_ResetCauseGet();
  RMU_ResetCauseClear();

  /* Retarget stdout to Virtual COM */
  RETARGET_SerialInit(); 

  /* Check if the watchdog triggered the last reset */
  if (resetCause & EMU_RSTCAUSE_WDOG0)
  {
    /* Write feedback to console */
    printf("\r\n\n\nWatchdog Timer System Reset!\n");
    printf("\r\nRMU Reset Cause Register: ");
    printf("0x%x\n", (unsigned int) resetCause);

    printf("\r\nPress RESET button to run another test  ");

    /* Stay in this loop forever */
    while (1)
    {
      EMU_EnterEM2(false);
    }
  }

  /* Initialize GPIO */
  initGPIO();

  uint32_t prevMode = T_WIN;   /* forces display before any button presses */
  startTest = false;

  /* Display information to guide user */
  printf("\f");
  printf("  Push PB1 to           Push PB0 to\n\r"
         "  cycle modes           start test\n\n\r");

  /* Loop to select test mode before start key pressed */
  while (!startTest)
  {
    if(prevMode != tMode){
    switch (tMode)
    {
      case T_DOG:
        printf("\rMode 0: Basic Watchdog         ");
        break;
      case T_LCK:
        printf("\rMode 1: Lock Operation         ");
        break;
      case T_INT:
        printf("\rMode 2: Watchdog Interrupt     ");
        break;
      case T_CLR:
        printf("\rMode 3: PRS Watchdog Clear     ");
        break;
      case T_EVT:
        printf("\rMode 4: PRS Event Monitoring   ");
        break;
      case T_WRN:
        printf("\rMode 5: Warning Interrupt      ");
        break;
      case T_WIN:
        printf("\rMode 6: Window Interrupt       ");
        break;
    }
    prevMode = tMode;

    }
    USTIMER_Delay(1000000);
  }

  /* Run the test */
  modeTest(tMode);
}
