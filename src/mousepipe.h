#pragma once
#include "hcd.h"
#include "usb.h"
#include "port.h"
#include "pipe.h"

#define SET_VALUE       0x21
#define SET_IDLE        0x0A

#define USB_CTRL_SET_IDLE(ctrl_req_ptr) ({  \
    (ctrl_req_ptr)->bRequestType = SET_VALUE;   \
    (ctrl_req_ptr)->bRequest = SET_IDLE;  \
    (ctrl_req_ptr)->wValue = (0x0);   \
    (ctrl_req_ptr)->wIndex = (0);    \
    (ctrl_req_ptr)->wLength = (0);   \
})

#define USB_CTRL_GET_HID_REPORT_DESC(ctrl_req_ptr, desc_index, desc_len) ({  \
    (ctrl_req_ptr)->bRequestType = USB_B_REQUEST_TYPE_DIR_IN | USB_B_REQUEST_TYPE_TYPE_STANDARD | USB_B_REQUEST_TYPE_RECIP_INTERFACE;   \
    (ctrl_req_ptr)->bRequest = USB_B_REQUEST_GET_DESCRIPTOR;   \
    (ctrl_req_ptr)->wValue = (0x22<<8) | ((desc_index) & 0xFF); \
    (ctrl_req_ptr)->wIndex = 0;    \
    (ctrl_req_ptr)->wLength = (desc_len);  \
})

void usbh_mouse_device_ready();

// TODO
// FIXME refactor class to inherit from port class
class USBHostMouse : public USBHostPort
{
private:

public:
    USBHostMouse() : USBHostPort() {}
    USBHostMouse(uint8_t addr) : USBHostPort(addr) {}
    bool begin(uint8_t *);
    bool init();
    void setIdle();
    void getHidReportMap();

    // OPTIMIZE move to parent class??
    /**
     * @brief Register on data IN callback
     */
    void onDataIn(ext_usb_pipe_cb_t);

    // OPTIMIZE make them private or protected
    // class related callbacks and pipes
    ext_usb_pipe_cb_t data_in;
    USBHostPipe *inpipe;
};

// USBH_WEAK_CB void usbh_mouse_device_ready(){}
