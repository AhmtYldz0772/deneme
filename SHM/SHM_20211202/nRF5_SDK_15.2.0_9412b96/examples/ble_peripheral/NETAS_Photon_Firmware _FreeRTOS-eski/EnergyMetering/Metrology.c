#include "Metrology.h"
#include "System/gpio.h"

STPM32_t stpm32 = {0};
uint8_t isMetrologyInitialized = false;

uint8_t InitializeMetrology(void)
{
    uint8_t returnValue = false;
    if (CheckGpioInitialized() == true)
    {
        if (InitializeSTPM32(&stpm32) == true)
        {
            returnValue = true;
            isMetrologyInitialized = true;
        }
    }
    return returnValue;
}

float GetRMSVoltage(void)
{
    float voltageRMS = 0.0;
    if (isMetrologyInitialized == true)
    {
        voltageRMS = ReadVoltageRMS(&stpm32);
    }
    return voltageRMS;
}

float GetRMSCurrent(void)
{
    float currentRMS = 0.0;
    if (isMetrologyInitialized == true)
    {
        currentRMS = ReadCurrentRMS(&stpm32);
    }
    return currentRMS;
}

float GetActiveEnergy(void)
{
    float activeEnergy = 0.0;
    if (isMetrologyInitialized == true)
    {
        activeEnergy = ReadActiveEnergy(&stpm32);
    }
    return activeEnergy;
}

float GetFundamentalEnergy(void)
{
    float fundamentalEnergy = 0.0;
    if (isMetrologyInitialized == true)
    {
        fundamentalEnergy = ReadFundamentalEnergy(&stpm32);
    }
    return fundamentalEnergy;
}

float GetReactiveEnergy(void)
{
    float reactiveEnergy = 0.0;
    if (isMetrologyInitialized == true)
    {
        reactiveEnergy = ReadReactiveEnergy(&stpm32);
    }
    return reactiveEnergy;
}

float GetApparentEnergy(void)
{
    float apparentEnergy = 0.0;
    if (isMetrologyInitialized == true)
    {
        apparentEnergy = ReadApparentEnergy(&stpm32);
    }
    return apparentEnergy;
}

float GetActivePower(void)
{
    float activePower = 0.0;
    if (isMetrologyInitialized == true)
    {
        activePower = ReadActivePower(&stpm32);
    }
    return activePower;
}

float GetFundamentalPower(void)
{
    float fundamentalPower = 0.0;
    if (isMetrologyInitialized == true)
    {
        fundamentalPower = ReadFundamentalPower(&stpm32);
    }
    return fundamentalPower;
}

float GetReactivePower(void)
{
    float reactivePower = 0.0;
    if (isMetrologyInitialized == true)
    {
        reactivePower = ReadReactivePower(&stpm32);
    }
    return reactivePower;
}

float GetApparentRMSPower(void)
{
    float apparentRMSPower = 0.0;
    if (isMetrologyInitialized == true)
    {
        apparentRMSPower = ReadApparentRMSPower(&stpm32);
    }
    return apparentRMSPower;
}

float GetLinePeriod(void)
{
    float linePeriod = 0.0;
    if (isMetrologyInitialized == true)
    {
        linePeriod = ReadLinePeriod(&stpm32);
    }
    return linePeriod;
}