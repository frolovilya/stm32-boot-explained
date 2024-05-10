# stm32-memory-explained

A detailed explanation of the STM32 memory mapping and bootloader process.

Even though all examples below are for STM32F446, basic principles apply to pretty much all MCUs.

## Linker File

STM32CubeMX produces [STM32F446RETx_FLASH.ld](./src/STM32F446RETx_FLASH.ld) which we'll be looking at as a reference for all our explorations.

The linker takes the object files produced by the compiler and makes a final compilation output, _.elf_ binary in our case.
The linker is always using a linker script file. Even if you don't specify any, a default one is used. 

Important to note, that the linker script only describes the memory based on the MCU specifications and doesn't alter any hardware memory adressing.

## Memory

Program memory, data memory, registers and I/O ports in STM32F4 are organized within the same linear address space.

```c
         ...
----- 0x2001FFFF -----
|                    |
|   RAM              |
|                    |
----- 0x20000000 -----
         ...          
----- 0x1FFF7A0F -----
|                    |
|   System           |
|                    |
----- 0x1FFF0000 -----
         ...
----- 0x081FFFFF -----
|                    |
|   Flash            |
|                    |
----- 0x08000000 -----
         ...
----- 0x001FFFFF -----
|                    |
|   Alias            |
|                    |
----- 0x00000000 -----
```

Where Alias memory is pointing to Flash, System or RAM memory depending on the `BOOT0` pin. By default it's FLASH.

Two spaces are of the most interest for us for now:

* **RAM** stores data produced by the running programm. Using heap or stack abstractions for memory management. The data isn't persisted.
* **FLASH** keeps the program binary and constants, used by bootloader to initialize RAM on startup. The data is persisted.

Though the functional division could be any in practice. You may want to load a program binary data into RAM instead and vice versa.

Memory structure is reflected in the linker file. FLASH starts at `ORIGIN = 0x8000000` and RAM at `ORIGIN = 0x20000000`.

```c
// linker file
MEMORY
{
  RAM (xrw) : ORIGIN = 0x20000000, LENGTH = 128K
  FLASH (rx) : ORIGIN = 0x8000000, LENGTH = 512K
}
```

Note that it's just a default implementation, you could easily split FLASH and RAM and add your blocks or sections as soon as they follow the MCU memory specification.


## Sections

Memory is then split in the linker file to sections, each having it's address and destination (FLASH or RAM).

```c
// linker file
SECTIONS
{
  ...
  .text :
  {
    ...
  } >FLASH
  ...
}
```


To explore .elf symbol table we'll be using _arm-none-eabi-objdump_ command and sort all entries by their addresses:
```sh
arm-none-eabi-objdump -t stm32-memory-explained.elf | sort
```

