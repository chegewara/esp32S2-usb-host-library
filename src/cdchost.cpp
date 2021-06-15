#include <cstring>
#include "cdchost.h"
#include "configparse.h"

/**
 * //IDEA ADD set of custom USB host events
 * add callbacks with custom events in ctrl pipe
 * 
 * 
 * 
 * 
 */ 
 /** dont change order of callbacks
 * port and EPs/pipes local callbacks
 */
IRAM_ATTR static void ctrl_pipe_cb(pipe_event_msg_t msg, usb_irp_t *irp, USBHostPipe *pipe)
{
    usb_ctrl_req_t *ctrl = (usb_ctrl_req_t *)irp->data_buffer;
    ext_pipe_event_msg_t event = msg.event + BASE_PIPE_EVENT;
    switch (msg.event)
    {
    case HCD_PIPE_EVENT_NONE:
        break;
    case HCD_PIPE_EVENT_IRP_DONE:
    {
        ESP_LOGD("Pipe: ", "XFER status: %d, num bytes: %d, actual bytes: %d", irp->status, irp->num_bytes, irp->actual_num_bytes);
        usb_ctrl_req_t *ctrl = (usb_ctrl_req_t *)irp->data_buffer;

        if (ctrl->bRequest == USB_B_REQUEST_GET_DESCRIPTOR)
        {
            event = BASE_PIPE_EVENT + ctrl->bRequest + (ctrl->wValue);
            if (ctrl->wValue == USB_W_VALUE_DT_CONFIG << 8)
            {
                if (irp->actual_num_bytes)
                {
                    USBHostCDC *p = (USBHostCDC *)pipe->port;
                    if (p->begin(irp->data_buffer))
                    {
                        usbh_cdc_device_ready();
                    }
                }
            }
        }
        else if (ctrl->bRequest == USB_B_REQUEST_GET_CONFIGURATION)
        {
        }
        else if (ctrl->bRequest == USB_B_REQUEST_SET_CONFIGURATION)
        {
        }
        else if (ctrl->bRequest == USB_B_REQUEST_SET_ADDRESS)
        {
        }
        // CDC CLASS specific events
        else if (ctrl->bRequestType == SET_VALUE && ctrl->bRequest == SET_LINE_CODING)
        {
            event = BASE_PIPE_EVENT + CDC_BASE_PIPE_EVENT + (ctrl->bRequestType << 4) + ctrl->bRequest;
            line_coding_t *data = (line_coding_t *)(irp->data_buffer + sizeof(usb_ctrl_req_t));
            ESP_LOGD("Set line coding", "Bitrate: %d, stop bits: %d, parity: %d, bits: %d",
                     data->dwDTERate, data->bCharFormat, data->bParityType, data->bDataBits);
        }
        else if (ctrl->bRequestType == GET_VALUE && ctrl->bRequest == GET_LINE_CODING)
        {
            event = BASE_PIPE_EVENT + CDC_BASE_PIPE_EVENT + (ctrl->bRequestType << 4) + ctrl->bRequest;
            line_coding_t *data = (line_coding_t *)(irp->data_buffer + sizeof(usb_ctrl_req_t));
            ESP_LOGD("Get line coding", "Bitrate: %d, stop bits: %d, parity: %d, bits: %d",
                     data->dwDTERate, data->bCharFormat, data->bParityType, data->bDataBits);
        }
        else if (ctrl->bRequestType == SET_VALUE && ctrl->bRequest == SET_CONTROL_LINE_STATE)
        {
            event = BASE_PIPE_EVENT + CDC_BASE_PIPE_EVENT + (ctrl->bRequestType << 4) + ctrl->bRequest;
            ESP_LOGD("Set control line state", "");
        }

        break;
    }
    }

    if (pipe->port->ctrl_callback != NULL)
    {
        pipe->port->ctrl_callback(event, irp);
    }
}

IRAM_ATTR static void port_cb(port_event_msg_t msg, USBHostPort *p)
{
    static USBHostCDC *port = (USBHostCDC *)p;
    hcd_port_state_t state;
    switch (msg.port_event)
    {
    case HCD_PORT_EVENT_CONNECTION:
    {
        ESP_LOGD("", "physical connection detected");
        // we are ready to establish connection with device
        if (port->connect())
        {
            // create ctrl callback
            USBHostPipe *pipe = new USBHostPipe();
            pipe->port = port;
            port->addCTRLpipe(pipe);
            pipe->onPipeEvent(ctrl_pipe_cb);
            pipe->getDeviceDescriptor();
            pipe->setDeviceAddress(port->address);
        }
        break;
    }
    case HCD_PORT_EVENT_SUDDEN_DISCONN:
        // OPTIMIZE this is called before event in port.cpp to delete all pipes here to properly recover port later in port.cpp
        if (port->inpipe)
            delete (port->inpipe);
        if (port->outpipe)
            delete (port->outpipe);
        break;

    default:
        ESP_LOGW("", "port event: %d", msg.port_event);
        break;
    }
}

static void in_pipe_cb(pipe_event_msg_t msg, usb_irp_t *irp, USBHostPipe *pipe)
{
    if (((USBHostCDC *)pipe->port)->data_in != NULL)
    {
        ESP_LOGV("", "callback IN: %d", irp->status);
        ((USBHostCDC *)pipe->port)->data_in(msg.event, irp);
    }
    switch (msg.event)
    {
    case HCD_PIPE_EVENT_IRP_DONE:
        ESP_LOGD("Pipe cdc: ", "XFER status: %d, num bytes: %d, actual bytes: %d", irp->status, irp->num_bytes, irp->actual_num_bytes);
        break;

    case HCD_PIPE_EVENT_ERROR_XFER:
        ESP_LOGW("", "XFER error: %d", irp->status);
        pipe->reset();
        break;

    case HCD_PIPE_EVENT_ERROR_STALL:
        ESP_LOGW("", "Device stalled CDC pipe, state: %d", pipe->getState());
        pipe->reset();
        break;

    default:
        break;
    }
    // previous IRP dequeued, time to send another request
    pipe->inData();
}

