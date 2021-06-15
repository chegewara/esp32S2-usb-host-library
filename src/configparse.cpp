
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
#include "esp_log.h"
#include "configparse.h"


char *USBHostConfigParser::class_to_str(uint8_t cls)
{
    switch (cls)
    {
    case USB_CLASS_PER_INTERFACE:
        return ">ifc";
    case USB_CLASS_AUDIO:
        return "Audio";
    case USB_CLASS_COMM:
        return "CDC";
    case USB_CLASS_HID:
        return "HID";
    case USB_CLASS_PHYSICAL:
        return "Physical";
    case USB_CLASS_STILL_IMAGE:
        return "Image";
    case USB_CLASS_PRINTER:
        return "Printer";
    case USB_CLASS_MASS_STORAGE:
        return "Mass Storage";
    case USB_CLASS_HUB:
        return "Hub";
    case USB_CLASS_CDC_DATA:
        return "CDC-data";
    case USB_CLASS_CSCID:
        return "Smart card";
    case USB_CLASS_CONTENT_SEC:
        return "Content security";
    case USB_CLASS_VIDEO:
        return "Video";
    case USB_CLASS_PERSONAL_HEALTHCARE:
        return "Personal heathcare";
    case USB_CLASS_AUDIO_VIDEO:
        return "Audio/Vdeo devices";
    case USB_CLASS_BILLBOARD:
        return "Bilboard";
    case USB_CLASS_USB_TYPE_C_BRIDGE:
        return "USB-C bridge";
    case 0xdc:
        return "Diagnostic device";
    case USB_CLASS_WIRELESS_CONTROLLER:
        return "Wireless controller";
    case USB_CLASS_MISC:
        return "Miscellaneous";
    case USB_CLASS_APP_SPEC:
        return "Application specific";
    case USB_CLASS_VENDOR_SPEC:
        return "Vendor specific";

    default:
        return "Wrong class type";
    }
}

void USBHostConfigParser::utf16_to_utf8(uint8_t *in, char *out, uint8_t len)
{
    for (size_t i = 0; i < len; i++)
    {
        // FIXME
        out[i / 2] = in[i++];
    }
}

