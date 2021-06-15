#include "cdchost.h"
#define DEVICE_ADDRESS 2
USBHostCDC port(DEVICE_ADDRESS);

void usbh_cdc_device_ready()
{
  port.setControlLine(1, 1);
  port.setLineCoding(115200, 0, 0, 5);
  port.inpipe->inData();
}

void ctrl_pipe_cb(ext_pipe_event_msg_t event, usb_irp_t *irp)
{
  Serial.printf("CTRL EVENT: 0x%x\n", event);
}

void port_cb(port_event_msg_t msg, USBHostPort *port)
{
  Serial.printf("PORT EVENT: 0x%x\n", msg.port_event);
}

void cdc_datain_cb(ext_pipe_event_msg_t event, usb_irp_t *irp)
{
  for (size_t i = 0; i < irp->actual_num_bytes; i++)
  {
    Serial.printf("%c", irp->data_buffer[i]);
  }
}

// this is optional callback, it is just a status check and/or echo from low level stack
void cdc_dataout_cb(ext_pipe_event_msg_t event, usb_irp_t *irp)
{
  Serial.printf("OUT EVENT: 0x%x, buffer_len: %d, sent: %d\n", event, irp->num_bytes, irp->actual_num_bytes);
  Serial.print("DATA: ");
  for (size_t i = 0; i < irp->actual_num_bytes; i++)
  {
    Serial.printf("%c", irp->data_buffer[i]);
  }
  Serial.println();
}

void setup()
{
  Serial.begin(115200);
  port.onPortEvent(port_cb);
  port.onControlEvent(ctrl_pipe_cb);
  port.onDataIn(cdc_datain_cb);
  port.onDataOut(cdc_dataout_cb);
  port.init();
}

void loop()
{
  delay(10);
  while (Serial.available())
  {
    test_strings();
    size_t l = Serial.available();
    uint8_t b[l];
    l = Serial.read(b, l);
    port.sendData(b, l);
  }
}
