#include "atCommandService.h"
#include "app_error.h"
#include "ble_srv_common.h"
#include "boards.h"
#include "nrf_gpio.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include <stdint.h>
#include <string.h>

static uint32_t write_char_add(ble_atCommands_t *p_atCommand_service)
{
    // Adding characteristics UUID.
    uint32_t err_code;
    ble_uuid_t char_uuid;
    ble_uuid128_t base_uuid = BLE_UUID_ATCOMMAND_BASE_UUID;
    char_uuid.uuid = BLE_UUID_WRITE_CHARACTERISTC_UUID;
    err_code = sd_ble_uuid_vs_add(&base_uuid, &char_uuid.type);
    APP_ERROR_CHECK(err_code);

    ble_gatts_char_md_t char_md;

    memset(&char_md, 0, sizeof(char_md));
    char_md.char_props.read = 1;
    char_md.char_props.write = 1;

    // Attribute meta data.
    ble_gatts_attr_md_t attr_md;
    attr_md.vlen = 1;
    memset(&attr_md, 0, sizeof(attr_md));
    attr_md.vloc = BLE_GATTS_VLOC_STACK;
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);
    ble_gatts_attr_t attr_char_value;
    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid = &char_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.max_len = 255;
    attr_char_value.init_len = 255;

    uint8_t value[255] = {0};
    attr_char_value.p_value = value;

    err_code = sd_ble_gatts_characteristic_add(p_atCommand_service->service_handle,
        &char_md,
        &attr_char_value,
        &p_atCommand_service->char_handles);
    APP_ERROR_CHECK(err_code);
    return NRF_SUCCESS;
}

static void on_ble_write(ble_atCommands_t *p_atCommand_service, const ble_evt_t *p_ble_evt)
{

    // Declare buffer variable to hold received data. The data can only be 32 bit long.
    volatile uint32_t data_buffer;
    // Populate ble_gatts_value_t structure to hold received data and metadata.
    ble_gatts_value_t rx_data;
    rx_data.len = sizeof(uint32_t);
    rx_data.offset = 0;
    rx_data.p_value = (uint8_t *)&data_buffer;

    // Check if write event is performed on our characteristic or the CCCD
    if (p_ble_evt->evt.gatts_evt.params.write.handle == p_atCommand_service->char_handles.value_handle)
    {
        // Get data
        sd_ble_gatts_value_get(p_atCommand_service->conn_handle, p_atCommand_service->char_handles.value_handle, &rx_data);
        // Print handle and value

        if ((data_buffer & 0x3) == 1)
        {
            //		nrf_gpio_pin_write(LED_4,0);
            NRF_LOG_INFO("Value is : %#010x \r\n", (uint32_t)(data_buffer));
        }
        else
        {
            //			nrf_gpio_pin_write(LED_4,1);
        }
        NRF_LOG_INFO("Value is : %#010x \r\n", (uint32_t)(data_buffer));
        //		 printf("Value received on handle %#06x: %#010x\r\n", p_ble_evt->evt.gatts_evt.params.write.handle, data_buffer);
    }
}
void ble_atCommand_service_on_ble_evt(ble_atCommands_t *p_bas, const ble_evt_t *p_ble_evt)
{
    if (p_bas == NULL || p_ble_evt == NULL)
    {
        return;
    }

    switch (p_ble_evt->header.evt_id)
    {
    case BLE_GAP_EVT_CONNECTED:
        // on_connect(p_bas, p_ble_evt);
        break;

    case BLE_GAP_EVT_DISCONNECTED:
        //on_disconnect(p_bas, p_ble_evt);
        break;

    case BLE_GATTS_EVT_WRITE:
        on_ble_write(p_bas, p_ble_evt);
        break;

    default:
        // No implementation needed.
        break;
    }
}

void atCommand_service_init(ble_atCommands_t *p_atCommand_service)
{
    uint32_t err_code;
    ble_uuid_t service_uuid;
    ble_uuid128_t base_uuid = BLE_UUID_ATCOMMAND_BASE_UUID;
    service_uuid.uuid = BLE_UUID_ATCOMMAND_SERVICE;
    err_code = sd_ble_uuid_vs_add(&base_uuid, &service_uuid.type);
    APP_ERROR_CHECK(err_code);

    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY,
        &service_uuid,
        &p_atCommand_service->service_handle);
    APP_ERROR_CHECK(err_code);

    write_char_add(p_atCommand_service);
}