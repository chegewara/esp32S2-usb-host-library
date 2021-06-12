#include <cstring>
#include "pipe.h"
#include "usb.h"

static void pipe_event_task(void *p)
{
    USBHostPipe *ctx = (USBHostPipe *)p;

    pipe_event_msg_t msg;
    while (1)
    {
        xQueueReceive(ctx->pipeEvtQueue, &msg, portMAX_DELAY);
        usb_irp_t *irp = hcd_irp_dequeue(msg.handle);
        if (irp == NULL)
            continue;

        if (ctx && ctx->callback != NULL)
        {
            ctx->callback(msg, irp, ctx);
        }

        ctx->freeIRP(irp);
    }
}

USBHostPipe::USBHostPipe(hcd_port_handle_t handle)
{
    port_hdl = handle;
}

USBHostPipe::~USBHostPipe()
{
    freePipe();
    vTaskDelete(taskHandle);
    free(pipeEvtQueue);
    ESP_LOGI("", "delete pipe");
}

static bool pipeCallback(hcd_pipe_handle_t pipe_hdl, hcd_pipe_event_t pipe_event, void *user_arg, bool in_isr)
{
    QueueHandle_t pipe_evt_queue = (QueueHandle_t)user_arg;
    pipe_event_msg_t msg = {
        .handle = pipe_hdl,
        .event = pipe_event,
    };
    if (in_isr)
    {
        BaseType_t xTaskWoken = pdFALSE;
        xQueueSendFromISR(pipe_evt_queue, &msg, &xTaskWoken);
        return (xTaskWoken == pdTRUE);
    }
    else
    {
        xQueueSend(pipe_evt_queue, &msg, portMAX_DELAY);
        return false;
    }
}

void USBHostPipe::onPipeEvent(pipe_cb_t cb)
{
    callback = cb;
}

void USBHostPipe::init(usb_desc_ep_t *ep_desc, uint8_t addr)
{
    // TODO add config parse object to check if it is actually endpoint data
    usb_speed_t port_speed;
    if(port_hdl == NULL)ESP_LOGE("", "FAILED");
    if (ESP_OK != hcd_port_get_speed(port_hdl, &port_speed))
    {
        return;
    }
    xTaskCreate(pipe_event_task, "pipe_task", 2 * 1024, this, 15, &taskHandle);
    pipeEvtQueue = xQueueCreate(5, sizeof(pipe_event_msg_t));

    if (pipeEvtQueue == NULL)
        ESP_LOGE("", "pipe queue not initialized");

    //Create default pipe
    ESP_LOGV("", "Creating pipe\n");
    hcd_pipe_config_t config = {
        .callback = pipeCallback,
        .callback_arg = (void *)pipeEvtQueue,
        .context = (void *)this,
        .ep_desc = ep_desc,
        .dev_speed = port_speed,
        .dev_addr = addr,
    };

    if (ESP_OK != hcd_pipe_alloc(port_hdl, &config, &handle))
        ESP_LOGE("", "cant allocate pipe");

    if (ep_desc != NULL)
        memcpy(&endpoint, ep_desc, sizeof(usb_desc_ep_t));
}

void USBHostPipe::freePipe()
{
    //Dequeue transfer requests
    do
    {
        usb_irp_t *irp = hcd_irp_dequeue(handle);
        if (irp == NULL)
            break;
        heap_caps_free(irp->data_buffer);
        heap_caps_free(irp);
    } while (1);

    ESP_LOGD("", "Freeing transfer requets\n");
    //Free transfer requests (and their associated objects such as IRPs and data buffers)
    ESP_LOGD("", "Freeing pipe\n");
    //Delete the pipe
    if (ESP_OK != hcd_pipe_free(handle))
    {
        ESP_LOGE("", "err to free pipe");
    }
}

void USBHostPipe::updateAddress(uint8_t addr)
{
    hcd_pipe_update_dev_addr(handle, addr);
}

void USBHostPipe::updatePort(hcd_port_handle_t handle)
{
    port_hdl = handle;
}

void USBHostPipe::reset()
{
    hcd_pipe_command(handle, HCD_PIPE_CMD_RESET);
}

hcd_pipe_state_t USBHostPipe::getState()
{
    return hcd_pipe_get_state(handle);
}

void USBHostPipe::getDeviceDescriptor()
{
    usb_irp_t *irp = allocateIRP(18);
    if (irp == NULL)
        return;
    USB_CTRL_REQ_INIT_GET_DEVC_DESC((usb_ctrl_req_t *)irp->data_buffer);

    //Enqueue those transfer requests
    esp_err_t err;
    if (ESP_OK != (err = hcd_irp_enqueue(handle, irp)))
    {
        ESP_LOGE("", "Get device desc: %d", err);
    }

    return;
}

