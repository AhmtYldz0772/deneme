#ifndef RTC_RT_H
#define RTC_RT_H

#include "nrf_drv_rtc.h"
#include "app_error.h"
#include "nrf_drv_clock.h"
#include "stdio.h"
#include "nrf_rtc.h"

#define RTC_INSTANCE    2
#define RTC_TICK_FREQ   32768

struct TimePacket {
    uint8_t hours;
    uint8_t minutes;
    uint8_t seconds;
};

extern uint32_t curr_time_in_seconds;
extern uint32_t rtc_last_counter;


extern char log_time(void);
extern void update_rtc(struct TimePacket time_packet);
extern uint32_t update_rtc_from_external_data(uint32_t received_hours, uint32_t received_minutes, uint32_t received_seconds);


#define GET_TIME_IN_SECONDS curr_time_in_seconds

extern uint8_t system_time_ready;

extern void update_system_time();

#endif // RTC_RT_H
