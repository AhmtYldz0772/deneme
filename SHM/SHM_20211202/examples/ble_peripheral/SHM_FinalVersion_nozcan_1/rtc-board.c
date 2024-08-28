/*!
 * \file      rtc-board.c
 *
 * \brief     Target board RTC timer and low power modes management
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
#include "rtc-board.h"
#include "System/delay.h"
#include "System/gpio.h"
#include "System/timer.h"
#include "nrf_drv_clock.h"
#include "nrf_drv_rtc.h"
#define COMPARE_COUNT_MAX_VALUE 0xFFFFFF
#define MS_TO_TICKS(n) ((n)*2.048)
#define TICKS_TO_MS(n) ((n) / 2.048)

/*!
 * \brief Indicates if the RTC is already Initialized or not
 */
static bool RtcInitialized = false;
static volatile bool RtcTimeoutPendingInterrupt = false;
static volatile bool RtcTimeoutPendingPolling = false;

// in ticks
static uint32_t TimeoutStart = 0;
// in ticks
static uint32_t TimeoutDuration = 0;

const nrf_drv_rtc_t rtc = NRF_DRV_RTC_INSTANCE(2); /**< Declaring an instance of nrf_drv_rtc for RTC2. */
/*!
 * \brief Callback for the hal_timer when timeout expired
 */
static void RtcTimerTaskCallback(nrf_drv_rtc_int_type_t int_type);
static bool RtcLoadAbsoluteTicks(uint32_t ticks);

static void lfclk_config(void)
{
    ret_code_t err_code = nrf_drv_clock_init();
    APP_ERROR_CHECK(err_code);

    nrf_drv_clock_lfclk_request(NULL);
}

void RtcInit(void)
{

    if (RtcInitialized == false)
    {
        lfclk_config();
        uint32_t err_code;
        //Initialize RTC instance
        nrf_drv_rtc_config_t config = NRF_DRV_RTC_DEFAULT_CONFIG;
        config.prescaler = RTC_FREQ_TO_PRESCALER(2048);
        config.reliable = false;
        err_code = nrf_drv_rtc_init(&rtc, &config, RtcTimerTaskCallback);
        APP_ERROR_CHECK(err_code);

        //Power on RTC instance
        nrf_drv_rtc_enable(&rtc);
        RtcInitialized = true;
    }
}

void RtcSetTimeout(uint32_t timeout)
{
    TimeoutStart = nrf_drv_rtc_counter_get(&rtc);
    TimeoutDuration = MS_TO_TICKS(timeout);

    RtcTimeoutPendingInterrupt = true;
    RtcTimeoutPendingPolling = false;

    if (RtcLoadAbsoluteTicks(TimeoutStart + TimeoutDuration) == false)
    {
        // If timer already passed
        if (RtcTimeoutPendingInterrupt == true)
        {
            // And interrupt not handled, mark as polling
            RtcTimeoutPendingPolling = true;
            RtcTimeoutPendingInterrupt = false;
        }
    }
}

bool RtcLoadAbsoluteTicks(uint32_t ticks)
{
    ret_code_t err_code;
    volatile uint32_t current = 0;
    err_code = nrf_drv_rtc_cc_set(&rtc, 0, ticks, true);
    APP_ERROR_CHECK(err_code);
    current = nrf_drv_rtc_counter_get(&rtc);

    if ((ticks - current - 1) >= (COMPARE_COUNT_MAX_VALUE >> 1))
    {
        // if difference is more than half of max assume timer has passed
        DelayMs(1);
        return false;
    }
    if ((ticks - current) < 10)
    {
        // if too close the matching interrupt does not trigger, so handle same as passed
        DelayMs(1);
        return false;
    }

    return true;
}

TimerTime_t RtcGetAdjustedTimeoutValue(uint32_t timeout)
{
    return timeout;
}

TimerTime_t RtcGetTimerValue(void)
{
    return TICKS_TO_MS(nrf_drv_rtc_counter_get(&rtc));
}

TimerTime_t RtcGetElapsedAlarmTime(void)
{
    return TICKS_TO_MS(nrf_drv_rtc_counter_get(&rtc) - TimeoutStart);
}

TimerTime_t RtcComputeElapsedTime(TimerTime_t eventInTime)
{
    // Calculate in ticks for correct 32bit wrapping.
    // Only works because ticks wrappes earlier than ms.
    return TICKS_TO_MS(nrf_drv_rtc_counter_get(&rtc) - MS_TO_TICKS(eventInTime));
}

void RtcEnterLowPowerStopMode(void)
{
}

void RtcProcess(void)
{
    if (RtcTimeoutPendingPolling == true)
    {
        if (RtcGetElapsedAlarmTime() >= TimeoutDuration)
        {
            // Because of one shot the task will be removed after the callback
            RtcTimeoutPendingPolling = false;
            // NOTE: The handler should take less then 1 ms otherwise the clock shifts
            TimerIrqHandler();
        }
    }
}

static void RtcTimerTaskCallback(nrf_drv_rtc_int_type_t type)
{
    // Because of one shot the task will be removed after the callback
    RtcTimeoutPendingInterrupt = false;
    // NOTE: The handler should take less then 1 ms otherwise the clock shifts
    TimerIrqHandler();
}