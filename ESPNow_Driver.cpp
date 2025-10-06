/*****************************************************************************
  | File        :   ESPNow_Driver.cpp
  |
  | Description :   ESP-NOW wireless receiver implementation for Td5 ECU data
******************************************************************************/

#include "ESPNow_Driver.h"

// Global data storage
ESPNowData espnow_data = {0};
volatile bool espnow_data_ready = false;

// Forward declaration of callback (updated for ESP32 Arduino Core v2.0.0+)
void onDataReceived(const esp_now_recv_info *recv_info, const uint8_t *data, int len);

/**
 * Initialize ESP-NOW in station mode
 * Sets up WiFi and registers receive callback
 */
void espnow_init(void) {
  Serial.println("Initializing ESP-NOW receiver...");

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  Serial.print("ESP32 MAC Address: ");
  Serial.println(WiFi.macAddress());

  // Initialize ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("ERROR: ESP-NOW initialization failed!");
    return;
  }

  // Register callback for received data
  esp_now_register_recv_cb(onDataReceived);

  Serial.println("ESP-NOW receiver initialized successfully");
  Serial.println("Waiting for Td5 ECU data broadcasts...");

  // Initialize reception statistics
  espnow_data.packetsReceived = 0;
  espnow_data.packetErrors = 0;
  espnow_data.lastPacketTime = 0;
}

/**
 * Calculate XOR checksum for packet validation
 * Must match transmitter checksum algorithm
 */
uint8_t calculateChecksum(uint8_t* data, size_t length) {
  uint8_t checksum = 0;
  for (size_t i = 0; i < length; i++) {
    checksum ^= data[i];  // XOR checksum
  }
  return checksum;
}

/**
 * Process fuelling data packet
 */
void processFuellingData(Td5FuellingData* data) {
  espnow_data.vehicleSpeed = data->vehicleSpeed;
  espnow_data.engineRPM = data->engineRPM;
  espnow_data.injectionQuantity = data->injectionQuantity;
  espnow_data.manifoldAirFlow = data->manifoldAirFlow;
  espnow_data.driverDemand = data->driverDemand;

  espnow_data_ready = true;  // Signal new data available
}

/**
 * Process inputs data packet
 */
void processInputsData(Td5InputsData* data) {
  espnow_data.brakePedalPressed = (data->switchStates & 0x01) != 0;
  espnow_data.clutchPedalPressed = (data->switchStates & 0x04) != 0;
  espnow_data.handbrakeEngaged = (data->switchStates & 0x08) != 0;
  espnow_data.gearPosition = data->gearPosition;

  espnow_data_ready = true;
}

/**
 * Process temperatures data packet
 */
void processTemperaturesData(Td5TemperaturesData* data) {
  espnow_data.coolantTemp = data->coolantTemp;
  espnow_data.fuelTemp = data->fuelTemp;
  espnow_data.inletAirTemp = data->inletAirTemp;
  espnow_data.ambientAirTemp = data->ambientAirTemp;
  espnow_data.batteryVoltage = data->batteryVoltage;

  espnow_data_ready = true;
}

/**
 * Process pressures data packet
 */
void processPressuresData(Td5PressuresData* data) {
  espnow_data.manifoldPressure = data->manifoldPressure;
  espnow_data.ambientPressure = data->ambientPressure;
  espnow_data.boostPressure = data->boostPressure;
  espnow_data.referenceVoltage = data->referenceVoltage;

  espnow_data_ready = true;
}

/**
 * Process actuators data packet
 */
void processActuatorsData(Td5ActuatorsData* data) {
  espnow_data.egrPosition = data->egrPosition;
  espnow_data.wastegatePosition = data->wastegatePosition;

  espnow_data_ready = true;
}

/**
 * Process status data packet
 */
void processStatusData(Td5StatusData* data) {
  espnow_data.connectionState = data->connectionState;
  espnow_data.connectionUptime = data->connectionUptime;

  // Note: Don't set data_ready for status packets since they don't contain gauge data
}

/**
 * ESP-NOW receive callback
 * Called when data is received from any sender
 * IMPORTANT: Runs in interrupt context - keep processing minimal
 * Updated for ESP32 Arduino Core v2.0.0+ (IDF 5.x)
 */
void onDataReceived(const esp_now_recv_info *recv_info, const uint8_t *data, int len) {
  // Extract sender MAC address from recv_info (if needed for debugging)
  // const uint8_t *senderMac = recv_info->src_addr;

  // Minimum packet size check (header + at least 1 byte payload + checksum)
  if (len < sizeof(Td5PacketHeader) + 1) {
    espnow_data.packetErrors++;
    return;
  }

  // Parse header
  Td5PacketHeader* header = (Td5PacketHeader*)data;
  uint8_t* payload = (uint8_t*)data + sizeof(Td5PacketHeader);
  uint8_t receivedChecksum = data[len - 1];

  // Validate checksum
  uint8_t calculatedChecksum = calculateChecksum((uint8_t*)data, len - 1);
  if (calculatedChecksum != receivedChecksum) {
    espnow_data.packetErrors++;
    return;
  }

  // Validate payload length
  if (header->dataLength != len - sizeof(Td5PacketHeader) - 1) {
    espnow_data.packetErrors++;
    return;
  }

  // Update reception timestamp
  espnow_data.lastPacketTime = millis();
  espnow_data.packetsReceived++;

  // Process by message type
  switch (header->messageType) {
    case TD5_MSG_FUELLING:
      processFuellingData((Td5FuellingData*)payload);
      break;

    case TD5_MSG_INPUTS:
      processInputsData((Td5InputsData*)payload);
      break;

    case TD5_MSG_TEMPERATURES:
      processTemperaturesData((Td5TemperaturesData*)payload);
      break;

    case TD5_MSG_PRESSURES:
      processPressuresData((Td5PressuresData*)payload);
      break;

    case TD5_MSG_ACTUATORS:
      processActuatorsData((Td5ActuatorsData*)payload);
      break;

    case TD5_MSG_STATUS:
      processStatusData((Td5StatusData*)payload);
      break;

    default:
      // Unknown message type
      espnow_data.packetErrors++;
      break;
  }
}
