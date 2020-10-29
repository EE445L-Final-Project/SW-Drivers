/*****************************************************************************
 * @file wdog_mode_test.c
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
#include "stdio.h"
#include "em_emu.h"
#include "em_cmu.h"
#include "em_wdog.h"
#include "em_prs.h"
#include "em_gpio.h"
#include "em_letimer.h"
#include "bspconfig.h"
#include "ustimer.h"

#include <stdbool.h>

/*******************************************************************************
 ***************************   GLOBAL VARIABLES   ******************************
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

extern volatile int tMode;    /* Selected test mode */
unsigned int i;

/*******************************************************************************
 ***************************   GLOBAL FUNCTIONS   ******************************
 ******************************************************************************/

void initTimer(void);         /* Located in main with other initializations */

/******************************************************************************
 * @brief WDOG Interrupt Handler. Clears interrupt flag.
 *        The interrupt table is in assembly startup file startup_efm32.s
 *****************************************************************************/
void WDOG0_IRQHandler(void)
{
  /* Clear flag for interrupt */
  switch (tMode)
  {
    case T_INT:
      printf("\r\nWatchdog interrupt: Overflow");
      WDOGn_IntClear(DEFAULT_WDOG, WDOG_IEN_TOUT);
      GPIO_PinOutToggle(BSP_GPIO_LED1_PORT, BSP_GPIO_LED1_PIN);
      break;

    case T_EVT:
      printf("\r\nWatchdog event: Missing PRS Event");
      WDOGn_IntClear(DEFAULT_WDOG, WDOG_IEN_PEM0);
      GPIO_PinOutToggle(BSP_GPIO_LED1_PORT, BSP_GPIO_LED1_PIN);
      break;

    case T_WRN:
      printf("\r\nWatchdog warning: Threshold exceeded");
      WDOGn_IntClear(DEFAULT_WDOG, WDOG_IEN_WARN);
      GPIO_PinOutToggle(BSP_GPIO_LED1_PORT, BSP_GPIO_LED1_PIN);
      WDOGn_Feed(DEFAULT_WDOG);
      break;

    case T_WIN:
      printf("\r\nWatchdog Window interrupt");
      WDOGn_IntClear(DEFAULT_WDOG, WDOG_IEN_WIN);
      GPIO_PinOutToggle(BSP_GPIO_LED1_PORT, BSP_GPIO_LED1_PIN);
      break;
  }
}

/******************************************************************************
 * @brief  Watchdog reset test.
 *
 * The test uses ULFRCO as watchdog clock source. It feeds watchdog every 50mS with usTimer
 * driver. The onboard LED toggles and a '.' is printed to the VCOM during feeding process.
 *****************************************************************************/
void testDog(void)
{
  WDOG_Init_TypeDef initWDOG = WDOG_INIT_DEFAULT;

  /* Configure Watchdog timer */
  CMU_ClockEnable(cmuClock_WDOG0, true);
  CMU_ClockSelectSet(cmuClock_WDOG0, cmuSelect_ULFRCO); /* ULFRCO as clock source */

  initWDOG.perSel = wdogPeriod_4k;      /* Set the watchdog period to 4092 clock periods (ie ~4 seconds) */

  printf("Starting WDOG basic test\r\n");

  /* Initializing watchdog and microsecond timers */
  USTIMER_Init();
  WDOGn_Init(DEFAULT_WDOG, &initWDOG);

  /* Do something for a while while feeding the watchdog to prevent a time out */
  printf("\r\n\nFeeding watchdog timer...");
  for (i = 0; i < 40; i++)
  {
    /* Processing takes place here */
    USTIMER_Delay(50000);
    GPIO_PinOutToggle(BSP_GPIO_LED1_PORT, BSP_GPIO_LED1_PIN);
    WDOGn_Feed(DEFAULT_WDOG);
    printf(".");
  }

  /* Stop feeding the watchdog, and it will trigger a reset */
  printf("\r\nStop feeding; wait for watchdog to trigger a reset (~4s)...\r\n");

  /* Enter loop, and wait for watchdog reset */
  while (1);
}

