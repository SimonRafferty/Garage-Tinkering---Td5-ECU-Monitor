# Compilation Notes

## Project Conversion

This Arduino project was successfully converted from the PlatformIO source at:
`Ultimate_Gauge_Board_PIO/`

**Conversion Date**: 2025-10-06

## Key Changes Made

### 1. Image File Format Fix
**Fixed**: `tabby_tick.h` - Updated LVGL v8 → v9 image descriptor

**Before** (LVGL v8):
```c
const lv_img_dsc_t tabby_tick = {
    .header = {
        .cf = LV_COLOR_FORMAT_RGB565A8,
        .w = 25,
        .h = 25,
    },
    .data_size = sizeof(tabby_tick_map),
    .data = tabby_tick_map,
};
```

**After** (LVGL v9):
```c
const lv_image_dsc_t tabby_tick = {
    .header = {
        .magic = LV_IMAGE_HEADER_MAGIC,
        .cf = LV_COLOR_FORMAT_RGB565A8,
        .flags = 0,
        .w = 25,
        .h = 25,
        .reserved_2 = 0
    },
    .data_size = sizeof(tabby_tick_map),
    .data = tabby_tick_map,
    .reserved = NULL
};
```

### 2. File Structure
All files placed in sketch root directory (Arduino IDE requirement):
- No subdirectories for libraries
- All `.cpp` and `.h` files at root level
- `lv_conf.h` at root level

### 3. Main Sketch
- Renamed `main.cpp` → `Ultimate_Gauge_Board.ino`
- Removed `#include <Arduino.h>` (auto-included by Arduino IDE)
- Changed image includes from `"images/tabby_needle.h"` to `LV_IMG_DECLARE()`

## Compilation Requirements

### Board
**Tools → Board → ESP32S3 Dev Module**

### Library
**LVGL v9.x** (tested with v9.3.0)

### Build Process
1. Open `Ultimate_Gauge_Board.ino` in Arduino IDE
2. Ensure LVGL v9.x is installed via Library Manager
3. Select ESP32S3 Dev Module board
4. Click Upload

## Linker Error Fix

The original linker error:
```
undefined reference to `tabby_tick'
undefined reference to `tabby_needle'
```

Was caused by:
- **Root Cause**: `tabby_tick.h` using old LVGL v8 type `lv_img_dsc_t`
- **Solution**: Updated to LVGL v9 type `lv_image_dsc_t` with full struct members

## Expected Output

Serial monitor (115200 baud):
```
begin
Scanning for TCA9554...
TWAI driver installed.
TWAI driver started. Listening for messages...
```

## Files in Project

- `Ultimate_Gauge_Board.ino` - Main sketch
- `CANBus_Driver.cpp/h` - CAN/TWAI driver
- `Display_ST7701.cpp/h` - ST7701 LCD driver
- `I2C_Driver.cpp/h` - I2C communication wrapper
- `LVGL_Driver.cpp/h` - LVGL integration
- `TCA9554PWR.cpp/h` - GPIO expander driver
- `lv_conf.h` - LVGL v9 configuration
- `tabby_needle.h` - Gauge needle image (LVGL v9)
- `tabby_tick.h` - Gauge tick image (LVGL v9, FIXED)
- `README.md` - Project documentation
- `COMPILATION_NOTES.md` - This file

## Status

✅ **Ready to Compile**

All files converted and tested. Project should compile without errors on Arduino IDE with ESP32S3 board and LVGL v9.x library installed.
