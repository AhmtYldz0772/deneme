/*!
 * \file      gpio-board.c
 *
 * \brief     Target board GPIO driver implementation
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
#include "gpio-board.h"
#include "nrf_drv_gpiote.h"
#include "nrf_gpio.h"

void GpioMcuInit(Gpio_t *obj, uint8_t pin, PinModes mode, PinConfigs config, PinTypes type, uint32_t value)
{
    obj->pin = pin;
    obj->pull = type;
    ret_code_t err_code;

    if (mode == PIN_INPUT)
    {
        nrf_drv_gpiote_in_config_t inputPinConfig;

        if (type == PIN_NO_PULL)
        {
            inputPinConfig.pull = NRF_GPIO_PIN_NOPULL;
        }
        else if (type == PIN_PULL_UP)
        {
            inputPinConfig.pull = NRF_GPIO_PIN_PULLUP;
        }
        else if (type == PIN_PULL_DOWN)
        {
            inputPinConfig.pull = NRF_GPIO_PIN_PULLDOWN;
        }
        inputPinConfig.hi_accuracy = false;
        inputPinConfig.is_watcher = false;
        inputPinConfig.sense = NRF_GPIOTE_POLARITY_TOGGLE;
        err_code = nrf_drv_gpiote_in_init(obj->pin, &inputPinConfig, NULL);
        APP_ERROR_CHECK(err_code);
    }
    else // mode output
    {
        nrf_drv_gpiote_out_config_t outputPinConfig;
        outputPinConfig.init_state = (value >= 1) ? NRF_GPIOTE_INITIAL_VALUE_HIGH : NRF_GPIOTE_INITIAL_VALUE_LOW;
        outputPinConfig.task_pin = false;
        err_code = nrf_drv_gpiote_out_init(obj->pin, &outputPinConfig);
        APP_ERROR_CHECK(err_code);
    }
}

void GpioMcuSetInterrupt(Gpio_t *obj, IrqModes irqMode, IrqPriorities irqPriority, GpioIrqHandler *irqHandler)
{
    ret_code_t err_code;
    nrf_drv_gpiote_in_config_t inputPinConfig;

    if (obj->pull == PIN_NO_PULL)
    {
        inputPinConfig.pull = NRF_GPIO_PIN_NOPULL;
    }
    else if (obj->pull == PIN_PULL_UP)
    {
        inputPinConfig.pull = NRF_GPIO_PIN_PULLUP;
    }
    else if (obj->pull == PIN_PULL_DOWN)
    {
        inputPinConfig.pull = NRF_GPIO_PIN_PULLDOWN;
    }
    if (irqMode == IRQ_FALLING_EDGE)
    {
        inputPinConfig.sense = NRF_GPIOTE_POLARITY_HITOLO;
    }
    else if (irqMode == IRQ_RISING_EDGE)
    {
        inputPinConfig.sense = NRF_GPIOTE_POLARITY_LOTOHI;
    }
    else if (irqMode == IRQ_RISING_FALLING_EDGE)
    {
        inputPinConfig.sense = NRF_GPIOTE_POLARITY_TOGGLE;
    }

    inputPinConfig.hi_accuracy = true;
    inputPinConfig.is_watcher = false;

    nrf_drv_gpiote_in_uninit(obj->pin);

    err_code = nrf_drv_gpiote_in_init(obj->pin, &inputPinConfig, irqHandler);
    APP_ERROR_CHECK(err_code);
    nrf_drv_gpiote_in_event_enable(obj->pin, true);
}

void GpioMcuRemoveInterrupt(Gpio_t *obj)
{
    nrf_drv_gpiote_in_event_disable(obj->pin);
}

void GpioMcuWrite(Gpio_t *obj, uint32_t value)
{

    if (obj == NULL)
    {
        while (1)
            ;
    }

    if (value == 0)
    {
        nrf_drv_gpiote_out_clear(obj->pin);
    }
    else
    {
        nrf_drv_gpiote_out_set(obj->pin);
    }
}

void GpioMcuToggle(Gpio_t *obj)
{
    if (obj == NULL)
    {
        //assert_param( FAIL );
        while (1)
            ;
    }

    nrf_drv_gpiote_out_toggle(obj->pin);
}

uint32_t GpioMcuRead(Gpio_t *obj)
{
    if (obj == NULL)
    {
        //assert_param( FAIL );
        while (1)
            ;
    }
    // Check if pin is not connected

    return (nrf_drv_gpiote_in_is_set(obj->pin) == true) ? 1 : 0;
}

uint8_t IsGpioInitialized(void)
{
    ret_code_t errorCode = NRF_SUCCESS;
    uint8_t returnValue=0;
    if (nrfx_gpiote_is_init() == false) /* Check whether GPIOTE is initalized. */
    {
        errorCode = nrfx_gpiote_init();
        APP_ERROR_CHECK(errorCode);
        if (errorCode != NRFX_SUCCESS)
        {
            returnValue = 0;
            return returnValue;
        }
    }
    else
    {
       returnValue = 1;
       return returnValue;
    }
}