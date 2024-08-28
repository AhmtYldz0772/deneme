#include "stpm32.h"
#include "math.h"
#include "nrf_delay.h"
static uint8_t CRCChecksum;
static float powerLSBValue = 0;
static float energyLSBValue = 0;
static float voltageRMSLSBValue = 0;
static float currentRMSLSBValue = 0;
static float instantaneousVoltageLSBValue = 0;
static float instantaneousCurrentLSBValue = 0;

uint8_t isSTPMInitialized = false;

static void SelectUartInterface(STPM32_t *stpm32)
{
    GpioWrite(&(stpm32->SCS), 1); /* Set SCS Pin to High while EN Pin is Low. */
    nrf_delay_ms(10);
    GpioWrite(&(stpm32->EN), 1); /* Set EN Pin to High. */
    nrf_delay_ms(50);
}

static void ApplyPORSequence(STPM32_t *stpm32)
{
    for (uint8_t i = 0; i < 3; i++)
    {
        GpioWrite(&(stpm32->SYN), 0);
        nrf_delay_us(20);
        GpioWrite(&(stpm32->SYN), 1);
        nrf_delay_us(20);
    }
    GpioWrite(&(stpm32->SCS), 0);
    nrf_delay_us(20);
    GpioWrite(&(stpm32->SCS), 1);
}

static void LatchRegisters(STPM32_t *stpm32)
{
    GpioWrite(&(stpm32->SCS), 1);
    nrf_delay_ms(50);
    GpioWrite(&(stpm32->SYN), 0);
    nrf_delay_us(20);
    GpioWrite(&(stpm32->SYN), 1);
}

static void CalculateCRC8(uint8_t inData)
{
    uint8_t locIndex;
    uint8_t locTemp;
    locIndex = 0;
    while (locIndex < 8)
    {
        locTemp = inData ^ CRCChecksum;
        CRCChecksum <<= 1;
        if (locTemp & 0x80)
        {
            CRCChecksum ^= CRC_8;
        }
        inData <<= 1;
        locIndex++;
    }
}

/**
  * @brief      Calculate CRC of a frame
  * @param      Buffer Frame
  * @retval     uint8 checksum of the frame
  */
static uint8_t CalculateCRC8Frame(uint8_t *pBuffer)
{
    uint8_t i;
    CRCChecksum = 0x00;

    for (i = 0; i < STPM32_FRAME_LENGTH - 1; i++)
    {
        CalculateCRC8(pBuffer[i]);
    }

    return CRCChecksum;
}

static uint8_t ByteReverse(uint8_t inByte)
{
    inByte = ((inByte >> 1) & 0x55) | ((inByte << 1) & 0xaa);
    inByte = ((inByte >> 2) & 0x33) | ((inByte << 2) & 0xcc);
    inByte = ((inByte >> 4) & 0x0F) | ((inByte << 4) & 0xF0);

    return inByte;
}

static void PrepareUARTFrame(uint8_t *pBuffer)
{
    uint8_t temp[4];
    uint8_t x = 0;
    uint8_t CRCOnReversedBuffer;
    for (x = 0; x < (STPM32_FRAME_LENGTH - 1); x++)
    {
        temp[x] = ByteReverse(pBuffer[x]);
    }
    CRCOnReversedBuffer = CalculateCRC8Frame(temp);
    pBuffer[4] = ByteReverse(CRCOnReversedBuffer);
}