/******************************************************************************
 * @brief  Watchdog Lock test.
 *
 * The test uses LFXO as watchdog clock source. It demonstrates the watchdog and
 * CMU lock features. Once locked, the WDOG configuration CANNOT be disabled or
 * modified. The CMU lock CAN be disabled and clock source changed.
 *****************************************************************************/
void testLock(void)
{
  WDOG_Init_TypeDef initWDOG = WDOG_INIT_DEFAULT;

  CMU_ClockEnable(cmuClock_WDOG0, true);
  CMU_ClockSelectSet(cmuClock_WDOG0, cmuSelect_LFXO);

  initWDOG.em2Run = true;
  initWDOG.perSel = wdogPeriod_4k;    /* Set the watchdog period to 32k clock periods (ie ~1/8 second) */
  initWDOG.lock = true;                /* after initialization, locks configuration register */

  printf("Starting WDOG lock test\r\n");

  /* Initializing watchdog and Timer1 */
  initTimer();
  WDOGn_Init(DEFAULT_WDOG, &initWDOG);

  /* Also lock watchdog clock source configuration */
  CMU_WdogLock();

  /*****************************************************
   *  Any attempts to change the watchdog timer while locked will result in a hard-fault!!
   *  Once locked, the watchdog timer cannot be disabled or reconfigured by software
   *****************************************************/

  WDOGn_Enable(DEFAULT_WDOG, false);   /* emlib library has built-in code that checks the lock status;
                                          prevents the user from doing this by accident */

  printf("\r\n\nWatchdog initialized and configuration locked."
          "\r\nOnly a reset can unlock the WDOG configuration.");

  /* Do something for a while while feeding the watchdog to prevent a time out */
  printf("\r\n\nFeeding watchdog timer (LFXO clock source)...");
  for (i = 0; i < 40; i++)
  {
    /* Enter EM1 while the watchdog is still counting */
    EMU_EnterEM1();

    /* Timer1 wakeup to feed watchdog */
    GPIO_PinOutToggle(BSP_GPIO_LED1_PORT, BSP_GPIO_LED1_PIN);
    WDOGn_Feed(DEFAULT_WDOG);
    printf(".");
  }

  printf("\r\n\nUnlock CMU and modify watchdog clock source...");

  /* Unlock CMU configuration registers */
  CMU_WdogUnlock();         /* Note that CMU_WDOGLOCK register is a write only register;
                               the unlock key will not be visible through the register view - always reads 0x0 */

  /* Change source clock */
  CMU_ClockSelectSet(cmuClock_WDOG0CLK, cmuSelect_ULFRCO); /* ULFRCO clock is 1000 Hz compared to LFXO 32768 Hz clock
                                                              Watchdog reset will now take ~32s  */

  /* Relock CMU configuration for watchdog timer */
  CMU_WdogLock();

  /* Do something for a while while feeding the watchdog to prevent a time out */
  printf("\r\n\nFeeding watchdog timer (ULFRCO clock source)...");
  for (i = 0; i < 40; i++)
  {
    /* Enter EM1 while the watchdog is still counting */
    EMU_EnterEM1();

    /* Timer1 wakeup to feed watchdog */
    GPIO_PinOutToggle(BSP_GPIO_LED1_PORT, BSP_GPIO_LED1_PIN);
    WDOGn_Feed(DEFAULT_WDOG);
    printf(".");
  }

  /* Stop feeding the watchdog, and it will trigger a reset */
  printf("\r\nStop feeding; Watchdog timer will trigger a reset (~4s)...\r\n");

  /* Enter loop, and wait for watchdog reset */
  while (1);
}

