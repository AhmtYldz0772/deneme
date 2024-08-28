
#include <stdio.h>
#include <stdlib.h>
#include "nrf_delay.h"
#include "flash.h"
/*---------------------------------------------FreeRTOS Includes-----------------------------------------*/
#include "FreeRTOS.h"
#include "SEGGER_RTT.h"
#include "fds.h"
#include "nrf_drv_gpiote.h"
#include "semphr.h"
#include "spi-board.h"
#include "task.h"
#include "timers.h"
/*-------------------------------------------------------------------------------------------------------*/

/*---------------------------------------------CMSIS Includes-----------------------------------------*/
#include "arm_math.h"
/*-------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------Initialize Includes-----------------------------------------*/
#include "Initialize.h"
/*-------------------------------------------------------------------------------------------------------*/

/*---------------------------------------------TWI Includes-----------------------------------------*/
#include "nrf_drv_twi.h"
/*-------------------------------------------------------------------------------------------------------*/

//raktas
#include "shmSignal.h"

extern uint32_t frequencyFFTSearch;


/**@brief Function for new treshold value set.
 *
 *@param[in] void
 */
void vInitTresholdValuesSet(void)
{

  uint32_t *flashAddress_32;  /* Data pointer */
  uint32_t initData = 0;

  flashAddress_32 = (uint32_t *)(XAXISDATAADDRESS); /* X axis flash address */
  memcpy(&X_axisTreshold, flashAddress_32, sizeof(float32_t)); /* Read treshold from flash */
  if(*flashAddress_32 == 0 || X_axisTreshold >  255.0 || *flashAddress_32 == 0XFFFFFFFF )
  {    
    X_axisTreshold = 0.5;
  }

  flashAddress_32 = (uint32_t *)(YAXISDATAADDRESS); /* Y axis flash address */
  memcpy(&Y_axisTreshold, flashAddress_32, sizeof(float32_t)); /* Read treshold from flash */
  if(*flashAddress_32 == 0 || Y_axisTreshold >  255.0 || *flashAddress_32 == 0XFFFFFFFF )
  {
    Y_axisTreshold = 0.5;
  }
  flashAddress_32 = (uint32_t *)(FREQUENCYADDRESS); /* frequency flash address */
  memcpy(&frequencyFFTSearch, flashAddress_32, sizeof(float32_t)); /* Read treshold from flash */
  if(frequencyFFTSearch == 0 || frequencyFFTSearch > 1024)
  {
    frequencyFFTSearch = 91;
  }
}


/**@brief Function for init values fot first time code running.
 *
 *@param[in] void
 */
void vInitFirstTimeInit (void)
{
  uint32_t flashValue_32; /* Flash Data */
  
  flashValue_32 = X_YAXISINITVALUES; /* X axis init value */
  vFlashWritetoFlash(flashValue_32, XAXISDATAADDRESS,XAXISDATAPAGE); /* Write x axis init value to x axis address */
  nrf_delay_ms(100);


  vFlashWritetoFlash(flashValue_32, YAXISDATAADDRESS,YAXISDATAPAGE); /* Write y axis init value to y axis address */
  nrf_delay_ms(100);

  flashValue_32 = FREQUENCYINITVALUE;
  vFlashWritetoFlash(flashValue_32, FREQUENCYADDRESS,FREQUENCYPAGE);  /* Write frequency init value to frequency address */
  nrf_delay_ms(100);

  flashValue_32 = INITVALUE;
  vFlashWritetoFlash(flashValue_32, INITADDRESS,INITPAGE);  /* Write init value to init address */
  nrf_delay_ms(100);
}



