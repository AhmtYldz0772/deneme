
#ifndef UART_SERVICE_H__
#define UART_SERVICE_H__

#include <stdint.h>
#include "ble.h"
#include "ble_srv_common.h"
#include "nrf_ble_gatt.h"
// Defining 16-bit service and 128-bit base UUIDs
#define BLE_UUID_UART_BASE_UUID              {{0xFB, 0x34, 0x9B, 0x5F, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0x89, 0x67, 0x00, 0x00}} // 128-bit base UUID
#define BLE_UUID_UART_SERVICE_UUID                0xEF00 // Just a random, but recognizable value

// Defining 16-bit characteristic UUID
#define BLE_UUID_UART_RX_CHARACTERISTIC_UUID          0xBE01 // Just a random, but recognizable value
#define BLE_UUID_UART_TX_CHARACTERISTIC_UUID          0xBE02 // Just a random, but recognizable value

#define MAXBLUETOOTHTRANSMITLENGHT  20
#define MAX_UART_LEN      (NRF_SDH_BLE_GATT_MAX_MTU_SIZE) /**< Maximum size of a transmitted Heart Rate Measurement. */

       
// This structure contains various status information for uart service. 
// The name is based on the naming convention used in Nordics SDKs. 
// 'ble’ indicates that it is a Bluetooth Low Energy relevant structure and 
// ‘uarts’ is short for UART Service). 
typedef struct
{
    uint16_t                    conn_handle;    /**< Handle of the current connection (as provided by the BLE stack, is BLE_CONN_HANDLE_INVALID if not in a connection).*/
    uint16_t                    service_handle; /**< Handle of UART Service (as provided by the BLE stack). */
    // Add handles for the characteristic attributes to uart struct
    ble_gatts_char_handles_t    rx_char_handles;
    ble_gatts_char_handles_t    tx_char_handles;
    uint8_t                     max_uart_len;                                          /**< Current maximum UART Service data length, adjusted according to the current ATT MTU. */
}ble_uarts_t;

/**@brief Function for handling BLE Stack events related to uart service and characteristic.
 *
 * @details Handles all events from the BLE stack of interest to UART Service.
 *
 * @param[in]   p_uart_service       UART Service structure.
 * @param[in]   p_ble_evt  Event received from the BLE stack.
 */
void ble_uart_service_on_ble_evt(ble_evt_t const * p_ble_evt, void * p_context);

/**@brief Function for initializing uart new service.
 *
 * @param[in]   p_uart_service       Pointer to UART Service structure.
 */
void uart_service_init(ble_uarts_t * p_uart_service);

/**@brief Function for updating and sending new characteristic values
 *
 * @details The application calls this function whenever uart timer_timeout_handler triggers
 *
 * @param[in]   p_uart_service           UART Service structure.
 * @param[in]   characteristic_value     New characteristic value.
 */
void uart_tx_characteristic_update(ble_uarts_t *p_uart_service, uint8_t *uart_tx_value);
void ble_uarts_on_gatt_evt(ble_uarts_t * p_uarts, nrf_ble_gatt_evt_t const * p_gatt_evt);
void prvBLEData();

extern uint8_t RxDataBuffer[MAX_UART_LEN];
extern ble_uarts_t m_uart_service;   

#endif  /* _ UART_SERVICE_H__ */