/******************************************************************************
 * @brief  Interrupt test.
 *
 * The test uses LFRCO as watchdog clock source. It tests the watchdog interrupt feature.
 * When watchdog triggers, it generates an interrupt instead reset the MCU.
 *****************************************************************************/
void testInterrupt(void)
{
  WDOG_Init_TypeDef initWDOG = WDOG_INIT_DEFAULT;

  CMU_ClockEnable(cmuClock_WDOG0, true);
  CMU_ClockSelectSet(cmuClock_WDOG0, cmuSelect_LFRCO);

  initWDOG.enable = false;              /* Wait and enable after enabling interrupts */
  initWDOG.em2Run = true;
  initWDOG.perSel = wdogPeriod_32k;     /* Set the watchdog period to 32K clock periods (ie ~1 second) */
  initWDOG.resetDisable = true;         /* Disable watchdog reset the device when fired */

  printf("Starting WDOG interrupt test\r\n");

  /* Initializing watchdog and microsecond timers */
  USTIMER_Init();
  WDOGn_Init(DEFAULT_WDOG, &initWDOG);

  /* Enable watchdog interrupt */
  WDOGn_IntEnable(DEFAULT_WDOG, WDOG_IEN_TOUT);
  NVIC_EnableIRQ(WDOG0_IRQn);

  /* Start watchdog counter */
  WDOGn_Enable(DEFAULT_WDOG, true);

  /* Do something for a while while feeding the watchdog to prevent a time out */
  printf("\r\n\nFeeding watchdog timer...");
  for (i = 0; i < 40; i++)
  {
    /* Processing takes place here */
    USTIMER_Delay(50000);
    GPIO_PinOutToggle(BSP_GPIO_LED1_PORT, BSP_GPIO_LED1_PIN);
    WDOGn_Feed(DEFAULT_WDOG);
    printf(".");
  }

  /* Stop feeding the watchdog; will trigger a watchdog interrupt */
  printf("\r\nStop feeding; wait for watchdog to trigger an interrupt...\r\n");

  /* Enter loop, and wait for user reset */
  while (1) ;
}

/******************************************************************************
 * @brief  PRS clear test.
 *
 * The test uses ULFRCO as watchdog clock source. It tests watchdog PRS clear feature.
 * The LETIMER0 is used to clear the watchdog regularly (~0.5 second). The LETIMER is
 * configured to also trigger an interrupt (though this is unnecessary) in order to
 * toggle the onboard LED and print a '.' to the VCOM to visualize the feeding process.
 *****************************************************************************/
