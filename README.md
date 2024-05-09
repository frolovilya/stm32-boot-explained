# stm32-memory

STM32 memory and bootloader process explained.

## Build
Install [ARM GNU Toolchain](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads), on Mac could be done with _brew_:
```sh
brew install --cask gcc-arm-embedded
```

Install [STM32CubeProgrammer](https://www.st.com/en/development-tools/stm32cubeprog.html) to flash STM32 via ST-Link.

There's also an all-in-one [STM32CubeCLT](https://www.st.com/en/development-tools/stm32cubeclt.html?rt=um&id=UM3088) command tools package available.

Build the project using CMake:

```sh
mkdir build; cd build

cmake ../ -DPROGRAMMER_CLI=/Applications/STMicroelectronics/STM32CubeProgrammer/STM32CubeProgrammer.app/Contents/MacOs/bin/STM32_Programmer_CLISTM32_Programmer_CLI

make VERBOSE=1
```

Not that the last command is a linking stage. If we strip all other compiler flags, the command could look like below. Basically we istruct the linker to link all .obj files into .elf binary by using provided linker script.

```sh
arm-none-eabi-gcc  
    ...
    -T "%SCRIPT_DIR%/STM32F446RETx_FLASH.ld"  
    "%OBJ_DIR%/main.c.obj" 
    "%OBJ_DIR%/bootloader.c.obj" 
    "%OBJ_DIR%/syscalls.c.obj" 
    "%OBJ_DIR%/sysmem.c.obj" 
    "%OBJ_DIR%/system_stm32f4xx.c.obj" 
    -o stm32-memory.elf
```

## Linker File

The linker takes the object files produced by the compiler and makes a final compilation output, .elf binary in our case.
The linker is always using a linker script file. Even if you don't specify any, a default one is used. 

STM32CubeMX produces [STM32F446RETx_FLASH.ld](./src/STM32F446RETx_FLASH.ld) which we'll be looking at as a reference for all out explorations below.

```c
/* Entry Point */
ENTRY(Reset_Handler)

...

/* Specify the memory areas */
MEMORY
{
  RAM (xrw) : ORIGIN = 0x20000000, LENGTH = 128K
  FLASH (rx) : ORIGIN = 0x8000000, LENGTH = 512K
}

/* Define output sections */
SECTIONS
{
    ...
}
```

## Memory

There're two basic memory peripherals present in SMT32F4:

* **RAM** stores data produced by the running programm. Using heap or stack abstractions for memory management.
* **FLASH** keeps the program binary and constants, used by bootloader to initialize RAM on startup.

In the address space, FLASH starts at `ORIGIN = 0x8000000` and RAM at `ORIGIN = 0x20000000`.

Even though all examples below are for STM32F4, basic principles apply to pretty much all MCUs.

## Sections

To explore .elf symbol table we'll be using the following command:
```sh
arm-none-eabi-objdump -t stm32-memory.elf | sort
```

The program produces output in the following [format](https://manpages.debian.org/unstable/binutils-arm-none-eabi/arm-none-eabi-objdump.1.en.html):
```c
Address Flag_Bits Section Size Name
```


### .text 
Compiled programm code goes into this section in **FLASH** memory.

The section starts from the flash's `ORIGIN` address. And `_etext` points to the last address of the section.

```c
08000000 l    d  .text	00000000 .text
080005c4 g       .text	00000000 _etext
```

We could see the location of the functions, that we use in our bootloader file:

```c
08000404 g     F .text	00000048 __libc_init_array
0800044c g     F .text	000000d8 main
08000524 g     F .text	00000064 Reset_Handler
08000588 g     F .text	00000024 SystemInit
```

### .data

The section resides in **RAM** memory and keeps all _defined_ variables values. Address range spans from `_sdata` to `_edata`.


```c
20000000 g       .data	00000000 _sdata
20000010 g       .data	00000000 _edata
```

Our defined variables from `main.c` are residing in this section:
```c
20000000 l     O .data	00000004 static_defined_int
20000008 g     O .data	00000008 defined_double
```

Actual values must be filled by a bootloader script by copying data from **FLASH** memory at address `_sidata`:
```c
080005cc g       *ABS*	00000000 _sidata
```

### .bss

Block Starting Symbol **RAM** data section for all _declared_ variables. That means that they have no value yet and the bootloader takes care of setting this block's data to `0`.

Memory range spans from `_sbss` to `_ebss`.
```c
20000010 g       .bss	00000000 _sbss
20000044 g       .bss	00000000 _ebss
```

Our declared variables from `main.c` are residing in this section:
```c
2000002c l     O .bss	00000004 static_declared_int
20000030 g     O .bss	00000008 declared_double
20000038 g     O .bss	00000008 declared_my_struct
20000040 g     O .bss	00000004 declared_my_union
```

### ._user_heap_stack

All **RAM** memory above `_end` and until `_estack` is dedicated to heap and stack memory.

```c
20000048 g       ._user_heap_stack	00000000 _end
20020000 g       *ABS*	00000000 _estack
```

`_estack` address is calculated as `ORIGIN(RAM) + LENGTH(RAM)`.
So that for 128Kb RAM:
```c
_estack = 0x20000000 + 128 * 1024 # dec
        = 0x20000000 + 0x20000 # hex
        = 0x20020000
```

Stack is a LIFO structure that starts at `_estack` and grows downwards. Min stack size is defined in the linker file as `_Min_Stack_Size`. Stack memory is freed automatically.

```
----- 0x20020000 -----
|                    |
|     Stack          |
|                    |
----- 0x200xxxxx -----
|                    |
|                    |
|     Free space     |
|                    |
|                    |
----- 0x200yyyyy -----
|                    |
|     Heap           |
|                    |
----- 0x20000048 -----
```

Heap in turns starts from `_end` and grows upwards up to `_estack - _Min_Stack_Size`. Heap memory is managed by `malloc` and `free` C functions. See the default implementation for `sbrk`, which is called by `malloc` in [sysmem.c](./src/sysmem.c).

## Boot Process