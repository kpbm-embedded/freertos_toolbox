# Shared: HAL + CMSIS + startup for STM32F411CEU6 from STM32CubeF4 pack.
# Defines target stm32f411_hal and helper stm32f411_finish(<target>).
set(CUBE ${CMAKE_CURRENT_LIST_DIR}/../../STM32CubeF4 CACHE PATH "STM32CubeF4")
get_filename_component(CUBE ${CUBE} ABSOLUTE)
set(F4_COMMON ${CMAKE_CURRENT_LIST_DIR})
set(MCU_FLAGS -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard)
set(HALSRC ${CUBE}/Drivers/STM32F4xx_HAL_Driver/Src)
add_library(stm32f411_hal STATIC
  ${HALSRC}/stm32f4xx_hal.c ${HALSRC}/stm32f4xx_hal_cortex.c ${HALSRC}/stm32f4xx_hal_dma.c
  ${HALSRC}/stm32f4xx_hal_flash.c ${HALSRC}/stm32f4xx_hal_flash_ex.c ${HALSRC}/stm32f4xx_hal_gpio.c
  ${HALSRC}/stm32f4xx_hal_pwr.c ${HALSRC}/stm32f4xx_hal_pwr_ex.c ${HALSRC}/stm32f4xx_hal_rcc.c
  ${HALSRC}/stm32f4xx_hal_rcc_ex.c ${HALSRC}/stm32f4xx_hal_uart.c
  ${CUBE}/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/system_stm32f4xx.c
  ${CUBE}/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/startup_stm32f411xe.s)
target_compile_definitions(stm32f411_hal PUBLIC STM32F411xE USE_HAL_DRIVER HSE_VALUE=25000000U)
target_include_directories(stm32f411_hal PUBLIC ${F4_COMMON}
  ${CUBE}/Drivers/STM32F4xx_HAL_Driver/Inc ${CUBE}/Drivers/CMSIS/Device/ST/STM32F4xx/Include
  ${CUBE}/Drivers/CMSIS/Core/Include)
target_compile_options(stm32f411_hal PUBLIC ${MCU_FLAGS} -Os -g -ffunction-sections -fdata-sections -Wall)
target_link_options(stm32f411_hal PUBLIC ${MCU_FLAGS} -T${F4_COMMON}/STM32F411CEUx_FLASH.ld
  --specs=nano.specs --specs=nosys.specs -Wl,--gc-sections -Wl,-Map=${CMAKE_BINARY_DIR}/app.map -Wl,--print-memory-usage)
function(stm32f411_finish tgt)
  set_target_properties(${tgt} PROPERTIES SUFFIX .elf)
  add_custom_command(TARGET ${tgt} POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${tgt}> ${tgt}.bin
    COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:${tgt}> ${tgt}.hex
    COMMAND ${CMAKE_SIZE} $<TARGET_FILE:${tgt}>)
endfunction()
