# Source files
add_executable(${PROJECT_NAME}
        main.cpp

        i2c/PicoI2C.cpp

        uart/PicoOsUart.cpp
        uart/PicoOsUart.hpp

        semaphore/Binary.cpp
        semaphore/Mutex.cpp

        task/BaseTask.cpp
        task/BlinkTask.cpp

        task/MHZTask.cpp
        task/MHZTask.h


        Display-lib/SSD1306_OLED.hpp
        Display-lib/SSD1306_OLED.cpp
        Display-lib/SSD1306_OLED_font.hpp
        Display-lib/SSD1306_OLED_font.cpp
        Display-lib/SSD1306_OLED_graphics.hpp
        Display-lib/SSD1306_OLED_graphics.cpp
        Display-lib/SSD1306_OLED_Print.hpp
        Display-lib/SSD1306_OLED_Print.cpp

        uart/PicoOsUart.cpp
        sensors/RTClib.h
        sensors/RTClib.cpp
        sensors/SPS30.h
        task/I2CTask.cpp
        task/I2CTask.h
        sensors/SensDust.cpp
        sensors/SensDust.h
        sensors/MS430.cpp
        sensors/MS430.h
        sensors/RTCModule.cpp
        sensors/RTCModule.h
        sensors/MHZ19CSensor.cpp
        sensors/MHZ19CSensor.h


)

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        config
        uart
)

# Compile definitions
target_compile_definitions(${PROJECT_NAME} PRIVATE
        NO_SYS=0 # don't want NO_SYS (generally this would be in your lwipopts.h)

        # PICO_CYW43_ARCH_DEFAULT_COUNTRY_CODE=CYW43_COUNTRY_FINLAND
)

# Compiler options
target_compile_options(${PROJECT_NAME} PRIVATE
        -Wall
        -Wno-format # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
        -Wno-unused-function # we have some for the docs that aren't called
        -Wno-maybe-uninitialized
)

# Linked libraries
target_link_libraries(${PROJECT_NAME}
        pico_stdlib
        hardware_i2c
        FreeRTOS-Kernel-Heap4
        pico_cyw43_arch_lwip_sys_freertos
        pico_lwip_mbedtls
        pico_mbedtls

)

# # Ignore warnings from lwip code
set_source_files_properties(
        ${PICO_LWIP_PATH}/src/apps/altcp_tls/altcp_tls_mbedtls.c
        PROPERTIES
        COMPILE_OPTIONS "-Wno-unused-result"
)

pico_add_extra_outputs(${PROJECT_NAME})

# Enable usb output, disable uart output
pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)
