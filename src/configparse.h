#pragma once
#if defined(ARDUINO_ARCH_ESP32) && defined(CONFIG_ARDUHAL_ESP_LOG)
#include "esp32-hal-log.h"
#else
#include "esp_log.h"
#endif
class USBHostConfigParser
{
private:
    
public:
    USBHostConfigParser();
    ~USBHostConfigParser();

    uint8_t* getInterfaceByClass(uint8_t *data, uint8_t cls = 0);
    uint8_t* getEndpointByDirection(uint8_t* interface, uint8_t dir = 0);
    void getString();
};

