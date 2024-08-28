/**
 * Copyright (c) 2014 - 2018, Nordic Semiconductor ASA
 * 
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 * 
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 * 
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 * 
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 * 
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */
/** @file
 *
 * @defgroup ble_sdk_app_template_main main.c
 * @{
 * @ingroup ble_sdk_app_template
 * @brief Template project main file.
 *
 * This file contains a template for creating a new application. It has the code necessary to wakeup
 * from button, advertise, get a connection restart advertising on disconnect and if no new
 * connection created go back to system-off mode.
 * It can easily be used as a starting point for creating a new application, the comments identified
 * with 'YOUR_JOB' indicates where and how you can customize.
 */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "app_error.h"
#include "app_timer.h"
#include "ble.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_conn_params.h"
#include "ble_conn_state.h"
#include "ble_hci.h"
#include "ble_srv_common.h"
#include "bsp_btn_ble.h"
#include "fds.h"
#include "nordic_common.h"
#include "nrf.h"
#include "nrf_ble_gatt.h"
#include "nrf_ble_qwr.h"
#include "nrf_pwr_mgmt.h"
#include "nrf_sdh.h"
#include "nrf_sdh_ble.h"
#include "nrf_sdh_freertos.h"
#include "nrf_sdh_soc.h"
#include "peer_manager.h"
#include "sensorsim.h"

#include "nrf52.h"
#include "nrf_drv_clock.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
/* ------------------------------------------- Status Module Includes --------------------------------------*/
#include "status.h"
/* ---------------------------------------------------------------------------------------------------------*/

//-------------------------------------------------------------------------------------------------------//

/*---------------------------------------------FreeRTOS Includes-----------------------------------------*/
#include "FreeRTOS.h"
#include "SEGGER_RTT.h"
#include "arm_math.h"
#include "fds.h"
#include "semphr.h"
#include "task.h"
#include "timers.h"
/*-------------------------------------------------------------------------------------------------------*/

//-------------------------------------------- LoRaWAN Includes ------------------------------------------//
#include "Commissioning.h"
#include "Mac/LoRaMac.h"
#include "System/gpio.h"
#include "board.h"
#include "utilities.h"

#include "System/spi.h"
#include "board-config.h"
//-------------------------------------------------------------------------------------------------------//

//-------------------------------------------- ADXL355 Includes ------------------------------------------//
#include "ADXL355.h"
#include "Communication.h"

//----------------------------------------------------------------------------------------------------------

//-------------------------------------------- BLE Service Includes -------------------------------------//

#include "BLE_Services/atCommandService.h"
#include "BLE_Services/ledService.h"

//-------------------------------------------------------------------------------------------------------//

#define DEVICE_NAME "NETAS_Photon" /**< Name of device. Will be included in the advertising data. */
#define MANUFACTURER_NAME "NETAS"  /**< Manufacturer. Will be passed to Device Information Service. */
#define APP_ADV_INTERVAL 300       /**< The advertising interval (in units of 0.625 ms. This value corresponds to 187.5 ms). */

#define APP_ADV_DURATION 18000  /**< The advertising duration (180 seconds) in units of 10 milliseconds. */
#define APP_BLE_OBSERVER_PRIO 3 /**< Application's BLE observer priority. You shouldn't need to modify this value. */
#define APP_BLE_CONN_CFG_TAG 1  /**< A tag identifying the SoftDevice BLE configuration. */

#define MIN_CONN_INTERVAL MSEC_TO_UNITS(100, UNIT_1_25_MS) /**< Minimum acceptable connection interval (0.1 seconds). */
#define MAX_CONN_INTERVAL MSEC_TO_UNITS(200, UNIT_1_25_MS) /**< Maximum acceptable connection interval (0.2 second). */
#define SLAVE_LATENCY 0                                    /**< Slave latency. */
#define CONN_SUP_TIMEOUT MSEC_TO_UNITS(4000, UNIT_10_MS)   /**< Connection supervisory timeout (4 seconds). */

#define FIRST_CONN_PARAMS_UPDATE_DELAY APP_TIMER_TICKS(5000) /**< Time from initiating event (connect or start of notification) to first time sd_ble_gap_conn_param_update is called (5 seconds). */
#define NEXT_CONN_PARAMS_UPDATE_DELAY APP_TIMER_TICKS(30000) /**< Time between each call to sd_ble_gap_conn_param_update after the first call (30 seconds). */
#define MAX_CONN_PARAMS_UPDATE_COUNT 3                       /**< Number of attempts before giving up the connection parameter negotiation. */

#define SEC_PARAM_BOND 1                               /**< Perform bonding. */
#define SEC_PARAM_MITM 0                               /**< Man In The Middle protection not required. */
#define SEC_PARAM_LESC 0                               /**< LE Secure Connections not enabled. */
#define SEC_PARAM_KEYPRESS 0                           /**< Keypress notifications not enabled. */
#define SEC_PARAM_IO_CAPABILITIES BLE_GAP_IO_CAPS_NONE /**< No I/O capabilities. */
#define SEC_PARAM_OOB 0                                /**< Out Of Band data not available. */
#define SEC_PARAM_MIN_KEY_SIZE 7                       /**< Minimum encryption key size. */
#define SEC_PARAM_MAX_KEY_SIZE 16                      /**< Maximum encryption key size. */

#define DEAD_BEEF 0xDEADBEEF /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */

NRF_BLE_GATT_DEF(m_gatt);           /**< GATT module instance. */
NRF_BLE_QWR_DEF(m_qwr);             /**< Context for the Queued Write module.*/
BLE_ADVERTISING_DEF(m_advertising); /**< Advertising module instance. */

float voltageValue = 0.0;

/*---------------------------------------------FreeRTOS Definitions--------------------------------------*/
#if NRF_LOG_ENABLED
static TaskHandle_t m_logger_thread; /**< Definition of Logger thread. */
#endif
static TaskHandle_t m_lorawan_state_machine_thread; /* Deifinition of LoRaWAN State Machine thread. */
static TaskHandle_t m_spi_test_thread;
static TaskHandle_t m_task_1; /**< Definition of Logger thread. */
static TaskHandle_t m_task_2; /**< Definition of Logger thread. */
TimerHandle_t m_timer_handle;
extern Gpio_t CSPin;

char JS_RTT_UpBuffer[4096]; // J-Scope RTT Buffer
int JS_RTT_Channel = 1;     // J-Scope RTT Channel
uint32_t counter;
float32_t inputSample, outputSample;

#define SIG_LENGTH 320
#define IMP_RSP_LENGTH 29
#define NUM_TAPS 29
#define BLOCK_SIZE 32
float32_t outputSignal_f32[SIG_LENGTH];
uint32_t numBlocks = SIG_LENGTH / BLOCK_SIZE;
static float32_t firStateF32[BLOCK_SIZE + NUM_TAPS - 1];

  #pragma pack(push, 1)
  struct {
     int sampleVal;
     int outputSampleVal;
     int impulseSample;
  } acValBuffer;
  #pragma pack(pop)

