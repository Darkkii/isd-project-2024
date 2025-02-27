# Source files
add_executable(${PROJECT_NAME}
        main.cpp

        fs/File.cpp

        i2c/PicoI2C.cpp

        network/dhcp/DhcpLease.cpp
        network/dhcp/DhcpMessage.cpp
        network/dns/DnsMessage.cpp
        network/http/HttpHeader.cpp
        network/NetworkGroup.cpp

        rtos/EventGroup.cpp
        rtos/semaphore/Binary.cpp
        rtos/semaphore/Mutex.cpp

        Display-lib/SSD1306_OLED.hpp
        Display-lib/SSD1306_OLED.cpp
        Display-lib/SSD1306_OLED_font.hpp
        Display-lib/SSD1306_OLED_font.cpp
        Display-lib/SSD1306_OLED_graphics.hpp
        Display-lib/SSD1306_OLED_graphics.cpp
        Display-lib/SSD1306_OLED_Print.hpp
        Display-lib/SSD1306_OLED_Print.cpp

        sensor/SensorData.cpp

        task/AccessPointTask.cpp
        task/BaseTask.cpp
        task/DhcpServerTask.cpp
        task/DnsServerTask.cpp
        task/HttpServerTask.cpp

        uart/PicoOsUart.cpp

        sensor/RTClib.h
        sensor/RTClib.cpp
        sensor/SPS30.h
        task/I2CTask.cpp
        task/I2CTask.h
        task/MHZTask.cpp
        task/MHZTask.h
        sensor/MS430.cpp
        sensor/MS430.h
        sensor/RTCModule.cpp
        sensor/RTCModule.h
        sensor/MHZ19CSensor.cpp
        sensor/MHZ19CSensor.h
)

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        config
)

# Compile definitions
target_compile_definitions(${PROJECT_NAME} PRIVATE
        NO_SYS=0 # don't want NO_SYS (generally this would be in your lwipopts.h)
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
)

pico_add_extra_outputs(${PROJECT_NAME})

# Enable usb output, disable uart output
pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)
