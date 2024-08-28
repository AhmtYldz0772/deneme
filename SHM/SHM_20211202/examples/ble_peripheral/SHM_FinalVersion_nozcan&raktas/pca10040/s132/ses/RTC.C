#include "nrf_drv_rtc.h"
#include "app_error.h"
#include "rtc-board.h"
#include "nrf_drv_clock.h"
#include "stdio.h"
#include "nrf_rtc.h"
#include "nrfx_rtc.h"
#include "RTC_RT.h"
#include "device_managment.h"

uint32_t rtc_last_counter = 0;
uint32_t curr_system_time_rtc_counter = 0;
uint32_t curr_time_in_seconds = 0;
uint8_t  system_time_ready = 0;


uint32_t lora_tick;
//uint32_t update_rtc(void);
//uint32_t update_rtc_from_external_data(uint32_t received_hours, uint32_t received_minutes, uint32_t received_seconds);

uint32_t update_rtc_from_external_data(uint32_t received_hours, uint32_t received_minutes, uint32_t received_seconds) {
    uint32_t APPROXIMATED_TRAVEL_TIME_IN_SECONDS = 4; 
    //printf("\nCurrent time: %02d:%02d:%02d\n", received_hours, received_minutes, received_seconds);
    printf("\nRTC is running");
    nrf_drv_rtc_t rtc = NRF_DRV_RTC_INSTANCE(2);
    //Reset Tick count
    nrf_drv_rtc_counter_clear(&rtc);
    printf("\nRTC syncing...");
    struct TimePacket new_time;
    new_time.hours = received_hours; 
    new_time.minutes = received_minutes; 
    new_time.seconds = received_seconds; 

    uint32_t lora_hour = new_time.hours;
    uint32_t lora_minutes = new_time.minutes;
    uint32_t lora_seconds = new_time.seconds;
    uint32_t Ltime = (lora_hour * 3600) + (lora_minutes * 60) + (lora_seconds) + APPROXIMATED_TRAVEL_TIME_IN_SECONDS;
    // mpdelete shm_printf("\n **********\n\nReceived Time Message:  time seconds: %d \n****\n", Ltime);
    lora_tick = Ltime*2048;

    rtc_last_counter = nrf_drv_rtc_counter_get(&rtc);
    curr_system_time_rtc_counter = lora_tick;

    system_time_ready = true;
    
    return  lora_tick;
      
     //printf("\lora_tick time:%02d\n", lora_tick);

}

int my_period = 0;
uint32_t mx_rtc_counter = 0;
uint32_t log_rtc_counter = 0;
uint32_t log_rtc_last_counter = 0;

void update_system_time()
{
   
   //nrf_rtc_task_trigger(NRF_RTC2, NRF_RTC_TASK_START);
   nrf_drv_rtc_t rtc = NRF_DRV_RTC_INSTANCE(RTC_INSTANCE);

   uint32_t rtc_counter = nrf_drv_rtc_counter_get(&rtc);

   if(rtc_counter > mx_rtc_counter)
    mx_rtc_counter = rtc_counter;

//0xFFFFFFFF
  if(rtc_last_counter <= rtc_counter)
  {
    curr_system_time_rtc_counter+= (rtc_counter - rtc_last_counter);

  }else{
    log_rtc_counter = rtc_counter;
    log_rtc_last_counter = rtc_last_counter;
    curr_system_time_rtc_counter+= (RTC_COUNTER_COUNTER_Msk - rtc_last_counter + rtc_counter); //0x00250FFF
  }
   
   rtc_last_counter = rtc_counter;
   uint32_t time_to_be_converted = curr_system_time_rtc_counter;

    time_to_be_converted = (time_to_be_converted)/2048;
    curr_time_in_seconds = time_to_be_converted;
    uint32_t hours = ((time_to_be_converted) / 3600) % 24;
    uint32_t minutes = (((time_to_be_converted) % 3600) / 60) ;
    uint32_t seconds = (time_to_be_converted) % 60;
    my_period++;
    if((my_period % 125) == 0){
      my_period = 0;
     // mpdelete shm_printf("\nMP Current time: %02d:%02d:%02d uintvalue %d time in sec %d\n", hours, minutes, seconds,curr_system_time_rtc_counter,time_to_be_converted);
    }

}


char log_time(void) {
    nrf_rtc_task_trigger(NRF_RTC2, NRF_RTC_TASK_START);
    nrf_drv_rtc_t rtc = NRF_DRV_RTC_INSTANCE(RTC_INSTANCE);

    uint32_t rtc_counter = nrf_drv_rtc_counter_get(&rtc);

     
    //uint32_t lora_tick = update_rtc_from_external_data();
    //printf("\lora_tick time:%02d\n", lora_tick);
    rtc_counter += lora_tick;
    rtc_counter = (rtc_counter)/2048;
    uint32_t hours = ((rtc_counter) / 3600) % 24;
    uint32_t minutes = (((rtc_counter) % 3600) / 60) ;
    uint32_t seconds = (rtc_counter) % 60;

     //printf("\nCurrent time: %02d:%02d:%02d\n", hours, minutes, seconds);
}