static uint32_t ReadSTPM32Register(STPM32_t *stpm32, STPM32Registers_t registerName)
{
    uint8_t returnValue = 0;
    uint8_t receiveBuffer[STPM32_FRAME_LENGTH] = {0};
    uint8_t txCommand[STPM32_FRAME_LENGTH] =
        {registerName * 2,
            STPM32_DUMMY_WRITE_ADDRESS,
            STPM32_DUMMY_WRITE_ADDRESS,
            STPM32_DUMMY_WRITE_ADDRESS};
    PrepareUARTFrame(txCommand);
    LatchRegisters(stpm32);
    nrf_delay_ms(50);
    if (UartTransmit((const uint8_t *)&txCommand, STPM32_FRAME_LENGTH) == true)
    {
        while (transmitCompletedFlag == false)
            ;
        transmitCompletedFlag == false;
    }
    else
    {
        return returnValue;
        while (1)
            ;
    }
    txCommand[0] = STPM32_DUMMY_READ_ADDRESS;
    PrepareUARTFrame(txCommand);
    nrf_delay_ms(200);
    if (UartReceive((uint8_t *)(&receiveBuffer), STPM32_FRAME_LENGTH) == true)
    {
        if (UartTransmit((const uint8_t *)&txCommand, STPM32_FRAME_LENGTH) == true)
        {
            while (transmitCompletedFlag == false)
                ;
            transmitCompletedFlag == false;
        }
        while (receiveCompletedFlag == false)
            ;
    }
    returnValue = 1;
    stpm32->Registers[registerName].value = (receiveBuffer[0]) | (receiveBuffer[1] << 8) | (receiveBuffer[2] << 16) | (receiveBuffer[3] << 24);
    return returnValue;
}

float ReadVoltageRMS(STPM32_t *stpm32)
{
    float voltageRMS = 0.0;
    uint16_t temp = 0;
    if (isSTPMInitialized == true)
    {
        if (ReadSTPM32Register(stpm32, DSP_REGISTER_1) == true)
        {
            temp = (uint16_t)((stpm32->Registers[DSP_REGISTER_1].value) & (0xFF));
            voltageRMS = (float)temp * voltageRMSLSBValue;
        }
    }

    return voltageRMS;
}

float ReadCurrentRMS(STPM32_t *stpm32)
{
    float currentRMS = 0.0;
    uint16_t temp = 0;
    if (isSTPMInitialized == true)
    {
        if (ReadSTPM32Register(stpm32, DSP_REGISTER_14) == true)
        {
            temp = (uint16_t)(((stpm32->Registers[DSP_REGISTER_14].value) & (0xFF00)) >> 16);
            currentRMS = (float)temp * currentRMSLSBValue;
        }
    }
    return currentRMS;
}

float ReadActiveEnergy(STPM32_t *stpm32)
{
    float activeEnergy = 0.0;
    if (isSTPMInitialized == true)
    {
        if (ReadSTPM32Register(stpm32, PH1_REGISTER_1) == true)
        {
            activeEnergy = (float)stpm32->Registers[PH1_REGISTER_1].value * energyLSBValue;
        }
    }
    return activeEnergy;
}

float ReadFundamentalEnergy(STPM32_t *stpm32)
{
    float fundamentalEnergy = 0.0;
    if (isSTPMInitialized == true)
    {
        if (ReadSTPM32Register(stpm32, PH1_REGISTER_2) == true)
        {
            fundamentalEnergy = (float)stpm32->Registers[PH1_REGISTER_2].value * energyLSBValue;
        }
    }
    return fundamentalEnergy;
}

float ReadReactiveEnergy(STPM32_t *stpm32)
{
    float reactiveEnergy = 0.0;
    if (isSTPMInitialized == true)
    {
        if (ReadSTPM32Register(stpm32, PH1_REGISTER_3) == true)
        {
            reactiveEnergy = (float)stpm32->Registers[PH1_REGISTER_3].value * energyLSBValue;
        }
    }
    return reactiveEnergy;
}

float ReadApparentEnergy(STPM32_t *stpm32)
{
    float apparentEnergy = 0.0;
    if (isSTPMInitialized == true)
    {
        if (ReadSTPM32Register(stpm32, PH1_REGISTER_4) == true)
        {
            apparentEnergy = (float)stpm32->Registers[PH1_REGISTER_4].value * energyLSBValue;
        }
    }
    return apparentEnergy;
}

float ReadActivePower(STPM32_t *stpm32)
{
    float activePower = 0.0;
    if (isSTPMInitialized == true)
    {
        if (ReadSTPM32Register(stpm32, PH1_REGISTER_5) == true)
        {
            activePower = (float)stpm32->Registers[PH1_REGISTER_5].value * powerLSBValue;
        }
    }
    return activePower;
}

float ReadFundamentalPower(STPM32_t *stpm32)
{
    float fundamentalPower = 0.0;
    if (isSTPMInitialized == true)
    {
        if (ReadSTPM32Register(stpm32, PH1_REGISTER_6) == true)
        {
            fundamentalPower = (float)stpm32->Registers[PH1_REGISTER_6].value * powerLSBValue;
        }
    }
    return fundamentalPower;
}

