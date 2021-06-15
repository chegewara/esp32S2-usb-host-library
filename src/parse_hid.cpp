#include "parse_hid.h"
#include <stdio.h>

#if defined(ARDUINO_ARCH_ESP32) && defined(CONFIG_ARDUHAL_ESP_LOG)
#include "esp32-hal-log.h"
#else
#include "esp_log.h"
#endif

static uint8_t global_item = 0;
static uint8_t local_item = 0;
static uint8_t usage_page_item = 0;
static uint8_t usage_item = 0;
static uint8_t report_count = 0;
static uint8_t report_size = 0;
static uint8_t reportID = 0;
static uint16_t report_bits = 0;

static hid_mouse_t mouse;
static bool set_x, set_y, set_wheel, set_buttons;

static void setup_buttons()
{
    mouse.buttons.bits_start = report_bits;
    mouse.buttons.count += report_count * report_size;
    mouse.buttons.size = report_size;
    ets_printf("buttons => start bits: %d, count: %d, size: %d\n", report_bits, report_count, report_size);
    report_bits += report_count * report_size;
    set_buttons = false;
}

static void setup_x_axis()
{
    mouse.axes.bits_start = report_bits;
    mouse.axes.count = report_count;
    mouse.axes.size = report_size;
    ets_printf("X => start bits: %d, count: %d, size: %d\n", report_bits, report_count, report_size);
    report_bits += report_size * report_count;
    set_x = false;
}

static void setup_y_axis()
{
    mouse.axes.bits_start = report_bits;
    mouse.axes.count = report_count;
    mouse.axes.size = report_size;
    ets_printf("Y => start bits: %d, count: %d, size: %d\n", report_bits, report_count, report_size);
    report_bits += report_size;
    set_y = false;
}

static void setup_wheel()
{
    mouse.wheel.bits_start = report_bits;
    mouse.wheel.count = report_count;
    mouse.wheel.size = report_size;
    ets_printf("wheel => start bits: %d, count: %d, size: %d\n", report_bits, report_count, report_size);
    report_bits += report_size;
    set_wheel = false;
}

static void setup_input_device()
{
    switch (usage_page_item)
    {
    case GENERIC_DESKTOP_POINTER:
        ets_printf("generic desktop pointer\n");
        if(set_x) setup_x_axis();
        // if(set_y) setup_y_axis();
        if(set_buttons) setup_buttons();
        if(set_wheel) setup_wheel();

        break;

    case GENERIC_DESKTOP_MOUSE:
        ets_printf("generic desktop mouse\n");
        // setup_mouse();
        break;

    case GENERIC_DESKTOP_JOYSTICK:
        ets_printf("generic desktop joystick\n");
        break;

    case GENERIC_DESKTOP_GAMEPAD:
        ets_printf("generic desktop gamepad\n");
        break;

    case GENERIC_DESKTOP_KEYBAORD:
        ets_printf("generic desktop keyboard\n");
        break;

    case GENERIC_DESKTOP_KEYPAD:
        ets_printf("generic desktop keypad\n");
        break;

    case GENERIC_DESKTOP_MULTIAXIS:
        ets_printf("generic desktop multi-axis\n");
        break;

    case GENERIC_DESKTOP_BUTTON:
        ets_printf("generic desktop button\n");
        set_buttons = true;
        setup_buttons();
        break;

    default:
        break;
    }
}

inline void parse_usage_page(uint32_t item)
{
    usage_page_item = item;
}

inline void parse_usage(uint32_t item)
{
    usage_item = item;
    switch (usage_item)
    {
    case GENERIC_DESKTOP_X:
        ets_printf("generic desktop X axis\n");
        set_x = true;
        break;

    case GENERIC_DESKTOP_Y:
        ets_printf("generic desktop Y axis\n");
        set_y = true;
        break;

    case GENERIC_DESKTOP_WHEEL:
        ets_printf("generic desktop wheel\n");
        set_wheel = true;
        break;

    default:
        break;
    }
}

static void parse_global_item(uint8_t type, uint8_t len, uint8_t *val)
{
    uint32_t value = 0;
    for (size_t i = 0; i < len; i++)
    {
        value += val[i] << (i * 8);
    }

    // ets_printf("global type: %02x, len: %d, value: %d\n", type, len, value);
    switch (type)
    {
    case USAGE_PAGE(0):
        parse_usage_page(value);
        break;
    case LOGICAL_MINIMUM(0):
        break;
    case LOGICAL_MAXIMUM(0):
        break;
    case 0x34:
        break;
    case 0x44:
        break;
    case 0x54:
        break;
    case 0x64:
        break;
    case 0x74:
        report_size = value;
        break;
    case 0x84:
        reportID = *val;
        break;
    case 0x94:
        report_count = value;
        break;
    case 0xa4:
        break;
    case 0xb4:
        break;

    default:
        break;
    }
}