uint8_t* USBHostConfigParser::parse_cfg_descriptor(uint8_t *data_buffer, uint8_t **out, uint8_t *_type)
{
    uint8_t offset = 0;
    uint8_t type = *(&data_buffer[0] + offset + 1);
    do
    {
        ESP_LOGD("", "type: %d", type);
        *_type = type;
        switch (type)
        {
        case 0x0:
            break;
        case USB_W_VALUE_DT_DEVICE:
            break;

        case USB_W_VALUE_DT_CONFIG:
        {
            ESP_LOGV("", "Config:");
            usb_desc_cfg_t *data = (usb_desc_cfg_t *)(data_buffer + offset);
            ESP_LOGV("", "Number of Interfaces: %d", data->bNumInterfaces);
            ESP_LOGV("", "bConfig: %d", data->bConfigurationValue);
            ESP_LOGV("", "iConfig: %d", data->iConfiguration);
            ESP_LOGV("", "Attributes: 0x%02x", data->bmAttributes);
            ESP_LOGV("", "Max power: %d mA", data->bMaxPower * 2);
            offset += data->bLength;
            break;
        }
        case USB_W_VALUE_DT_STRING:
        {
            usb_desc_str_t *data = (usb_desc_str_t *)(data_buffer + offset);
            uint8_t len = 0;
            len = data->bLength;
            offset += len;
            char *str = (char *)calloc(1, len);
            utf16_to_utf8(&data->val[2], str, len);
            ESP_LOGV("", "strings: %s", str);
            free(str);
            break;
        }
        case USB_W_VALUE_DT_INTERFACE:
        {
            ESP_LOGV("", "Interface:");
            usb_desc_intf_t *data = (usb_desc_intf_t *)(data_buffer + offset);
            offset += data->bLength;
            ESP_LOGV("", "bInterfaceNumber: %d", data->bInterfaceNumber);
            ESP_LOGV("", "bAlternateSetting: %d", data->bAlternateSetting);
            ESP_LOGV("", "bNumEndpoints: %d", data->bNumEndpoints);
            ESP_LOGV("", "bInterfaceClass: 0x%02x (%s)", data->bInterfaceClass, class_to_str(data->bInterfaceClass));
            ESP_LOGV("", "bInterfaceSubClass: 0x%02x", data->bInterfaceSubClass);
            ESP_LOGV("", "bInterfaceProtocol: 0x%02x", data->bInterfaceProtocol);
            *out = (uint8_t *)data;
            break;
        }
        case USB_W_VALUE_DT_ENDPOINT:
        {
            ESP_LOGV("", "Endpoint:");
            usb_desc_ep_t *data = (usb_desc_ep_t *)(data_buffer + offset);
            offset += data->bLength;
            ESP_LOGV("", "bEndpointAddress: 0x%02x", data->bEndpointAddress);
            ESP_LOGV("", "bmAttributes: 0x%02x", data->bmAttributes);
            ESP_LOGV("", "bDescriptorType: %d", data->bDescriptorType);
            ESP_LOGV("", "wMaxPacketSize: %d", data->wMaxPacketSize);
            ESP_LOGV("", "bInterval: %d ms", data->bInterval);
            *out = (uint8_t *)data;
            break;
        }
        case 0x21:
        {
            ESP_LOGD("", "HID descriptor");
            uint8_t *data = (data_buffer + offset);
            offset += data[0];
            ESP_LOGD("Report map size", "0x%x", (uint16_t)data[7]);
            // report_map_size = (uint16_t)data[7];
            *out = (uint8_t *)data;
            break;
        }
        case USB_W_VALUE_DT_CS_INTERFACE:
        {
            ESP_LOGV("", "CS_Interface:");
            usb_desc_intf_t *data = (usb_desc_intf_t *)(data_buffer + offset);
            offset += data->bLength;

            break;
        }
        default:
            ESP_LOGD("", "unknown descriptor: %d", type);

            offset += *(data_buffer + offset);
            break;
        }
    } while (0);
    return (uint8_t *)data_buffer + offset;
}

uint8_t* USBHostConfigParser::getInterfaceByClass(uint8_t *data, uint8_t cls)
{
    uint8_t *ep;
    uint8_t type;
    uint8_t *next = data + sizeof(usb_ctrl_req_t);
    do
    {
        next = parse_cfg_descriptor(next, &ep, &type);
        if (type == 0x0)
            break;
        if (ep && type == USB_W_VALUE_DT_INTERFACE)
        {
            ESP_LOGV("", "find interface class: %s", class_to_str(cls));
            if (!cls || ((usb_desc_intf_t *)ep)->bInterfaceClass == cls) // IN EP
            {
                ESP_LOGV("", "found interface class: %s", class_to_str(((usb_desc_intf_t *)ep)->bInterfaceClass));
                return ep;
            }
        }
    } while (1);

    return NULL;
}

uint8_t* USBHostConfigParser::getEndpointByDirection(uint8_t *interface, uint8_t dir)
{
    uint8_t *ep;
    uint8_t type;
    uint8_t *next = interface;
    do
    {
        next = parse_cfg_descriptor(next, &ep, &type);
        if (type == 0x0)
            break;
        if (ep && type == USB_W_VALUE_DT_ENDPOINT)
        {
            ESP_LOGV("", "we have new endpoint");
            if (!dir || (USB_DESC_EP_GET_EP_DIR((usb_desc_ep_t *)ep) == dir)) // IN EP == 1
            {
                return ep;
            }
        }
    } while (1);

    return NULL;
}

void USBHostConfigParser::getString(uint8_t *in, char *out, uint8_t len)
{
    return utf16_to_utf8(in, out, len);
}



