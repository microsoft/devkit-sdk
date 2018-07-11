#include "Watchdog.h"

Watchdog::Watchdog()
{
    _causedReset = false;
}

// Reference: STM32F412 user manual
// http://www.st.com/content/ccc/resource/technical/document/reference_manual/group0/4f/7b/2b/bd/04/b3/49/25/DM00180369/files/DM00180369.pdf/jcr:content/translations/en.DM00180369.pdf
bool Watchdog::configure(float timeoutInMs)
{
    // The internal Low Speed oscillator (LSI) has a frequency of 32kHz.
    // #define LSI_VALUE  ((uint32_t)32000U)

    // Min timeout period at 32kHz LSI: 0.125ms, with 4 prescaler divider
    // Max timeout period: 32768ms, with 256 prescaler divider
    if (timeoutInMs < 0.125 || timeoutInMs > 32768)
    {
        return false;
    }

    uint16_t prescalerCode;
    uint16_t prescalerDivider;
    uint16_t reloadValue;  

    // Calculate prescaler value for prescaler registry (IWDG_PR)
    float timeoutSeconds = timeoutInMs / 1000;
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

    // printf("Watchdog is configured with: prescaler = %d, reload = 0x%X, timeout milliseconds: %d\r\n", prescalerDivider, reloadValue, timeout);

    IWDG->KR = 0x5555;              // Enable write access to prescaler (IWDG_PR) and reload (IWDG_PLR) register
    IWDG->PR = prescalerCode;       // Set prescaler register
    IWDG->RLR = reloadValue;        // Set reload register
    IWDG->KR = 0xAAAA;              // Reload max counter value from IWDG_PLR register to the counter
    IWDG->KR = 0xCCCC;              // Start Watchdog

    return true;
}

void Watchdog::resetTimer()
{
    IWDG->KR = 0xAAAA;
}

bool Watchdog::resetTriggered()
{
    // Check if the Watchdog trigger a reset by the RCC_CSR register
    // RCC_CSR reset value: 0x0E00 0000
    if (RCC->CSR & (1<<29)) {
        _causedReset = true;

        // Clear reset flag
        RCC->CSR |= (1<<24);
    }

    return _causedReset;
}