static void parse_local_item(uint8_t type, uint8_t len, uint8_t *val)
{
    uint32_t value = 0;
    for (size_t i = 0; i < len; i++)
    {
        value += val[i] << (i * 8);
    }

    // ets_printf("local type: %02x, len: %d, value: %d\n", type, len, value);
    switch (type)
    {
    case 0x08:
        parse_usage(value);
        break;
    case 0x18:
        break;
    case 0x28:
        break;
    case 0x38:
        break;
    case 0x48:
        break;
    case 0x58:
        break;
    case 0x68:
        break;
    case 0x78:
        break;
    case 0x88:
        break;
    case 0x98:
        break;
    case 0xA8:
        break;

    default:
        break;
    }
}

inline void parse_feature_item(uint8_t type, uint8_t len, uint8_t *val)
{
    uint32_t value = 0;
    for (size_t i = 0; i < len; i++)
    {
        value += val[i] << (i * 8);
    }

    // ets_printf("feature type: %02x, len: %d, value: %d\n", type, len, value);
    switch (type)
    {
    case 0x80:
        setup_input_device();
        break;
    case 0x90:
        break;
    case 0xA0:
        break;
    case 0xB0:
        break;
    case 0xC0:
        break;

    default:
        break;
    }
}

void parse_hid_report_map(uint8_t *map, size_t size)
{
    ESP_LOGI("", "size: %d", size);
    for (size_t i = 0; i < size; i++)
    {
        uint8_t type = map[i] & 0xfc;
        uint8_t len = map[i] & 0x03;
        uint8_t *value = &map[i + 1];

        if (type & (1 << 2))
        {
            parse_global_item(type, len, value);
        }
        else if (type & (1 << 3))
        {
            parse_local_item(type, len, value);
        }
        else
        {
            parse_feature_item(type, len, value);
        }

        i += len;
    }
}

hid_mouse_t *get_mouse_struct()
{
    return &mouse;
}

uint16_t bitmask(uint8_t i, uint8_t count)
{
    uint16_t val = 0;
    for (i = 0; i < count; i++)
    {
        val |= 1<<i;
    }
    return val;
}

// always add 1 bytes, is byte is always reportID
uint8_t get_buttons(uint8_t *data)
{
    uint8_t start = 1 + mouse.buttons.bits_start / 8;    
    return data[start];
}

int16_t get_x_axis(uint8_t *data)
{
    uint16_t value = 0;
    uint8_t start = 1 + mouse.axes.bits_start / 8;
    uint8_t bits = mouse.axes.count * mouse.axes.size; // 2 * 12 = 24
    uint8_t n = bits / 8;
    for (size_t i = 0; i < n; i++)
    {
        value += data[start + i] << ((n-i-1) * 8);
    }

    ets_printf("X => start: %d, bits: %d, n: %d, value: %d\n", start, bits, n, value);
    return ((value >> (bits - mouse.axes.size)) | (value & 1<<(mouse.axes.size-1)));
}

int16_t get_y_axis(uint8_t *data)
{
    uint16_t value = 0;
    uint8_t start = 1 + mouse.axes.bits_start / 8;
    uint8_t bits = mouse.axes.count * mouse.axes.size; // 3 * 8 = 24
    uint8_t n = bits / 8;
    for (size_t i = 0; i < n; i++)
    {
        value += data[start + i] << ((n-i-1) * 8);
    }

    ets_printf("Y => start: %d, bits: %d, n: %d, value: %d\n", start, bits, n, value);
    return (((value & (1<< mouse.axes.size)) >> (bits - mouse.axes.size * 2)) | (value & 1<<(mouse.axes.size-1)));
}

int8_t get_wheel(uint8_t *data)
{
    uint16_t value = 0;
    uint8_t start = 1 + mouse.wheel.bits_start / 8;
    uint8_t bits = mouse.wheel.count * mouse.wheel.size; // 3 * 8 = 24
    uint8_t n = bits / 8;
    for (size_t i = 0; i < n; i++)
    {
        value += data[start + i] << ((n-i-1) * 8);
    }

    ets_printf("wheel => start: %d, bits: %d, n: %d, value: %d\n", start, bits, n, value);
    return ((value >> (bits - mouse.wheel.size)) | (value & 1<<(mouse.wheel.size-1)));
}


