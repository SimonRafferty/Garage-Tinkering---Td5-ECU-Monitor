# ESP32-S3 Ultimate Gauge Board - Arduino IDE

ESP32-S3 automotive gauge display for CAN bus data using LVGL v9 and ST7701 RGB LCD.

## Hardware Requirements

- **MCU**: ESP32-S3 (with RGB LCD peripheral support)
- **Display**: ST7701 480x480 RGB LCD (16-bit parallel interface)
- **I2C Expander**: TCA9554PWR (optional, at address 0x20)
- **CAN Interface**: 500kbps, GPIO 4 (RX), GPIO 5 (TX)
- **I2C**: GPIO 7 (SCL), GPIO 15 (SDA)

## Arduino IDE Setup

### Board Selection
**Tools → Board → esp32 → ESP32S3 Dev Module**

⚠️ **CRITICAL**: Must select ESP32-S3 (NOT ESP32-C6 or other variants)

### Required Libraries

Install via Arduino Library Manager:
- **LVGL**: Version 9.x (tested with v9.3.0)

### Build Settings

Recommended settings:
- **Upload Speed**: 921600
- **USB CDC On Boot**: Enabled
- **Flash Size**: 8MB
- **Partition Scheme**: Huge App
- **PSRAM**: OPI PSRAM

## File Structure

```
Ultimate_Gauge_Board/
├── Ultimate_Gauge_Board.ino    (Main sketch)
├── CANBus_Driver.cpp/h         (CAN/TWAI driver)
├── Display_ST7701.cpp/h        (ST7701 LCD driver)
├── I2C_Driver.cpp/h            (I2C wrapper)
├── LVGL_Driver.cpp/h           (LVGL integration)
├── TCA9554PWR.cpp/h            (GPIO expander driver)
├── lv_conf.h                   (LVGL v9 configuration)
├── tabby_needle.h              (Gauge needle image)
├── tabby_tick.h                (Gauge tick image)
└── README.md                   (this file)
```

## Key Features

- **CAN Bus**: Reads Nissan coolant temperature (ID 0x551)
- **FreeRTOS**: Dual-core task management
  - Core 1: CAN message reception and processing
- **LVGL v9 UI**: Custom gauge with animated needle
- **Moving Average**: 10-sample smoothing filter
- **Auto-detect**: TCA9554 I2C expander (gracefully skips if not found)

## Configuration

Edit these constants in `Ultimate_Gauge_Board.ino`:

```cpp
const int SCALE_MIN = -200;      // Gauge minimum (°C × 10)
const int SCALE_MAX = 1400;      // Gauge maximum (°C × 10)
const int SCALE_TICKS_COUNT = 9; // Number of tick marks
const bool TESTING = false;      // Set true for sweep test
```

## Compilation

1. Open `Ultimate_Gauge_Board.ino` in Arduino IDE
2. Select ESP32S3 Dev Module board
3. Ensure LVGL v9.x is installed
4. Click **Upload**

## Serial Monitor

**Baud Rate**: 115200

Expected output:
```
begin
Scanning for TCA9554...
TWAI driver installed.
TWAI driver started. Listening for messages...
```

## Troubleshooting

### "esp_lcd_rgb_panel_config_t was not declared"
- Wrong board selected → Use **ESP32S3 Dev Module**

### "lv_image_dsc_t does not name a type"
- Wrong LVGL version → Install **LVGL v9.x**

### Compilation errors with LVGL
- Clean build cache: **Tools → Clean**
- Restart Arduino IDE

## Pin Assignments

### LCD RGB Interface (16-bit)
- HSYNC: GPIO 38
- VSYNC: GPIO 39
- DE: GPIO 40
- PCLK: GPIO 41
- DATA0-15: See Display_ST7701.h

### CAN Bus
- TX: GPIO 5
- RX: GPIO 4

### I2C
- SCL: GPIO 7
- SDA: GPIO 15

### LCD Control (via TCA9554)
- Backlight: EXIO_PIN4
- Reset: EXIO_PIN1
- CS: EXIO_PIN3

## Customization

### Change CAN Message
Edit `process_can_queue_task()` in Ultimate_Gauge_Board.ino:
```cpp
case 0x551:  // Change to your CAN ID
  process_scale_value(message.data);
  break;
```

### Update Gauge Range
Modify `SCALE_MIN` and `SCALE_MAX` constants

### Replace Images
Replace `tabby_needle.h` and `tabby_tick.h` with your own LVGL image assets

## Version Info

- **Created**: 2025-10-06
- **Source**: Refactored from PlatformIO project
- **LVGL**: v9.3.0
- **ESP32 Core**: Compatible with ESP-IDF 5.1+

## Notes

- Image files are LVGL v9 format (RGB565A8)
- All driver files are in sketch root (Arduino IDE requirement)
- TCA9554 detection has 10-attempt retry with 50ms delay
- CAN queue holds 32 messages to prevent drops

---

**Original PlatformIO Project**: `Ultimate_Gauge_Board_PIO/`