static void out_pipe_cb(pipe_event_msg_t msg, usb_irp_t *irp, USBHostPipe *pipe)
{
    if (((USBHostCDC *)pipe->port)->data_out != NULL)
    {
        ((USBHostCDC *)pipe->port)->data_out(msg.event, irp);
        ESP_LOGV("", "callback OUT: %d", irp->status);
    }
    switch (msg.event)
    {
    case HCD_PIPE_EVENT_IRP_DONE:
        ESP_LOGD("Pipe cdc: ", "XFER status: %d, num bytes: %d, actual bytes: %d", irp->status, irp->num_bytes, irp->actual_num_bytes);
        break;

    case HCD_PIPE_EVENT_ERROR_XFER:
        ESP_LOGW("", "XFER error: %d", irp->status);
        pipe->reset();
        break;

    case HCD_PIPE_EVENT_ERROR_STALL:
        ESP_LOGW("", "Device stalled CDC pipe, state: %d", pipe->getState());
        pipe->reset();
        break;

    default:
        break;
    }
}

void USBHostCDC::init()
{
    USBHostPort::init(port_cb);
}

// initialize endpoints after getting config descriptor
bool USBHostCDC::begin(uint8_t *irp)
{
    USBHostConfigParser parser;
    // uint8_t *itf0 = parser.getInterfaceByClass(irp, USB_CLASS_COMM); // TODO add INTR endpoint
    uint8_t *itf1 = parser.getInterfaceByClass(irp, USB_CLASS_CDC_DATA);
    if (itf1 != nullptr)
    {
        uint8_t *ep = parser.getEndpointByDirection(itf1, 1);
        if (ep)
        {
            inpipe = new USBHostPipe(handle);
            inpipe->onPipeEvent(in_pipe_cb);
            inpipe->init((usb_desc_ep_t *)ep, address);
            inpipe->port = this;
        }

        ep = parser.getEndpointByDirection(itf1, 0);
        if (ep)
        {
            outpipe = new USBHostPipe(handle);
            outpipe->onPipeEvent(out_pipe_cb);
            outpipe->init((usb_desc_ep_t *)ep, address);
            outpipe->port = this;
        }

        if (inpipe != nullptr && outpipe != nullptr)
            return true;
    }
    return false;
}

// CDC class control pipe requests
void USBHostCDC::setLineCoding(uint32_t bitrate, uint8_t cf, uint8_t parity, uint8_t bits)
{
    if (ctrlPipe == NULL)
    {
        ESP_LOGE("", "CTRL pipe is not set");
        return;
    }
    usb_irp_t *irp = ctrlPipe->allocateIRP(7);
    if (irp == NULL)
    {
        ESP_LOGE("", "CTRL pipe IRP is not set");
        return;
    }

    USB_CTRL_REQ_CDC_SET_LINE_CODING((cdc_ctrl_line_t *)irp->data_buffer, 0, bitrate, cf, parity, bits);

    esp_err_t err;
    if (ESP_OK != (err = hcd_irp_enqueue(ctrlPipe->getHandle(), irp)))
    {
        ESP_LOGW("", "SET_LINE_CODING enqueue err: 0x%x", err);
    }
}

bool USBHostCDC::setControlLine(bool dtr, bool rts)
{
    if (ctrlPipe == nullptr)
    {
        return false;
    }

    usb_irp_t *irp = ctrlPipe->allocateIRP(0);
    if (irp == NULL)
    {
        return false;
    }

    USB_CTRL_REQ_CDC_SET_CONTROL_LINE_STATE((usb_ctrl_req_t *)irp->data_buffer, 0, dtr, rts);

    esp_err_t err;
    if (ESP_OK != (err = hcd_irp_enqueue(ctrlPipe->getHandle(), irp)))
    {
        ESP_LOGW("", "SET_CONTROL_LINE_STATE enqueue err: 0x%x", err);
    }
    return true;
}

void USBHostCDC::getLineCoding()
{
    usb_irp_t *irp = ctrlPipe->allocateIRP(7);
    USB_CTRL_REQ_CDC_GET_LINE_CODING((usb_ctrl_req_t *)irp->data_buffer, 0);

    esp_err_t err;
    if (ESP_OK != (err = hcd_irp_enqueue(ctrlPipe->getHandle(), irp)))
    {
        ESP_LOGW("", "GET_LINE_CODING enqueue err: 0x%x", err);
    }
}

void USBHostCDC::sendData(uint8_t *data, size_t len)
{
    outpipe->outData(data, len);
}

// IN/OUT callbacks
void USBHostCDC::onDataIn(ext_usb_pipe_cb_t cb)
{
    data_in = cb;
}

void USBHostCDC::onDataOut(ext_usb_pipe_cb_t cb)
{
    data_out = cb;
}

/**
 * TODO
 */
void USBHostCDC::intrData()
{
    // usb_irp_t *irp = allocateIRP(70);
    // USB_CTRL_GET_HID_REPORT_DESC((usb_ctrl_req_t *)irp->data_buffer, 0, 70);

    // esp_err_t err;
    // if(ESP_OK != (err = hcd_irp_enqueue(handle, irp))) {
    //     ESP_LOGW("", "INTR enqueue err: 0x%x", err);
    // }
}

USBH_WEAK_CB void usbh_cdc_device_ready(){}

