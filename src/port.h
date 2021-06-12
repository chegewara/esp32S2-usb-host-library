#pragma once
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "esp_intr_alloc.h"
#include "esp_err.h"
#include "esp_attr.h"
#include "esp_rom_gpio.h"
#include "soc/gpio_pins.h"
#include "soc/gpio_sig_map.h"
#include "hal/usbh_ll.h"
#include "hcd.h"
#include "pipe.h"

#if defined(ARDUINO_ARCH_ESP32) && defined(CONFIG_ARDUHAL_ESP_LOG)
#include "esp32-hal-log.h"
#else
#include "esp_log.h"
#endif

#define USBH_WEAK_CB __attribute__((weak))
#define PORT_NUM 1 // we have only 1 port

typedef uint16_t usb_host_even_t;

typedef struct
{
    hcd_port_handle_t port_hdl;
    hcd_port_event_t port_event;
} port_event_msg_t;

class USBHostPort;

/**
 * @brief Port events callback for user space
 */
typedef void (*port_evt_cb_t)(port_event_msg_t msg, USBHostPort *port);
/**
 * @brief Pipe events callback for user space
 */
typedef void (*usb_host_event_cb_t)(usb_host_even_t event, void *data);

class USBHostPort
{
protected:
    // port handle
    hcd_port_handle_t handle;
    // control pipe
    USBHostPipe *ctrlPipe;

public:
    uint8_t address = 0;
    port_evt_cb_t callback;
    USBHostPort();
    USBHostPort(uint8_t addr);
    ~USBHostPort();

    /**
     * @brief Initialize USB host port and create low level port callback task
     */
    void init(port_evt_cb_t cb);

    /**
     * @brief Call this function when physical connection is detected to properly reset port
     */
    bool connect();

    /**
     * @brief Get port state
     */
    hcd_port_state_t getState();

    /**
     * @brief Get speed of connected device
     */
    esp_err_t getSpeed(usb_speed_t *port_speed);

    /**
     * @brief Power ON port
     */
    esp_err_t powerON();

    /**
     * @brief Power OFF port
     */
    void powerOFF();

    /**
     * @brief Reset port 
     */
    esp_err_t reset();

    /**
     * @brief Recover port when state is 
     */
    void recover();

    /**
     * @brief Call when port event is detected 
     */
    void suddenDisconnect();

    /**
     * @brief Add control pipe to port/device
     */
    void addCTRLpipe(USBHostPipe *pipe = nullptr);

    /**
     * @brief Get control pipe
     */
    USBHostPipe *getCTRLpipe();

    /**
     * @brief Delete control pipe when device disconnect detected
     */
    void freeCTRLpipe();

    /**
     * @brief Register port events callback for class space, user space callback 
     */
    void onPortEvent(port_evt_cb_t cb);

    /**
     * @brief Add control pipe callback for user space
     */
    void onControlEvent(usb_host_event_cb_t cb);

    /**
     * @brief Get port handle
     */
    hcd_port_handle_t getHandle();


    /**
     * @brief cllbacks
     */
    usb_host_event_cb_t ctrl_callback;
    port_evt_cb_t _port_cb;
};
