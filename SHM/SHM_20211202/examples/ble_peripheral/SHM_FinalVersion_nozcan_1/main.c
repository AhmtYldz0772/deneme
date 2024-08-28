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
#include <stdio.h>
#include <stdlib.h>

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
#include "spi-board.h"


#include "nrf52.h"
#include "nrf_drv_clock.h"
#include "nrf_drv_gpiote.h"
#include "nrf_drv_timer.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "peer_manager_handler.h"

/* ------------------------------------------- Status Module Includes --------------------------------------*/
#include "status.h"
/* ---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------gpio-board Includes-----------------------------------------*/
#include "gpio-board.h"
/*-------------------------------------------------------------------------------------------------------*/
//-------------------------------------------------------------------------------------------------------//

/*---------------------------------------------FreeRTOS Includes-----------------------------------------*/
#include "FreeRTOS.h"
#include "SEGGER_RTT.h"
#include "fds.h"
#include "semphr.h"
#include "task.h"
#include "timers.h"
#include "queue.h"
/*-------------------------------------------------------------------------------------------------------*/

//-------------------------------------------- LoRaWAN Includes ------------------------------------------//
#include "Commissioning.h"
#include "Mac/LoRaMac.h"
#include "System/gpio.h"
#include "System/spi.h"
#include "board-config.h"
#include "board.h"
#include "utilities.h"
//-------------------------------------------------------------------------------------------------------//

//-------------------------------------------- BLE Service Includes -------------------------------------//

#include "BLE_Services/atCommandService.h"
#include "BLE_Services/ledService.h"
#include "BLE_Services/UART_Service.h"
#include "BLE_Services/messageCodes.h"

//-------------------------------------------------------------------------------------------------------//

//-------------------------------------------- ADXL355 Includes -------------------------------------//

#include "ADXL355.h"
#include "Communication.h"

//-------------------------------------------------------------------------------------------------------//

//-------------------------------------------- Initialize Includes -------------------------------------//

#include "Initialize.h"

//-------------------------------------------------------------------------------------------------------//

//-------------------------------------------- eeprom Includes -------------------------------------//

#include "eeprom.h"

//-------------------------------------------------------------------------------------------------------//

//-------------------------------------------- NRF-WDT Includes -------------------------------------//

#include "nrf_drv_wdt.h"

//-------------------------------------------------------------------------------------------------------//


//-------------------------------------------- CMSIS Includes -------------------------------------//

#include "arm_math.h"

//-------------------------------------------------------------------------------------------------------//

//-------------------------------------------- Flash Includes -------------------------------------//

#include "flash.h"

//-------------------------------------------------------------------------------------------------------//

#include "nrf_drv_rtc.h"  //nozcan
#include "shmSignal.h" //raktas gerek kalmayabilir
#include "nrfRam.h" //raktas gerek kalmayabilir
#include "loraRam.h" //raktas gerek kalmayabilir
 
uint8_t DEVICE_NAME[20] = {'S','H','M','-'}; /**< Name of device. Will be included in the advertising data. */
#define MANUFACTURER_NAME "NETAS"  /**< Manufacturer. Will be passed to Device Information Service. */
#define APP_ADV_INTERVAL 300       /**< The advertising interval (in units of 0.625 ms. This value corresponds to 187.5 ms). */

#define APP_ADV_DURATION 0  /**< The advertising duration (180 seconds) in units of 10 milliseconds. */
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


/*---------------------------------------------BLE Definitions----------------------------------------*/

NRF_BLE_GATT_DEF(m_gatt);           /**< GATT module instance. */
NRF_BLE_QWR_DEF(m_qwr);             /**< Context for the Queued Write module.*/
BLE_ADVERTISING_DEF(m_advertising); /**< Advertising module instance. */

#define MAX_UART_LEN      (NRF_SDH_BLE_GATT_MAX_MTU_SIZE) /**< Maximum size of a transmitted Heart Rate Measurement. */


bool erase_bonds;

extern  uint8_t bluetoothBuffer[MAX_UART_LEN];	
extern uint16_t bluetoothCounter; 							
/*-------------------------------------------------------------------------------------------------------*/

/*---------------------------------------------FreeRTOS Definitions--------------------------------------*/
#if NRF_LOG_ENABLED
static TaskHandle_t m_logger_thread; /**< Definition of Logger thread. */
#endif

