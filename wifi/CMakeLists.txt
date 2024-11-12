if (EXISTS ${PICO_LWIP_CONTRIB_PATH}/apps/ping/ping.c)
    add_executable(proj_server
            server.c
            ${PICO_LWIP_CONTRIB_PATH}/apps/ping/ping.c
            )
    target_compile_definitions(proj_server PRIVATE
            WIFI_SSID=\"${WIFI_SSID}\"
            WIFI_PASSWORD=\"${WIFI_PASSWORD}\"
            NO_SYS=0            # don't want NO_SYS (generally this would be in your lwipopts.h)
            LWIP_SOCKET=1       # we need the socket API (generally this would be in your lwipopts.h)
            PING_USE_SOCKETS=1
            )
    target_include_directories(proj_server PRIVATE
            ${CMAKE_CURRENT_LIST_DIR}
            ${CMAKE_CURRENT_LIST_DIR}/../.. # for our common lwipopts
            ${PICO_LWIP_CONTRIB_PATH}/apps/ping
            )
    target_link_libraries(proj_server
            hardware_adc
            pico_cyw43_arch_lwip_sys_freertos
            pico_stdlib
            pico_lwip_iperf
            FreeRTOS-Kernel-Heap4 # FreeRTOS kernel and dynamic heap
            )
    pico_add_extra_outputs(proj_server)
    pico_enable_stdio_usb(proj_server 1)   
endif()