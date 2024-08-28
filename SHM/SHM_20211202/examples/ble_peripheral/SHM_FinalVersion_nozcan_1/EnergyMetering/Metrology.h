#ifndef METROLOGY_H__
#define METROLOGY_H__

#include "stpm32.h"
#include <stdint.h>

extern uint8_t isMetrologyInitialized;
uint8_t InitializeMetrology(void);
float GetRMSVoltage(void);        /* This function is used to read RMS Voltage.        */
float GetRMSCurrent(void);        /* This function is used to read RMS Current.        */
float GetActiveEnergy(void);      /* This function is used to read Active Energy.      */
float GetFundamentalEnergy(void); /* This function is used to read Fundamental Energy. */
float GetReactiveEnergy(void);    /* This function is used to read Reactive Energy.    */
float GetApparentEnergy(void);    /* This function is used to read Apparent Energy.    */
float GetActivePower(void);       /* This function is used to read Active Power.       */
float GetFundamentalPower(void);  /* This function is used to read Fundamental Power.  */
float GetReactivePower(void);     /* This function is used to read Reactive Power.     */
float GetApparentRMSPower(void);  /* This function is used to read Apparent RMS Power. */
float GetLinePeriod(void);        /* This function is used to read Period of Line.     */

#endif /* _ METROLOGY_H__ */