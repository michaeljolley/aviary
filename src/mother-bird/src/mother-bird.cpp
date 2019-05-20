#include "application.h"
#include "JsonParserGeneratorRK.h"

/*
 * Project mother-bird
 * Description: Mother bird listens to messages from baby birds and responds 
 * if they need to activate/deactivate their water source.
 * Author: Michael Jolley <mike@sparcapp.io>
 * Date: 2019-05-19
 */

/// Based on the provided moisture level (and any other factors we decide to add),
/// determine whether the baby-bird should be hydrating the planter box
bool shouldHydrate(String deviceName, int moistureLevel) {
    // To be safe (and save water), we default watering to false
    bool hydrationNeeded = false;

    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // Currently we're using an ultrasonic range sensor
    // but this will be replaced with a moisture sensor
    // in upcoming releases.  Until then the "moistureLevel"
    // being reported is actually a distance in centimeters.
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    if (moistureLevel <= 12) {
      hydrationNeeded = true;
    }

    // Notify particle cloud that we told the baby-bird to hydrate and what the 
    // baby-bird reported as the moisture level that resulted in this decision
    if (hydrationNeeded == true) {
      Particle.publish("hydration-needed", deviceName + " (" + moistureLevel + ")");
    }

    return hydrationNeeded;
}

/// Reviews the moisture level sent by a baby-bird and decides whether or not
/// the baby-bird should activate watering.  Regardless, it responds to the baby-bird
/// with a true or false.
void reviewMoistureLevel(const char *event, const char *data) {

  JsonParser jsonParser;
	jsonParser.addString(data);

  // We'll only send a response if we were able to parse the incoming data
  if (jsonParser.parse()) {

    String deviceName;
    int moistureLevel;

    jsonParser.getOuterValueByKey("deviceName", deviceName);
    jsonParser.getOuterValueByKey("moistureLevel", moistureLevel);

    // Calculate if we should hydrate and return as JSON object with the baby-bird's
    // device name and a boolean denoting whether to water or not
    bool shouldWeHydrate = false;
    shouldWeHydrate = shouldHydrate(deviceName, moistureLevel);

    JsonWriterStatic<256> jsonWriter;
    {
      JsonWriterAutoObject obj(&jsonWriter);
      jsonWriter.insertKeyValue("deviceName", deviceName);
      jsonWriter.insertKeyValue("shouldHydrate", shouldWeHydrate);
    }

    Mesh.publish("hydration-needed", jsonWriter.getBuffer());
  }
}

// setup() runs once, when the device is first turned on.
void setup() {
  Mesh.subscribe("moisture-check", reviewMoistureLevel);
}

// loop() runs over and over again, as quickly as it can execute.
void loop() {
}