float32_t inputSignal_f32_1kHz_15kHz[SIG_LENGTH] =
    {
        +0.0000000000f,
        +0.5924659585f,
        -0.0947343455f,
        +0.1913417162f,
        +1.0000000000f,
        +0.4174197128f,
        +0.3535533906f,
        +1.2552931065f,
        +0.8660254038f,
        +0.4619397663f,
        +1.3194792169f,
        +1.1827865776f,
        +0.5000000000f,
        +1.1827865776f,
        +1.3194792169f,
        +0.4619397663f,
        +0.8660254038f,
        +1.2552931065f,
        +0.3535533906f,
        +0.4174197128f,
        +1.0000000000f,
        +0.1913417162f,
        -0.0947343455f,
        +0.5924659585f,
        -0.0000000000f,
        -0.5924659585f,
        +0.0947343455f,
        -0.1913417162f,
        -1.0000000000f,
        -0.4174197128f,
        -0.3535533906f,
        -1.2552931065f,
        -0.8660254038f,
        -0.4619397663f,
        -1.3194792169f,
        -1.1827865776f,
        -0.5000000000f,
        -1.1827865776f,
        -1.3194792169f,
        -0.4619397663f,
        -0.8660254038f,
        -1.2552931065f,
        -0.3535533906f,
        -0.4174197128f,
        -1.0000000000f,
        -0.1913417162f,
        +0.0947343455f,
        -0.5924659585f,
        +0.0000000000f,
        +0.5924659585f,
        -0.0947343455f,
        +0.1913417162f,
        +1.0000000000f,
        +0.4174197128f,
        +0.3535533906f,
        +1.2552931065f,
        +0.8660254038f,
        +0.4619397663f,
        +1.3194792169f,
        +1.1827865776f,
        +0.5000000000f,
        +1.1827865776f,
        +1.3194792169f,
        +0.4619397663f,
        +0.8660254038f,
        +1.2552931065f,
        +0.3535533906f,
        +0.4174197128f,
        +1.0000000000f,
        +0.1913417162f,
        -0.0947343455f,
        +0.5924659585f,
        +0.0000000000f,
        -0.5924659585f,
        +0.0947343455f,
        -0.1913417162f,
        -1.0000000000f,
        -0.4174197128f,
        -0.3535533906f,
        -1.2552931065f,
        -0.8660254038f,
        -0.4619397663f,
        -1.3194792169f,
        -1.1827865776f,
        -0.5000000000f,
        -1.1827865776f,
        -1.3194792169f,
        -0.4619397663f,
        -0.8660254038f,
        -1.2552931065f,
        -0.3535533906f,
        -0.4174197128f,
        -1.0000000000f,
        -0.1913417162f,
        +0.0947343455f,
        -0.5924659585f,
        +0.0000000000f,
        +0.5924659585f,
        -0.0947343455f,
        +0.1913417162f,
        +1.0000000000f,
        +0.4174197128f,
        +0.3535533906f,
        +1.2552931065f,
        +0.8660254038f,
        +0.4619397663f,
        +1.3194792169f,
        +1.1827865776f,
        +0.5000000000f,
        +1.1827865776f,
        +1.3194792169f,
        +0.4619397663f,
        +0.8660254038f,
        +1.2552931065f,
        +0.3535533906f,
        +0.4174197128f,
        +1.0000000000f,
        +0.1913417162f,
        -0.0947343455f,
        +0.5924659585f,
        +0.0000000000f,
        -0.5924659585f,
        +0.0947343455f,
        -0.1913417162f,
        -1.0000000000f,
        -0.4174197128f,
        -0.3535533906f,
        -1.2552931065f,
        -0.8660254038f,
        -0.4619397663f,
        -1.3194792169f,
        -1.1827865776f,
        -0.5000000000f,
        -1.1827865776f,
        -1.3194792169f,
        -0.4619397663f,
        -0.8660254038f,
        -1.2552931065f,
        -0.3535533906f,
        -0.4174197128f,
        -1.0000000000f,
        -0.1913417162f,
        +0.0947343455f,
        -0.5924659585f,
        -0.0000000000f,
        +0.5924659585f,
        -0.0947343455f,
        +0.1913417162f,
        +1.0000000000f,
        +0.4174197128f,
        +0.3535533906f,
        +1.2552931065f,
        +0.8660254038f,
        +0.4619397663f,
        +1.3194792169f,
        +1.1827865776f,
        +0.5000000000f,
        +1.1827865776f,
        +1.3194792169f,
        +0.4619397663f,
        +0.8660254038f,
        +1.2552931065f,
        +0.3535533906f,
        +0.4174197128f,
        +1.0000000000f,
        +0.1913417162f,
        -0.0947343455f,
        +0.5924659585f,
        -0.0000000000f,
        -0.5924659585f,
        +0.0947343455f,
        -0.1913417162f,
        -1.0000000000f,
        -0.4174197128f,
        -0.3535533906f,
        -1.2552931065f,
        -0.8660254038f,
        -0.4619397663f,
        -1.3194792169f,
        -1.1827865776f,
        -0.5000000000f,
        -1.1827865776f,
        -1.3194792169f,
        -0.4619397663f,
        -0.8660254038f,
        -1.2552931065f,
        -0.3535533906f,
        -0.4174197128f,
        -1.0000000000f,
        -0.1913417162f,
        +0.0947343455f,
        -0.5924659585f,
        +0.0000000000f,
        +0.5924659585f,
        -0.0947343455f,
        +0.1913417162f,
        +1.0000000000f,
        +0.4174197128f,
        +0.3535533906f,
        +1.2552931065f,
        +0.8660254038f,
        +0.4619397663f,
        +1.3194792169f,
        +1.1827865776f,
        +0.5000000000f,
        +1.1827865776f,
        +1.3194792169f,
        +0.4619397663f,
        +0.8660254038f,
        +1.2552931065f,
        +0.3535533906f,
        +0.4174197128f,
        +1.0000000000f,
        +0.1913417162f,
        -0.0947343455f,
        +0.5924659585f,
        +0.0000000000f,
        -0.5924659585f,
        +0.0947343455f,
        -0.1913417162f,
        -1.0000000000f,
        -0.4174197128f,
        -0.3535533906f,
        -1.2552931065f,
        -0.8660254038f,
        -0.4619397663f,
        -1.3194792169f,
        -1.1827865776f,
        -0.5000000000f,
        -1.1827865776f,
        -1.3194792169f,
        -0.4619397663f,
        -0.8660254038f,
        -1.2552931065f,
        -0.3535533906f,
        -0.4174197128f,
        -1.0000000000f,
        -0.1913417162f,
        +0.0947343455f,
        -0.5924659585f,
        -0.0000000000f,
        +0.5924659585f,
        -0.0947343455f,
        +0.1913417162f,
        +1.0000000000f,
        +0.4174197128f,
        +0.3535533906f,
        +1.2552931065f,
        +0.8660254038f,
        +0.4619397663f,
        +1.3194792169f,
        +1.1827865776f,
        +0.5000000000f,
        +1.1827865776f,
        +1.3194792169f,
        +0.4619397663f,
        +0.8660254038f,
        +1.2552931065f,
        +0.3535533906f,
        +0.4174197128f,
        +1.0000000000f,
        +0.1913417162f,
        -0.0947343455f,
        +0.5924659585f,
        +0.0000000000f,
        -0.5924659585f,
        +0.0947343455f,
        -0.1913417162f,
        -1.0000000000f,
        -0.4174197128f,
        -0.3535533906f,
        -1.2552931065f,
        -0.8660254038f,
        -0.4619397663f,
        -1.3194792169f,
        -1.1827865776f,
        -0.5000000000f,
        -1.1827865776f,
        -1.3194792169f,
        -0.4619397663f,
        -0.8660254038f,
        -1.2552931065f,
        -0.3535533906f,
        -0.4174197128f,
        -1.0000000000f,
        -0.1913417162f,
        +0.0947343455f,
        -0.5924659585f,
        -0.0000000000f,
        +0.5924659585f,
        -0.0947343455f,
        +0.1913417162f,
        +1.0000000000f,
        +0.4174197128f,
        +0.3535533906f,
        +1.2552931065f,
        +0.8660254038f,
        +0.4619397663f,
        +1.3194792169f,
        +1.1827865776f,
        +0.5000000000f,
        +1.1827865776f,
        +1.3194792169f,
        +0.4619397663f,
        +0.8660254038f,
        +1.2552931065f,
        +0.3535533906f,
        +0.4174197128f,
        +1.0000000000f,
        +0.1913417162f,
        -0.0947343455f,
        +0.5924659585f,
        +0.0000000000f,
        -0.5924659585f,
        +0.0947343455f,
        -0.1913417162f,
        -1.0000000000f,
        -0.4174197128f,
        -0.3535533906f,
        -1.2552931065f,
};



const float32_t impulse_response[IMP_RSP_LENGTH] = {
    -0.0018225230f, -0.0015879294f, +0.0000000000f, +0.0036977508f, +0.0080754303f, +0.0085302217f, -0.0000000000f, -0.0173976984f,
    -0.0341458607f, -0.0333591565f, +0.0000000000f, +0.0676308395f, +0.1522061835f, +0.2229246956f, +0.2504960933f, +0.2229246956f,
    +0.1522061835f, +0.0676308395f, +0.0000000000f, -0.0333591565f, -0.0341458607f, -0.0173976984f, -0.0000000000f, +0.0085302217f,
    +0.0080754303f, +0.0036977508f, +0.0000000000f, -0.0015879294f, -0.0018225230f};

void plot_output_signal(void);
void plot_impulse_signal(void);
/*-------------------------------------------------------------------------------------------------------*/

//---------------------------------------------- LoRaWAN Definitions ------------------------------------//

#define ACTIVE_REGION LORAMAC_REGION_EU868

#ifndef ACTIVE_REGION

#warning "No active region defined, LORAMAC_REGION_EU868 will be used as default."

#define ACTIVE_REGION LORAMAC_REGION_EU868

#endif

/*!
 * Indicate if the TTN network parameters have to be used.
 */
#define USE_TTN_NETWORK 0

