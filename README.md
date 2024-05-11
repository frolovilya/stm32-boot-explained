# stm32-boot-explained

A detailed explanation of the STM32 memory mapping and bootloading process.

While all the examples below are for STM32F446, the basic principles apply to most MCUs.

## Linker File

STM32CubeMX produces [STM32F446RETx_FLASH.ld](./src/STM32F446RETx_FLASH.ld) which we'll be looking at as a reference for all our explorations.

The linker takes the object files produced by the compiler and generates the final compilation output, which in our case is the _.elf_ binary.
The linker always uses a linker script file; even if you don't specify one, a default script is used.

Important to note, that the linker script only describes the memory based on the MCU specifications and does not alter any hardware memory adressing.

## Memory

We can't talk about bootloader process without understanding the memory structure. Actually it's the purpose of the bootloader to have the memory in a state ready to execute our application's `main()` method.

Program memory, data memory, registers and I/O ports in STM32F4 are organized within the same linear address space.

```c
         ...
+---- 0x2001FFFF ----+
|                    |
|   RAM              |
|                    |
+---- 0x20000000 ----+
|        ...         |
+---- 0x1FFF7A0F ----+
|                    |
|   System           |
|                    |
+---- 0x1FFF0000 ----+
|        ...         |
+---- 0x081FFFFF ----+
|                    |
|   Flash            |
|                    |
+---- 0x08000000 ----+
|        ...         |
+---- 0x001FFFFF ----+
|                    |
|   Alias            |
|                    |
+---- 0x00000000 ----+
```

Where Alias memory is pointing to Flash, System or RAM memory depending on the `BOOT0` pin. By default it's FLASH.

Two spaces are of the most interest for us for now:

* **RAM** stores data produced by the running programm, utilizing heap or stack abstractions for memory management. The data isn't persisted.
* **FLASH** keeps the program binary, constants and initial variables values, used by bootloader to initialize RAM on startup. The data is persisted.

However, in practice, the functional division could vary. For instance, you may need to load program binary data into RAM or vice versa.

Memory structure is reflected in the linker file. FLASH starts at `ORIGIN = 0x8000000` and RAM at `ORIGIN = 0x20000000`.

```c
// linker file
MEMORY
{
  RAM (xrw) : ORIGIN = 0x20000000, LENGTH = 128K
  FLASH (rx) : ORIGIN = 0x8000000, LENGTH = 512K
}
```

Note that this is just a default implementation, you could easily split FLASH and RAM and add your own blocks or sections as soon as they adhere to the MCU memory specification.


## Sections

Memory is then split in the linker file into sections, each having its address and destination (for instance, FLASH or RAM).

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


To explore _.elf_ symbol table we'll be using _arm-none-eabi-objdump_ command and _sort_ all entries by their addresses:
```sh
arm-none-eabi-objdump -t stm32-boot-explained.elf | sort
```

