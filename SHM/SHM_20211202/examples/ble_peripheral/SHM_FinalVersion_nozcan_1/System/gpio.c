/*!
 * \file      gpio.c
 *
 * \brief     GPIO driver implementation
 *
 * \remark: Relies on the specific board GPIO implementation as well as on
 *          IO expander driver implementation if one is available on the target
 *          board.
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
 */
#include "gpio-board.h"

void GpioInit( Gpio_t *obj, uint8_t pin, PinModes mode,  PinConfigs config, PinTypes type, uint32_t value )
{
    GpioMcuInit( obj, pin, mode, config, type, value );
}

void GpioSetInterrupt( Gpio_t *obj, IrqModes irqMode, IrqPriorities irqPriority, nrf_drv_gpiote_evt_handler_t irqHandler )
{
    GpioMcuSetInterrupt( obj, irqMode, irqPriority, irqHandler );
}

void GpioRemoveInterrupt( Gpio_t *obj )
{
    GpioMcuRemoveInterrupt( obj );
}

void GpioEnableInterrupt( Gpio_t *obj )
{
    GpioMcuEnableInterrupt( obj );
}

void GpioWrite( Gpio_t *obj, uint32_t value )
{
    GpioMcuWrite( obj, value );
}

void GpioToggle( Gpio_t *obj )
{
    GpioMcuToggle( obj );
}

uint32_t GpioRead( Gpio_t *obj )
{
    return GpioMcuRead( obj );
}
uint8_t CheckGpioInitialized(void)
{
    return IsGpioInitialized();
}