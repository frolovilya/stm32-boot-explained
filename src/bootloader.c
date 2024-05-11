#include "stdint.h"
#include "system_stm32f4xx.h"
#include "stm32f4xx.h"

/**
 * Simple Bootloader implementation
*/

// start address for the initialization values of the .data section. defined in
// linker script
extern uint32_t _sidata;
// start address for the .data section. defined in linker script
extern uint32_t _sdata;
// end address for the .data section. defined in linker script
extern uint32_t _edata;
// start address for the .bss section. defined in linker script
extern uint32_t _sbss;
// end address for the .bss section. defined in linker script
extern uint32_t _ebss;
// Highest address of the user mode stack, end of RAM
extern uint32_t _estack;

extern void __libc_init_array();
extern int main();

/**
 * Application boot point
 */
void Reset_Handler() {
  // Call the clock system initialization function
  SystemInit();

  // Copy the data segment initializers from flash to SRAM
  uint32_t *initData = &_sidata;
  uint32_t *dataPtr = &_sdata;
  while (dataPtr < &_edata) {
    *dataPtr++ = *initData++;
  }

  // Zero fill the bss segment
  uint32_t *bssPtr = &_sbss;
  while (bssPtr < &_ebss) {
    *bssPtr++ = 0;
  }

  // Call static constructors
  __libc_init_array();

  // Call the application's entry point
  main();

  // Infinite loop
  while (1)
    ;
}

/**
 * Catches all unhandled interrupts
 */
void Default_Handler() {
  while (1)
    ;
}

/**
 * Catches all class of fault
 */
void HardFault_Handler() {
  while (1)
    ;
}

