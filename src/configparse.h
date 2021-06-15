#pragma once
#if defined(ARDUINO_ARCH_ESP32) && defined(CONFIG_ARDUHAL_ESP_LOG)
#include "esp32-hal-log.h"
#else
#include "esp_log.h"
#endif
#define USB_W_VALUE_DT_HID 0x22
#define USB_W_VALUE_DT_CS_INTERFACE 0x24

class USBHostConfigParser
{
private:
    
public:
    USBHostConfigParser() {}
    ~USBHostConfigParser() {}

    // IDEA add more functions, like get all interfaces, all endpoints as array etc, get endpoint by address
    static uint8_t* getInterfaceByClass(uint8_t *data, uint8_t cls = 0);
    static uint8_t* getEndpointByDirection(uint8_t* interface, uint8_t dir = 0);
    static void getString(uint8_t *in, char *out, uint8_t len);

    static char* class_to_str(uint8_t cls);
    static void utf16_to_utf8(uint8_t *in, char *out, uint8_t len);
    static uint8_t* parse_cfg_descriptor(uint8_t *data_buffer, uint8_t **out, uint8_t *_type);
};