/*!
 * Defines the application data transmission duty cycle. 5s, value in [ms].
 */
#define APP_TX_DUTYCYCLE 5000

/*!
 * Defines a random delay for application data transmission duty cycle. 1s,
 * value in [ms].
 */
#define APP_TX_DUTYCYCLE_RND 1000

/*!
 * Default datarate
 */
#define LORAWAN_DEFAULT_DATARATE DR_5

/*!
 * LoRaWAN confirmed messages
 */
#define LORAWAN_CONFIRMED_MSG_ON false

/*!
 * LoRaWAN Adaptive Data Rate
 *
 * \remark Please note that when ADR is enabled the end-device should be static
 */
#define LORAWAN_ADR_ON 1

#if defined(REGION_EU868)

#include "Mac/LoRaMacTest.h"

/*!
 * LoRaWAN ETSI duty cycle control enable/disable
 *
 * \remark Please note that ETSI mandates duty cycled transmissions. Use only for test purposes
 */
#define LORAWAN_DUTYCYCLE_ON false

#endif

//-------------------------------------------------------------------------------------------------------//

//---------------------------------------------- LoRaWAN Variables --------------------------------------//

/*!
 * LoRaWAN application port
 */
#define LORAWAN_APP_PORT 2

volatile bool isBLEConnected = false;
static uint8_t DevEui[] = LORAWAN_DEVICE_EUI;
static uint8_t AppEui[] = LORAWAN_APPLICATION_EUI;
static uint8_t AppKey[] = LORAWAN_APPLICATION_KEY;

#if (OVER_THE_AIR_ACTIVATION == 0)

static uint8_t NwkSKey[] = LORAWAN_NWKSKEY;
static uint8_t AppSKey[] = LORAWAN_APPSKEY;

/*!
 * Device address
 */
static uint32_t DevAddr = LORAWAN_DEVICE_ADDRESS;

#endif

/*!
 * Application port
 */
static uint8_t AppPort = LORAWAN_APP_PORT;

/*!
 * User application data size
 */
static uint8_t AppDataSize = 1;
static uint8_t AppDataSizeBackup = 1;
/*!
 * User application data buffer size
 */
#define LORAWAN_APP_DATA_MAX_SIZE 242

/*!
 * User application data
 */
static uint8_t AppData[LORAWAN_APP_DATA_MAX_SIZE];

/*!
 * Indicates if the node is sending confirmed or unconfirmed messages
 */
static uint8_t IsTxConfirmed = LORAWAN_CONFIRMED_MSG_ON;

/*!
 * Defines the application data transmission duty cycle
 */
static uint32_t TxDutyCycleTime;

/*!
 * Timer to handle the application data transmission duty cycle
 */
static TimerEvent_t TxNextPacketTimer;

/*!
 * Specifies the state of the application LED
 */
static bool AppLedStateOn = false;

/*!
 * Timer to handle the state of LED1
 */
static TimerEvent_t Led1Timer;

/*!
 * Timer to handle the state of LED2
 */
static TimerEvent_t Led2Timer;

/*!
 * Indicates if a new packet can be sent
 */
static bool NextTx = true;

static bool isConfirmed = false;

/*!
 * Device states
 */
static enum eDeviceState {
    DEVICE_STATE_INIT,
    DEVICE_STATE_JOIN,
    DEVICE_STATE_SEND,
    DEVICE_STATE_CYCLE,
    DEVICE_STATE_SLEEP
} DeviceState;

/*!
 * LoRaWAN compliance tests support data
 */
struct ComplianceTest_s
{
    bool Running;
    uint8_t State;
    bool IsTxConfirmed;
    uint8_t AppPort;
    uint8_t AppDataSize;
    uint8_t *AppDataBuffer;
    uint16_t DownLinkCounter;
    bool LinkCheck;
    uint8_t DemodMargin;
    uint8_t NbGateways;
} ComplianceTest;

/*!
 * LED GPIO pins objects
 */
extern Gpio_t Led1;
extern Gpio_t Led2;

//-------------------------------------------------------------------------------------------------------//

//---------------------------------------------- LoRaWAN Functions --------------------------------------//

/*!
 * \brief	 Prepares the payload of the frame
 */
static void PrepareTxFrame(uint8_t port)
{
    switch (port)
    {

    case 0:
    {
        AppDataSize = 0;
        break;
    }

    case 2:
    {
        AppDataSize = 1;
        AppData[0] = AppLedStateOn;
    }
    break;
    case 224:
        if (ComplianceTest.LinkCheck == true)
        {
            ComplianceTest.LinkCheck = false;
            AppDataSize = 3;
            AppData[0] = 5;
            AppData[1] = ComplianceTest.DemodMargin;
            AppData[2] = ComplianceTest.NbGateways;
            ComplianceTest.State = 1;
        }
        else
        {
            switch (ComplianceTest.State)
            {
            case 4:
                ComplianceTest.State = 1;
                break;
            case 1:
                AppDataSize = 2;
                AppData[0] = ComplianceTest.DownLinkCounter >> 8;
                AppData[1] = ComplianceTest.DownLinkCounter;
                break;
            }
        }
        break;
    default:
        break;
    }
}

/*!
 * \brief	 Prepares the payload of the frame
 *
 * \retval	[0: frame could be send, 1: error]
 */
static bool SendFrame(void)
{
    McpsReq_t mcpsReq;
    LoRaMacTxInfo_t txInfo;

    if (LoRaMacQueryTxPossible(AppDataSize, &txInfo) != LORAMAC_STATUS_OK)
    {
        // Send empty frame in order to flush MAC commands
        mcpsReq.Type = MCPS_UNCONFIRMED;
        mcpsReq.Req.Unconfirmed.fBuffer = NULL;
        mcpsReq.Req.Unconfirmed.fBufferSize = 0;
        mcpsReq.Req.Unconfirmed.Datarate = LORAWAN_DEFAULT_DATARATE;
    }
    else
    {
        if (IsTxConfirmed == false)
        {
            mcpsReq.Type = MCPS_UNCONFIRMED;
            mcpsReq.Req.Unconfirmed.fPort = AppPort;
            mcpsReq.Req.Unconfirmed.fBuffer = AppData;
            mcpsReq.Req.Unconfirmed.fBufferSize = AppDataSize;
            mcpsReq.Req.Unconfirmed.Datarate = LORAWAN_DEFAULT_DATARATE;
        }
        else
        {
            mcpsReq.Type = MCPS_CONFIRMED;
            mcpsReq.Req.Confirmed.fPort = AppPort;
            mcpsReq.Req.Confirmed.fBuffer = AppData;
            mcpsReq.Req.Confirmed.fBufferSize = AppDataSize;
            mcpsReq.Req.Confirmed.NbTrials = 8;
            mcpsReq.Req.Confirmed.Datarate = LORAWAN_DEFAULT_DATARATE;
        }
    }

    if (LoRaMacMcpsRequest(&mcpsReq) == LORAMAC_STATUS_OK)
    {
        return false;
    }
    return true;
}

/*!
 * \brief Function executed on TxNextPacket Timeout event
 */
static void OnTxNextPacketTimerEvent(void)
{
    MibRequestConfirm_t mibReq;
    LoRaMacStatus_t status;

    TimerStop(&TxNextPacketTimer);

    mibReq.Type = MIB_NETWORK_JOINED;
    status = LoRaMacMibGetRequestConfirm(&mibReq);

    if (status == LORAMAC_STATUS_OK)
    {
        if (mibReq.Param.IsNetworkJoined == true)
        {
            DeviceState = DEVICE_STATE_SEND;
            NextTx = true;
        }
        else
        {
            // Network not joined yet. Try to join again
            MlmeReq_t mlmeReq;
            mlmeReq.Type = MLME_JOIN;
            mlmeReq.Req.Join.DevEui = DevEui;
            mlmeReq.Req.Join.AppEui = AppEui;
            mlmeReq.Req.Join.AppKey = AppKey;
            mlmeReq.Req.Join.Datarate = LORAWAN_DEFAULT_DATARATE;

            if (LoRaMacMlmeRequest(&mlmeReq) == LORAMAC_STATUS_OK)
            {
                DeviceState = DEVICE_STATE_SLEEP;
            }
            else
            {
                DeviceState = DEVICE_STATE_CYCLE;
            }
        }
    }
}

/*!
 * \brief Function executed on Led 1 Timeout event
 */
static void OnLed1TimerEvent(void)
{
    TimerStop(&Led1Timer);
    // Switch LED 1 OFF
    GpioWrite(&Led1, 1);
}