TaskHandle_t xBLEData;      /* Deifinition of BLE  Data thread. */
/*-------------------------------------------------------------------------------------------------------*/



//---------------------------------------------- LoRaWAN Definitions ------------------------------------//


//#define ACTIVE_REGION LORAMAC_REGION_EU868
//
//#ifndef ACTIVE_REGION
//
//#warning "No active region defined, LORAMAC_REGION_EU868 will be used as default."
//
//#define ACTIVE_REGION LORAMAC_REGION_EU868
//
//#endif

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
//#define LORAWAN_ADR_ON 1

//#if defined(REGION_EU868)
//
//#include "Mac/LoRaMacTest.h"
//
///*!
// * LoRaWAN ETSI duty cycle control enable/disable
// *
// * \remark Please note that ETSI mandates duty cycled transmissions. Use only for test purposes
// */
//#define LORAWAN_DUTYCYCLE_ON false
//
//#endif

//-------------------------------------------------------------------------------------------------------//

//---------------------------------------------- LoRaWAN Variables --------------------------------------//

/*!
 * LoRaWAN application port
 */
#define LORAWAN_APP_PORT 2


//raktas incele
//volatile bool isBLEConnected = false;

//static uint8_t AppEui[] = LORAWAN_APPLICATION_EUI;
//static uint8_t AppKey[] = LORAWAN_APPLICATION_KEY;

#if (OVER_THE_AIR_ACTIVATION == 0)

static uint8_t NwkSKey[] = LORAWAN_NWKSKEY;
static uint8_t AppSKey[] = LORAWAN_APPSKEY;

/*!
 * Device address
 */
static uint32_t DevAddr = LORAWAN_DEVICE_ADDRESS;

#endif

#define LORAWAN_APP_DATA_MAX_SIZE 242

/*!
 * Defines the application data transmission duty cycle
 */
static uint32_t TxDutyCycleTime;

/*!
 * Specifies the state of the application LED
 */
static bool AppLedStateOn = false;


/*!
 * Timer to handle the state of LoRaSendPacket
 */
static TimerEvent_t LoRaSendPacketTimer;



//static bool isConfirmed = false;





/*!
 * LED GPIO pins objects
 */
extern Gpio_t Led1;
extern Gpio_t Led2;

//---------------------------------------------- ADXL355 --------------------------------------//
uint8_t adxl355Data = 0U;
float32_t adxl355Scale = 0.0f;

//extern uint8_t fifoBuffer[FIFOBUFFERSIZE];
//extern float32_t XFifoBuffer[fifoFFTBufferSize];
//extern float32_t YFifoBuffer[fifoFFTBufferSize];

extern uint8_t SPIRxBuf[SPIRXSIZE];  /* RX buffer. */

#define RANGE 0x2C
#define ADXLRANGE_2 0x81
#define ADXLRANGE_4 0x82
#define ADXLRANGE_8 0x83

//-------------------------------------------------------------------------------------------------------//

//---------------------------------------------- Timer Parameters --------------------------------------//
const nrf_drv_timer_t adxl355AutomaticTimer = NRF_DRV_TIMER_INSTANCE(0);
uint32_t time_ticks = 0U;
uint32_t time_ms = 3540000; //Time(in miliseconds) between consecutive compare events.
//-------------------------------------------------------------------------------------------------------//

//---------------------------------------------- FFT Parameters --------------------------------------//
//arm_rfft_fast_instance_f32 fft_instance;
//-------------------------------------------------------------------------------------------------------//

//---------------------------------------------- Plot Parameters --------------------------------------//

//#define UPBUFFERSIZE  1024
//char X_Y_RTT_UpBuffer[UPBUFFERSIZE]; // J-Scope RTT Buffer
//int X_Y_RTT_Channel = 1;     // J-Scope RTT Channel



//-------------------------------------------------------------------------------------------------------//


//---------------------------------- BLE Buffers --------------------------------------------------------//

extern uint8_t RxDataBuffer[MAX_UART_LEN];

//-------------------------------------------------------------------------------------------------------//

//raktas incele
//uint32_t initValue;
//-------------------------------------------------------------------------------------------------------//
/**@brief A function resume task.
 */
void TaskSuspend(TaskHandle_t taskHandle) {
  vTaskSuspend(taskHandle);
}

/**@brief A function resume task.
 */
void TaskResume(TaskHandle_t taskHandle) {
  vTaskResume(taskHandle);
}

