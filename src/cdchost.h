#pragma once
#include "hcd.h"
#include "usb.h"
#include "port.h"
#include "pipe.h"

#define SET_VALUE 0x21
#define GET_VALUE 0xA1

// DTR/RTS control in SET_CONTROL_LINE_STATE
#define ENABLE_DTR(val) (val << 0)
#define ENABLE_RTS(val) (val << 1)

#define SET_LINE_CODING 0x20
#define GET_LINE_CODING 0x21
#define SET_CONTROL_LINE_STATE 0x22
#define SERIAL_STATE 0x20

typedef struct
{
    uint32_t dwDTERate;
    uint8_t bCharFormat;
    uint8_t bParityType;
    uint8_t bDataBits;
} line_coding_t;

typedef union
{
    struct
    {
        uint8_t bRequestType;
        uint8_t bRequest;
        uint16_t wValue;
        uint16_t wIndex;
        uint16_t wLength;
        line_coding_t data;
    } USB_CTRL_REQ_ATTR;
    uint8_t val[USB_CTRL_REQ_SIZE + 7];
} cdc_ctrl_line_t;

/**
 * @brief Initializer for a SET_LINE_CODING request
 *
 * Sets the address of a connected device
 */
#define USB_CTRL_REQ_CDC_SET_LINE_CODING(ctrl_req_ptr, index, bitrate, cf, parity, bits) ( \
    {                                                                                      \
        (ctrl_req_ptr)->bRequestType = SET_VALUE;                                          \
        (ctrl_req_ptr)->bRequest = SET_LINE_CODING;                                        \
        (ctrl_req_ptr)->wValue = 0;                                                        \
        (ctrl_req_ptr)->wIndex = (index);                                                  \
        (ctrl_req_ptr)->wLength = (7);                                                     \
        (ctrl_req_ptr)->data.dwDTERate = (bitrate);                                        \
        (ctrl_req_ptr)->data.bCharFormat = (cf);                                           \
        (ctrl_req_ptr)->data.bParityType = (parity);                                       \
        (ctrl_req_ptr)->data.bDataBits = (bits);                                           \
    })

/**
 * @brief Initializer for a GET_LINE_CODING request
 *
 * Sets the address of a connected device
 */
#define USB_CTRL_REQ_CDC_GET_LINE_CODING(ctrl_req_ptr, index) ( \
    {                                                           \
        (ctrl_req_ptr)->bRequestType = GET_VALUE;               \
        (ctrl_req_ptr)->bRequest = GET_LINE_CODING;             \
        (ctrl_req_ptr)->wValue = 0;                             \
        (ctrl_req_ptr)->wIndex = (index);                       \
        (ctrl_req_ptr)->wLength = (7);                          \
    })

/**
 * @brief Initializer for a SET_CONTROL_LINE_STATE request
 *
 * Sets the address of a connected device
 */
#define USB_CTRL_REQ_CDC_SET_CONTROL_LINE_STATE(ctrl_req_ptr, index, dtr, rts) ( \
    {                                                                            \
        (ctrl_req_ptr)->bRequestType = SET_VALUE;                                \
        (ctrl_req_ptr)->bRequest = SET_CONTROL_LINE_STATE;                       \
        (ctrl_req_ptr)->wValue = ENABLE_DTR(dtr) | ENABLE_RTS(rts);              \
        (ctrl_req_ptr)->wIndex = (index);                                        \
        (ctrl_req_ptr)->wLength = (0);                                           \
    })

void usbh_cdc_device_ready();

class USBHostCDC : public USBHostPort
{
private:
public:
    USBHostCDC() : USBHostPort() {}
    USBHostCDC(uint8_t addr) : USBHostPort(addr) {}
    bool begin(uint8_t *);
    void init();

    // ----------- CDC related requests ----------- //

    /**
     * @brief Enqueue set line codding on control pipe
     */
    void setLineCoding(uint32_t bitrate, uint8_t cf, uint8_t parity, uint8_t bits);

    /**
     * @brief Enqueue set control line request on control pipe
     */
    bool setControlLine(bool dtr, bool rts);

    /**
     * @brief Enqueue get line codding request on control pipe
     */
    void getLineCoding();

    // ---------------- IN/OUT data related functions -------------------- //
    // IDEA add handling INTR
    /**
     * @brief Enqueue INTR in request
     */
    void intrData();

    /**
     * @brief Enqueue send data requests on OUT endpoint pipe
     */
    void sendData(uint8_t *, size_t);

    // OPTIMIZE move to parent class??
    /**
     * @brief Register on data IN callback
     */
    void onDataIn(ext_usb_pipe_cb_t);

    /**
     * @brief Register on data OUT callback
     */
    void onDataOut(ext_usb_pipe_cb_t);

    // OPTIMIZE make them private or protected
    // class related callbacks and pipes
    ext_usb_pipe_cb_t data_in;
    ext_usb_pipe_cb_t data_out;
    USBHostPipe *inpipe;
    USBHostPipe *outpipe;
};



