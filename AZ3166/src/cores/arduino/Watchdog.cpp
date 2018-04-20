#include "Watchdog.h"

Watchdog::Watchdog()
{
}

// Reference: STM32F412 user manual
// http://www.st.com/content/ccc/resource/technical/document/reference_manual/group0/4f/7b/2b/bd/04/b3/49/25/DM00180369/files/DM00180369.pdf/jcr:content/translations/en.DM00180369.pdf
void Watchdog::configure(int timeout)
{
    uint16_t prescalerCode;
    uint16_t prescalerDivider;
    uint16_t reloadValue;

    // The internal Low Speed oscillator (LSI) has a frequency of 32kHz.
    // #define LSI_VALUE  ((uint32_t)32000U)

    // Calculate prescaler value for prescaler registry (IWDG_PR)
    float timeoutSeconds = timeout / 1000;
    if ((timeoutSeconds * (LSI_VALUE / 4)) < 0xFFF)
    {
        prescalerCode = IWDG_PRESCALER_4;
        prescalerDivider = 4;
    }
    else if ((timeoutSeconds * (LSI_VALUE / 8)) < 0xFFF)
    {
        prescalerCode = IWDG_PRESCALER_8;
        prescalerDivider = 8;
    }
    else if ((timeoutSeconds * (LSI_VALUE / 16)) < 0xFFF)
    {
        prescalerCode = IWDG_PRESCALER_16;
        prescalerDivider = 16;
    }
    else if ((timeoutSeconds * (LSI_VALUE / 32)) < 0xFFF)
    {
        prescalerCode = IWDG_PRESCALER_32;
        prescalerDivider = 32;
    }
    else if ((timeoutSeconds * (LSI_VALUE / 64)) < 0xFFF)
    {
        prescalerCode = IWDG_PRESCALER_64;
        prescalerDivider = 64;
    }
    else if ((timeoutSeconds * (LSI_VALUE / 128)) < 0xFFF)
    {
        prescalerCode = IWDG_PRESCALER_128;
        prescalerDivider = 128;
    }
    else
    {
        prescalerCode = IWDG_PRESCALER_256;
        prescalerDivider = 256;
    }

    // Calculate the IWDG reload value for reload register (IWDG_RLR)
    int clk = LSI_VALUE / prescalerDivider;
    reloadValue  = (uint16_t)(timeoutSeconds * clk);
    
    printf("Watchdog is configured with: prescaler = %d, reload = 0x%X, timeout miliiseconds: %d\r\n", prescalerDivider, reloadValue, timeout);

    IWDG->KR = 0x5555;              // Enable write access to prescaler (IWDG_PR) and reload (IWDG_PLR) register
    IWDG->PR = prescalerCode;       // Set prescaler register
    IWDG->RLR = reloadValue;        // Set reload register
    IWDG->KR = 0xAAAA;              // Reload max counter value from IWDG_PLR register to the counter
    IWDG->KR = 0xCCCC;              // Start Watchdog
}

void Watchdog::resetTimer()
{
    IWDG->KR = 0xAAAA;
}
