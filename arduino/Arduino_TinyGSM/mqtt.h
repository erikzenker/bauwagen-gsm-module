#include <PubSubClient.h>
#include <Adafruit_AHTX0.h>

Adafruit_AHTX0 aht;
// MQTT details
const char *broker = "broker.hivemq.com";
const char *topicLed = "v1/devices/bauwagen/led";
const char *topicInit = "v1/devices/bauwagen/init";
const char *topicSensors = "v1/devices/bauwagen/sensors";
const char *topic = "v1/devices/bauwagen";

uint32_t lastReconnectAttempt = 0;
int ledStatus = LOW;

PubSubClient mqtt(client);

void setupAht(){
  if (! aht.begin()) {
    Serial.println("Could not find AHT? Check wiring");
    while (1) delay(10);
  }
  Serial.println("AHT10 or AHT20 found");
}

void mqttCallback(char *topic, byte *payload, unsigned int len) {
  SerialMon.print("Message arrived [");
  SerialMon.print(topic);
  SerialMon.print("]: ");
  SerialMon.write(payload, len);
  SerialMon.println();

  sensors_event_t humidityEvent, tempEvent;
  aht.getEvent(&humidityEvent, &tempEvent);
  Serial.print("Temperature: "); Serial.print(tempEvent.temperature); Serial.println(" degrees C");
  Serial.print("Humidity: "); Serial.print(humidityEvent.relative_humidity); Serial.println("% rH");

  // if (String(topic) == topicLed) {
  ledStatus = !ledStatus;

  digitalWrite(LED_GPIO, ledStatus);
  String comma = String(",");
  String ledStatus = String("\"ledState\":") + (ledStatus ? String("1") : String("0"));
  String temperature = String("\"temperature\":") + String(tempEvent.temperature);
  String humidity = String("\"humidity\":") + String(humidityEvent.relative_humidity);
  String message = String("{") + ledStatus + comma + temperature + comma + humidity + String("}");
  mqtt.publish(topicSensors, message.c_str());
  // }
}

void sendSensorData() {
  sensors_event_t humidityEvent, tempEvent;
  aht.getEvent(&humidityEvent, &tempEvent);
  Serial.print("Temperature: "); Serial.print(tempEvent.temperature); Serial.println(" degrees C");
  Serial.print("Humidity: "); Serial.print(humidityEvent.relative_humidity); Serial.println("% rH");

  digitalWrite(LED_GPIO, ledStatus);
  String comma = String(",");
  String ledStatus = String("\"ledState\":") + (ledStatus ? String("1") : String("0"));
  String temperature = String("\"temperature\":") + String(tempEvent.temperature);
  String humidity = String("\"humidity\":") + String(humidityEvent.relative_humidity);
  String message = String("{") + ledStatus + comma + temperature + comma + humidity + String("}");
  mqtt.publish(topicSensors, message.c_str());
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
