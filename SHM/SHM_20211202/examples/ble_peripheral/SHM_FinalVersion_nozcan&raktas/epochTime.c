#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include <stdio.h>

#include "System/timer.h"
#include "epochTime.h"
#include "time.h"


struct tm startingTimeS;
struct tm tempTimeS;


/** @brief The purpose of this function is to return nrf system type. */
uint32_t GetTime (void)
{
  time_t tempTime = TimerGetCurrentTime();
  tempTime  = tempTime/1000 + 1588243922;

  tempTimeS = *localtime(&tempTime);

  tempTimeS.tm_year = tempTimeS.tm_year + 1900;
  tempTimeS.tm_hour = tempTimeS.tm_hour + 3;
  tempTimeS.tm_mon = tempTimeS.tm_mon + 1;

  //printf("%d.%d.%d  %d:%d:%d\r\n",tempTimeS.tm_mday, tempTimeS.tm_mon,tempTimeS.tm_year,tempTimeS.tm_hour,tempTimeS.tm_min,tempTimeS.tm_sec);
 // printf("tempTime %d\n", tempTime);
}


/** @brief This message is sent to set time the device. */
void SetTime(uint8_t pMessage[])
{
  time_t epochtime = 0;

  epochtime = pMessage[4];
  epochtime = (pMessage[3] << 8) | epochtime;
  epochtime = ((pMessage[2]  & 0x000000FF) << 16) | epochtime;
  epochtime = ((pMessage[1]  & 0x000000FF) << 24) | epochtime;   /* Final epoch time value. */

 
}

