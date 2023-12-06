set( COMPONENT_SRCDIRS . )
set( COMPONENT_ADD_INCLUDEDIRS . )

if ( ${IDF_VERSION_MAJOR} VERSION_LESS 5 )
set( COMPONENT_REQUIRES nvs_flash app_update fatfs spiffs esp_adc_cal tcpip_adapter )
else()
set( COMPONENT_REQUIRES nvs_flash app_update fatfs spiffs esp_adc esp_netif esp_timer vfs esp_system spi_flash )
endif()

register_component()

         
