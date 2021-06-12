#pragma once

#define HIDINPUT(size) (0x80 | size)
#define HIDOUTPUT(size) (0x90 | size)

#define FEATURE(size) (0xb0 | size)
#define COLLECTION(size) (0xa0 | size)
#define END_COLLECTION(size) (0xc0 | size)

/* Global items */
#define USAGE_PAGE(size) (0x04 | size)
#define LOGICAL_MINIMUM(size) (0x14 | size)
#define LOGICAL_MAXIMUM(size) (0x24 | size)
#define PHYSICAL_MINIMUM(size) (0x34 | size)
#define PHYSICAL_MAXIMUM(size) (0x44 | size)
#define UNIT_EXPONENT(size) (0x54 | size)
#define UNIT(size) (0x64 | size)
#define REPORT_SIZE(size) (0x74 | size) //bits
#define REPORT_ID(size) (0x84 | size)
#define REPORT_COUNT(size) (0x94 | size) //bytes
#define PUSH(size) (0xa4 | size)
#define POP(size) (0xb4 | size)

/* Local items */
#define USAGE(size) (0x08 | size)
#define USAGE_MINIMUM(size) (0x18 | size)
#define USAGE_MAXIMUM(size) (0x28 | size)
#define DESIGNATOR_INDEX(size) (0x38 | size)
#define DESIGNATOR_MINIMUM(size) (0x48 | size)
#define DESIGNATOR_MAXIMUM(size) (0x58 | size)
#define STRING_INDEX(size) (0x78 | size)
#define STRING_MINIMUM(size) (0x88 | size)
#define STRING_MAXIMUM(size) (0x98 | size)
#define DELIMITER(size) (0xa8 | size)

#define GENERIC_DESKTOP_POINTER         0x01
#define GENERIC_DESKTOP_MOUSE           0x02
#define GENERIC_DESKTOP_JOYSTICK        0x04
#define GENERIC_DESKTOP_GAMEPAD         0x05
#define GENERIC_DESKTOP_KEYBAORD        0x06
#define GENERIC_DESKTOP_KEYPAD          0x07
#define GENERIC_DESKTOP_MULTIAXIS       0x08
#define GENERIC_DESKTOP_BUTTON          0x09


#define GENERIC_DESKTOP_X               0x30
#define GENERIC_DESKTOP_Y               0x31
#define GENERIC_DESKTOP_WHEEL           0x38


typedef struct {
    struct 
    {
        uint16_t bits_start;
        uint8_t count;
        uint8_t size;       // bits
        uint16_t value;     // up to 16 buttons
        uint8_t reserved;   // number of dummy bits
    }buttons __attribute__((packed));
    struct 
    {
        uint16_t bits_start;
        uint8_t count;
        uint8_t size;       // bits
        uint8_t* value;
    }axis_x __attribute__((packed));
    struct 
    {
        uint16_t bits_start;
        uint8_t count;
        uint8_t size;       // bits
        uint8_t* value;
    }axis_y __attribute__((packed));
    struct 
    {
        uint16_t bits_start;
        uint8_t count;
        uint8_t size;       // bits
        uint8_t* value;
    }axes __attribute__((packed));
    struct 
    {
        uint16_t bits_start;
        uint8_t count;
        uint8_t size;       // bits
        uint8_t* value;
    }wheel __attribute__((packed));
}hid_mouse_t;


hid_mouse_t* get_mouse_struct();
uint8_t get_buttons(uint8_t* data);
int16_t get_x_axis(uint8_t* data);
int16_t get_y_axis(uint8_t* data);
int8_t get_wheel(uint8_t* data);