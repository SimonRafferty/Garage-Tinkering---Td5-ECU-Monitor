# ESP32-S3 Ultimate Gauge Board - Arduino IDE

This is a re-factor of the Garage Tinkering project: https://github.com/garagetinkering/Ultimate_Gauge_Board_PIO
to compile within the Arduino IDE

While I agree PlatformIO is better, I just have too much code written in the Arduino IDE for it to be worth switching!

I have added support for ESPNow. in order to connect it to my Land Rover Td5 Diagnostic project: https://github.com/SimonRafferty/Land-Rover-Td5-Arduino-Diagnostics


## Hardware Requirements
- **Garage Tinkering Gauge** https://www.garagetinkering.com/shop

## Arduino IDE Setup

### Board Selection
**Tools → Board → esp32 → ESP32S3 Dev Module**

⚠️ **CRITICAL**: Must select ESP32-S3 Dev Module

### Required Libraries

Install via Arduino Library Manager:
- **LVGL**: Version 9.x (tested with v9.3.0)

### Build Settings

Required settings:
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

- **Dual Data Source Support**:
  - **CAN Bus Mode**: Direct CAN bus reception (original Garage Tinkering functionality)
  - **ESP-NOW Mode**: Wireless reception from Td5 ECU Interface transmitter
- **FreeRTOS**: Dual-core task management (CAN mode only)
  - Core 1: CAN message reception and processing
- **LVGL v9 UI**: Custom gauge with animated needle
- **Configurable Gauge**: Easy selection of data source, range, units, and appearance
- **Moving Average**: Configurable smoothing filter (1-30 samples)
- **Auto-detect**: TCA9554 I2C expander (gracefully skips if not found)

## Configuration

### Data Source Selection

Choose between CAN bus or ESP-NOW in `Ultimate_Gauge_Board.ino`:

```cpp
//#define USE_CANBUS        // Receive data via CAN bus (original mode)
#define USE_ESPNOW        // Receive data via ESP-NOW wireless (Td5 ECU Interface)
```

### Gauge Configuration

Configure what to display using the `gaugeConfig` structure in `Ultimate_Gauge_Board.ino`:

```cpp
struct GaugeConfig {
  GaugeDataSource dataSource;   // What to display
  const char* displayName;      // Name shown on gauge
  const char* units;            // Units shown on gauge
  int scaleMin;                 // Minimum scale value
  int scaleMax;                 // Maximum scale value
  int tickCount;                // Number of tick marks
  int overrangePercent;         // Red arc above this % (100 = disabled)
  int underrangePercent;        // Red arc below this % (0 = disabled)
  int averageWindow;            // Smoothing window (1-30 samples)
  float displayMultiplier;      // Display scaling factor
} gaugeConfig = {
  .dataSource = GAUGE_ENGINE_RPM,
  .displayName = "Tacho",
  .units = "RPM",
  .scaleMin = 0,
  .scaleMax = 5700,
  .tickCount = 6,
  .overrangePercent = 85,       // Red line at 85% (4845 RPM)
  .underrangePercent = 20,      // Red line below 20% (1140 RPM)
  .averageWindow = 10,
  .displayMultiplier = 1.0
};
```

### Available Data Sources (ESP-NOW Mode)

When using ESP-NOW with Td5 ECU Interface, you can display:
- `GAUGE_COOLANT_TEMP` - Engine coolant temperature (°C)
- `GAUGE_ENGINE_RPM` - Engine speed (RPM)
- `GAUGE_BOOST_PRESSURE` - Turbo boost pressure (kPa)
- `GAUGE_VEHICLE_SPEED` - Vehicle speed (km/h)
- `GAUGE_FUEL_TEMP` - Fuel temperature (°C)
- `GAUGE_INLET_AIR_TEMP` - Inlet air temperature (°C)
- `GAUGE_BATTERY_VOLTAGE` - Battery voltage (V)
- `GAUGE_THROTTLE_POS` - Throttle position (%)
- `GAUGE_MANIFOLD_PRESSURE` - Manifold absolute pressure (kPa)
- `GAUGE_EGR_POSITION` - EGR valve position (%)

### Test Mode

Enable continuous needle sweep for testing:

```cpp
const bool TESTING = true;      // Set true for sweep test
```

## Compilation

1. Open `Ultimate_Gauge_Board.ino` in Arduino IDE
2. Select ESP32S3 Dev Module board
3. Ensure LVGL v9.x is installed
4. Click **Upload**

## Serial Monitor

**Baud Rate**: 115200

### CAN Bus Mode Output:
```
begin
Scanning for TCA9554...
TWAI driver installed.
TWAI driver started. Listening for messages...
Data source: CAN Bus
```

### ESP-NOW Mode Output:
```
begin
Scanning for TCA9554...
Initializing ESP-NOW receiver...
ESP32 MAC Address: XX:XX:XX:XX:XX:XX
ESP-NOW receiver initialized successfully
Waiting for Td5 ECU data broadcasts...
Data source: ESP-NOW
LVGL buffers allocated successfully from PSRAM
Buffer size: 153600 bytes each
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

## ESP-NOW Wireless Mode

### Overview
When `USE_ESPNOW` is defined, the gauge receives data wirelessly from a Land Rover Td5 ECU Interface transmitter. The transmitter reads real-time vehicle data via K-Line and broadcasts it over ESP-NOW.

### Td5 ECU Interface Transmitter
- **Repository**: https://github.com/SimonRafferty/Land-Rover-Td5-Arduino-Diagnostics
- **Protocol**: 6 message types broadcast in rotation:
  - Fuelling Data (speed, RPM, injection, MAF, throttle)
  - Input Status (brake, clutch, handbrake, gear)
  - Temperature Data (coolant, fuel, inlet air, ambient, battery)
  - Pressure Data (MAP, AAP, boost, reference voltage)
  - Actuator Data (EGR, wastegate positions)
  - Status Data (connection state, uptime, errors)

### Pairing
ESP-NOW broadcasts are received by **all** devices in range - no pairing required. Simply power on both devices and they will communicate automatically.

### Reception Statistics
View packet reception statistics in serial monitor:
- Packets received count
- Packet error count
- Last packet timestamp

## CAN Bus Mode

### Change CAN Message
Edit `process_can_queue_task()` in Ultimate_Gauge_Board.ino:
```cpp
case 0x551:  // Change to your CAN ID
  process_scale_value(message.data);
  break;
```

## Customization

### Replace Images
Replace `tabby_needle.h` and `tabby_tick.h` with your own LVGL image assets (RGB565A8 format)

## Notes

- Image files are LVGL v9 format (RGB565A8)
- All driver files are in sketch root (Arduino IDE requirement)
- TCA9554 detection has 10-attempt retry with 50ms delay
- CAN queue holds 32 messages to prevent drops

---

