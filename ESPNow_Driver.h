/*****************************************************************************
  | File        :   ESPNow_Driver.h
  |
  | Description :   ESP-NOW wireless receiver for Td5 ECU data
  |                 Compatible with Td5_ECU_Interface ESP-NOW broadcast protocol
******************************************************************************/

#ifndef ESPNOW_DRIVER_H
#define ESPNOW_DRIVER_H

#include <esp_now.h>
#include <WiFi.h>

// ESP-NOW Message Types (must match transmitter)
enum Td5MessageType {
  TD5_MSG_FUELLING = 0x01,
  TD5_MSG_INPUTS = 0x02,
  TD5_MSG_TEMPERATURES = 0x03,
  TD5_MSG_PRESSURES = 0x04,
  TD5_MSG_ACTUATORS = 0x05,
  TD5_MSG_STATUS = 0x06
};

// ESP-NOW Packet Header Structure (8 bytes)
struct __attribute__((packed)) Td5PacketHeader {
  uint32_t timestamp;      // milliseconds since transmitter boot
  uint16_t sequence;       // incrementing sequence number
  uint8_t messageType;     // Td5MessageType enum
  uint8_t dataLength;      // length of data payload
};

// Data Payload Structures (must match transmitter exactly)
struct __attribute__((packed)) Td5FuellingData {
  uint16_t vehicleSpeed;      // km/h
  uint16_t engineRPM;         // RPM
  uint16_t injectionQuantity; // mg/stroke * 100
  uint16_t manifoldAirFlow;   // kg/h * 10
  uint16_t driverDemand;      // % * 100
};

struct __attribute__((packed)) Td5InputsData {
  uint8_t switchStates;       // Bit field of switch states
  uint8_t gearPosition;       // Gear position (0-7)
  uint8_t reserved[2];        // Reserved for future use
};

struct __attribute__((packed)) Td5TemperaturesData {
  int16_t coolantTemp;        // °C
  int16_t fuelTemp;           // °C
  int16_t inletAirTemp;       // °C
  int16_t ambientAirTemp;     // °C
  uint16_t batteryVoltage;    // mV
};

struct __attribute__((packed)) Td5PressuresData {
  uint16_t manifoldPressure;  // kPa
  uint16_t ambientPressure;   // kPa
  uint16_t boostPressure;     // kPa (calculated MAP - AAP)
  uint16_t referenceVoltage;  // mV
};

struct __attribute__((packed)) Td5ActuatorsData {
  uint16_t egrPosition;       // % * 100
  uint16_t wastegatePosition; // % * 100
  uint8_t reserved[4];        // Reserved for future actuators
};

struct __attribute__((packed)) Td5StatusData {
  uint8_t connectionState;    // ECU connection state (0-4)
  uint8_t lastErrorCode;      // Last ECU error code received
  uint16_t connectionUptime;  // Seconds since successful connection
  uint32_t totalPacketsSent;  // Total ESP-NOW packets transmitted
};

// Complete ESP-NOW packet structure
struct __attribute__((packed)) Td5EspNowPacket {
  Td5PacketHeader header;
  union {
    Td5FuellingData fuelling;
    Td5InputsData inputs;
    Td5TemperaturesData temperatures;
    Td5PressuresData pressures;
    Td5ActuatorsData actuators;
    Td5StatusData status;
    uint8_t rawData[240];     // Maximum payload size
  } payload;
  uint8_t checksum;
};

// Global data storage (latest received values)
struct ESPNowData {
  // Fuelling
  uint16_t vehicleSpeed;
  uint16_t engineRPM;
  uint16_t injectionQuantity;
  uint16_t manifoldAirFlow;
  uint16_t driverDemand;

  // Inputs
  bool brakePedalPressed;
  bool clutchPedalPressed;
  bool handbrakeEngaged;
  uint8_t gearPosition;

  // Temperatures
  int16_t coolantTemp;
  int16_t fuelTemp;
  int16_t inletAirTemp;
  int16_t ambientAirTemp;
  uint16_t batteryVoltage;

  // Pressures
  uint16_t manifoldPressure;
  uint16_t ambientPressure;
  uint16_t boostPressure;
  uint16_t referenceVoltage;

  // Actuators
  uint16_t egrPosition;
  uint16_t wastegatePosition;

  // Status
  uint8_t connectionState;
  uint16_t connectionUptime;

  // Reception statistics
  uint32_t packetsReceived;
  uint32_t packetErrors;
  uint32_t lastPacketTime;
};

// External access to received data
extern ESPNowData espnow_data;
extern volatile bool espnow_data_ready;

// Function declarations
void espnow_init(void);
uint8_t calculateChecksum(uint8_t* data, size_t length);

#endif // ESPNOW_DRIVER_H
