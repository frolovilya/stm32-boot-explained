#include "rcc.h"
#include <stm32f446xx.h>

/**
 * Configure oscillator and scale system clocks.
 * 
 * System clock 84MHz
 * 
 * APB1 peripherals scaled to 42MHz
 * APB1 timer 84MHz
 * 
 * APB2 peripherals and timer 42MHz
*/
void configure_clocks() {
  RCC->CR |= RCC_CR_HSION; // enable internal 16MHz oscillator
  while (!(RCC->CR & RCC_CR_HSIRDY))
    ;

  FLASH->ACR &= ~FLASH_ACR_LATENCY; // set latency to 2 cycles
  FLASH->ACR |= FLASH_ACR_LATENCY_2WS;

  RCC->PLLCFGR &= ~(RCC_PLLCFGR_PLLSRC | RCC_PLLCFGR_PLLM | RCC_PLLCFGR_PLLN |
                    RCC_PLLCFGR_PLLP);
  RCC->PLLCFGR |= RCC_PLLCFGR_PLLSRC_HSI;      // internal clock as PLL source
  RCC->PLLCFGR |= 16 << RCC_PLLCFGR_PLLM_Pos;  // divided before VCO
  RCC->PLLCFGR |= 336 << RCC_PLLCFGR_PLLN_Pos; // multiplied at VCO
  RCC->PLLCFGR |= 1 << RCC_PLLCFGR_PLLP_Pos;   // divided after VCO (= 84MHz)
  RCC->PLLCFGR &= ~(RCC_PLLCFGR_PLLQ | RCC_PLLCFGR_PLLR);

  RCC->CR |= RCC_CR_PLLON; // enable PLL
  while (!(RCC->CR & RCC_CR_PLLRDY))
    ;

  RCC->CFGR &= ~(RCC_CFGR_SW | RCC_CFGR_HPRE | RCC_CFGR_PPRE1 | RCC_CFGR_PPRE2);
  RCC->CFGR |= RCC_CFGR_SW_PLL;     // PLL clock as a system source
  RCC->CFGR |= RCC_CFGR_HPRE_DIV1;  // AHB prescaler by 1
  RCC->CFGR |= RCC_CFGR_PPRE1_DIV2; // APB1 prescaler by 2 (= 42MHz)
  RCC->CFGR |= RCC_CFGR_PPRE2_DIV1; // APB2 prescaler by 1
  while (!(RCC->CFGR & RCC_CFGR_SWS_PLL))
    ;

  SystemCoreClockUpdate();
}