The program produces output as described in details [in the manual](https://manpages.debian.org/unstable/binutils-arm-none-eabi/arm-none-eabi-objdump.1.en.html):
```c
Address Flag_Bits Section Size Name
```

### .text 
Compiled programm code goes into this section in **FLASH** memory.

The section starts from the flash's `ORIGIN` address, and `_etext` points to the last address of the section.

```c
08000000 l    d  .text	00000000 .text
08000c00 g       .text	00000000 _etext
```

We can see the location of the functions, that we use in our bootloader file:

```c
080005c8 g     F .text	00000048 __libc_init_array
08000610 g     F .text	000000d8 main
080006e8 g     F .text	00000064 Reset_Handler
08000bc4 g     F .text	00000024 SystemInit
```

### .data

The section resides in **RAM** memory and holds all variables with defined values. The address range spans from `_sdata` to `_edata`.


```c
20000000 g       .data	00000000 _sdata
20000010 g       .data	00000000 _edata
```

Let's check what variables from [main.c](./src/main.c) reside in this section:

```c
20000000 l     O .data	00000004 static_data_int
20000008 g     O .data	00000008 data_double
```

Actual values must be filled by a bootloader script by copying data from **FLASH** memory at address `_sidata`:
```c
08000c08 g       *ABS*	00000000 _sidata
```

### .bss

Block Starting Symbol **RAM** data section for all variables without assigned value. The bootloader takes care of setting this block's data to `0`.

Memory range spans from `_sbss` to `_ebss`.
```c
20000010 g       .bss	00000000 _sbss
20000044 g       .bss	00000000 _ebss
```

All variables from [main.c](./src/main.c) without explicit value reside in this section:
```c
2000002c l     O .bss	00000004 static_bss_int
20000030 g     O .bss	00000008 bss_double
20000038 g     O .bss	00000008 bss_my_struct
20000040 g     O .bss	00000004 bss_my_union
```

Let's check with _gdb_:
```sh
(gdb) p bss_double
$1 = 0
(gdb) p &bss_double
$2 = (double *) 0x20000030 <bss_double>
```

Once you assign some value to the variable, its address doesn't change and it still stays in `.bss`:

```sh
(gdb) p bss_double
$3 = 86
(gdb) p &bss_double
$4 = (double *) 0x20000030 <bss_double>
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

Stack is a LIFO structure that starts at `_estack` and grows downwards. The minimum stack size is defined in the linker file as `_Min_Stack_Size`. Stack memory is  automatically freed.

```c
+---- 0x20020000 ----+ <-- _estack
|                    |
|     Stack          |
|                    |
+ - - 0x2001FC00 - - + <-- -_Min_Stack_Size
|                    |
+---- 0x200sssss ----+ <-- $msp register
|                    |
|                    |
|     Free space     |
|                    |
|                    |
+---- 0x200hhhhh ----+ <-- Actual heap end
|                    |
|     Heap           |
|                    |
+ - - 0x20000248 - - + <-- +_Min_Heap_Size
|                    |
+---- 0x20000048 ----+ <-- _end
```

Heap in turn starts from `_end` and grows upwards up to `_estack - _Min_Stack_Size` when requested by `malloc`.

We have a `stack_int` variable defined in [main.c](./src/main.c). Let's check the address with _gdb_ after it's been initialized:

```sh
(gdb) p &stack_int
$1 = (unsigned short *) 0x2001ffd6

(gdb) p $msp
$2 = (void *) 0x2001ffd0
```

Which matches with our expectations, when the variable is above the `$msp`.


## C stdlib

It's worth mentioning that the standard C library implementation commonly used in embedded C applications is [Newlib](https://sourceware.org/newlib/).
This library requires the implementation of certain system-specific functions. STM32CubeMX generates the [syscalls.c](./src/syscalls.c) file with the necessary default implementations.

It's also the case for `sbrk` call that increases program data space. `malloc` is using this function to allocate more heap memory. You may find an implementation generated by STM32CubeMX in [sysmem.c](./src/sysmem.c). It simply allows the heap to grow from `_end` up to `_estack - _Min_Stack_Size`.

## Boot Process

Now when we understand the MCUs memory, let's connect to our programm with _gdb_, here's what we see as the first output:
```sh
...
Reading symbols from ./stm32-boot-explained.elf...
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

You may have noticed that there's an `ENTRY` instruction in the linker file:
```c
// linker file
ENTRY(Reset_Handler)
```

Actually, it saves a reference to the `Reset_Handler` function address in the _.elf_ file header:

```sh
arm-none-eabi-objdump -t -f stm32-boot-explained.elf | grep "start address"

start address 0x080006e9
```

Looking at our symbol table, `Reset_Handler` is present in the FLASH `.text` section, just like all other functions:
```c
080006e8 g     F .text	00000064 Reset_Handler
```

Addresses are page aligned, which is why there's a possibility for a mismatch between the _.elf_ header and the actual address.

Though this information is mostly used by the linker to verify the existence of the entry point symbol within the code, it has no practical meaning for the MCU.

### Alias Memory

According to the STM32 specification, the CPU fetches the top-of-stack `_estack` value from address `0x00000000`, then begins code execution from the boot memory starting at `0x00000004`.

```c
+---- 0x001FFFFF ----+
|                    |
|   Alias            |
|                    |
+---- 0x00000000 ----+
```

This is exactly where the Alias memory mentioned above is defined. With the default configuration, when pin value `BOOT0 = 0`, it aliases to the FLASH memory block starting at `0x8000000`.

Other options based on the `BOOT0` and `BOOT1` include System memory with an embedded bootloader or RAM memory.
The embedded bootloader is programmed by ST during production and out of scope of this manual.

But `Reset_Handler` has an address `0x08000524` which is not exactly the beginning of the FLASH memory, how does the MCU find the bootstrap method then?

### Vector Table

Here's where Vector Table comes into play. 

```c
08000000 g     O .isr_vector	000001c4 Vector_Table
```

MCU treats the beginning of memory as a Vector Table, that contains pointers to various interrupt service routines and essential startup functions, including the `Reset_Handler`. Consult the spec to see exact table structure that MCU expects to load from `0x00000000`. Actual table must be filled by the bootloader.

| Address    | Name                   |
|------------|------------------------|
| 0x00000000 | Reserved               |
| 0x00000004 | **Reset Handler**      |
| 0x00000008 | Non Maskable Interrupt |
| 0x00000012 | Hard Fault             |
| ...        | Other Interrupts       |


### Bootloader 

This `Reset_Handler` is a bootloader function that can be used for many applications, from security-specific tasks to firmware auto-updating. Here, we'll explore the basic default implementation to understand its interaction with the MCU's memory.

By default `Reset_Handler` method is defined in the _startup_stm32f436xx.s_ ASM file provided by STM23CubeMX. Actual [bootloader.c](./src/bootloader.c) implementation in this project is written in C for clarity.

Note that variables defined in the linker script can be accessed in C code:
```c
extern uint32_t _estack;
```

So that it's easily possible to replicate the ASM version.

The minimal loading process could be split into the following steps:

1. Setup the microcontroller system, initialize the FPU setting, vector table location and External memory configuration (`SystemInit()` function)
2. Copy the `.data` segment initializers from FLASH to RAM
3. Zero fill the `.bss` segment
4. Call static constructors (`__libc_init_array()` function)
5. Call the application's entry point (`main()` function)

For steps #1 and #4, STM32CubeMX provides function implementations, you can check details in [system_stm32f4xx.c](./src/system_stm32f4xx.c).

## Try It Yourself

The project has a minimal set of files required to boot up the STM32. You may want to try it yourself to check the output of _arm-none-eabi-objdump_ and step through with _gdb_.

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

Note that the last command is the linking stage. If we strip all other compiler flags, the command could look like the following. This is where the linker is instructed to use our linker script.

```sh
arm-none-eabi-gcc  
    ...
    -T "%SCRIPT_DIR%/STM32F446RETx_FLASH.ld"  
    ...
    "%OBJ_DIR%/%OBJECT_NAME%.c.obj" 
    ...
    -o stm32-boot-explained.elf
```

### Upload

_STM32_Programmer_CLI_ is preconfigured for SWD procotol, just run:
```sh
make flash
```

Take a note on the programmer output, which is using `0x08000000` address as a starting point:

```sh
...
Memory Programming ...
Opening and parsing file: stm32-boot-explained.elf
  File          : stm32-boot-explained.elf
  Size          : 1,46 KB
  Address       : 0x08000000
...
```

It's actually the starting address of the FLASH memory, which we already know.

### Debug 

There's a custom target pre-configured to run _ST-LINK_gdbserver_:

```sh
# start ST-Link gdb server
make gdb-server

# connect with gdb debugger
gdb -ex 'target remote localhost:61234' ./stm32-boot-explained.elf
```
