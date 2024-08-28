#ifndef LEDSERVICE_H__
#define LEDSERVICE_H__

#include <stdint.h>
#include "ble.h"
#include "ble_srv_common.h"

#define BLE_UUID_LED_BASE_UUID                   {0x23, 0xD1, 0x13, 0xEF, 0x5F, 0x78, 0x23, 0x15, 0xDE, 0xEF, 0x12, 0x12, 0x00, 0x00, 0x00, 0x00} // 128-bit base UUID.
#define BLE_UUID_LED_SERVICE                     0xAAA1		 // Just a random, but recognizable value.
#define BLE_UUID_LED_CHARACTERISTC_UUID          0x2A56    // "Digital" characteristic format.


typedef struct
{
    uint16_t                    conn_handle; 
    uint16_t                    service_handle;        
    ble_gatts_char_handles_t    char_handles;
}ble_leds_t;

/**@brief Function for initializing our new service.
 *
 * @param[in]   p_led_service       Pointer to LED Service structure.
 */
void led_service_init(ble_leds_t * p_led_service);


void ble_led_service_on_ble_evt(const ble_evt_t * p_ble_evt, void *p_context);

#endif  /* _ OUR_SERVICE_H__ */