// Interrupts list according to the STM32F4 spec
__attribute__((weak)) void NMI_Handler(void) { Default_Handler(); }
__attribute__((weak)) void MemManage_Handler(void) { Default_Handler(); }
__attribute__((weak)) void BusFault_Handler(void) { Default_Handler(); }
__attribute__((weak)) void UsageFault_Handler(void) { Default_Handler(); }
__attribute__((weak)) void SVC_Handler(void) { Default_Handler(); }
__attribute__((weak)) void DebugMon_Handler(void) { Default_Handler(); }
__attribute__((weak)) void PendSV_Handler(void) { Default_Handler(); }
__attribute__((weak)) void SysTick_Handler(void) { Default_Handler(); }
__attribute__((weak)) void WWDG_IRQHandler(void) { Default_Handler(); }
__attribute__((weak)) void PVD_IRQHandler(void) { Default_Handler(); }
__attribute__((weak)) void TAMP_STAMP_IRQHandler(void) { Default_Handler(); }
__attribute__((weak)) void RTC_WKUP_IRQHandler(void) { Default_Handler(); }
__attribute__((weak)) void FLASH_IRQHandler(void) { Default_Handler(); }
__attribute__((weak)) void RCC_IRQHandler(void) { Default_Handler(); }
__attribute__((weak)) void EXTI0_IRQHandler(void) { Default_Handler(); }
__attribute__((weak)) void EXTI1_IRQHandler(void) { Default_Handler(); }
__attribute__((weak)) void EXTI2_IRQHandler(void) { Default_Handler(); }
__attribute__((weak)) void EXTI3_IRQHandler(void) { Default_Handler(); }
__attribute__((weak)) void EXTI4_IRQHandler(void) { Default_Handler(); }
__attribute__((weak)) void DMA1_Stream0_IRQHandler(void) { Default_Handler(); }
__attribute__((weak)) void DMA1_Stream1_IRQHandler(void) { Default_Handler(); }
__attribute__((weak)) void DMA1_Stream2_IRQHandler(void) { Default_Handler(); }
__attribute__((weak)) void DMA1_Stream3_IRQHandler(void) { Default_Handler(); }
__attribute__((weak)) void DMA1_Stream4_IRQHandler(void) { Default_Handler(); }
__attribute__((weak)) void DMA1_Stream5_IRQHandler(void) { Default_Handler(); }
__attribute__((weak)) void DMA1_Stream6_IRQHandler(void) { Default_Handler(); }
__attribute__((weak)) void ADC_IRQHandler(void) { Default_Handler(); }
__attribute__((weak)) void CAN1_TX_IRQHandler(void) { Default_Handler(); }
__attribute__((weak)) void CAN1_RX0_IRQHandler(void) { Default_Handler(); }
__attribute__((weak)) void CAN1_RX1_IRQHandler(void) { Default_Handler(); }
__attribute__((weak)) void CAN1_SCE_IRQHandler(void) { Default_Handler(); }
__attribute__((weak)) void EXTI9_5_IRQHandler(void) { Default_Handler(); }
__attribute__((weak)) void TIM1_BRK_TIM9_IRQHandler(void) { Default_Handler(); }
__attribute__((weak)) void TIM1_UP_TIM10_IRQHandler(void) { Default_Handler(); }
__attribute__((weak)) void TIM1_TRG_COM_TIM11_IRQHandler(void) { Default_Handler(); }
__attribute__((weak)) void TIM1_CC_IRQHandler(void) { Default_Handler(); }
__attribute__((weak)) void TIM2_IRQHandler(void) { Default_Handler(); }
__attribute__((weak)) void TIM3_IRQHandler(void) { Default_Handler(); }
__attribute__((weak)) void TIM4_IRQHandler(void) { Default_Handler(); }
__attribute__((weak)) void I2C1_EV_IRQHandler(void) { Default_Handler(); }
__attribute__((weak)) void I2C1_ER_IRQHandler(void) { Default_Handler(); }
__attribute__((weak)) void I2C2_EV_IRQHandler(void) { Default_Handler(); }
__attribute__((weak)) void I2C2_ER_IRQHandler(void) { Default_Handler(); }
__attribute__((weak)) void SPI1_IRQHandler(void) { Default_Handler(); }
__attribute__((weak)) void SPI2_IRQHandler(void) { Default_Handler(); }
__attribute__((weak)) void USART1_IRQHandler(void) { Default_Handler(); }
__attribute__((weak)) void USART2_IRQHandler(void) { Default_Handler(); }
__attribute__((weak)) void USART3_IRQHandler(void) { Default_Handler(); }
__attribute__((weak)) void EXTI15_10_IRQHandler(void) { Default_Handler(); }
__attribute__((weak)) void RTC_Alarm_IRQHandler(void) { Default_Handler(); }
__attribute__((weak)) void OTG_FS_WKUP_IRQHandler(void) { Default_Handler(); }
__attribute__((weak)) void TIM8_BRK_TIM12_IRQHandler(void) { Default_Handler(); }
__attribute__((weak)) void TIM8_UP_TIM13_IRQHandler(void) { Default_Handler(); }
__attribute__((weak)) void TIM8_TRG_COM_TIM14_IRQHandler(void) { Default_Handler(); }
__attribute__((weak)) void TIM8_CC_IRQHandler(void) { Default_Handler(); }
__attribute__((weak)) void DMA1_Stream7_IRQHandler(void) { Default_Handler(); }
__attribute__((weak)) void FMC_IRQHandler(void) { Default_Handler(); }
__attribute__((weak)) void SDIO_IRQHandler(void) { Default_Handler(); }
__attribute__((weak)) void TIM5_IRQHandler(void) { Default_Handler(); }
__attribute__((weak)) void SPI3_IRQHandler(void) { Default_Handler(); }
__attribute__((weak)) void UART4_IRQHandler(void) { Default_Handler(); }
__attribute__((weak)) void UART5_IRQHandler(void) { Default_Handler(); }
__attribute__((weak)) void TIM6_DAC_IRQHandler(void) { Default_Handler(); }
__attribute__((weak)) void TIM7_IRQHandler(void) { Default_Handler(); }
__attribute__((weak)) void DMA2_Stream0_IRQHandler(void) { Default_Handler(); }
__attribute__((weak)) void DMA2_Stream1_IRQHandler(void) { Default_Handler(); }
__attribute__((weak)) void DMA2_Stream2_IRQHandler(void) { Default_Handler(); }
__attribute__((weak)) void DMA2_Stream3_IRQHandler(void) { Default_Handler(); }
__attribute__((weak)) void DMA2_Stream4_IRQHandler(void) { Default_Handler(); }
__attribute__((weak)) void CAN2_TX_IRQHandler(void) { Default_Handler(); }
__attribute__((weak)) void CAN2_RX0_IRQHandler(void) { Default_Handler(); }
__attribute__((weak)) void CAN2_RX1_IRQHandler(void) { Default_Handler(); }
__attribute__((weak)) void CAN2_SCE_IRQHandler(void) { Default_Handler(); }
__attribute__((weak)) void OTG_FS_IRQHandler(void) { Default_Handler(); }
__attribute__((weak)) void DMA2_Stream5_IRQHandler(void) { Default_Handler(); }
__attribute__((weak)) void DMA2_Stream6_IRQHandler(void) { Default_Handler(); }
__attribute__((weak)) void DMA2_Stream7_IRQHandler(void) { Default_Handler(); }
__attribute__((weak)) void USART6_IRQHandler(void) { Default_Handler(); }
__attribute__((weak)) void I2C3_EV_IRQHandler(void) { Default_Handler(); }
__attribute__((weak)) void I2C3_ER_IRQHandler(void) { Default_Handler(); }
__attribute__((weak)) void OTG_HS_EP1_OUT_IRQHandler(void) { Default_Handler(); }
__attribute__((weak)) void OTG_HS_EP1_IN_IRQHandler(void) { Default_Handler(); }
__attribute__((weak)) void OTG_HS_WKUP_IRQHandler(void) { Default_Handler(); }
__attribute__((weak)) void OTG_HS_IRQHandler(void) { Default_Handler(); }
__attribute__((weak)) void DCMI_IRQHandler(void) { Default_Handler(); }
__attribute__((weak)) void FPU_IRQHandler(void) { Default_Handler(); }
__attribute__((weak)) void SPI4_IRQHandler(void) { Default_Handler(); }
__attribute__((weak)) void SAI1_IRQHandler(void) { Default_Handler(); }
__attribute__((weak)) void SAI2_IRQHandler(void) { Default_Handler(); }
__attribute__((weak)) void QUADSPI_IRQHandler(void) { Default_Handler(); }
__attribute__((weak)) void CEC_IRQHandler(void) { Default_Handler(); }
__attribute__((weak)) void SPDIF_RX_IRQHandler(void) { Default_Handler(); }
__attribute__((weak)) void FMPI2C1_EV_IRQHandler(void) { Default_Handler(); }
__attribute__((weak)) void FMPI2C1_ER_IRQHandler(void) { Default_Handler(); }

