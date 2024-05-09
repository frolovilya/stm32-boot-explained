cmake_minimum_required(VERSION 3.22)

project(stm32-drivers)
add_library(stm32-drivers INTERFACE)

# Enable CMake support for ASM and C languages
enable_language(C ASM)

target_compile_definitions(stm32-drivers INTERFACE 
	STM32F446xx
    $<$<CONFIG:Debug>:DEBUG>
)

target_include_directories(stm32-drivers INTERFACE
    ./CMSIS/Include
    ./CMSIS/Device/ST/STM32F4xx/Include
)