void testClear(void)
{
  WDOG_Init_TypeDef initWDOG = WDOG_INIT_DEFAULT;
  LETIMER_Init_TypeDef LETIMERInit = LETIMER_INIT_DEFAULT;
  uint32_t clockCnt = 50; /* corresponds to 50ms LETIMER when using ULFRCO */

  /* Use ULFRCO for all low energy peripherals including LETIMER */
  CMU_ClockSelectSet(cmuClock_EM23GRPACLK, cmuSelect_ULFRCO);

  /* Enable LETIMER0 clock */
  CMU_ClockEnable(cmuClock_LETIMER0, true);

  /* Configure LETIMER for 50ms period and toggle output on PRS */
  LETIMERInit.comp0Top = true;
  LETIMERInit.topValue = clockCnt;
  LETIMERInit.ufoa0 = letimerUFOAToggle;

  /* Time out value configuration */
  LETIMER_CompareSet(LETIMER0, 0, clockCnt);

  /* Initializing LETIMER with chosen settings */
  LETIMER_Init(LETIMER0, &LETIMERInit);

  /* Enable underflow interrupt */
  LETIMER_IntEnable(LETIMER0, LETIMER_IF_UF);
  NVIC_EnableIRQ(LETIMER0_IRQn);

  /* Enable PRS clock */
  CMU_ClockEnable(cmuClock_PRS, true);

  /* Configure LETIMER0 to create PRS interrupt signal and watchdog to consume as feed */
  PRS_ConnectSignal(0, prsTypeAsync, prsSignalLETIMER0_CH0);
  PRS_ConnectConsumer(0, prsTypeAsync, prsConsumerWDOG0_SRC0);

  CMU_ClockEnable(cmuClock_WDOG0, true);
  CMU_ClockSelectSet(cmuClock_WDOG0, cmuSelect_ULFRCO);

  initWDOG.enable = false;              /* Wait until configuration includes PRS as clear source */
  initWDOG.perSel = wdogPeriod_4k;      /* Set the watchdog period to 4096 clock periods (ie ~4 seconds) */

  /* Initializing watchdog with chosen settings */
  WDOGn_Init(DEFAULT_WDOG, &initWDOG);

  /* set WDOG clear source to rising edge of PRS Source 0 */
  DEFAULT_WDOG->CFG |= WDOG_CFG_CLRSRC;

  printf("Starting WDOG PRS clear test\r\n");

  /* Start watchdog counter */
  WDOGn_Enable(DEFAULT_WDOG, true);

  /* Do something for a while and make sure that the watchdog does not time out */
  printf("\r\n\nFeeding watchdog timer...");
  for (uint32_t index = 0; index < 40; index++)
  {
    /* LETIMER0 interrupt is enabled to wakeup MCU and visualize feeding watchdog regularly */
    EMU_EnterEM2(false);

    /* Processing takes place here */
    GPIO_PinOutToggle(BSP_GPIO_LED1_PORT, BSP_GPIO_LED1_PIN);
    printf(".");
  }

  /* Stop feeding the watchdog, and it will trigger a reset */
  printf("\r\nStop feeding; disable LETIMER0 and wait for watchdog to trigger a reset (~4s)...   \r\n");

  /* Disable LETIMER0 */
  LETIMER_Enable(LETIMER0, false);

  /* Enter loop, and wait for wdog reset */
  while (1) ;
}

/******************************************************************************
 * @brief  Watchdog PRS event test.
 *
 * The test uses ULFRCO as watchdog clock source. It feeds watchdog every 100ms seconds
 * with usTimer driver. The LETIMER0 trigger event is monitored on PRS channel 0 and is
 * configured to trigger every 50ms, twice during a feed cycle. The WDOG interrupt will
 * trigger if the event does not happen within the feeding window. The onboard LED toggles
 * and a '.' is printed to the VCOM during feeding process.
 *****************************************************************************/
