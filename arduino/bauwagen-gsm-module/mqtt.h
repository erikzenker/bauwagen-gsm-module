#include <PubSubClient.h>

// MQTT details
const char *broker = "broker.hivemq.com";
const char *topicLed = "v1/devices/bauwagen/led";
const char *topicInit = "v1/devices/bauwagen/init";
const char *topicSensors = "v1/devices/bauwagen/sensors";
const char *topic = "v1/devices/bauwagen";

uint32_t lastReconnectAttempt = 0;
RTC_DATA_ATTR int bootCount = 0;

PubSubClient mqtt(client);


void sendSensorData() {
  sensors_event_t humidityEvent, tempEvent;
  aht.getEvent(&humidityEvent, &tempEvent);

  String comma = String(",");
  String signalQuality = String("\"signalQuality\":") + String(modem.getSignalQuality());
  String temperature = String("\"temperature\":") + String(tempEvent.temperature);
  String bootCountS = String("\"bootCount\":") + String(bootCount);
  String humidity = String("\"humidity\":") + String(humidityEvent.relative_humidity);
  String message = String("{") + signalQuality + comma + bootCountS + comma + temperature + comma + humidity + String("}");
  Serial.print("Send mqtt message: " + message);
  mqtt.publish(topicSensors, message.c_str());

  bootCount++;
}

void mqttCallback(char *topic, byte *payload, unsigned int len) {
  SerialMon.print("Message arrived [");
  SerialMon.print(topic);
  SerialMon.print("]: ");
  SerialMon.write(payload, len);
  SerialMon.println();

  sendSensorData();
}

boolean mqttConnect() {
  SerialMon.print("Connecting to ");
  SerialMon.print(broker);

  // Connect to MQTT Broker
  boolean status = mqtt.connect(topic);

  // Or, if you want to authenticate MQTT:
  // boolean status = mqtt.connect("GsmClientName", "mqtt_user", "mqtt_pass");

  if (status == false) {
    SerialMon.println(" fail");
    return false;
  }
  SerialMon.println(" success");
  mqtt.publish(topicInit, "v1/devices/bauwagen started");
  mqtt.subscribe(topicLed);
  return mqtt.connected();
}

void reconnectMqtt() {
  if (!mqtt.connected()) {
    SerialMon.println("=== MQTT NOT CONNECTED ===");
    // Reconnect every 10 seconds
    uint32_t t = millis();
    if (t - lastReconnectAttempt > 10000L) {
      lastReconnectAttempt = t;
      if (mqttConnect()) {
        lastReconnectAttempt = 0;
      }
    }
    delay(100);
    return;
  }
}

void setupMqtt() {
  mqtt.setServer(broker, 1883);
  mqtt.setCallback(mqttCallback);
}
