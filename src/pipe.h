
#pragma once
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "esp_err.h"
#include "esp_attr.h"
#include "hal/usbh_ll.h"
#include "hcd.h"

#if defined(ARDUINO_ARCH_ESP32) && defined(CONFIG_ARDUHAL_ESP_LOG)
#include "esp32-hal-log.h"
#else
#include "esp_log.h"
#endif
#define TRANSFER_DATA_MAX_BYTES 255 //Just assume that will only IN/OUT 64 bytes for now

#define BASE_PIPE_EVENT 0x8000
#define CDC_BASE_PIPE_EVENT 0x1000
#define HID_BASE_PIPE_EVENT 0x2000
#define USBH_WEAK_CB __attribute__((weak))

/**
 * @brief Build get string request
 */
#define USB_CTRL_REQ_INIT_GET_STRING(ctrl_req_ptr, lang, desc_index, len) (                                                            \
    {                                                                                                                                  \
        (ctrl_req_ptr)->bRequestType = USB_B_REQUEST_TYPE_DIR_IN | USB_B_REQUEST_TYPE_TYPE_STANDARD | USB_B_REQUEST_TYPE_RECIP_DEVICE; \
        (ctrl_req_ptr)->bRequest = USB_B_REQUEST_GET_DESCRIPTOR;                                                                       \
        (ctrl_req_ptr)->wValue = (USB_W_VALUE_DT_STRING << 8) | ((desc_index)&0xFF);                                                   \
        (ctrl_req_ptr)->wIndex = (lang);                                                                                               \
        (ctrl_req_ptr)->wLength = (len);                                                                                               \
    })

typedef struct
{
    hcd_pipe_handle_t handle;
    hcd_pipe_event_t event;
} pipe_event_msg_t;

class USBHostPipe;
class USBHostPort;

/**
 * @brief Pipe callback definition to pass events to class callback
 */
typedef void (*pipe_cb_t)(pipe_event_msg_t msg, usb_irp_t *irp, USBHostPipe *context);

void onSerialString(char* str);
void onProductString(char* str);
void onManufacturerString(char* str);

class USBHostPipe
{
protected:
    //
    hcd_pipe_handle_t handle;
    // 
    hcd_port_handle_t port_hdl;
    // 
    xTaskHandle taskHandle;
public:
    // friend void pipe_event_task(void *p);
    // 
    usb_desc_ep_t endpoint;
    // 
    pipe_cb_t callback = nullptr;
    // 
    QueueHandle_t pipeEvtQueue;

    usb_desc_devc_t device_desc;


    USBHostPipe(hcd_port_handle_t handle = nullptr);
    ~USBHostPipe();
    // master port which this pipe belongs to
    USBHostPort *port;

    /**
     * @brief Register pipe event from class space
     */
    void onPipeEvent(pipe_cb_t cb);

    /**
     * @brief Initialize pipe from endpoint data
     */
    void init(usb_desc_ep_t *ep_desc = nullptr, uint8_t addr = 0);

    /**
     * @brief Free all queues and pipe belongs to this object
     */
    void freePipe();

    /**
     * @brief Update address, usually used with control pipe
     */
    void updateAddress(uint8_t);

    /**
     * @brief Update port handle, do we need it???
     */
    void updatePort(hcd_port_handle_t handle);

    /**
     * @brief Reset pipe
     */
    void reset();

    /**
     * @brief Get pipe state
     */
    hcd_pipe_state_t getState();

    /**
     * @brief Allocate IRP before enqueue new request
     */
    usb_irp_t *allocateIRP(size_t size);

    /**
     * @brief Free IRP and data buffer
     */
    void freeIRP(usb_irp_t *);

    /**
     * @brief Get pipe handle
     */
    hcd_pipe_handle_t getHandle();

    /**
     * @brief Send data IN request
     */
    void inData(size_t size = 0);

    /**
     * @brief Send data OUT request
     */
    void outData(uint8_t *data, size_t len);

// ------------------- standard USB requests ------------------------ //
    /**
     * @brief Prepare and enqueue get device descriptor request
     */
    void getDeviceDescriptor();

    /**
     * @brief Prepare and enqueue set device address request
     */
    void setDeviceAddress(uint8_t addr);

    /**
     * @brief Prepare and enqueue get configuration descriptor request
     */
    void getCurrentConfiguration();

    /**
     * @brief Prepare and enqueue set configuration request
     */
    void setConfiguration(uint8_t);

    /**
     * @brief Prepare and enqueue get configuration descriptor request
     */
    void getConfigDescriptor(size_t n = 9);

    /**
     * @brief Prepare and enqueue get string by id request
     */
    void getString(uint8_t);

    void setDeviceDesc(uint8_t* data);

    void getSerialString();
    void getProductString();
    void getManufacturerString();
};

// TODO add abort IRP


