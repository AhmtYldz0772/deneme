
#include "UART_Service.h"
#include "ble_srv_common.h"
#include "nrf_gpio.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "flash.h"
#include "task.h"

#include <stdint.h>
#include <string.h>


extern TaskHandle_t xBLEData;      /* Deifinition of BLE  Data thread. */
#define MAX_UART_LEN      (NRF_SDH_BLE_GATT_MAX_MTU_SIZE) /**< Maximum size of a transmitted Heart Rate Measurement. */

uint8_t RxDataBuffer[MAX_UART_LEN] = {0};
uint8_t maxUartServiceDataLength = MAX_UART_LEN;
/**@brief Function for checking rx data.
 *
 * If rx data is equal to 0x64, send eeprom data over bluetooth.
 *
 * @param[in]   p_ble_evt   Bluetooth stack event.
 * @param[in]   p_context   Unused.
 */
void CheckReceiveData(ble_evt_t const *p_ble_evt, void *p_context) {  //Handling the data received over BLE

  ble_gatts_value_t rx_data; 
  rx_data.len = 140;
  rx_data.offset = 0;
  rx_data.p_value = (uint8_t *)&RxDataBuffer;

  /* copy comming data to RxDataBuffer */
  if (p_ble_evt->evt.gatts_evt.params.write.handle == ((ble_uarts_t *)p_context)->rx_char_handles.value_handle) {                     
    sd_ble_gatts_value_get(((ble_uarts_t *)p_context)->conn_handle, ((ble_uarts_t *)p_context)->rx_char_handles.value_handle, &rx_data);   
   }   
      printf("Rx data = %d\n",RxDataBuffer[0]);
}

// Declaration of a function that will take care of some housekeeping of ble connections related to uart service and characteristic.
void ble_uart_service_on_ble_evt(ble_evt_t const *p_ble_evt, void *p_context) {
  ble_uarts_t *p_uart_service = (ble_uarts_t *)p_context;
  //Implement switch case handling BLE events related to uart service.
  switch (p_ble_evt->header.evt_id) {
  case BLE_GAP_EVT_CONNECTED:
    xTaskResumeFromISR(xBLEData); /* Resume BLE Data task when bluetooth device connected */
    p_uart_service->conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
    break;
  case BLE_GAP_EVT_DISCONNECTED:
    vTaskSuspend(xBLEData); /* Suspend BLE Data task when bluetooth device disconnected */
    p_uart_service->conn_handle = BLE_CONN_HANDLE_INVALID;
    break;
  case BLE_GATTS_EVT_WRITE:
    CheckReceiveData(p_ble_evt, p_context); /* BLE comming data interrupt handler */
    break;
  default:
    // No implementation needed.
    break;
  }
}

/**@brief Function for adding uart new characterstic to "uart service". 
 *
 * @param[in]   p_uart_service        UART Service structure.
 *
 */
static uint32_t uart_rx_char_add(ble_uarts_t *p_uart_service) {
  // Add a custom characteristic UUID
  uint32_t err_code; // Variable to hold return codes from library and softdevice functions
  ble_uuid_t char_uuid;
  ble_uuid128_t base_uuid = BLE_UUID_UART_BASE_UUID;
  char_uuid.uuid = BLE_UUID_UART_RX_CHARACTERISTIC_UUID;
  err_code = sd_ble_uuid_vs_add(&base_uuid, &char_uuid.type);
  APP_ERROR_CHECK(err_code);

  // Add read/write properties to uart characteristic
  ble_gatts_char_md_t char_md;
  memset(&char_md, 0, sizeof(char_md));
  char_md.char_props.read = 0;
  char_md.char_props.write = 1;

  // Configuring Client Characteristic Configuration Descriptor metadata and add to char_md structure
  ble_gatts_attr_md_t cccd_md;
  memset(&cccd_md, 0, sizeof(cccd_md));
  BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
  BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);
  cccd_md.vloc = BLE_GATTS_VLOC_STACK;
  char_md.p_cccd_md = &cccd_md;
  char_md.char_props.notify = 1;

  // Configure the attribute metadata
  ble_gatts_attr_md_t attr_md;
  memset(&attr_md, 0, sizeof(attr_md));
  attr_md.vloc = BLE_GATTS_VLOC_STACK;
  attr_md.vlen = true;
  // Set read/write security levels to uart characteristic
  BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
  BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);

  // Configure the characteristic value attribute
  ble_gatts_attr_t attr_char_value;
  memset(&attr_char_value, 0, sizeof(attr_char_value));
  attr_char_value.p_uuid = &char_uuid;
  attr_char_value.p_attr_md = &attr_md;
  
  // Set characteristic length in number of bytes
  attr_char_value.max_len = MAX_UART_LEN;
  attr_char_value.init_len = MAX_UART_LEN;
  //attr_char_value.p_attr_md->vlen = true;
  uint8_t value[MAX_UART_LEN] = {0};
  attr_char_value.p_value = value;

  // Add uart new characteristic to the service
  err_code = sd_ble_gatts_characteristic_add(p_uart_service->service_handle,
      &char_md,
      &attr_char_value,
      &p_uart_service->rx_char_handles);

  APP_ERROR_CHECK(err_code);

  return NRF_SUCCESS;
}