The program produces output as described in details [here](https://manpages.debian.org/unstable/binutils-arm-none-eabi/arm-none-eabi-objdump.1.en.html):
```c
Address Flag_Bits Section Size Name
```

### .text 
Compiled programm code goes into this section in **FLASH** memory.

The section starts from the flash's `ORIGIN` address. And `_etext` points to the last address of the section.

```c
08000000 l    d  .text	00000000 .text
08000c00 g       .text	00000000 _etext
```

We could see the location of the functions, that we use in our bootloader file:

```c
080005c8 g     F .text	00000048 __libc_init_array
08000610 g     F .text	000000d8 main
080006e8 g     F .text	00000064 Reset_Handler
08000bc4 g     F .text	00000024 SystemInit
```

### .data

The section resides in **RAM** memory and keeps all _defined_ variables values. Address range spans from `_sdata` to `_edata`.


```c
20000000 g       .data	00000000 _sdata
20000010 g       .data	00000000 _edata
```

Our defined variables from [main.c](./src/main.c) are residing in this section:
```c
20000000 l     O .data	00000004 static_defined_int
20000008 g     O .data	00000008 defined_double
```

Actual values must be filled by a bootloader script by copying data from **FLASH** memory at address `_sidata`:
```c
08000c08 g       *ABS*	00000000 _sidata
```

### .bss

Block Starting Symbol **RAM** data section for all _declared_ variables. That means that they have no value yet and the bootloader takes care of setting this block's data to `0`.

Memory range spans from `_sbss` to `_ebss`.
```c
20000010 g       .bss	00000000 _sbss
20000044 g       .bss	00000000 _ebss
```

Our declared variables from [main.c](./src/main.c) are residing in this section:
```c
2000002c l     O .bss	00000004 static_declared_int
20000030 g     O .bss	00000008 declared_double
20000038 g     O .bss	00000008 declared_my_struct
20000040 g     O .bss	00000004 declared_my_union
```

Let's check with _gdb_:
```sh
(gdb) p declared_double
$1 = 0
(gdb) p &declared_double
$2 = (double *) 0x20000030 <declared_double>
```

Once you define the variable, means you assign some value to it, it's address doesn't change:

```sh
(gdb) p declared_double
$3 = 86
(gdb) p &declared_double
$4 = (double *) 0x20000030 <declared_double>
```

### ._user_heap_stack

All **RAM** memory above `_end` and until `_estack` is dedicated to heap and stack memory.

```c
20000048 g       ._user_heap_stack	00000000 _end
20020000 g       .isr_vector	00000000 _estack
```

`_estack` address is calculated as `ORIGIN(RAM) + LENGTH(RAM)`.
So that for 128Kb RAM:
```c
_estack = 0x20000000 + 128 * 1024 # dec
        = 0x20000000 + 0x20000 # hex
        = 0x20020000
```

Stack is a LIFO structure that starts at `_estack` and grows downwards. Min stack size is defined in the linker file as `_Min_Stack_Size`. Stack memory is freed automatically.

```c
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

Heap in turn starts from `_end` and grows upwards up to `_estack - _Min_Stack_Size`. Heap memory is managed by `malloc` and `free` C functions. See the default implementation for `sbrk`, which is called by `malloc` in [sysmem.c](./src/sysmem.c).

## Boot Process

Let's connect to our programm with _gdb_, here's what we see as the first output:
```sh
...
Reading symbols from ./stm32-memory-explained.elf...
Remote debugging using localhost:61234
Reset_Handler () at %PATH%/bootloader.c:25
25	void Reset_Handler() {
(gdb) info registers
...
pc             0x80006e8           0x80006e8 <Reset_Handler>
...
```

`Reset_Handler` was somehow identified as a boot point for our application.

### Linker Entry Point

You may have noticed, that there's an `ENTRY` instruction on the linker file:
```c
// linker file
ENTRY(Reset_Handler)
```

Actually it saves a reference to the `Reset_Handler` function address to the _.elf_ file header:

```sh
arm-none-eabi-objdump -t -f stm32-memory-explained.elf | grep "start address"

start address 0x080006e9
```

Looking at our symbol table, `Reset_Handler` is present in the FLASH `.text` section as all other functions:
```c
080006e8 g     F .text	00000064 Reset_Handler
```

Addresses are page aligned, that's why there's a possibility for _.elf_ header and actual address mismatch.

Though this information is used mostly by the linker to check if the entry point symbol really exist somewhere in the code. It has no practical meaning for the MCU.

### Alias Memory

Based on the STM32 specification, the CPU fetches the top-of-stack value from address `0x00000000`, then starts code execution from the boot memory starting from `0x00000004`.

```c
----- 0x001FFFFF -----
|                    |
|   Alias            |
|                    |
----- 0x00000000 -----
```

It's exactly where Alias memory mentioned above is defined. With the default configuration, when `BOOT0 = 0`, it's aliasing to the FLASH memory block starting from `0x8000000`.

Other options based on the `BOOT0` and `BOOT1` include System memory with an embedded bootloader or RAM memory.
The embedded bootloader is programmed by ST during production and out of scope for now.

But `Reset_Handler` has an address `0x08000524` which is not exactly the beginning of the FLASH memory, how does the MCU find the bootstrap method then?

### Vector Table

Here's where Vector Table comes into play.
```c
08000000 g     O .isr_vector	000001c4 Vector_Table
```

Rows in the table are addresses of the MCU hard-defined functions for various events and interrupts. Consult the spec to see all events and interrupts supported. Actual table must be filled by the bootloader.

| Address    | Name                   |
|------------|------------------------|
| 0x00000000 | Reserved               |
| 0x00000004 | **Reset Handler**      |
| 0x00000008 | Non Maskable Interrupt |
| 0x00000012 | Hard Fault             |
| ...        | Other Interrupts       |


### Bootloader 

This `Reset_Handler` is a bootloader function that could be used for many applications from security-specific to auto updating the firmware. Here we'll explore the basic default implementation to understand how it interacts with the MCUs memory.

By default `Reset_Handler` method is declared in the _startup_stm32f436xx.s_ ASM file provided by STM23CubeMX. Actual [bootloader.c](./src/bootloader.c) implementation in this project is written in C for clarity.

Note that variables defined in the linker script could be accessed in C code:
```c
extern uint32_t _estack;
```

So that it's easily possible to replicate the ASM version.

Minimal loading process could be split into the following steps:

1. Setup the microcontroller system, initialize the FPU setting, vector table location and External memory configuration (`SystemInit()` function)
2. Copy the `.data` segment initializers from FLASH to RAM
3. Zero fill the `.bss` segment
4. Call static constructors (`__libc_init_array()` function)
5. Call the application's entry point (`main()` function)

For steps #1 and #4 STM32CubeMX provides function implementations, you could check details in [system_stm32f4xx.c](./src/system_stm32f4xx.c).

## C stdlib

TODO

## Try It Yourself

The project has a minimal set of files required to boot up the STM32. You may want to try it yourself to check _arm-none-eabi-objdump_ and step through _gdb_.

### Build
[ARM GNU Toolchain](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads) is required to build the project.

It's recommended to install an all-in-one [STM32CubeCLT](https://www.st.com/en/development-tools/stm32cubeclt.html?rt=um&id=UM3088) command tools package with _arm-none-eabi-gcc_, _STM32_Programmer_CLI_ and _ST-LINK_gdbserver_ tools included.

Build the project using _CMake_:

```sh
mkdir build; cd build

cmake ../ -DPROGRAMMER_CLI=/opt/ST/STM32CubeCLT_1.15.1/STM32CubeProgrammer/bin \
    -DGDB_SERVER=/opt/ST/STM32CubeCLT_1.15.1/STLink-gdb-server/bin

make VERBOSE=1
```

Note that the last command is a linking stage. If we strip all other compiler flags, the command could look like below. Here's where the linker is instructed to use our linker script.

```sh
arm-none-eabi-gcc  
    ...
    -T "%SCRIPT_DIR%/STM32F446RETx_FLASH.ld"  
    ...
    "%OBJ_DIR%/%OBJECT_NAME%.c.obj" 
    ...
    -o stm32-memory-explained.elf
```

### Upload

_STM32_Programmer_CLI_ is preconfigured for SWD procotol, just run:
```sh
make flash
```

Take a note on the programmer output. It's using `0x08000000` address as a starting point:

```sh
...
Memory Programming ...
Opening and parsing file: stm32-memory-explained.elf
  File          : stm32-memory-explained.elf
  Size          : 1,46 KB
  Address       : 0x08000000
...
```

It's actually the FLASH memory starting address that we already know.

### Debug 

There's a custom target pre-configured to run _ST-LINK_gdbserver_:

```sh
# start ST-Link gdb server
make gdb-server

# connect with gdb debugger
gdb -ex 'target remote localhost:61234' ./stm32-memory-explained.elf
```