#include "port.h"
static QueueHandle_t port_evt_queue;

// -------------------------------------------------- PHY Control ------------------------------------------------------

static void phy_force_conn_state(bool connected, TickType_t delay_ticks)
{
    vTaskDelay(delay_ticks);
    usb_wrap_dev_t *wrap = &USB_WRAP;
    if (connected)
    {
        //Swap back to internal PHY that is connected to a devicee
        wrap->otg_conf.phy_sel = 0;
    }
    else
    {
        //Set externa PHY input signals to fixed voltage levels mimicing a disconnected state
        esp_rom_gpio_connect_in_signal(GPIO_MATRIX_CONST_ZERO_INPUT, USB_EXTPHY_VP_IDX, false);
        esp_rom_gpio_connect_in_signal(GPIO_MATRIX_CONST_ZERO_INPUT, USB_EXTPHY_VM_IDX, false);
        esp_rom_gpio_connect_in_signal(GPIO_MATRIX_CONST_ONE_INPUT, USB_EXTPHY_RCV_IDX, false);
        //Swap to the external PHY
        wrap->otg_conf.phy_sel = 1;
    }
}

// ------------------------------------------------ Helper Functions ---------------------------------------------------

static bool port_callback(hcd_port_handle_t port_hdl, hcd_port_event_t port_event, void *user_arg, bool in_isr)
{
    QueueHandle_t port_evt_queue = (QueueHandle_t)user_arg;
    port_event_msg_t msg = {
        .port_hdl = port_hdl,
        .port_event = port_event,
    };

    BaseType_t xTaskWoken = pdFALSE;
    xQueueSendFromISR(port_evt_queue, &msg, &xTaskWoken);
    return (xTaskWoken == pdTRUE);
}

static void port_event_task(void *p)
{
    USBHostPort *port = (USBHostPort *)p;
    port_event_msg_t msg;
    while (1)
    {
        xQueueReceive(port_evt_queue, &msg, portMAX_DELAY);
        hcd_port_handle_event(msg.port_hdl);
        // class callback
        if (port->callback != NULL)
        {
            port->callback(msg, port);
        }
        // user callback
        if (port->_port_cb != NULL)
        {
            port->_port_cb(msg, port);
        }

        switch (msg.port_event)
        {
        case HCD_PORT_EVENT_NONE:
        case HCD_PORT_EVENT_CONNECTION:
        case HCD_PORT_EVENT_ERROR:
        case HCD_PORT_EVENT_OVERCURRENT:
            break;
        case HCD_PORT_EVENT_DISCONNECTION:
            ESP_LOGD("", "physical disconnection detected");
            port->powerOFF();
            break;

        case HCD_PORT_EVENT_SUDDEN_DISCONN:
            ESP_LOGD("", "physical disconnection detected");
            port->suddenDisconnect();
            break;
        }
    }
}

// ------------------------------------------------ CLASS ---------------------------------------------------

USBHostPort::USBHostPort()
{
    address = 1;
}

USBHostPort::USBHostPort(uint8_t addr) : address(addr)
{
    port_evt_queue = xQueueCreate(5, sizeof(port_event_msg_t));
}

USBHostPort::~USBHostPort() {}

void USBHostPort::init(port_evt_cb_t cb)
{
    hcd_config_t config = {
        .intr_flags = ESP_INTR_FLAG_LEVEL1,
    };
    if (hcd_install(&config) == ESP_OK)
    {
        //Initialize a port
        hcd_port_config_t port_config = {
            .callback = port_callback,
            .callback_arg = (void *)port_evt_queue,
            .context = NULL,
        };
        esp_err_t err;
        if (ESP_OK == (err = hcd_port_init(PORT_NUM, &port_config, &handle)))
        {
            if (HCD_PORT_STATE_NOT_POWERED == hcd_port_get_state(handle))
                ESP_LOGI("", "USB host setup properly");

            phy_force_conn_state(false, 0); //Force disconnected state on PHY
            if (ESP_OK != powerON())
                return;
            ESP_LOGI("", "Port is power ON now");
            phy_force_conn_state(true, pdMS_TO_TICKS(10)); //Allow for connected state on PHY
            // return true;
        }
        else
        {
            ESP_LOGE("", "Error to init port: %d!!!", err);
            return;
        }
    }
    else
    {
        ESP_LOGE("", "Error to install HCD!!!");
        return;
    }

    xTaskCreate(port_event_task, "port_task", 3 * 1024, this, 16, NULL);
    callback = cb;
}

bool USBHostPort::connect()
{
    hcd_port_state_t state;
    if (HCD_PORT_STATE_DISABLED == getState())
        ESP_LOGD("", "HCD_PORT_STATE_DISABLED");
    if (ESP_OK == reset())
        ESP_LOGD("", "USB device reset");
    else
        return false;
    if (HCD_PORT_STATE_ENABLED == getState())
    {
        ESP_LOGD("", "HCD_PORT_STATE_ENABLED");
        return true;
    }
    return false;
}

void USBHostPort::onPortEvent(port_evt_cb_t cb)
{
    _port_cb = cb;
}

void USBHostPort::onControlEvent(usb_host_event_cb_t cb)
{
    ctrl_callback = cb;
}


hcd_port_handle_t USBHostPort::getHandle()
{
    return handle;
}

esp_err_t USBHostPort::reset()
{
    ESP_LOGD(__func__, "");
    return hcd_port_command(handle, HCD_PORT_CMD_RESET);
}

hcd_port_state_t USBHostPort::getState()
{
    return hcd_port_get_state(handle);
}

esp_err_t USBHostPort::getSpeed(usb_speed_t *port_speed)
{
    return hcd_port_get_speed(handle, port_speed);
}

USBHostPipe *USBHostPort::getCTRLpipe()
{
    return ctrlPipe;
}

void USBHostPort::freeCTRLpipe()
{
    delete (ctrlPipe);
    ctrlPipe = NULL;
}

esp_err_t USBHostPort::powerON()
{
    ESP_LOGD(__func__, "");
    return hcd_port_command(handle, HCD_PORT_CMD_POWER_ON);
}

void USBHostPort::powerOFF()
{
    ESP_LOGD(__func__, "%d", hcd_port_command(handle, HCD_PORT_CMD_POWER_OFF));
}

void USBHostPort::suddenDisconnect()
{
    esp_err_t err = reset();
    ESP_LOGI("", "reset status: %d", err);
    freeCTRLpipe();
    recover();
    err = powerON();
    ESP_LOGI("", "powerON status: %d", err);
}

void USBHostPort::recover()
{
    hcd_port_state_t state;
    if (HCD_PORT_STATE_RECOVERY == (state = hcd_port_get_state(handle)))
    {
        esp_err_t err = hcd_port_recover(handle);
        ESP_LOGI(__func__, "%d", err);
    }
    else
    {
        ESP_LOGE("", "hcd_port_state_t: %d", state);
    }
}

void USBHostPort::addCTRLpipe(USBHostPipe *pipe)
{
    ctrlPipe = pipe;
    if (ctrlPipe == nullptr)
        ctrlPipe = new USBHostPipe(handle);

    ctrlPipe->updatePort(handle);
    ctrlPipe->init();
}