/*!
 * \brief Function executed on Led 2 Timeout event
 */
static void OnLed2TimerEvent(void)
{
    TimerStop(&Led2Timer);
    // Switch LED 1 OFF
    GpioWrite(&Led1, 1);
}

/*!
 * \brief	 MCPS-Confirm event function
 *
 * \param	 [IN] mcpsConfirm - Pointer to the confirm structure,
 *							 containing confirm attributes.
 */
static void McpsConfirm(McpsConfirm_t *mcpsConfirm)
{
    if (mcpsConfirm->Status == LORAMAC_EVENT_INFO_STATUS_OK)
    {
        switch (mcpsConfirm->McpsRequest)
        {
        case MCPS_UNCONFIRMED:
        {

            // Check Datarate
            // Check TxPower
            break;
        }
        case MCPS_CONFIRMED:
        {
            // Check Datarate
            // Check TxPower
            // Check AckReceived
            // Check NbTrials
            break;
        }
        case MCPS_PROPRIETARY:
        {
            break;
        }
        default:
            break;
        }

        // Switch LED 1 ON
        GpioWrite(&Led1, 0);
        TimerStart(&Led1Timer);
    }
    NextTx = true;
}

/*!
 * \brief	 MCPS-Indication event function
 *
 * \param	 [IN] mcpsIndication - Pointer to the indication structure,
 *							 containing indication attributes.
 */
static void McpsIndication(McpsIndication_t *mcpsIndication)
{
    isConfirmed = false;
    if (mcpsIndication->Status != LORAMAC_EVENT_INFO_STATUS_OK)
    {
        return;
    }

    switch (mcpsIndication->McpsIndication)
    {
    case MCPS_UNCONFIRMED:
    {
        break;
    }
    case MCPS_CONFIRMED:
    {
        isConfirmed = true;
        break;
    }
    case MCPS_PROPRIETARY:
    {
        break;
    }
    case MCPS_MULTICAST:
    {
        break;
    }
    default:
        break;
    }

    // Check Multicast
    // Check Port
    // Check Datarate
    // Check FramePending

    if (mcpsIndication->FramePending == true)
    {
        // The server signals that it has pending data to be sent.
        // We schedule an uplink as soon as possible to flush the server.
        OnTxNextPacketTimerEvent();
    }
    // Check Buffer
    // Check BufferSize
    // Check Rssi
    // Check Snr
    // Check RxSlot

    if (ComplianceTest.Running == true)
    {
        ComplianceTest.DownLinkCounter++;
    }

    if (mcpsIndication->RxData == true)
    {
        switch (mcpsIndication->Port)
        {
        case 1: // The application LED can be controlled on port 1 or 2
        case 2:
            if (mcpsIndication->BufferSize == 1)
            {
                AppLedStateOn = mcpsIndication->Buffer[0] & 0x01;
                if (AppLedStateOn == 1)
                {
                    GpioWrite(&Led2, 0);
                }
                else
                {
                    GpioWrite(&Led2, 1);
                }
            }

            isConfirmed = true;
            OnTxNextPacketTimerEvent();
            break;
        case 224:
            if (ComplianceTest.Running == false)
            {
                // Check compliance test enable command (i)
                if ((mcpsIndication->BufferSize == 4) &&
                    (mcpsIndication->Buffer[0] == 0x01) &&
                    (mcpsIndication->Buffer[1] == 0x01) &&
                    (mcpsIndication->Buffer[2] == 0x01) &&
                    (mcpsIndication->Buffer[3] == 0x01))
                {
                    IsTxConfirmed = false;
                    AppPort = 224;
                    AppDataSizeBackup = AppDataSize;
                    AppDataSize = 2;
                    ComplianceTest.DownLinkCounter = 0;
                    ComplianceTest.LinkCheck = false;
                    ComplianceTest.DemodMargin = 0;
                    ComplianceTest.NbGateways = 0;
                    ComplianceTest.Running = true;
                    ComplianceTest.State = 1;

                    MibRequestConfirm_t mibReq;

                    mibReq.Type = MIB_ADR;
                    mibReq.Param.AdrEnable = true;
                    LoRaMacMibSetRequestConfirm(&mibReq);

#if defined(REGION_EU868)
                    LoRaMacTestSetDutyCycleOn(false);
#endif
                }
            }
            else
            {
                ComplianceTest.State = mcpsIndication->Buffer[0];
                switch (ComplianceTest.State)
                {
                case 0: // Check compliance test disable command (ii)
                    IsTxConfirmed = LORAWAN_CONFIRMED_MSG_ON;
                    AppPort = LORAWAN_APP_PORT;
                    AppDataSize = AppDataSizeBackup;
                    ComplianceTest.DownLinkCounter = 0;
                    ComplianceTest.Running = false;

                    MibRequestConfirm_t mibReq;
                    mibReq.Type = MIB_ADR;
                    mibReq.Param.AdrEnable = LORAWAN_ADR_ON;
                    LoRaMacMibSetRequestConfirm(&mibReq);
#if defined(REGION_EU868)
                    LoRaMacTestSetDutyCycleOn(LORAWAN_DUTYCYCLE_ON);
#endif
                    break;
                case 1: // (iii, iv)
                    AppDataSize = 2;
                    break;
                case 2: // Enable confirmed messages (v)
                    IsTxConfirmed = true;
                    ComplianceTest.State = 1;
                    break;
                case 3: // Disable confirmed messages (vi)
                    IsTxConfirmed = false;
                    ComplianceTest.State = 1;
                    break;
                case 4: // (vii)
                    AppDataSize = mcpsIndication->BufferSize;

                    AppData[0] = 4;
                    for (uint8_t i = 1; i < MIN(AppDataSize, LORAWAN_APP_DATA_MAX_SIZE); i++)
                    {
                        AppData[i] = mcpsIndication->Buffer[i] + 1;
                    }
                    break;
                case 5: // (viii)
                {
                    MlmeReq_t mlmeReq;
                    mlmeReq.Type = MLME_LINK_CHECK;
                    LoRaMacMlmeRequest(&mlmeReq);
                }
                break;
                case 6: // (ix)
                {
                    MlmeReq_t mlmeReq;

                    // Disable TestMode and revert back to normal operation
                    IsTxConfirmed = LORAWAN_CONFIRMED_MSG_ON;
                    AppPort = LORAWAN_APP_PORT;
                    AppDataSize = AppDataSizeBackup;
                    ComplianceTest.DownLinkCounter = 0;
                    ComplianceTest.Running = false;

                    MibRequestConfirm_t mibReq;
                    mibReq.Type = MIB_ADR;
                    mibReq.Param.AdrEnable = LORAWAN_ADR_ON;
                    LoRaMacMibSetRequestConfirm(&mibReq);
#if defined(REGION_EU868)
                    LoRaMacTestSetDutyCycleOn(LORAWAN_DUTYCYCLE_ON);
#endif

                    mlmeReq.Type = MLME_JOIN;

                    mlmeReq.Req.Join.DevEui = DevEui;
                    mlmeReq.Req.Join.AppEui = AppEui;
                    mlmeReq.Req.Join.AppKey = AppKey;
                    mlmeReq.Req.Join.Datarate = LORAWAN_DEFAULT_DATARATE;

                    if (LoRaMacMlmeRequest(&mlmeReq) == LORAMAC_STATUS_OK)
                    {
                        DeviceState = DEVICE_STATE_SLEEP;
                    }
                    else
                    {
                        DeviceState = DEVICE_STATE_CYCLE;
                    }
                }
                break;
                case 7: // (x)
                {
                    if (mcpsIndication->BufferSize == 3)
                    {
                        MlmeReq_t mlmeReq;
                        mlmeReq.Type = MLME_TXCW;
                        mlmeReq.Req.TxCw.Timeout = (uint16_t)((mcpsIndication->Buffer[1] << 8) | mcpsIndication->Buffer[2]);
                        LoRaMacMlmeRequest(&mlmeReq);
                    }
                    else if (mcpsIndication->BufferSize == 7)
                    {
                        MlmeReq_t mlmeReq;
                        mlmeReq.Type = MLME_TXCW_1;
                        mlmeReq.Req.TxCw.Timeout = (uint16_t)((mcpsIndication->Buffer[1] << 8) | mcpsIndication->Buffer[2]);
                        mlmeReq.Req.TxCw.Frequency = (uint32_t)((mcpsIndication->Buffer[3] << 16) | (mcpsIndication->Buffer[4] << 8) | mcpsIndication->Buffer[5]) * 100;
                        mlmeReq.Req.TxCw.Power = mcpsIndication->Buffer[6];
                        LoRaMacMlmeRequest(&mlmeReq);
                    }
                    ComplianceTest.State = 1;
                }
                break;
                default:
                    break;
                }
            }
            break;
        default:
            isConfirmed = false;
            break;
        }
    }

    // Switch LED 1 ON for each received downlink
    GpioWrite(&Led1, 0);
    TimerStart(&Led2Timer);
}