void testEvent(void)
{
  WDOG_Init_TypeDef initWDOG = WDOG_INIT_DEFAULT;
  LETIMER_Init_TypeDef LETIMERInit = LETIMER_INIT_DEFAULT;
  uint32_t clockCnt = 50; /* corresponds to 50ms LETIMER when using ULFRCO */

  /* Use ULFRCO for all low energy peripherals including LETIMER */
  CMU_ClockSelectSet(cmuClock_EM23GRPACLK, cmuSelect_ULFRCO);

  /* Enable LETIMER0 clock */
  CMU_ClockEnable(cmuClock_LETIMER0, true);

  /* Configure LETIMER for 50us period and pulse output on PRS */
  LETIMERInit.comp0Top = true;
  LETIMERInit.topValue = clockCnt;
  LETIMERInit.ufoa0 = letimerUFOAPulse;

  /* Time out value configuration */
  LETIMER_CompareSet(LETIMER0, 0, clockCnt);

  /* Initializing LETIMER with chosen settings */
  LETIMER_Init(LETIMER0, &LETIMERInit);

  /* usTimer driver initialization */
  USTIMER_Init();

  /* Enable PRS clock */
  CMU_ClockEnable(cmuClock_PRS, true);

  /* Configure LETIMER0 to create PRS interrupt signal and watchdog to consume as event monitor */
  PRS_ConnectSignal(0, prsTypeAsync, prsSignalLETIMER0_CH0);
  PRS_ConnectConsumer(0, prsTypeAsync, prsConsumerWDOG0_SRC0);

  CMU_ClockEnable(cmuClock_WDOG0, true);
  CMU_ClockSelectSet(cmuClock_WDOG0, cmuSelect_LFRCO);
  initWDOG.perSel = wdogPeriod_128k;     /* Set the watchdog period to 128K clock periods (ie ~4 second) */

  printf("Starting WDOG PRS event monitor test\r\n");

  /* Initializing watchdog with chosen settings */
  WDOGn_Init(DEFAULT_WDOG, &initWDOG);

  /* Enable watchdog warning interrupt */
  WDOGn_IntEnable(DEFAULT_WDOG, WDOG_IEN_PEM0);
  NVIC_EnableIRQ(WDOG0_IRQn);

  /* Do something for a while and make sure that the watchdog does not time out */
  printf("\r\n\nFeeding watchdog timer...");
  for (i = 0; i < 40; i++)
  {
      /* Processing takes place here
       *  LETIMER events should occur between 100ms WDOG feeds, preventing WDOG interrupt */
      USTIMER_Delay(100000);
      GPIO_PinOutToggle(BSP_GPIO_LED1_PORT, BSP_GPIO_LED1_PIN);
      printf(".");
      WDOGn_Feed(DEFAULT_WDOG);
  }

  /* Turn off LETIMER0 and feed watchdog; observe WDOG interrupt when PRS event is missing */
  printf("\r\nStopping LETIMER0; PRS event missing interrupts will occur with each WDOG feed");

  /* Disable LETIMER0 */
  LETIMER_Enable(LETIMER0, false);

  /* Do something for a while and make sure that the watchdog does not time out */
  printf("\r\n\nFeeding watchdog timer...");
  for (i = 0; i < 40; i++)
  {
      /* Processing takes place here
       *  LETIMER events should occur between 50us WDOG feeds, preventing WDOG interrupt */
      USTIMER_Delay(100000);
      GPIO_PinOutToggle(BSP_GPIO_LED1_PORT, BSP_GPIO_LED1_PIN);
      printf(".");
      WDOGn_Feed(DEFAULT_WDOG);
  }

  /* short delay to handle last interrupt */
  USTIMER_Delay(5000);

  /* Stop feeding the watchdog, and it will trigger a reset */
  printf("\r\nStop feeding; wait for watchdog to trigger a reset (~4s)...\r\n");

  /* Enter loop, and wait for wdog reset */
  while (1) ;
}

/******************************************************************************
 * @brief  Watchdog warning test.
 *
 * The test uses ULFRCO as watchdog clock source. It tests the watchdog warning feature.
 * Watchdog generates warning regularly (~1 seconds, 25% of watchdog interval). The watchdog
 * is fed when a warning is generated. The onboard LED toggles and a '.' is printed to
 * the VCOM during feeding process.
 *****************************************************************************/
void testWarning(void)
{
  WDOG_Init_TypeDef initWDOG = WDOG_INIT_DEFAULT;

  CMU_ClockEnable(cmuClock_WDOG0, true);
  CMU_ClockSelectSet(cmuClock_WDOG0, cmuSelect_ULFRCO);

  initWDOG.perSel = wdogPeriod_4k;      /* Set the watchdog period to 4096 clock periods (ie ~4 seconds) */
  initWDOG.warnSel = wdogWarnTime25pct; /* Set warning to 25 percent (ie ~1 seconds) */

  printf("Starting WDOG warning test\r\n");

  /* Initializing watchdog with choosen settings */
  WDOGn_Init(DEFAULT_WDOG, &initWDOG);

  /* Enable watchdog warning interrupt */
  WDOGn_IntEnable(DEFAULT_WDOG, WDOG_IEN_WARN);
  NVIC_EnableIRQ(WDOG0_IRQn);

  /* Do something for a while and make sure that the watchdog does not time out */
  printf("\r\n\nWait; 25%% warning interrupt feeds watchdog...");
  for (i = 0; i < 4; i++)
  {
    /* Warning interrupt would happen and wakeup CPU */
    /* Feeding was executed in the warning ISR */
    EMU_EnterEM1();
    printf(".");
  }

  /* Disable warning interrupt */
  WDOGn_IntDisable(DEFAULT_WDOG, WDOG_IEN_WARN);
  NVIC_DisableIRQ(WDOG0_IRQn);

  /* Stop feeding the watchdog, and it will trigger a reset */
  printf("\r\nStop feeding; disable warning interrupt and wait for watchdog to trigger a reset (~4s)...\r\n");

  /* Enter loop, and wait for wdog reset */
  while (1) ;
}

