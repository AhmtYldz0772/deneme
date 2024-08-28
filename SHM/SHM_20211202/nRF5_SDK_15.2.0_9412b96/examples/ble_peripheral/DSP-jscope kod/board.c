/*!
 * \file      board.c
 *
 * \brief     Target board general functions implementation
 *
 * \copyright Revised BSD License, see section \ref LICENSE.
 *
 * \code
 *                ______                              _
 *               / _____)             _              | |
 *              ( (____  _____ ____ _| |_ _____  ____| |__
 *               \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 *               _____) ) ____| | | || |_| ____( (___| | | |
 *              (______/|_____)_|_|_| \__)_____)\____)_| |_|
 *              (C)2013-2017 Semtech
 *
 * \endcode
 *
 * \author    Miguel Luis ( Semtech )
 *
 * \author    Gregory Cristian ( Semtech )
 *
 * \author    Marten Lootsma(TWTG) on behalf of Microchip/Atmel (c)2017
 */

#include "board.h"
#include "EnergyMetering/Metrology.h"
#include "EnergyMetering/stpm32.h"
#include "System/delay.h"
#include "System/gpio.h"
#include "System/spi.h"
#include "System/timer.h"
#include "board-config.h"
#include "nrf_drv_spi.h"
#include "rtc-board.h"
#include "sx1272-board.h"
#include "utilities.h"
/*
 * MCU objects
 */
Gpio_t Led1;
Gpio_t Led2;

/*!
 * Flag to indicate if the MCU is Initialized
 */

/*!
 * Nested interrupt counter.
 *
 * \remark Interrupt should only be fully disabled once the value is 0
 */
static uint8_t IrqNestLevel = 0;

void BoardDisableIrq(void)
{
    __disable_irq();
    IrqNestLevel++;
}

void BoardEnableIrq(void)
{
    IrqNestLevel--;
    if (IrqNestLevel == 0)
    {
        __enable_irq();
    }
}

void BoardInitPeriph(void)
{
    GpioInit(&Led1, LED_1, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0);
    //GpioInit(&Led2, LED_4, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1);
}

void BoardInitMcu(void)
{
    ret_code_t err_code;
    err_code = nrf_drv_gpiote_init();
    APP_ERROR_CHECK(err_code);  
    RtcInit();
   // SpiInit(&SX1272.Spi, SPI_1, RADIO_MOSI, RADIO_MISO, RADIO_SCLK, NRF_DRV_SPI_PIN_NOT_USED, NRF_DRV_SPI_MODE_0);
   // SX1272IoInit();
}

void BoardDeInitMcu(void)
{
    SpiDeInit(&SX1272.Spi);
}

uint32_t BoardGetRandomSeed(void)
{
    //return 0;
    return ((*(uint32_t *)NRF_FICR->DEVICEID[0]) ^ (*(uint32_t *)NRF_FICR->DEVICEID[0]));
}

void BoardGetUniqueId(uint8_t *id)
{
    // We don't have an ID, so use the one from Commissioning.h
    //TODO: burasi deveui'yi belirlemekte kullaniliyor bu yüzden doldurmuyoruz çünkü dev eui'yi biz kendimiz commisionning.h dosyasi içerisinde tanimlayaca
}

uint8_t BoardGetBatteryLevel(void)
{
    return 0; //  Battery level [0: node is connected to an external power source ...
}

uint8_t GetBoardPowerSource(void)
{
    return USB_POWER;
}

#ifdef USE_FULL_ASSERT
/*
 * Function Name  : assert_failed
 * Description    : Reports the name of the source file and the source line number
 *                  where the assert_param error has occurred.
 * Input          : - file: pointer to the source file name
 *                  - line: assert_param error line source number
 * Output         : None
 * Return         : None
 */
void assert_failed(uint8_t *file, uint32_t line)
{
    /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %u\r\n", file, line) */

    printf("Wrong parameters value: file %s on line %u\r\n", (const char *)file, line);
    /* Infinite loop */
    while (1)
    {
    }
}
#endif