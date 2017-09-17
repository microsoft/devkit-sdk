/*(Copyright)

Microsoft Copyright 2017
Confidential Information

*/
#include "DiceInit.h"
#include "DiceCore.h"
#include "RiotCore.h"
#include "iothub_client_dps.h"
#include <stdio.h>

extern void* __start_riot_core;
extern void* __stop_riot_core;

extern void* __start_nvdata_protected_data;
extern void* __stop_nvdata_protected_data;

// Protected data
extern DICE_UDS      DiceUDS;                       // NV
extern DICE_CMPND_ID DiceCDI;                       // V
extern uint8_t       vDigest[DICE_DIGEST_LENGTH];   // V
extern uint8_t       rDigest[DICE_DIGEST_LENGTH];   // V

// Non-protected data
DICE_DATA DiceData = { 0 };

// Prototypes
//void MCUInit(void);
//void DiceInit(void);
//void DiceCore(void);
//void SystemClock_Config(void);
																		 
// Functions

void DiceRemediate(void)
{
    // TODO: FORCE RESET HERE
    while (1);
}

int StartDiceInit(void)
{
    (void)printf("The riot_core start address: %p\r\n", &__start_riot_core);
    (void)printf("The riot_core end address: %p\r\n", &__stop_riot_core);

    (void)printf("The riot_nvdata_protected_data start address: %p\r\n", &__start_nvdata_protected_data);
    (void)printf("The riot_nvdata_protected_data end address: %p\r\n", &__stop_nvdata_protected_data);

    //int intMask;
	(void)printf("#DEBUG: func: %s, file: %s, line: %d, start to run the program.\r\n", __FUNCTION__, __FILE__, __LINE__);
    // Initialize MCU
    //MCUInit();
    (void)printf("#DEBUG: func: %s, file: %s, line: %d, MCUInit done.\r\n", __FUNCTION__, __FILE__, __LINE__);

    // Initialize DICE
    DiceInit();

    // Disable interrupts
	//__disable_irq();
	
    // Launch protected DICE code. This will measure RIoT Core, derive the
    // CDI value. It must execute with interrupts disabled. Therefore, it
    // must return so we can restore interrupt state.
    DiceCore();
	
    // Enable interrupts
    //__enable_irq();
	
    // If DiceCore detects an error condition, it will not enable access to
    // the volatile storage segment. This attempt to transfer control to RIoT
    // will trigger a system reset. We will not be able to proceed.
    // TODO: DETECT WHEN A RESET HAS OCCURRED AND TAKE SOME ACTION.
    RiotStart(DiceCDI.bytes, (uint16_t)DICE_DIGEST_LENGTH);

    // Transfer control to firmware
    IoTHubClientStart();

    // Whould not return, enter remediation on return. This MUST force system reset.
    //DiceRemediate();
    return 0;
}

void DiceInit(void)
{
    (void)printf("#DEBUG: func: %s, file: %s, line: %d, entered DiceInit func.\r\n", __FUNCTION__, __FILE__, __LINE__);
    // Firewall configuration structure
    //FIREWALL_InitTypeDef fw_init;

    // Up-front sanity check
    if (DiceUDS.tag != DICE_UDS_TAG) {
        // We must be provisioned.
        DiceRemediate();
    }

    // Initialize CDI structure
    memset(&DiceCDI, 0x00, sizeof(DICE_CMPND_ID));
    DiceCDI.tag = DICE_CMPND_TAG;

    // Pointers to protected DICE Data
    DiceData.UDS = &DiceUDS;
    DiceData.CDI = &DiceCDI;

    // Start of RIoT Invariant Code
    DiceData.riotCore = (uint8_t*)&__start_riot_core;

    // Calculate size of RIoT Core
    DiceData.riotSize = (uint8_t*)&__stop_riot_core - DiceData.riotCore;
    (void)printf("#DEBUG: func: %s, file: %s, line: %d, DiceData.riotSize == %d.\r\n", __FUNCTION__, __FILE__, __LINE__, DiceData.riotSize);

    // Protected code segment start address and length
    /*fw_init.CodeSegmentStartAddress = 0x08070000;
    fw_init.CodeSegmentLength = 0x700;

    // Protected non-volatile data segment (in FLASH memory) start address and length
    fw_init.NonVDataSegmentStartAddress = 0x08074000;
    fw_init.NonVDataSegmentLength = 0x200;

    // Protected volatile data segment (in SRAM memory) start address and length
    fw_init.VDataSegmentStartAddress = 0x20000000;
    fw_init.VDataSegmentLength = 0x200;

    // The protected volatile data segment can't be executed
    fw_init.VolatileDataExecution = FIREWALL_VOLATILEDATA_NOT_EXECUTABLE;

    // At startup, the protected volatile data segment is not shared with non-
    // protected application code. Once the CDI is established this area is shared.
    fw_init.VolatileDataShared = FIREWALL_VOLATILEDATA_NOT_SHARED;

    // Firewall configuration
    if (HAL_FIREWALL_Config(&fw_init) != HAL_OK) {
        Error_Handler();
    }

    // Close the firewall
    HAL_FIREWALL_EnableFirewall();*/
    return;
}

/*void
MCUInit(void)
{
    //
    // Implementation-specific initialization
    //

    // STM32L4xx HAL library initialization
    HAL_Init();
    (void)printf("#DEBUG: func: %s, file: %s, line: %d, HAL_Init done.\r\n", __FUNCTION__, __FILE__, __LINE__);

    // Configure the system clock to 80 MHz
    //SystemClock_Config();
    //(void)printf("#DEBUG: func: %s, file: %s, line: %d, SystemClock_Config done.\r\n", __FUNCTION__, __FILE__, __LINE__);

    // Initialize LED2
    BSP_LED_Init(LED2);

    // Enable SYSCFG clock to be able to enable the Firewall
    //__HAL_RCC_SYSCFG_CLK_ENABLE();

    // Enable Power Clock to be able to use RTC back-up registers.
    // The latter are used to keep track of the example code proper unrolling.
    // They are not related to Firewall functionality.
    __HAL_RCC_PWR_CLK_ENABLE();
    HAL_PWR_EnableBkUpAccess();
}

void
SystemClock_Config(
    void
)
{
  (void)printf("#DEBUG: func: %s, file: %s, line: %d, enter SystemClock_Config.\r\n", __FUNCTION__, __FILE__, __LINE__);
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};

  // MSI is enabled after System reset, activate PLL with MSI as source
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.MSICalibrationValue = RCC_MSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 40;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLP = 7;
  RCC_OscInitStruct.PLL.PLLQ = 4;

  (void)printf("#DEBUG: func: %s, file: %s, line: %d, HAL_RCC_OscConfig before.\r\n", __FUNCTION__, __FILE__, __LINE__);
  if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
    // Initialization Error
    while(1);
  }

  (void)printf("#DEBUG: func: %s, file: %s, line: %d, HAL_RCC_OscConfig done.\r\n", __FUNCTION__, __FILE__, __LINE__);
  
  // Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 clocks dividers
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;  
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;  
  if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    // Initialization Error
    while(1);
  }
}

void
HAL_SYSTICK_Callback(
    void
)
{
  HAL_IncTick();  
}


#ifdef  USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
    while (1);
}
#endif*/

