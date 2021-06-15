#include "port.h"
// #include "pipe.h"
#include "mousepipe.h"
#if defined(ARDUINO_ARCH_ESP32) && defined(CONFIG_ARDUHAL_ESP_LOG)
#include "esp32-hal-log.h"
#else
#include "esp_log.h"
#endif
#include "parse_hid.h"


#define DEVICE_ADDRESS 2

USBHostMouse port(2);

extern void parse_hid_report_map(uint8_t *map, size_t size);

void usbh_mouse_device_ready()
{
  port.setIdle();
  port.getHidReportMap();
}

void ctrl_pipe_cb(ext_pipe_event_msg_t event, usb_irp_t *irp)
{
  Serial.printf("CTRL EVENT: 0x%x\n", event);
  if (event == 0xA206)
  {
    port.inpipe->inData();
  }
}

/**
 * hid report with buttons, wheel and move
 */
static void hid_datain_cb(ext_pipe_event_msg_t event, usb_irp_t *irp)
{
    switch (event)
    {
    case HCD_PIPE_EVENT_IRP_DONE:
        ESP_LOGD("Pipe cdc: ", "XFER status: %d, num bytes: %d, actual bytes: %d", irp->status, irp->num_bytes, irp->actual_num_bytes);
        // ESP_LOG_BUFFER_HEX("", irp->data_buffer, irp->num_bytes);
        ESP_LOGI("", "HID REPORT ID: %d", irp->data_buffer[0]);
        ESP_LOGI("", "Mouse buttons: %d", irp->data_buffer[1]);
        ESP_LOGI("", "X axis(raw bytes): %i", (int8_t)irp->data_buffer[2]);
        ESP_LOGI("", "Y axis(raw bytes): %i", (int8_t)irp->data_buffer[4]);
        ESP_LOGI("", "Mouse wheel: %i\n", (int8_t)irp->data_buffer[5]);

        // ESP_LOGI("", "Mouse buttons: %d", get_buttons(irp->data_buffer));
        // ESP_LOGI("", "X axis(raw bytes): %i", get_x_axis(irp->data_buffer));
        // ESP_LOGI("", "Y axis(raw bytes): %i", get_y_axis(irp->data_buffer));
        // ESP_LOGI("", "Mouse wheel: %i\n", get_wheel(irp->data_buffer));
        break;

    }
}

void port_cb(port_event_msg_t msg, USBHostPort *port)
{
  Serial.printf("PORT EVENT: 0x%x\n", msg.port_event);
}

void setup()
{
  Serial.begin(115200);
  if(port.init()){
    port.onPortEvent(port_cb);
    port.onControlEvent(ctrl_pipe_cb);
    port.onDataIn(hid_datain_cb);
  }
}

void loop()
{
    delay(1000);
    // hid_mouse_t * mouse = get_mouse_struct();
    // Serial.printf("buttons first bits: %d, bits count: %d\n", mouse->buttons.bits_start + 8, (mouse->buttons.count));
    // Serial.printf("axes first bits: %d, bits count: %d\n", mouse->axes.bits_start + 8, (mouse->axes.count) * mouse->axes.size);
    // Serial.printf("wheel first bits: %d, bits count: %d\n", mouse->wheel.bits_start + 8, (mouse->wheel.count) * mouse->wheel.size);
}