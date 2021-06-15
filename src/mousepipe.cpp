#include <cstring>
#include "configparse.h"
#include "mousepipe.h"

extern void parse_hid_report_map(uint8_t *map, size_t size);

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
                    USBHostMouse *p = (USBHostMouse *)pipe->port;
                    if (p->begin(irp->data_buffer))
                    {
                        usbh_mouse_device_ready();
                    }
                } else if ((ctrl->wValue >> 8) == 0x22) // HID report map descriptor
                {
                    ESP_LOGI("", "HID report map");
                    for (int i = 0; i < (irp->actual_num_bytes + 8); i++)
                    {
                        ets_printf("%02x ", irp->data_buffer[i]);
                    }
                    parse_hid_report_map(irp->data_buffer + 8, irp->actual_num_bytes);
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
        // HID CLASS specific events

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
    static USBHostMouse *port = (USBHostMouse *)p;
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
        break;

    default:
        ESP_LOGW("", "port event: %d", msg.port_event);
        break;
    }
}

static void in_pipe_cb(pipe_event_msg_t msg, usb_irp_t *irp, USBHostPipe *pipe)
{
    if (((USBHostMouse *)pipe->port)->data_in != NULL)
    {
        ESP_LOGV("", "callback IN: %d", irp->status);
        ((USBHostMouse *)pipe->port)->data_in(msg.event, irp);
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

bool USBHostMouse::init()
{
    return USBHostPort::init(port_cb);
}

// initialize endpoints after getting config descriptor
bool USBHostMouse::begin(uint8_t *irp)
{
    USBHostConfigParser parser;
    uint8_t *itf1 = parser.getInterfaceByClass(irp, USB_CLASS_HID);
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

        if (inpipe != nullptr)
            return true;
    }
    return false;
}

void USBHostMouse::setIdle()
{
    usb_irp_t *irp = ctrlPipe->allocateIRP(0);
    USB_CTRL_SET_IDLE((usb_ctrl_req_t *)irp->data_buffer);

    esp_err_t err;
    if(ESP_OK != (err = hcd_irp_enqueue(ctrlPipe->getHandle(), irp))) {
        ESP_LOGW("", "SET_IDLE enqueue err: 0x%x", err);
    }
}

void USBHostMouse::getHidReportMap()
{
    usb_irp_t *irp = ctrlPipe->allocateIRP(70);
    USB_CTRL_GET_HID_REPORT_DESC((usb_ctrl_req_t *)irp->data_buffer, 0, 70);

    esp_err_t err;
    if(ESP_OK != (err = hcd_irp_enqueue(ctrlPipe->getHandle(), irp))) {
        ESP_LOGW("", "SET_IDLE enqueue err: 0x%x", err);
    }
}


void USBHostMouse::onDataIn(ext_usb_pipe_cb_t cb)
{
    data_in = cb;
}
