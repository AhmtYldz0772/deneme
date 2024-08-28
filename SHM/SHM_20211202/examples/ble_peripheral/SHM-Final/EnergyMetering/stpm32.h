#ifndef STPM32_H__
#define STPM32_H__

#include "System/gpio.h"
#include "System/uart.h"
#include <stdint.h>

#define STPM32_TOTAL_REGISTER_NUMBER 69
#define STPM32_DUMMY_READ_ADDRESS 0xFF
#define STPM32_DUMMY_WRITE_ADDRESS 0xFF
#define STPM32_FRAME_LENGTH 5
#define CRC_8 0x07

#define VREF 1.18          /* Internal voltage reference. */
#define K_INT 1            /* Integrator gain.            */
#define A_V 2              /* Voltage gain.               */
#define A_I 16             /* Shunt current gain.         */
#define CAL_V 0.875        /* Voltage calibrator.         */
#define CAL_I 0.875        /* Current calibrator.         */
#define K_S 30E-3          /* Current sensitivity.        */
#define R1 (270E3) * 3     /* Voltage Divider R1.         */
#define R2 470             /* Voltage Divider R1.         */
#define K_V R2 / (R1 + R2) /* Voltage sensitivity.        */
#define DCLK 7812.5        /* Internal decimation clock.  */
#define LSB_PERIOD  8E-6   /* LSB of Period (fixed).      */
      
extern uint8_t isSTPMInitialized;

typedef struct STPM32Register_s
{
    uint32_t value;
} STPM32Register_t;

typedef enum STPM32Registers_e
{
    DSP_CONTROL_REGISTER_1 = 0,
    DSP_CONTROL_REGISTER_2,
    DSP_CONTROL_REGISTER_3,
    DSP_CONTROL_REGISTER_4,
    DSP_CONTROL_REGISTER_5,
    DSP_CONTROL_REGISTER_6,
    DSP_CONTROL_REGISTER_7,
    DSP_CONTROL_REGISTER_8,
    DSP_CONTROL_REGISTER_9,
    DSP_CONTROL_REGISTER_10,
    DSP_CONTROL_REGISTER_11,
    DSP_CONTROL_REGISTER_12,
    DFE_CONTROL_REGISTER_1,
    DFE_CONTROL_REGISTER_2,
    DSP_INTERRUPT_REGISTER_1,
    DSP_INTERRUPT_REGISTER_2,
    DSP_STATUS_REGISTER_1,
    DSP_STATUS_REGISTER_2,
    US_CONTROL_REGISTER_1,
    US_CONTROL_REGISTER_2,
    US_CONTROL_REGISTER_3,
    DSP_EVENT_REGISTER_1,
    DSP_EVENT_REGISTER_2,
    DSP_REGISTER_1,
    DSP_REGISTER_2,
    DSP_REGISTER_3,
    DSP_REGISTER_4,
    DSP_REGISTER_5,
    DSP_REGISTER_6,
    DSP_REGISTER_7,
    DSP_REGISTER_8,
    DSP_REGISTER_9,
    DSP_REGISTER_10,
    DSP_REGISTER_11,
    DSP_REGISTER_12,
    DSP_REGISTER_13,
    DSP_REGISTER_14,
    DSP_REGISTER_15,
    DSP_REGISTER_16,
    DSP_REGISTER_17,
    DSP_REGISTER_18,
    DSP_REGISTER_19,
    PH1_REGISTER_1,
    PH1_REGISTER_2,
    PH1_REGISTER_3,
    PH1_REGISTER_4,
    PH1_REGISTER_5,
    PH1_REGISTER_6,
    PH1_REGISTER_7,
    PH1_REGISTER_8,
    PH1_REGISTER_9,
    PH1_REGISTER_10,
    PH1_REGISTER_11,
    PH1_REGISTER_12,
    PH2_REGISTER_1,
    PH2_REGISTER_2,
    PH2_REGISTER_3,
    PH2_REGISTER_4,
    PH2_REGISTER_5,
    PH2_REGISTER_6,
    PH2_REGISTER_7,
    PH2_REGISTER_8,
    PH2_REGISTER_9,
    PH2_REGISTER_10,
    PH2_REGISTER_11,
    PH2_REGISTER_12,
    TOT_REGISTER_1,
    TOT_REGISTER_2,
    TOT_REGISTER_3
} STPM32Registers_t;

/* Structure for STPM32 Device */
typedef struct STPM32_s
{
    Gpio_t EN;                                                /* Enable Pin.                */
    Gpio_t SCS;                                               /* SCS Pin.                   */
    Gpio_t SYN;                                               /* SYN Pin.                   */
    Gpio_t INT1;                                              /* INT1 Pin.                  */
    Gpio_t TXD;                                               /* TXD Pin.                   */
    Gpio_t RXD;                                               /* RCD Pin.                   */
    UART_t UART;                                              /* UART Properties.           */
    STPM32Register_t Registers[STPM32_TOTAL_REGISTER_NUMBER]; /* STPM32 Internal Registers. */
} STPM32_t;

uint8_t InitializeSTPM32(STPM32_t *stpm32);    /* This function initializes STPM32 device.          */
float ReadVoltageRMS(STPM32_t *stpm32);        /* This function is used to read RMS Voltage.        */
float ReadCurrentRMS(STPM32_t *stpm32);        /* This function is used to read RMS Current.        */
float ReadActiveEnergy(STPM32_t *stpm32);      /* This function is used to read Active Energy.      */
float ReadFundamentalEnergy(STPM32_t *stpm32); /* This function is used to read Fundamental Energy. */
float ReadReactiveEnergy(STPM32_t *stpm32);    /* This function is used to read Reactive Energy.    */
float ReadApparentEnergy(STPM32_t *stpm32);    /* This function is used to read Apparent Energy.    */
float ReadActivePower(STPM32_t *stpm32);       /* This function is used to read Active Power.       */
float ReadFundamentalPower(STPM32_t *stpm32);  /* This function is used to read Fundamental Power.  */
float ReadReactivePower(STPM32_t *stpm32);     /* This function is used to read Reactive Power.     */
float ReadApparentRMSPower(STPM32_t *stpm32);  /* This function is used to read Apparent RMS Power. */
float ReadLinePeriod(STPM32_t *stpm32);        /* This function is used to read Period of Line.     */
#endif                                         /* _ STPM32_H__ */