/**@brief A function deveui init.
 */
void devEUIInit(void){
    uint32_t did[2];

    did[0] = NRF_FICR->DEVICEID[0]; /* uniq id */
    did[1] = NRF_FICR->DEVICEID[1]; /* uniq id */
    memcpy(&DevEui[0], did, 8);

    sprintf(&DEVICE_NAME[4], "%02X%02X%02X%02X%02X%02X%02X%02X", DevEui[0],DevEui[1],DevEui[2],DevEui[3]
    ,DevEui[4],DevEui[5],DevEui[6],DevEui[7]); 
}

//---------------------------------------------- LoRaWAN Functions --------------------------------------//



///*!
// * \brief  MLME-Indication event function
// *
// * \param  [IN] mlmeIndication - Pointer to the indication structure.
// */
//static void MlmeIndication(MlmeIndication_t *mlmeIndication) {
//  switch (mlmeIndication->MlmeIndication) {
//  case MLME_SCHEDULE_UPLINK: { // The MAC signals that we shall provide an uplink as soon as possible
//    SendFrame();
//    break;
//  }
//  default:
//    break;
//  }
//}

//--------------------------------------------- BLE Definitons ----------------------------------------------------------//

static uint16_t m_conn_handle = BLE_CONN_HANDLE_INVALID; /**< Handle of the current connection. */

static ble_uuid_t m_adv_uuids[] = {{0x9292, BLE_UUID_TYPE_BLE}}; /**< Universally unique service identifiers. */

ble_uarts_t m_uart_service;
NRF_SDH_BLE_OBSERVER(m_uart_service_observer, APP_BLE_OBSERVER_PRIO, ble_uart_service_on_ble_evt, (void *)&m_uart_service); 
                                                      

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
void assert_nrf_callback(uint16_t line_num, const uint8_t *p_file_name) {
  app_error_handler(DEAD_BEEF, line_num, p_file_name);
}

/**@brief Function for handling Peer Manager events.
 *
 * @param[in] p_evt  Peer Manager event.
 */
static void pm_evt_handler(pm_evt_t const *p_evt) {
  bool delete_bonds = false;

  pm_handler_on_pm_evt(p_evt);
  pm_handler_flash_clean(p_evt);

  switch (p_evt->evt_id) {
  case PM_EVT_PEERS_DELETE_SUCCEEDED:
    advertising_start(&delete_bonds);
   
    break;

  default:
    break;
  }
}


/**
 * @brief WDT events handler.
 */
void wdt_event_handler(void)
{   
    printf("systemReset\n");
    NVIC_SystemReset();
}


void initWDT (void)
{    //Configure WDT.
    nrf_drv_wdt_config_t config = NRF_DRV_WDT_DEAFULT_CONFIG;
    APP_ERROR_CHECK(nrf_drv_wdt_init(&config, wdt_event_handler));

    APP_ERROR_CHECK(nrf_drv_wdt_channel_alloc(&m_channel_id));

    nrf_drv_wdt_enable();
}



/**@brief Function for the GAP initialization.
 *
 * @details This function sets up all the necessary GAP (Generic Access Profile) parameters of the
 *          device including the device name, appearance, and the preferred connection parameters.
 */