static uint32_t uart_tx_char_add(ble_uarts_t *p_uart_service) {
  // Add a custom characteristic UUID
  uint32_t err_code; // Variable to hold return codes from library and softdevice functions
  ble_uuid_t char_uuid;
  ble_uuid128_t base_uuid = BLE_UUID_UART_BASE_UUID;
  char_uuid.uuid = BLE_UUID_UART_TX_CHARACTERISTIC_UUID;
  err_code = sd_ble_uuid_vs_add(&base_uuid, &char_uuid.type);
  APP_ERROR_CHECK(err_code);

  // Add read/write properties to uart characteristic
  ble_gatts_char_md_t char_md;
  memset(&char_md, 0, sizeof(char_md));
  char_md.char_props.read = 1;
  char_md.char_props.write = 0;

  // Configuring Client Characteristic Configuration Descriptor metadata and add to char_md structure
  ble_gatts_attr_md_t cccd_md;
  memset(&cccd_md, 0, sizeof(cccd_md));
  BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
  BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);
  cccd_md.vloc = BLE_GATTS_VLOC_STACK;
  char_md.p_cccd_md = &cccd_md;
  char_md.char_props.notify = 1;

  // Configure the attribute metadata
  ble_gatts_attr_md_t attr_md;
  memset(&attr_md, 0, sizeof(attr_md));
  attr_md.vloc = BLE_GATTS_VLOC_STACK;
  attr_md.vlen = true;
  // Set read/write security levels to uart characteristic
  BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
  BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);

  // Configure the characteristic value attribute
  ble_gatts_attr_t attr_char_value;
  memset(&attr_char_value, 0, sizeof(attr_char_value));
  attr_char_value.p_uuid = &char_uuid;
  attr_char_value.p_attr_md = &attr_md;

  // Set characteristic length in number of bytes
  attr_char_value.max_len = maxUartServiceDataLength;
  attr_char_value.init_len = maxUartServiceDataLength;
  //attr_char_value.p_attr_md->vlen=true;
  uint8_t value[MAX_UART_LEN] = {0};
  attr_char_value.p_value = value;

  // Add uart new characteristic to the service
  err_code = sd_ble_gatts_characteristic_add(p_uart_service->service_handle,
      &char_md,
      &attr_char_value,
      &p_uart_service->tx_char_handles);

  APP_ERROR_CHECK(err_code);

  return NRF_SUCCESS;
}

/**@brief Function for initiating uart new service.
 *
 * @param[in]   p_uart_service        UART Service structure.
 *
 */
void uart_service_init(ble_uarts_t *p_uart_service) {

  uint32_t err_code; // Variable to hold return codes from library and softdevice functions
  ble_uuid_t service_uuid;
  ble_uuid128_t base_uuid = BLE_UUID_UART_BASE_UUID;
  service_uuid.uuid = BLE_UUID_UART_SERVICE_UUID;
  p_uart_service->max_uart_len = MAX_UART_LEN;
  err_code = sd_ble_uuid_vs_add(&base_uuid, &service_uuid.type);
  APP_ERROR_CHECK(err_code);

  // Set uart service connection handle to default value. I.e. an invalid handle since we are not yet in a connection.
  p_uart_service->conn_handle = BLE_CONN_HANDLE_INVALID;

  // Add uart service
  err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY,
      &service_uuid,
      &p_uart_service->service_handle);

  APP_ERROR_CHECK(err_code);

  uart_rx_char_add(p_uart_service);
  uart_tx_char_add(p_uart_service);
}

void ble_uarts_on_gatt_evt(ble_uarts_t * p_uarts, nrf_ble_gatt_evt_t const * p_gatt_evt)
{
    if (    (p_uarts->conn_handle == p_gatt_evt->conn_handle)
        &&  (p_gatt_evt->evt_id == NRF_BLE_GATT_EVT_ATT_MTU_UPDATED))
    {
        p_uarts->max_uart_len = p_gatt_evt->params.att_mtu_effective;
        maxUartServiceDataLength =  p_uarts->max_uart_len;
    }
}

void uart_tx_characteristic_update(ble_uarts_t *p_uart_service, uint8_t *uart_tx_value) {
  if (p_uart_service->conn_handle != BLE_CONN_HANDLE_INVALID) {
    ble_gatts_hvx_params_t hvx_params;
    uint16_t len = maxUartServiceDataLength;
    memset(&hvx_params, 0, sizeof(hvx_params));
    //printf("print length %ld and address %X\n", len, &len);
    hvx_params.handle = p_uart_service->tx_char_handles.value_handle;
    hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
    hvx_params.offset = 0;
    hvx_params.p_len  = &len;
    hvx_params.p_data = uart_tx_value;
sd_ble_gatts_hvx(p_uart_service->conn_handle, &hvx_params);
  }
}