// Vector Table, list of function pointers
__attribute__((section(".isr_vector"))) void (*Vector_Table[])(void) = {
    (void (*)(void)) & _estack,
    Reset_Handler,
    NMI_Handler,
    HardFault_Handler,
    MemManage_Handler,
    BusFault_Handler,
    UsageFault_Handler,
    0,
    0,
    0,
    0,
    SVC_Handler,
    DebugMon_Handler,
    0,
    PendSV_Handler,
    SysTick_Handler,
    WWDG_IRQHandler,
    PVD_IRQHandler,
    TAMP_STAMP_IRQHandler,
    RTC_WKUP_IRQHandler,
    FLASH_IRQHandler,
    RCC_IRQHandler,
    EXTI0_IRQHandler,
    EXTI1_IRQHandler,
    EXTI2_IRQHandler,
    EXTI3_IRQHandler,
    EXTI4_IRQHandler,
    DMA1_Stream0_IRQHandler,
    DMA1_Stream1_IRQHandler,
    DMA1_Stream2_IRQHandler,
    DMA1_Stream3_IRQHandler,
    DMA1_Stream4_IRQHandler,
    DMA1_Stream5_IRQHandler,
    DMA1_Stream6_IRQHandler,
    ADC_IRQHandler,
    CAN1_TX_IRQHandler,
    CAN1_RX0_IRQHandler,
    CAN1_RX1_IRQHandler,
    CAN1_SCE_IRQHandler,
    EXTI9_5_IRQHandler,
    TIM1_BRK_TIM9_IRQHandler,
    TIM1_UP_TIM10_IRQHandler,
    TIM1_TRG_COM_TIM11_IRQHandler,
    TIM1_CC_IRQHandler,
    TIM2_IRQHandler,
    TIM3_IRQHandler,
    TIM4_IRQHandler,
    I2C1_EV_IRQHandler,
    I2C1_ER_IRQHandler,
    I2C2_EV_IRQHandler,
    I2C2_ER_IRQHandler,
    SPI1_IRQHandler,
    SPI2_IRQHandler,
    USART1_IRQHandler,
    USART2_IRQHandler,
    USART3_IRQHandler,
    EXTI15_10_IRQHandler,
    RTC_Alarm_IRQHandler,
    OTG_FS_WKUP_IRQHandler,
    TIM8_BRK_TIM12_IRQHandler,
    TIM8_UP_TIM13_IRQHandler,
    TIM8_TRG_COM_TIM14_IRQHandler,
    TIM8_CC_IRQHandler,
    DMA1_Stream7_IRQHandler,
    FMC_IRQHandler,
    SDIO_IRQHandler,
    TIM5_IRQHandler,
    SPI3_IRQHandler,
    UART4_IRQHandler,
    UART5_IRQHandler,
    TIM6_DAC_IRQHandler,
    TIM7_IRQHandler,
    DMA2_Stream0_IRQHandler,
    DMA2_Stream1_IRQHandler,
    DMA2_Stream2_IRQHandler,
    DMA2_Stream3_IRQHandler,
    DMA2_Stream4_IRQHandler,
    0,
    0,
    CAN2_TX_IRQHandler,
    CAN2_RX0_IRQHandler,
    CAN2_RX1_IRQHandler,
    CAN2_SCE_IRQHandler,
    OTG_FS_IRQHandler,
    DMA2_Stream5_IRQHandler,
    DMA2_Stream6_IRQHandler,
    DMA2_Stream7_IRQHandler,
    USART6_IRQHandler,
    I2C3_EV_IRQHandler,
    I2C3_ER_IRQHandler,
    OTG_HS_EP1_OUT_IRQHandler,
    OTG_HS_EP1_IN_IRQHandler,
    OTG_HS_WKUP_IRQHandler,
    OTG_HS_IRQHandler,
    DCMI_IRQHandler,
    0,
    0,
    FPU_IRQHandler,
    0,
    0,
    SPI4_IRQHandler,
    0,
    0,
    SAI1_IRQHandler,
    0,
    0,
    0,
    SAI2_IRQHandler,
    QUADSPI_IRQHandler,
    CEC_IRQHandler,
    SPDIF_RX_IRQHandler,
    FMPI2C1_EV_IRQHandler,
    FMPI2C1_ER_IRQHandler,
};