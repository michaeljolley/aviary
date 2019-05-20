#include "application.h"
#include "Grove-Ultrasonic-Ranger.h"
#include "JsonParserGeneratorRK.h"

/*
 * Project baby-bird
 * Description: Baby bird monitors soil moisture levels and notifies mother bird.  
 * It also listens for messages from mother bird to activate/deactivate its water source. 
 * Author: Michael Jolley <mike@sparcapp.io>
 * Date: 2019-05-19
 */

Ultrasonic ultrasonic(D2);
int led = D7;

String deviceName = "";
bool isHydrated = true;

/// Sets the device name from Particle cloud so we can send in 
/// all requests to mother-bird.  Also allows us to distinguish 
/// whether messages received from mother-bird are meant for us.
void nameHandler(const char *topic, const char *data) {
    deviceName = String(data);
}

/// Processes messages from mother-bird to activate/deactivate hydration
/// of our planter box.
void updateHydrationStatus(const char *event, const char *data) {
  JsonParser jsonParser;
  jsonParser.addString(data);

  // If we can't parse the JSON object sent from mother-bird we'll
  // just disregard this request
  if (jsonParser.parse()) {
    String dName;
    bool shouldHydrate;

    jsonParser.getOuterValueByKey("deviceName", dName);
    jsonParser.getOuterValueByKey("shouldHydrate", shouldHydrate);

    // Only respond to messages directed to us directly
    if (dName == deviceName) {

      // Update hydration status      
      if (shouldHydrate == isHydrated) {
        isHydrated = !shouldHydrate;
      }
    }
  }
}

/// Checks soil moisture levels and sends to mother-bird
/// for processing.
void checkAndSendRange() {
  JsonWriterStatic<256> jsonWriter;

  int moistureLevel;

  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  // Currently we're using an ultrasonic range sensor
  // but this will be replaced with a moisture sensor
  // in upcoming releases.  Until then the "moistureLevel"
  // being reported is actually a distance in centimeters.
  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	moistureLevel = ultrasonic.MeasureInCentimeters(); //0~400cm

  {
    JsonWriterAutoObject obj(&jsonWriter);

    jsonWriter.insertKeyValue("deviceName", deviceName);
    jsonWriter.insertKeyValue("moistureLevel", moistureLevel);
  }

  Mesh.publish("moisture-check", jsonWriter.getBuffer());
}

/// Monitors the isHydrated property and activates/deactivates
/// our solenoid valve to control water supply to the planter box.
void solenoidControl() {

  // If we're in a hydrated state, ensure the solenoid valve
  // is closed and turn off the LED
  if (isHydrated) {

    digitalWrite(led, LOW);

  }
  // If we're not in a hydrated state, ensure the solenoid valve
  // is open and turn on the LED to denote we are watering
  else {

    digitalWrite(led, HIGH);

  }
}

// setup() runs once, when the device is first turned on.
void setup() {
  Serial.begin(9600);

  Particle.subscribe("particle/device/name", nameHandler);
  Particle.publish("particle/device/name");

  Mesh.subscribe("hydration-needed", updateHydrationStatus);
}

// loop() runs over and over again, as quickly as it can execute.
void loop() {
  
  solenoidControl();

  checkAndSendRange();
	delay(2000);
}