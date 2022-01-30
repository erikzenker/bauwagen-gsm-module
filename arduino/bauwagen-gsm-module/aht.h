#include <Adafruit_AHTX0.h>

Adafruit_AHTX0 aht;

void setupAht(){
  if (! aht.begin()) {
    Serial.println("Could not find AHT? Check wiring");
    while (1) delay(10);
  }
  Serial.println("AHT10 or AHT20 found");
}