float ReadReactivePower(STPM32_t *stpm32)
{
    float reactivePower = 0.0;
    if (isSTPMInitialized == true)
    {
        if (ReadSTPM32Register(stpm32, PH1_REGISTER_7) == true)
        {
            reactivePower = (float)stpm32->Registers[PH1_REGISTER_7].value * powerLSBValue;
        }
    }
    return reactivePower;
}

float ReadApparentRMSPower(STPM32_t *stpm32)
{
    float apparentRMSPower = 0.0;
    if (isSTPMInitialized == true)
    {
        if (ReadSTPM32Register(stpm32, PH1_REGISTER_8) == true)
        {
            apparentRMSPower = (float)stpm32->Registers[PH1_REGISTER_8].value * powerLSBValue;
        }
    }
    return apparentRMSPower;
}

float ReadLinePeriod(STPM32_t *stpm32)
{
   float period = 0.0;
   if (isSTPMInitialized == true)
   {
      if (ReadSTPM32Register(stpm32, DSP_REGISTER_1) == true)
      {
          period = (float)stpm32->Registers[DSP_REGISTER_1].value * (float)LSB_PERIOD;
      }
   }
   return period;
}

uint8_t InitializeSTPM32(STPM32_t *stpm32)
{
    voltageRMSLSBValue = VREF * (1 + R1 / R2) / (CAL_V * A_V * pow(2, 15));
    currentRMSLSBValue = VREF / (CAL_I * A_I * pow(2, 17) * K_S * K_INT);
    instantaneousVoltageLSBValue = VREF * (1 + R1 / R2) / (CAL_V * A_V * pow(2, 23));
    instantaneousCurrentLSBValue = VREF / (CAL_I * A_I * pow(2, 23) * K_S * K_INT);
    powerLSBValue = pow(VREF, 2) * (1 + R1 / R2) / (K_INT * A_V * A_I * K_S * CAL_V * CAL_I * pow(2, 28));
    energyLSBValue = pow(VREF, 2) * (1 + R1 / R2) / (3600 * DCLK * K_INT * A_V * A_I * K_S * CAL_V * CAL_I * pow(2, 17));
    uint8_t returnCode = 0;
    /* Initialize STPM32 Pins. */
    GpioInit(&(stpm32->EN), STPM32_EN_PIN, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0);
    GpioInit(&(stpm32->INT1), STPM32_INT1_PIN, PIN_INPUT, PIN_PUSH_PULL, PIN_PULL_DOWN, 0);
    GpioInit(&(stpm32->RXD), STPM32_RX_PIN, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0);
    GpioInit(&(stpm32->TXD), STPM32_TX_PIN, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0);
    GpioInit(&(stpm32->SCS), STPM32_SCS_PIN, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0);
    GpioInit(&(stpm32->SYN), STPM32_SYN_PIN, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1);
    nrf_delay_ms(10);
    SelectUartInterface(stpm32);
    nrf_delay_ms(1);
    ApplyPORSequence(stpm32);
    nrf_delay_ms(1);

    ((UART_t *)(&(stpm32->UART)))->baudRate = BAUD_RATE_9600;
    ((UART_t *)(&(stpm32->UART)))->hardwareFlowControl = UART_HARDWARE_CONTROL_DISABLED;
    ((UART_t *)(&(stpm32->UART)))->interruptPriority = 3;
    ((UART_t *)(&(stpm32->UART)))->p_context = NULL;
    ((UART_t *)(&(stpm32->UART)))->parity = UART_PARITY_DISABLED;
    ((UART_t *)(&(stpm32->UART)))->CTSPin = NC;
    ((UART_t *)(&(stpm32->UART)))->RTSPin = NC;
    ((UART_t *)(&(stpm32->UART)))->RXDPin = ((Gpio_t *)(&stpm32->RXD))->pin;
    ((UART_t *)(&(stpm32->UART)))->TXDPin = ((Gpio_t *)(&stpm32->TXD))->pin;

    if (UartInit(&(stpm32->UART)) == true)
    {
        returnCode = 1;
    }
    isSTPMInitialized = true;
    return returnCode;
}