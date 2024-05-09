#include <stdint.h>
#include "system_stm32f4xx.h"

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

extern void __libc_init_array();
extern int main();

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