/*!
 * \brief	 MLME-Confirm event function
 *
 * \param	 [IN] mlmeConfirm - Pointer to the confirm structure,
 *							 containing confirm attributes.
 */
static void MlmeConfirm(MlmeConfirm_t *mlmeConfirm)
{
    switch (mlmeConfirm->MlmeRequest)
    {
    case MLME_JOIN:
    {
        if (mlmeConfirm->Status == LORAMAC_EVENT_INFO_STATUS_OK)
        {
            // Status is OK, node has joined the network
            DeviceState = DEVICE_STATE_SEND;
        }
        else
        {
            // Join was not successful. Try to join again
            MlmeReq_t mlmeReq;
            mlmeReq.Type = MLME_JOIN;
            mlmeReq.Req.Join.DevEui = DevEui;
            mlmeReq.Req.Join.AppEui = AppEui;
            mlmeReq.Req.Join.AppKey = AppKey;
            mlmeReq.Req.Join.Datarate = LORAWAN_DEFAULT_DATARATE;

            if (LoRaMacMlmeRequest(&mlmeReq) == LORAMAC_STATUS_OK)
            {
                DeviceState = DEVICE_STATE_SLEEP;
            }
            else
            {
                DeviceState = DEVICE_STATE_CYCLE;
            }
        }
        break;
    }
    case MLME_LINK_CHECK:
    {
        if (mlmeConfirm->Status == LORAMAC_EVENT_INFO_STATUS_OK)
        {
            // Check DemodMargin
            // Check NbGateways
            if (ComplianceTest.Running == true)
            {
                ComplianceTest.LinkCheck = true;
                ComplianceTest.DemodMargin = mlmeConfirm->DemodMargin;
                ComplianceTest.NbGateways = mlmeConfirm->NbGateways;
            }
        }
        break;
    }
    default:
        break;
    }
    NextTx = true;
}

/*!
 * \brief	 MLME-Indication event function
 *
 * \param	 [IN] mlmeIndication - Pointer to the indication structure.
 */
static void MlmeIndication(MlmeIndication_t *mlmeIndication)
{
    switch (mlmeIndication->MlmeIndication)
    {
    case MLME_SCHEDULE_UPLINK:
    { // The MAC signals that we shall provide an uplink as soon as possible
        OnTxNextPacketTimerEvent();
        break;
    }
    default:
        break;
    }
}

//-------------------------------------------------------------------------------------------------------//

static uint16_t m_conn_handle = BLE_CONN_HANDLE_INVALID; /**< Handle of the current connection. */

static ble_uuid_t m_adv_uuids[] = {{BLE_UUID_DEVICE_INFORMATION_SERVICE, BLE_UUID_TYPE_BLE}}; /**< Universally unique service identifiers. */
static ble_leds_t m_led_service;                                                              // Definition of LED Service Structure
NRF_SDH_BLE_OBSERVER(m_led_service_observer, APP_BLE_OBSERVER_PRIO, ble_led_service_on_ble_evt, (void *)&m_led_service);

static void advertising_start(void *erase_bonds);

/**@brief Callback function for asserts in the SoftDevice.
 *
 * @details This function will be called in case of an assert in the SoftDevice.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyze
 *          how your product is supposed to react in case of Assert.
 * @warning On assert from the SoftDevice, the system can only recover on reset.
 *
 * @param[in] line_num   Line number of the failing ASSERT call.
 * @param[in] file_name  File name of the failing ASSERT call.
 */
void assert_nrf_callback(uint16_t line_num, const uint8_t *p_file_name)
{
    app_error_handler(DEAD_BEEF, line_num, p_file_name);
}

/**@brief Function for handling Peer Manager events.
 *
 * @param[in] p_evt  Peer Manager event.
 */
static void pm_evt_handler(pm_evt_t const *p_evt)
{
    bool delete_bonds = false;

    pm_handler_on_pm_evt(p_evt);
    pm_handler_flash_clean(p_evt);

    switch (p_evt->evt_id)
    {
    case PM_EVT_PEERS_DELETE_SUCCEEDED:
        advertising_start(&delete_bonds);
        break;

    default:
        break;
    }
}

/**@brief Function for the Timer initialization.
 *
 * @details Initializes the timer module. This creates and starts application timers.
 */
static void timers_init(void)
{
    // Initialize timer module.
    ret_code_t err_code = app_timer_init();
    APP_ERROR_CHECK(err_code);

    // Create timers.

    /* YOUR_JOB: Create any timers to be used by the application.
                 Below is an example of how to create a timer.
                 For every new timer needed, increase the value of the macro APP_TIMER_MAX_TIMERS by
                 one.
       ret_code_t err_code;
       err_code = app_timer_create(&m_app_timer_id, APP_TIMER_MODE_REPEATED, timer_timeout_handler);
       APP_ERROR_CHECK(err_code); */
}

/**@brief Function for the GAP initialization.
 *
 * @details This function sets up all the necessary GAP (Generic Access Profile) parameters of the
 *          device including the device name, appearance, and the preferred connection parameters.
 */
static void gap_params_init(void)
{
    ret_code_t err_code;
    ble_gap_conn_params_t gap_conn_params;
    ble_gap_conn_sec_mode_t sec_mode;

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);

    err_code = sd_ble_gap_device_name_set(&sec_mode,
        (const uint8_t *)DEVICE_NAME,
        strlen(DEVICE_NAME));
    APP_ERROR_CHECK(err_code);

    /* YOUR_JOB: Use an appearance value matching the application's use case.
       err_code = sd_ble_gap_appearance_set(BLE_APPEARANCE_);
       APP_ERROR_CHECK(err_code); */

    memset(&gap_conn_params, 0, sizeof(gap_conn_params));

    gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL;
    gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL;
    gap_conn_params.slave_latency = SLAVE_LATENCY;
    gap_conn_params.conn_sup_timeout = CONN_SUP_TIMEOUT;

    err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for initializing the GATT module.
 */
static void gatt_init(void)
{
    ret_code_t err_code = nrf_ble_gatt_init(&m_gatt, NULL);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for handling Queued Write Module errors.
 *
 * @details A pointer to this function will be passed to each service which may need to inform the
 *          application about an error.
 *
 * @param[in]   nrf_error   Error code containing information about what went wrong.
 */
static void nrf_qwr_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}

/**@brief Function for handling the YYY Service events.
 * YOUR_JOB implement a service handler function depending on the event the service you are using can generate
 *
 * @details This function will be called for all YY Service events which are passed to
 *          the application.
 *
 * @param[in]   p_yy_service   YY Service structure.
 * @param[in]   p_evt          Event received from the YY Service.
 *
 *
static void on_yys_evt(ble_yy_service_t     * p_yy_service,
                       ble_yy_service_evt_t * p_evt)
{
    switch (p_evt->evt_type)
    {
        case BLE_YY_NAME_EVT_WRITE:
            APPL_LOG("[APPL]: charact written with value %s. ", p_evt->params.char_xx.value.p_str);
            break;

        default:
            // No implementation needed.
            break;
    }
}
*/

/**@brief Function for initializing services that will be used by the application.
 */
static void services_init(void)
{
    ret_code_t err_code;
    nrf_ble_qwr_init_t qwr_init = {0};

    // Initialize Queued Write Module.
    qwr_init.error_handler = nrf_qwr_error_handler;

    err_code = nrf_ble_qwr_init(&m_qwr, &qwr_init);
    APP_ERROR_CHECK(err_code);

    led_service_init(&m_led_service);
    //	atCommand_service_init(&m_atCommand_service);
}

/**@brief Function for handling the Connection Parameters Module.
 *
 * @details This function will be called for all events in the Connection Parameters Module which
 *          are passed to the application.
 *          @note All this function does is to disconnect. This could have been done by simply
 *                setting the disconnect_on_fail config parameter, but instead we use the event
 *                handler mechanism to demonstrate its use.
 *
 * @param[in] p_evt  Event received from the Connection Parameters Module.
 */
static void on_conn_params_evt(ble_conn_params_evt_t *p_evt)
{
    ret_code_t err_code;

    if (p_evt->evt_type == BLE_CONN_PARAMS_EVT_FAILED)
    {
        err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_CONN_INTERVAL_UNACCEPTABLE);
        APP_ERROR_CHECK(err_code);
    }
}