static void gap_params_init(void) {
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


/**@brief GATT module event handler.
 */
static void gatt_evt_handler(nrf_ble_gatt_t * p_gatt, nrf_ble_gatt_evt_t const * p_evt)
{
    if (p_evt->evt_id == NRF_BLE_GATT_EVT_ATT_MTU_UPDATED)
    {
        printf("GATT ATT MTU on connection 0x%x changed to %d.\n",
                     p_evt->conn_handle,
                     p_evt->params.att_mtu_effective);
    }

    ble_uarts_on_gatt_evt(&m_uart_service, p_evt);
}

/**@brief Function for initializing the GATT module.
 */
static void gatt_init(void) {
  ret_code_t err_code = nrf_ble_gatt_init(&m_gatt, gatt_evt_handler);
  APP_ERROR_CHECK(err_code);
}

/**@brief Function for handling Queued Write Module errors.
 *
 * @details A pointer to this function will be passed to each service which may need to inform the
 *          application about an error.
 *
 * @param[in]   nrf_error   Error code containing information about what went wrong.
 */
static void nrf_qwr_error_handler(uint32_t nrf_error) {
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
static void services_init(void) {
  ret_code_t err_code;
  nrf_ble_qwr_init_t qwr_init = {0};

  // Initialize Queued Write Module.
  qwr_init.error_handler = nrf_qwr_error_handler;

  err_code = nrf_ble_qwr_init(&m_qwr, &qwr_init);
  APP_ERROR_CHECK(err_code);

    uart_service_init(&m_uart_service);
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
static void on_conn_params_evt(ble_conn_params_evt_t *p_evt) {
  ret_code_t err_code;

  if (p_evt->evt_type == BLE_CONN_PARAMS_EVT_FAILED) {
    err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_CONN_INTERVAL_UNACCEPTABLE);
    APP_ERROR_CHECK(err_code);
  }
}

/**@brief Function for handling a Connection Parameters error.
 *
 * @param[in] nrf_error  Error code containing information about what went wrong.
 */
static void conn_params_error_handler(uint32_t nrf_error) {
  APP_ERROR_HANDLER(nrf_error);
}

/**@brief Function for initializing the Connection Parameters module.
 */
static void conn_params_init(void) {
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
static void application_timers_start(void) {
}

/**@brief Function for putting the chip into sleep mode.
 *
 * @note This function will not return.
 */
static void sleep_mode_enter(void) {
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
static void on_adv_evt(ble_adv_evt_t ble_adv_evt) {

  switch (ble_adv_evt) {
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




/**@brief Function for the Peer Manager initialization.
 */
static void peer_manager_init(void) {
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
static void delete_bonds(void) {
  ret_code_t err_code;

  NRF_LOG_INFO("Erase bonds!");

  err_code = pm_peers_delete();
  APP_ERROR_CHECK(err_code);
}



/**@brief Function for handling BLE events.
 *
 * @param[in]   p_ble_evt   Bluetooth stack event.
 * @param[in]   p_context   Unused.
 */
static void ble_evt_handler(ble_evt_t const *p_ble_evt, void *p_context) {
  ret_code_t err_code = NRF_SUCCESS;
  bool delete_bonds = true;

  switch (p_ble_evt->header.evt_id) {
    case BLE_GAP_EVT_DISCONNECTED:
    NRF_LOG_INFO("Disconnected.");
    // LED indication will 7be changed when advertising starts.   
    ble_advertising_start(&m_advertising, BLE_ADV_MODE_FAST);   
    break;

  case BLE_GAP_EVT_CONNECTED:
    NRF_LOG_INFO("Connected.");
    //err_code = bsp_indication_set(BSP_INDICATE_CONNECTED);
    //APP_ERROR_CHECK(err_code);
    m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
    err_code = nrf_ble_qwr_conn_handle_assign(&m_qwr, m_conn_handle);
    APP_ERROR_CHECK(err_code);

    break;

  case BLE_GAP_EVT_PHY_UPDATE_REQUEST: {
    NRF_LOG_DEBUG("PHY update request.");
    ble_gap_phys_t const phys =
        {
            .rx_phys = BLE_GAP_PHY_AUTO,
            .tx_phys = BLE_GAP_PHY_AUTO,
        };
    err_code = sd_ble_gap_phy_update(p_ble_evt->evt.gap_evt.conn_handle, &phys);
    APP_ERROR_CHECK(err_code);
  } break;

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
static void ble_stack_init(void) {
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



/**@brief Function for initializing the Advertising functionality.
 */
static void advertising_init(void) {
  ret_code_t err_code;
  ble_advertising_init_t init;

  memset(&init, 0, sizeof(init));

  init.advdata.name_type = BLE_ADVDATA_SHORT_NAME;
  init.advdata.short_name_len = 22;
  init.advdata.include_appearance = false;
  init.advdata.flags = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;
  init.advdata.uuids_complete.uuid_cnt = sizeof(m_adv_uuids) / sizeof(m_adv_uuids[0]);
  init.advdata.uuids_complete.p_uuids = m_adv_uuids;

  init.config.ble_adv_fast_enabled = true;
  init.config.ble_adv_fast_interval = APP_ADV_INTERVAL;
  init.config.ble_adv_fast_timeout = 0;

  init.evt_handler = on_adv_evt;

  err_code = ble_advertising_init(&m_advertising, &init);
  APP_ERROR_CHECK(err_code);

  ble_advertising_conn_cfg_tag_set(&m_advertising, APP_BLE_CONN_CFG_TAG);
}

/**@brief Function for initializing power management.
 */
static void power_management_init(void) {
  ret_code_t err_code;
  err_code = nrf_pwr_mgmt_init();
  APP_ERROR_CHECK(err_code);
}

/**@brief Function for handling the idle state (main loop).
 *
 * @details If there is no pending log operation, then sleep until next the next event occurs.
 */
static void idle_state_handle(void) {
  if (NRF_LOG_PROCESS() == false) {
    nrf_pwr_mgmt_run();
  }
}

/**@brief Function for starting advertising.
 */
static void advertising_start(void *p_erase_bonds) {
  bool erase_bonds = *(bool *)p_erase_bonds;

  if (erase_bonds) {
    delete_bonds();
    // Advertising is started by PM_EVT_PEERS_DELETE_SUCCEEDED event.
  } else {
    ret_code_t err_code = ble_advertising_start(&m_advertising, BLE_ADV_MODE_FAST);
    APP_ERROR_CHECK(err_code);
    
  }
}


/**@brief A function which is hooked to idle task.
 * @note Idle hook must be enabled in FreeRTOS configuration (configUSE_IDLE_HOOK).
 */
void vApplicationIdleHook(void) {
#if NRF_LOG_ENABLED
  vTaskResume(m_logger_thread);
#endif
}



/**@brief Function BLE data task function.
   @param[in] *arg
 */
//static void prvBLEData(void * arg)
//{
//    UNUSED_PARAMETER(arg);
//
//
//    while (1)
//    {
//        nrf_drv_wdt_channel_feed(m_channel_id);
//        ret_code_t err_code;
//
//        if (RxDataBuffer[0] == SET_XAXISTHRESHOLD_VALUE_MESSAGE)
//        {
//            RxDataBuffer[0] = 0xFF;
//            X_axisTresholdFlash = RxDataBuffer[1] << 24 | RxDataBuffer[2] << 16 |
//                                      RxDataBuffer[3] << 8 | RxDataBuffer[4];
//            memcpy(&X_axisTreshold, &X_axisTresholdFlash,
//                sizeof(float32_t)); /* set new treshold */
//            vFlashWritetoFlash(X_axisTresholdFlash, XAXISDATAADDRESS, XAXISDATAPAGE);
//            nrf_delay_ms(100);
//        }
//
//        if (RxDataBuffer[0] == SET_YAXISTHRESHOLD_VALUE_MESSAGE)
//        {
//            RxDataBuffer[0] = 0xFF;
//            Y_axisTresholdFlash = RxDataBuffer[1] << 24 | RxDataBuffer[2] << 16 |
//                                      RxDataBuffer[3] << 8 | RxDataBuffer[4];
//            memcpy(&Y_axisTreshold, &Y_axisTresholdFlash,
//                sizeof(float32_t)); /* set new treshold */
//  
//            vFlashWritetoFlash(Y_axisTresholdFlash, YAXISDATAADDRESS, YAXISDATAPAGE);
//            nrf_delay_ms(100);
//        }
//
//        if (RxDataBuffer[0] == READ_AXISTHRESHOLD_VALUE_MESSAGE)
//        {
//            RxDataBuffer[0] = 0xFF;
//            vFlashReadAxisFromFlash();
//        }
//
//        if (RxDataBuffer[0] == SEND_EEPROM_DATA_MESSAGE)
//        {
//            RxDataBuffer[0] = 0xFF;
////            TimerStop(&LoRaSendPacketTimer);
//            bluetoothCounter = RxDataBuffer[1] << 8 | RxDataBuffer[2];
//            prvSendEepromData();
////            TimerSetValue(&LoRaSendPacketTimer, 180000);
////            TimerStart(&LoRaSendPacketTimer);
//        }
//        if (RxDataBuffer[0] == READLORAPARAMETERS)
//        {
//            RxDataBuffer[0] = 0xFF;
//            memcpy(&bluetoothBuffer[0], DevEui, 8);
//            uart_tx_characteristic_update(&m_uart_service, &bluetoothBuffer[0]);
//        }
//
//        if (RxDataBuffer[0] == SET_FREQUENCY)
//        {
//
//            receivedTreshold = (RxDataBuffer[1] << 24) | (RxDataBuffer[2] << 16) |
//                               (RxDataBuffer[3] << 8) | RxDataBuffer[4];
//            memcpy(&frequencyFFTSearch, &receivedTreshold, sizeof(float32_t)); /* set new frequency */
//            vFlashWritetoFlash(receivedTreshold, FREQUENCYADDRESS, FREQUENCYPAGE);
//            receivedTreshold = 0;
//        }
//
//        if (RxDataBuffer[0] == READ_FREQUENCY)
//        {
//
//            memcpy(&frequencyFFTSearch, &receivedTreshold, sizeof(float32_t)); /* set new frequency */
//            receivedTreshold = vFlashReadFromFlash(FREQUENCYADDRESS);
//            memcpy(&bluetoothBuffer[0], &receivedTreshold, sizeof(uint32_t));
//            uart_tx_characteristic_update(&m_uart_service, &bluetoothBuffer[0]);
//        }
//    }
//}

//raktas&nozcan
void createQueues(){

  //accelerometer Data for 2 seconds X and Y axis. Queue size is 1500 byte.
  if(createFifoBuffQueue() == false){  
    //error log tut, ne yapilacagini dusun.
    return;
  }

  if(createTxBuffQueue() == false){
    //error log tut, ne yapilacagini dusun.
    return;
  }

}


//raktas&nozcan
void createTimerTasks(){
  createReadSensorDataTask();

}

//raktas&nozcan
void createTasks(){
  createCollectSensorDataTask();       
  //mpataci createLoraStateMachineTask();
}


int main_exec_counter = 0;

/**@brief Function for application main entry.
 */

int main(void) {

  DWT->CTRL |= (1 << 0);
  BoardInitMcu();
  BoardInitPeriph();
  
 

  // Set the external high frequency clock source to 32 MHz
 
  // Start the external high frequency crystal
  NRF_CLOCK->EVENTS_HFCLKSTARTED = 0;
  NRF_CLOCK->TASKS_HFCLKSTART = 1;

  // Wait for the external oscillator to start up
  while (NRF_CLOCK->EVENTS_HFCLKSTARTED == 0) {
  };

  

  // Initialize.
  power_management_init();
  devEUIInit();
  ble_stack_init();
  gap_params_init();
  gatt_init();
  advertising_init();
  services_init();
  conn_params_init();
  peer_manager_init(); //bss kullanimini incele
  twi_init();
  initWDT();
  
  // Start execution.
  application_timers_start();

  /* Initialize ADXL355 */
  vAdxlAccelerometerInit();
  

  
  /* Set ADXL355 Sensor Range */
#if ADXL_RANGE == 2
  SPIWrite(RANGE, ADXLRANGE_2, 0x00, SPI_WRITE_ONE_REG); /* Set sensor range within RANGE register */
  adxl355Scale = 256000.0f;
#endif

#if ADXL_RANGE == 4
  SPI_Write(RANGE, ADXLRANGE_4, 0x00, SPI_WRITE_ONE_REG); /* Set sensor range within RANGE register */
  adxl355Scale = 128000.0f;
#endif

#if ADXL_RANGE == 8
  SPI_Write(RANGE, ADXLRANGE_8, 0x00, SPI_WRITE_ONE_REG); /* Set sensor range within RANGE register */
  adxl355Scale = 64000.0f;
#endif

  // Create a FreeRTOS task for the BLE stack.
  // The task will run advertising_start() before entering its loop.
  nrf_sdh_freertos_init(advertising_start, &erase_bonds);

  SEGGER_RTT_Init(); /* RTT Init */
  NRF_LOG_INIT(NULL); /* NRF Log Init */

  NRF_LOG_DEBUG("DevEUI:\r\n");
  NRF_LOG_HEXDUMP_DEBUG(DevEui, sizeof(DevEui));

  //createMutex() fonksiyonu olusturulacak, raktas
  xNextTx = xSemaphoreCreateMutex(); //nozcan  
  xDeviceState = xSemaphoreCreateMutex(); //nozcan

  createQueues();
  createTimerTasks();
  createTasks();
  main_exec_counter = 1;
  uint32_t freeHeapSize = xPortGetFreeHeapSize();

  main_exec_counter = 2;

  //printf("Free heap size %d\n",freeHeapSize);

  vTaskStartScheduler(); /* Start FreeRTOS scheduler. */

  main_exec_counter = 2; // not expected. Code shouldnt be at this point

  for (;;) {
    APP_ERROR_HANDLER(NRF_ERROR_FORBIDDEN);
  }

}

 

/**
 * @}
 */