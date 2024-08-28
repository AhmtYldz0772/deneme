#include "uart.h"

volatile uint8_t transmitCompletedFlag = false;
volatile uint8_t receiveCompletedFlag = false;
const nrfx_uart_t uartInstance = NRFX_UART_INSTANCE(0); /*!< Needs to be revised according to the microprocessor. */
/**
 * @brief UART interrupt event handler. This is microcontroller specific implementation and needs to be ported. 
 *
 * @param[in] p_event    Pointer to event structure. Event is allocated on the stack so it is available
 *                       only within the context of the event handler.
 * @param[in] p_context  Context passed to interrupt handler, set on initialization.
 */
void uartInterruptEventHandler(nrfx_uart_event_t const *p_event, void *p_context)
{
    if ( p_event->type == NRFX_UART_EVT_TX_DONE )
    {
       transmitCompletedFlag = true;
    }
    else if( p_event->type == NRFX_UART_EVT_RX_DONE)
    {
       receiveCompletedFlag = true;
    }
}

/**
 * @brief Function for initializing the UART driver.
 *
 * This function configures and enables UART. After this function GPIO pins are controlled by UART.
 *
 * @param[in] uartConfigStructure      Structure for configuring UART Peripheral Instance
 
 * @retval    true                     If initialization was successful.
 * @retval    false                    If initialization was unsuccessful.
 */
uint8_t UartInit(UART_t *uartConfigStructure)
{

    nrfx_err_t errorCode;
    nrfx_uart_config_t uartConfig;
    uartConfig.baudrate = uartConfigStructure->baudRate;
    uartConfig.hwfc = uartConfigStructure->hardwareFlowControl;
    uartConfig.interrupt_priority = uartConfigStructure->interruptPriority;
    uartConfig.parity = uartConfigStructure->parity;
    uartConfig.pselcts = uartConfigStructure->CTSPin;
    uartConfig.pselrts = uartConfigStructure->RTSPin;
    uartConfig.pselrxd = uartConfigStructure->RXDPin;
    uartConfig.pseltxd = uartConfigStructure->TXDPin;
    uartConfig.p_context = uartConfigStructure->p_context;

    errorCode = nrfx_uart_init(&uartInstance, &uartConfig, uartInterruptEventHandler);

    if (errorCode == NRF_SUCCESS)
    {
        return true;
    }
    else
    {
        return false;
    }
}

/**
 * @brief Function for receiving data over UART.
 *
 * This function is used to receive data over UART. 
 *
 * @param[in] *dataPointer             Pointer variable that stores received characters.
 * @param[in]  length                  Variable that specifies length of the transfer in bytes.
 
 * @retval    true                     If reception was successful.
 * @retval    false                    If reception was unsuccessful.
 */
uint8_t UartReceive(uint8_t *dataPointer, size_t length)
{
    uint8_t returnCode = false;
    if (dataPointer != NULL)
    {
        if (length > 0)
        {
            if (nrfx_uart_rx(&uartInstance, dataPointer, length) == NRFX_SUCCESS)
            {
                returnCode = true;
            }
        }
    }

    return returnCode;
}

/**
 * @brief Function for transmitting data over UART.
 *
 * This function is used to transmit data over UART. 
 *
 * @param[in] *dataPointer             Pointer variable that stores characters to be transferred.
 * @param[in]  length                  Variable that specifies length of the transfer in bytes.
 
 * @retval    true                     If transmission was successful.
 * @retval    false                    If transmission was unsuccessful.
 */
uint8_t UartTransmit(uint8_t const *dataPointer, size_t length)
{
    transmitCompletedFlag = false;
    uint8_t returnCode = false;
    if (dataPointer != NULL)
    {
        if (length > 0)
        {
            if (nrfx_uart_tx(&uartInstance, dataPointer, length) == NRFX_SUCCESS)
            {
                returnCode = true;
            }
        }
    }
    return returnCode;
}

void UartRxDisable(void)
{
    nrfx_uart_rx_disable(&uartInstance);
}

void UartRxEnable(void)
{
    nrfx_uart_rx_enable(&uartInstance);
}

void UartRxAbort(void)
{
    nrfx_uart_rx_abort(&uartInstance);
}
void UartTxAbort(void)
{
    nrfx_uart_tx_abort(&uartInstance);
}
void UartUninit(void)
{
    nrfx_uart_uninit(&uartInstance);
}