void USBHostPipe::setDeviceAddress(uint8_t addr)
{
    usb_irp_t *irp = allocateIRP(0);
    if (irp == NULL)
        return;

    USB_CTRL_REQ_INIT_SET_ADDR((usb_ctrl_req_t *)irp->data_buffer, addr);

    //Enqueue those transfer requests
    esp_err_t err;
    if (ESP_OK != (err = hcd_irp_enqueue(handle, irp)))
    {
        ESP_LOGE("", "Set address: %d", err);
    }
}

void USBHostPipe::getCurrentConfiguration()
{
    usb_irp_t *irp = allocateIRP(1);
    if (irp == NULL)
        return;

    USB_CTRL_REQ_INIT_GET_CONFIG((usb_ctrl_req_t *)irp->data_buffer);

    //Enqueue those transfer requests
    esp_err_t err;
    if (ESP_OK != (err = hcd_irp_enqueue(handle, irp)))
    {
        ESP_LOGE("", "Get current config: %d", err);
    }
}

void USBHostPipe::setConfiguration(uint8_t cfg)
{
    usb_irp_t *irp = allocateIRP(0);
    if (irp == NULL)
        return;

    USB_CTRL_REQ_INIT_SET_CONFIG((usb_ctrl_req_t *)irp->data_buffer, cfg);

    //Enqueue those transfer requests
    esp_err_t err;
    if (ESP_OK != (err = hcd_irp_enqueue(handle, irp)))
    {
        ESP_LOGE("", "Set current config: %d", err);
    }
}

void USBHostPipe::getConfigDescriptor()
{
    usb_irp_t *irp = allocateIRP(TRANSFER_DATA_MAX_BYTES);
    if (irp == NULL)
        return;
    USB_CTRL_REQ_INIT_GET_CFG_DESC((usb_ctrl_req_t *)irp->data_buffer, 0, TRANSFER_DATA_MAX_BYTES);

    //Enqueue those transfer requests
    esp_err_t err;
    if (ESP_OK != (err = hcd_irp_enqueue(handle, irp)))
    {
        ESP_LOGE("", "Get config desc: %d", err);
    }
}

void USBHostPipe::getString(uint8_t num)
{
    usb_irp_t *irp = allocateIRP(TRANSFER_DATA_MAX_BYTES);
    if (irp == NULL)
        return;
    USB_CTRL_REQ_INIT_GET_STRING((usb_ctrl_req_t *)irp->data_buffer, 0, num, TRANSFER_DATA_MAX_BYTES);

    //Enqueue those transfer requests
    esp_err_t err;
    if (ESP_OK != (err = hcd_irp_enqueue(handle, irp)))
    {
        ESP_LOGE("", "Get string: %d, err: %d", num, err);
    }
}

usb_irp_t *USBHostPipe::allocateIRP(size_t size)
{
    // ESP_LOGI("", "Creating new IRP, free memory: %d", heap_caps_get_free_size(MALLOC_CAP_INTERNAL));
    usb_irp_t *irp = (usb_irp_t *)heap_caps_calloc(1, sizeof(usb_irp_t), MALLOC_CAP_DMA);
    if (NULL == irp)
    {
        ESP_LOGE("", "err to alloc IRP");
        return irp;
    }
    //Allocate data buffer
    uint8_t *data_buffer = (uint8_t *)heap_caps_calloc(1, sizeof(usb_ctrl_req_t) + size, MALLOC_CAP_DMA);
    if (NULL == data_buffer)
    {
        ESP_LOGE("", "err to alloc data buffer");
        // free(irp);
        return NULL;
    }
    //Initialize IRP and IRP list
    irp->data_buffer = data_buffer;
    irp->num_iso_packets = 0;
    irp->num_bytes = size;

    return irp;
}

void USBHostPipe::freeIRP(usb_irp_t *irp)
{
    free(irp->data_buffer);
    free(irp);
}

void USBHostPipe::inData(size_t size)
{
    ESP_LOGV("", "EP: 0x%02x", USB_DESC_EP_GET_ADDRESS(endpoint));
    size_t len = endpoint.wMaxPacketSize;
    usb_irp_t *irp = allocateIRP(size? size:len);

    esp_err_t err;
    if(ESP_OK != (err = hcd_irp_enqueue(handle, irp))) {
        ESP_LOGW("", "IN endpoint 0x%02x enqueue err: 0x%x", USB_DESC_EP_GET_ADDRESS(endpoint), err);
    }
}

void USBHostPipe::outData(uint8_t *data, size_t len)
{
    usb_irp_t *irp = allocateIRP(len);
    ESP_LOGV("", "EP: 0x%02x", USB_DESC_EP_GET_ADDRESS(endpoint));
    memcpy(irp->data_buffer, data, len);

    esp_err_t err;
    if (ESP_OK != (err = hcd_irp_enqueue(handle, irp)))
    {
        ESP_LOGW("", "BULK OUT enqueue err: 0x%x", err);
    }
}

hcd_pipe_handle_t USBHostPipe::getHandle()
{
    return handle;
}
