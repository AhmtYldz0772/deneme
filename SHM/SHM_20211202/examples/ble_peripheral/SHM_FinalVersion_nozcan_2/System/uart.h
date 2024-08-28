/* UART Middleware Implementation for NEOS Photon */
#ifndef UART_H__
#define UART_H__

#include "bsp.h"
#include "nrfx_uart.h"
#include <stdbool.h>

extern volatile uint8_t transmitCompletedFlag;
extern volatile uint8_t receiveCompletedFlag;
extern const nrfx_uart_t uartInstance;

typedef enum uartBaudRate_e
{
    BAUD_RATE_1200 = NRF_UART_BAUDRATE_1200,       /**< 1200 baud. */
    BAUD_RATE_2400 = NRF_UART_BAUDRATE_2400,       /**< 2400 baud. */
    BAUD_RATE_4800 = NRF_UART_BAUDRATE_4800,       /**< 4800 baud. */
    BAUD_RATE_9600 = NRF_UART_BAUDRATE_9600,       /**< 9600 baud. */
    BAUD_RATE_14400 = NRF_UART_BAUDRATE_14400,     /**< 14400 baud. */
    BAUD_RATE_19200 = NRF_UART_BAUDRATE_19200,     /**< 19200 baud. */
    BAUD_RATE_28800 = NRF_UART_BAUDRATE_28800,     /**< 28800 baud. */
    BAUD_RATE_31250 = NRF_UART_BAUDRATE_31250,     /**< 31250 baud. */
    BAUD_RATE_38400 = NRF_UART_BAUDRATE_38400,     /**< 38400 baud. */
    BAUD_RATE_56000 = NRF_UART_BAUDRATE_56000,     /**< 56000 baud. */
    BAUD_RATE_57600 = NRF_UART_BAUDRATE_57600,     /**< 57600 baud. */
    BAUD_RATE_76800 = NRF_UART_BAUDRATE_76800,     /**< 76800 baud. */
    BAUD_RATE_115200 = NRF_UART_BAUDRATE_115200,   /**< 115200 baud. */
    BAUD_RATE_230400 = NRF_UART_BAUDRATE_230400,   /**< 230400 baud. */
    BAUD_RATE_250000 = NRF_UART_BAUDRATE_250000,   /**< 250000 baud. */
    BAUD_RATE_460800 = NRF_UART_BAUDRATE_460800,   /**< 460800 baud. */
    BAUD_RATE_921600 = NRF_UART_BAUDRATE_921600,   /**< 921600 baud. */
    BAUD_RATE_1000000 = NRF_UART_BAUDRATE_1000000, /**< 1000000 baud. */
} uartBaudRate_t;

typedef enum uartParity_e
{
    UART_PARITY_ENABLED  = NRF_UART_PARITY_INCLUDED,
    UART_PARITY_DISABLED  = NRF_UART_PARITY_EXCLUDED,
} uartParity_t;

typedef enum uartHardwareFlowControl_e
{
    UART_HARDWARE_CONTROL_DISABLED = NRF_UART_HWFC_ENABLED,
    UART_HARDWARE_CONTROL_ENABLED  = NRF_UART_HWFC_DISABLED,
} uartHardwareFlowControl_t;

typedef struct UART_s
{
    uartBaudRate_t baudRate;
    uartParity_t parity;
    uartHardwareFlowControl_t hardwareFlowControl;
    uint32_t RTSPin;
    uint32_t CTSPin;
    uint32_t RXDPin;
    uint32_t TXDPin;
    uint8_t interruptPriority;
    void *p_context;
} UART_t;

uint8_t UartInit(UART_t *uartConfigStructure);
void UartInterruptEventHandler(nrfx_uart_event_t const *p_event, void *p_context);
uint8_t UartReceive(uint8_t *dataPointer, size_t length);
uint8_t UartTransmit(uint8_t const *dataPointer, size_t length);
void UartRxDisable(void);
void UartRxEnable(void);
void UartRxAbort(void);
void UartTxAbort(void);
void UartUninit(void);

#endif