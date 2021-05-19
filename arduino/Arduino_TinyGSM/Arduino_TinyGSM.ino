#define SIM800L_IP5306_VERSION_20190610
// Set serial for debug console (to the Serial Monitor, default speed 115200)
#define SerialMon Serial
// Set serial for AT commands (to the module)
#define SerialAT Serial1

#include "utilities.h"
#include "gsm.h"
#include "mqtt.h"


#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  5        /* Time ESP32 will go to sleep (in seconds) */

RTC_DATA_ATTR int bootCount = 0;

void setup() {
  // Set console baud rate
  SerialMon.begin(115200);

  delay(10);

  // Start power management
  if (setupPMU() == false) {
    Serial.println("Setting power error");
  }

  setupAht();

  setupModem();

  // Set GSM module baud rate and UART pins
  SerialAT.begin(115200, SERIAL_8N1, MODEM_RX, MODEM_TX);

  startModem();
  setupMqtt();
}

void loop() {

  reconnectMqtt();

  mqtt.loop();

  sendSensorData();
  delay(1000);
  // stopModem();
}