/******************************************************************************
 * @brief  Watchdog window test.
 *
 * The test uses ULFRCO as watchdog clock source. It tests the watchdog window feature.
 * Watchdog generates window interrupt when it is fed (1 second) before window period time
 * has elapsed (2 seconds, 50% of watchdog interval period). The onboard LED toggles and
 * a '.' is printed to the VCOM during feeding process.
 *****************************************************************************/
void testWindow(void)
{
  WDOG_Init_TypeDef initWDOG = WDOG_INIT_DEFAULT;

  CMU_ClockEnable(cmuClock_WDOG0, true);
  CMU_ClockSelectSet(cmuClock_WDOG0,cmuSelect_ULFRCO);

  initWDOG.perSel = wdogPeriod_4k;                  /* Set the watchdog period to 4096 clock periods (ie ~4 seconds) */
  initWDOG.winSel = wdogIllegalWindowTime50_0pct;   /* Set window to 50 percent (ie ~2 seconds) */

  printf("Starting WDOG window test\r\n");

  /* Initializing watchdog and uSecond timers */
  USTIMER_Init();
  WDOGn_Init(DEFAULT_WDOG, &initWDOG);

  /* Enable watchdog window interrupt */
  WDOGn_IntEnable(DEFAULT_WDOG, WDOG_IEN_WIN);
  NVIC_EnableIRQ(WDOG0_IRQn);

  /* Do something for a while and make sure that the watchdog does not time out */
  printf("\r\n\nFeeding watchdog timer after illegal window...");
  for (i = 0; i < 5; i++)
  {
    /* Feed watchdog every 3 second, after window setting 2 seconds */
    USTIMER_Delay(3000000);
    printf(".");
    WDOGn_Feed(DEFAULT_WDOG);
  }

  /* Do something for a while and make sure that the watchdog does not time out */
  printf("\r\n\nFeeding watchdog timer too soon from last feeding...");
  for (i = 0; i < 5; i++)
  {
    /* Feed watchdog every 1 second, that was lower than window setting 2 seconds */
    USTIMER_Delay(1000000);
    printf(".");
    WDOGn_Feed(DEFAULT_WDOG);
  }

  /* Disable window interrupt */
  WDOGn_IntDisable(DEFAULT_WDOG, WDOG_IEN_WIN);
  NVIC_DisableIRQ(WDOG0_IRQn);

  /* Stop feeding the watchdog, and it will trigger a reset */
  printf("\r\nStop feeding; wait for watchdog to trigger a reset (~4s)...\r\n");

  /* Enter loop, and wait for wdog reset */
  while (1) ;
}

/******************************************************************************
 * @brief  Run watchdog different mode test.
 *****************************************************************************/
void modeTest(unsigned int tMode)
{
  switch (tMode)
  {
    case T_DOG:
      testDog();
      break;
    case T_LCK:
      testLock();
      break;
    case T_INT:
      testInterrupt();
      break;
    case T_CLR:
      testClear();
      break;
    case T_EVT:
      testEvent();
      break;
    case T_WRN:
      testWarning();
      break;
    case T_WIN:
      testWindow();
      break;
  }
}