/**@brief Function for handling a Connection Parameters error.
 *
 * @param[in] nrf_error  Error code containing information about what went wrong.
 */
static void conn_params_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}

/**@brief Function for initializing the Connection Parameters module.
 */
static void conn_params_init(void)
{
    ret_code_t err_code;
    ble_conn_params_init_t cp_init;

    memset(&cp_init, 0, sizeof(cp_init));

    cp_init.p_conn_params = NULL;
    cp_init.first_conn_params_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY;
    cp_init.next_conn_params_update_delay = NEXT_CONN_PARAMS_UPDATE_DELAY;
    cp_init.max_conn_params_update_count = MAX_CONN_PARAMS_UPDATE_COUNT;
    cp_init.start_on_notify_cccd_handle = BLE_GATT_HANDLE_INVALID;
    cp_init.disconnect_on_fail = false;
    cp_init.evt_handler = on_conn_params_evt;
    cp_init.error_handler = conn_params_error_handler;

    err_code = ble_conn_params_init(&cp_init);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for starting timers.
 */
static void application_timers_start(void)
{
}

/**@brief Function for putting the chip into sleep mode.
 *
 * @note This function will not return.
 */
static void sleep_mode_enter(void)
{
    /*
    ret_code_t err_code;

    err_code = bsp_indication_set(BSP_INDICATE_IDLE);
    APP_ERROR_CHECK(err_code);

    // Prepare wakeup buttons.
    err_code = bsp_btn_ble_sleep_mode_prepare();
    APP_ERROR_CHECK(err_code);

    // Go to system-off mode (this function will not return; wakeup will cause a reset).
    err_code = sd_power_system_off();
    APP_ERROR_CHECK(err_code);
	*/
}

/**@brief Function for handling advertising events.
 *
 * @details This function will be called for advertising events which are passed to the application.
 *
 * @param[in] ble_adv_evt  Advertising event.
 */
static void on_adv_evt(ble_adv_evt_t ble_adv_evt)
{

    switch (ble_adv_evt)
    {
    case BLE_ADV_EVT_FAST:
        NRF_LOG_INFO("Fast advertising.");
        break;

    case BLE_ADV_EVT_IDLE:
        //sleep_mode_enter();
        break;

    default:
        break;
    }
}

/**@brief Function for handling BLE events.
 *
 * @param[in]   p_ble_evt   Bluetooth stack event.
 * @param[in]   p_context   Unused.
 */
static void ble_evt_handler(ble_evt_t const *p_ble_evt, void *p_context)
{
    ret_code_t err_code = NRF_SUCCESS;

    switch (p_ble_evt->header.evt_id)
    {
    case BLE_GAP_EVT_DISCONNECTED:
        NRF_LOG_INFO("Disconnected.");
        // LED indication will be changed when advertising starts.
        break;

    case BLE_GAP_EVT_CONNECTED:
        NRF_LOG_INFO("Connected.");
        //err_code = bsp_indication_set(BSP_INDICATE_CONNECTED);
        //APP_ERROR_CHECK(err_code);
        m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
        err_code = nrf_ble_qwr_conn_handle_assign(&m_qwr, m_conn_handle);
        APP_ERROR_CHECK(err_code);

        break;

    case BLE_GAP_EVT_PHY_UPDATE_REQUEST:
    {
        NRF_LOG_DEBUG("PHY update request.");
        ble_gap_phys_t const phys =
            {
                .rx_phys = BLE_GAP_PHY_AUTO,
                .tx_phys = BLE_GAP_PHY_AUTO,
            };
        err_code = sd_ble_gap_phy_update(p_ble_evt->evt.gap_evt.conn_handle, &phys);
        APP_ERROR_CHECK(err_code);
    }
    break;

    case BLE_GATTC_EVT_TIMEOUT:
        // Disconnect on GATT Client timeout event.
        NRF_LOG_DEBUG("GATT Client Timeout.");
        err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gattc_evt.conn_handle,
            BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
        APP_ERROR_CHECK(err_code);
        break;

    case BLE_GATTS_EVT_TIMEOUT:
        // Disconnect on GATT Server timeout event.
        NRF_LOG_DEBUG("GATT Server Timeout.");
        err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gatts_evt.conn_handle,
            BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
        APP_ERROR_CHECK(err_code);
        break;

    default:
        // No implementation needed.
        break;
    }
}

/**@brief Function for initializing the BLE stack.
 *
 * @details Initializes the SoftDevice and the BLE event interrupt.
 */
static void ble_stack_init(void)
{
    ret_code_t err_code;

    err_code = nrf_sdh_enable_request();
    APP_ERROR_CHECK(err_code);

    // Configure the BLE stack using the default settings.
    // Fetch the start address of the application RAM.
    uint32_t ram_start = 0;
    err_code = nrf_sdh_ble_default_cfg_set(APP_BLE_CONN_CFG_TAG, &ram_start);
    APP_ERROR_CHECK(err_code);

    // Enable BLE stack.
    err_code = nrf_sdh_ble_enable(&ram_start);
    APP_ERROR_CHECK(err_code);

    // Register a handler for BLE events.
    NRF_SDH_BLE_OBSERVER(m_ble_observer, APP_BLE_OBSERVER_PRIO, ble_evt_handler, NULL);
}

/**@brief Function for the Peer Manager initialization.
 */
static void peer_manager_init(void)
{
    ble_gap_sec_params_t sec_param;
    ret_code_t err_code;

    err_code = pm_init();
    APP_ERROR_CHECK(err_code);

    memset(&sec_param, 0, sizeof(ble_gap_sec_params_t));

    // Security parameters to be used for all security procedures.
    sec_param.bond = SEC_PARAM_BOND;
    sec_param.mitm = SEC_PARAM_MITM;
    sec_param.lesc = SEC_PARAM_LESC;
    sec_param.keypress = SEC_PARAM_KEYPRESS;
    sec_param.io_caps = SEC_PARAM_IO_CAPABILITIES;
    sec_param.oob = SEC_PARAM_OOB;
    sec_param.min_key_size = SEC_PARAM_MIN_KEY_SIZE;
    sec_param.max_key_size = SEC_PARAM_MAX_KEY_SIZE;
    sec_param.kdist_own.enc = 1;
    sec_param.kdist_own.id = 1;
    sec_param.kdist_peer.enc = 1;
    sec_param.kdist_peer.id = 1;

    err_code = pm_sec_params_set(&sec_param);
    APP_ERROR_CHECK(err_code);

    err_code = pm_register(pm_evt_handler);
    APP_ERROR_CHECK(err_code);
}

/**@brief Clear bond information from persistent storage.
 */
static void delete_bonds(void)
{
    ret_code_t err_code;

    NRF_LOG_INFO("Erase bonds!");

    err_code = pm_peers_delete();
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for initializing the Advertising functionality.
 */
static void advertising_init(void)
{
    ret_code_t err_code;
    ble_advertising_init_t init;

    memset(&init, 0, sizeof(init));

    init.advdata.name_type = BLE_ADVDATA_FULL_NAME;
    init.advdata.include_appearance = true;
    init.advdata.flags = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;
    init.advdata.uuids_complete.uuid_cnt = sizeof(m_adv_uuids) / sizeof(m_adv_uuids[0]);
    init.advdata.uuids_complete.p_uuids = m_adv_uuids;

    init.config.ble_adv_fast_enabled = true;
    init.config.ble_adv_fast_interval = APP_ADV_INTERVAL;
    init.config.ble_adv_fast_timeout = APP_ADV_DURATION;

    init.evt_handler = on_adv_evt;

    err_code = ble_advertising_init(&m_advertising, &init);
    APP_ERROR_CHECK(err_code);

    ble_advertising_conn_cfg_tag_set(&m_advertising, APP_BLE_CONN_CFG_TAG);
}

/**@brief Function for initializing power management.
 */
static void power_management_init(void)
{
    ret_code_t err_code;
    err_code = nrf_pwr_mgmt_init();
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for handling the idle state (main loop).
 *
 * @details If there is no pending log operation, then sleep until next the next event occurs.
 */
static void idle_state_handle(void)
{
    if (NRF_LOG_PROCESS() == false)
    {
        nrf_pwr_mgmt_run();
    }
}

/**@brief Function for starting advertising.
 */
static void advertising_start(void *p_erase_bonds)
{
    bool erase_bonds = *(bool *)p_erase_bonds;

    if (erase_bonds)
    {
        delete_bonds();
        // Advertising is started by PM_EVT_PEERS_DELETE_SUCCEEDED event.
    }
    else
    {
        ret_code_t err_code = ble_advertising_start(&m_advertising, BLE_ADV_MODE_FAST);
        APP_ERROR_CHECK(err_code);
    }
}

void HardFault_Handler(void)
{
    uint32_t *sp = (uint32_t *)__get_MSP(); // Get stack pointer
    uint32_t ia = sp[12];                   // Get instruction address from stack

    //    printf("Hard Fault at address: 0x%08x\r\n", (unsigned int)ia);
    while (1)
        ;
}

#if NRF_LOG_ENABLED
/**@brief Thread for handling the logger.
 *
 * @details This thread is responsible for processing log entries if logs are deferred.
 *          Thread flushes all log entries and suspends. It is resumed by idle task hook.
 *
 * @param[in]   arg   Pointer used for passing some arbitrary information (context) from the
 *                    osThreadCreate() call to the thread.
 */
static void logger_thread(void *arg)
{
    UNUSED_PARAMETER(arg);

    while (1)
    {
        NRF_LOG_FLUSH();

        vTaskSuspend(NULL); // Suspend myself
    }
}
#endif //NRF_LOG_ENABLED

/**@brief A function which is hooked to idle task.
 * @note Idle hook must be enabled in FreeRTOS configuration (configUSE_IDLE_HOOK).
 */
void vApplicationIdleHook(void)
{
#if NRF_LOG_ENABLED
    vTaskResume(m_logger_thread);
#endif
}

static void lorawan_state_machine_task(void *pvParameter)
{
    vTaskDelay(1);
    traceISR_EXIT_TO_SCHEDULER();

    LoRaMacPrimitives_t LoRaMacPrimitives;
    LoRaMacCallback_t LoRaMacCallbacks;
    MibRequestConfirm_t mibReq;

    // Tick the RTC to execute callback in context of the main loop (in stead of the IRQ)
    while (1)
    {
        if (isBLEConnected == false)
        {
            TimerProcess();

            switch (DeviceState)
            {
            case DEVICE_STATE_INIT:
            {
                LoRaMacPrimitives.MacMcpsConfirm = McpsConfirm;
                LoRaMacPrimitives.MacMcpsIndication = McpsIndication;
                LoRaMacPrimitives.MacMlmeConfirm = MlmeConfirm;
                LoRaMacPrimitives.MacMlmeIndication = MlmeIndication;
                LoRaMacCallbacks.GetBatteryLevel = BoardGetBatteryLevel;
                LoRaMacInitialization(&LoRaMacPrimitives, &LoRaMacCallbacks, ACTIVE_REGION);

                TimerInit(&TxNextPacketTimer, OnTxNextPacketTimerEvent);

                TimerInit(&Led1Timer, OnLed1TimerEvent);
                TimerSetValue(&Led1Timer, 25);

                TimerInit(&Led2Timer, OnLed2TimerEvent);
                TimerSetValue(&Led2Timer, 25);

                mibReq.Type = MIB_ADR;
                mibReq.Param.AdrEnable = LORAWAN_ADR_ON;
                LoRaMacMibSetRequestConfirm(&mibReq);

                mibReq.Type = MIB_SYSTEM_MAX_RX_ERROR;
                mibReq.Param.SystemMaxRxError = 20;
                LoRaMacMibSetRequestConfirm(&mibReq);

                mibReq.Type = MIB_PUBLIC_NETWORK;
                mibReq.Param.EnablePublicNetwork = LORAWAN_PUBLIC_NETWORK;
                LoRaMacMibSetRequestConfirm(&mibReq);

#if defined(REGION_EU868)
                LoRaMacTestSetDutyCycleOn(LORAWAN_DUTYCYCLE_ON);
#endif

#if (USE_TTN_NETWORK == 1)
                mibReq.Type = MIB_RX2_DEFAULT_CHANNEL;
                mibReq.Param.Rx2DefaultChannel = (Rx2ChannelParams_t){869525000, DR_3};
                LoRaMacMibSetRequestConfirm(&mibReq);

                mibReq.Type = MIB_RX2_CHANNEL;
                mibReq.Param.Rx2Channel = (Rx2ChannelParams_t){869525000, DR_3};
                LoRaMacMibSetRequestConfirm(&mibReq);
#endif

                static MulticastParams_t channelParam;

                channelParam.Address = (uint32_t)0x11223344;

                channelParam.AppSKey[0] = 0x2B;
                channelParam.AppSKey[1] = 0x7E;
                channelParam.AppSKey[2] = 0x15;
                channelParam.AppSKey[3] = 0x16;
                channelParam.AppSKey[4] = 0x28;
                channelParam.AppSKey[5] = 0xAE;
                channelParam.AppSKey[6] = 0xD2;
                channelParam.AppSKey[7] = 0xA6;
                channelParam.AppSKey[8] = 0xAB;
                channelParam.AppSKey[9] = 0xF7;
                channelParam.AppSKey[10] = 0x15;
                channelParam.AppSKey[11] = 0x88;
                channelParam.AppSKey[12] = 0x09;
                channelParam.AppSKey[13] = 0xCF;
                channelParam.AppSKey[14] = 0x4F;
                channelParam.AppSKey[15] = 0x3C;

                channelParam.NwkSKey[0] = 0x2B;
                channelParam.NwkSKey[1] = 0x7E;
                channelParam.NwkSKey[2] = 0x15;
                channelParam.NwkSKey[3] = 0x16;
                channelParam.NwkSKey[4] = 0x28;
                channelParam.NwkSKey[5] = 0xAE;
                channelParam.NwkSKey[6] = 0xD2;
                channelParam.NwkSKey[7] = 0xA6;
                channelParam.NwkSKey[8] = 0xAB;
                channelParam.NwkSKey[9] = 0xF7;
                channelParam.NwkSKey[10] = 0x15;
                channelParam.NwkSKey[11] = 0x88;
                channelParam.NwkSKey[12] = 0x09;
                channelParam.NwkSKey[13] = 0xCF;
                channelParam.NwkSKey[14] = 0x4F;
                channelParam.NwkSKey[15] = 0x3C;

                channelParam.DownLinkCounter = 0;

                channelParam.Next = 0;
                //				LoRaMacMulticastChannelLink(&channelParam);

                mibReq.Type = MIB_MULTICAST_CHANNEL;
                mibReq.Param.MulticastList = &channelParam;
                LoRaMacMibSetRequestConfirm(&mibReq);

                mibReq.Type = MIB_DEVICE_CLASS;
                mibReq.Param.Class = CLASS_C;
                LoRaMacMibSetRequestConfirm(&mibReq);
                DeviceState = DEVICE_STATE_JOIN;

                break;
            }
            case DEVICE_STATE_JOIN:
            {
#if (OVER_THE_AIR_ACTIVATION != 0)
                MlmeReq_t mlmeReq;

                // Initialize LoRaMac device unique ID
                BoardGetUniqueId(DevEui);

                mlmeReq.Type = MLME_JOIN;

                mlmeReq.Req.Join.DevEui = DevEui;
                mlmeReq.Req.Join.AppEui = AppEui;
                mlmeReq.Req.Join.AppKey = AppKey;
                mlmeReq.Req.Join.Datarate = LORAWAN_DEFAULT_DATARATE;

                if (LoRaMacMlmeRequest(&mlmeReq) == LORAMAC_STATUS_OK)
                {
                    DeviceState = DEVICE_STATE_SLEEP;
                }
                else
                {
                    DeviceState = DEVICE_STATE_CYCLE;
                }
#else
                // Choose a random device address if not already defined in Commissioning.h
                if (DevAddr == 0)
                {
                    // Random seed initialization
                    srand1(BoardGetRandomSeed());

                    // Choose a random device address
                    DevAddr = randr(0, 0x01FFFFFF);
                }

                mibReq.Type = MIB_NET_ID;
                mibReq.Param.NetID = LORAWAN_NETWORK_ID;
                LoRaMacMibSetRequestConfirm(&mibReq);

                mibReq.Type = MIB_DEV_ADDR;
                mibReq.Param.DevAddr = DevAddr;
                LoRaMacMibSetRequestConfirm(&mibReq);

                mibReq.Type = MIB_NWK_SKEY;
                mibReq.Param.NwkSKey = NwkSKey;
                LoRaMacMibSetRequestConfirm(&mibReq);

                mibReq.Type = MIB_APP_SKEY;
                mibReq.Param.AppSKey = AppSKey;
                LoRaMacMibSetRequestConfirm(&mibReq);

                mibReq.Type = MIB_NETWORK_JOINED;
                mibReq.Param.IsNetworkJoined = true;
                LoRaMacMibSetRequestConfirm(&mibReq);

                DeviceState = DEVICE_STATE_SEND;
#endif
                break;
            }
            case DEVICE_STATE_SEND:
            {
                if (NextTx == true)
                {
                    if (isConfirmed == true)
                    {
                        PrepareTxFrame(0);
                    }
                    else
                    {
                        PrepareTxFrame(AppPort);
                    }

                    NextTx = SendFrame();
                }
                if (ComplianceTest.Running == true)
                {
                    // Schedule next packet transmission
                    TxDutyCycleTime = 5000; // 5000 ms
                }
                else
                {
                    // Schedule next packet transmission
                    TxDutyCycleTime = APP_TX_DUTYCYCLE + randr(-APP_TX_DUTYCYCLE_RND, APP_TX_DUTYCYCLE_RND);
                }
                DeviceState = DEVICE_STATE_CYCLE;
                break;
            }
            case DEVICE_STATE_CYCLE:
            {
                DeviceState = DEVICE_STATE_SLEEP;

                // Schedule next packet transmission
                TimerSetValue(&TxNextPacketTimer, TxDutyCycleTime);
                TimerStart(&TxNextPacketTimer);
                break;
            }
            case DEVICE_STATE_SLEEP:
            {
                // Wake up through events
                // TimerLowPowerHandler( );
                break;
            }
            default:
            {
                DeviceState = DEVICE_STATE_INIT;
                break;
            }
            }
        }
        SEGGER_SYSVIEW_Print("LoRaWAN State Machine Task is yielding");
    }
}
void vTimerCallback(TimerHandle_t xTimer)
{
    //printf("Timer callback\r\n");
}
static void task1_thread(void *arg)
{
    UNUSED_PARAMETER(arg);
    m_timer_handle = xTimerCreate("TestTimer", 1000, pdTRUE, (void *)0, vTimerCallback);
    xTimerStart(m_timer_handle, portMAX_DELAY);
    uint32_t data = 0;

    uint8_t size = 4;

    while (1)
    {
        traceISR_EXIT_TO_SCHEDULER();
        vTaskDelay(1);
        //printf("\r\nHello from TASK 1\r\n");

   //     SEGGER_SYSVIEW_Print("Task 1 is yielding");
        // taskYIELD();

        // vTaskSuspend(NULL); // Suspend myself
    }
}
static void task2_thread(void *arg)
{
    UNUSED_PARAMETER(arg);

    while (1)
    {
        vTaskDelay(1);
        traceISR_EXIT_TO_SCHEDULER();
        //  printf("\r\nHello from TASK 2\r\n");
        //plot_output_signal();
     //   SEGGER_SYSVIEW_Print("Task 2 is yielding");

        //taskYIELD();
        //vTaskSuspend(NULL); // Suspend myself
    }
}

static void spi_test_task(void *pvParameter)
{

    while (1)
    {
        vTaskDelay(1);
        traceISR_EXIT_TO_SCHEDULER();
        GpioWrite(&CSPin, 0);
        vTaskDelay(10);
        GetDeviceMemsID();
        //NSS = 1;
        GpioWrite(&CSPin, 1);

        vTaskDelay(1000);
        // GetDeviceMemsID();

        // vTaskSuspend(NULL);
    }
}

void plot_output_signal(void)
{
    int i, j;
    for (i = 0; i < SIG_LENGTH; i++)
    {
        acValBuffer.outputSampleVal = floor(100.f * outputSignal_f32[i]);
        acValBuffer.sampleVal = floor(100.f * inputSignal_f32_1kHz_15kHz[i]);
       // printf("%d\r\n", acValBuffer.outputSampleVal);
        SEGGER_RTT_Write(JS_RTT_Channel, &acValBuffer, sizeof(acValBuffer));
        for (j = 0; j < 3000; j++)
        {
        }
    }
}

void plot_impulse_signal(void)
{

    int i, j;
    for (i = 0; i < IMP_RSP_LENGTH; i++)
    {
        acValBuffer.impulseSample = floor(100.f * impulse_response[i]);
        SEGGER_RTT_Write(JS_RTT_Channel, &acValBuffer, sizeof(acValBuffer));
        for (j = 0; j < 3000; j++)
        {
        }
    }
}

/**@brief Function for application main entry.
 */
int main(void)
{

    DWT->CTRL |= (1 << 0);
    BoardInitMcu();
    BoardInitPeriph();
    // Set the external high frequency clock source to 32 MHz

    // Start the external high frequency crystal
    NRF_CLOCK->EVENTS_HFCLKSTARTED = 0;
    NRF_CLOCK->TASKS_HFCLKSTART = 1;

    // Wait for the external oscillator to start up
    while (NRF_CLOCK->EVENTS_HFCLKSTARTED == 0)
    {
    }

    DeviceState = DEVICE_STATE_INIT;
    bool erase_bonds;

    // Initialize.
    timers_init();
    power_management_init();
    ble_stack_init();
    gap_params_init();
    gatt_init();
    advertising_init();
    services_init();
    conn_params_init();
    peer_manager_init();
    // Start execution.
    application_timers_start();
    /* Initialize SPI */
    //    SPI_Init();

    /* Initialize ADXL355 */
    //   ADXL355_Init();

    BaseType_t xReturned;

    SEGGER_SYSVIEW_Conf();
    SEGGER_SYSVIEW_Start();

    // Create a FreeRTOS task for the BLE stack.
    // The task will run advertising_start() before entering its loop.

    SEGGER_RTT_ConfigUpBuffer(JS_RTT_Channel, "JScope_I4I4I4", &JS_RTT_UpBuffer[0], sizeof(JS_RTT_UpBuffer), SEGGER_RTT_MODE_BLOCK_IF_FIFO_FULL);

  //  arm_fir_instance_f32 _1Khz_15Khz_sig;

    //arm_fir_init_f32(&_1Khz_15Khz_sig, NUM_TAPS, (float32_t *)&firCoeffs32[0], &firStateF32[0], BLOCK_SIZE);
 //   int i = 0;
  //  for (i = 0; i < numBlocks; i++)
   // {
   //     arm_fir_f32(&_1Khz_15Khz_sig, &inputSignal_f32_1kHz_15kHz[0] + (i * BLOCK_SIZE), &outputSignal_f32[0] + (i * BLOCK_SIZE), BLOCK_SIZE);
   // }
    // plot_output_signal(); 
     // plot_input_signal(); 
     plot_impulse_signal();

    nrf_sdh_freertos_init(advertising_start, &erase_bonds);
    
    xReturned = xTaskCreate(task1_thread, "T1", 128, NULL, 2, &m_task_1);
    if (xReturned != pdPASS)
    {
        NRF_LOG_ERROR("SoftDevice task not created.");
        APP_ERROR_HANDLER(NRF_ERROR_NO_MEM);
    }

    xReturned = xTaskCreate(task2_thread, "T2 ", 128, NULL, 2, &m_task_2);
    if (xReturned != pdPASS)
    {
        NRF_LOG_ERROR("SoftDevice task not created.");
        APP_ERROR_HANDLER(NRF_ERROR_NO_MEM);
    }

    
#if NRF_LOG_ENABLED
    // Start execution.
    xReturned =  xTaskCreate(logger_thread, "LOGGER", 256, NULL, 1, &m_logger_thread);
    if (xReturned != pdPASS)
    {
        APP_ERROR_HANDLER(NRF_ERROR_NO_MEM);
    }
#endif

    /*   
    xReturned = xTaskCreate(lorawan_state_machine_task, "LORAWANSTATEMACHINE", 256, NULL, 2, &m_lorawan_state_machine_thread);
    if (xReturned != pdPASS)
    {
        NRF_LOG_ERROR("SoftDevice task not created.");
        APP_ERROR_HANDLER(NRF_ERROR_NO_MEM);
    }
   
/*
    xReturned = xTaskCreate(spi_test_task, "SPITESTTASK", 256, NULL, 3, &m_spi_test_thread);
    if (xReturned != pdPASS)
    {
        NRF_LOG_ERROR("SoftDevice task not created.");
        APP_ERROR_HANDLER(NRF_ERROR_NO_MEM);
    }
*/
    //NRF_LOG_INFO("Started.");
    // Start FreeRTOS scheduler.
    vTaskStartScheduler();
    while (1)
    {
    }
    for (;;)
    {
        APP_ERROR_HANDLER(NRF_ERROR_FORBIDDEN);
    }

    //   advertising_start(erase_bonds);
}

/**
 * @}
 */