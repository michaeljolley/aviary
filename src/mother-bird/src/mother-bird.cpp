#include "application.h"
#include "JsonParserGeneratorRK.h"

/*
 * Project mother-bird
 * Description: Mother bird listens to messages from baby birds and responds
 * if they need to activate/deactivate their water source.
 * Author: Michael Jolley <mike@sparcapp.io>
 * Date: 2019-05-19
 */

struct MotherBirdSettings {
  int version;
  int startHour;
  int endHour;
};

int defaultStartHour = 11; // 11AM UTC = 6AM CDT
int defaultEndHour = 23; // 11PM UTC = 6PM CDT

MotherBirdSettings settings;
int settingsEEPROMAddress = 0;

/// Calculates whether the current hour is within our
/// allowed window for watering.
bool insideWateringWindow(String deviceName, int moistureLevel) {
  bool response = false;

  int currentHour = Time.hour();

  if (settings.endHour > settings.startHour) {
    if (currentHour < settings.startHour || currentHour > settings.endHour) {
        response = false;
    }
  }
  else {
    if (currentHour < settings.startHour && currentHour > settings.endHour) {
      response = false;
    }
  }

  if (!response) {
    if (Particle.connected) {
      String message = deviceName + " reported: " + String(moistureLevel) + " at " + String(currentHour) + ". (Outside hours of " + String(settings.startHour) + " and " + String(settings.endHour) + ")";
      Particle.publish("hydration-prevented", message, PUBLIC);
    }
  }

  return response;
}

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

    if (hydrationNeeded) {
      // Check to ensure we're within our approved watering hours
      hydrationNeeded = insideWateringWindow(deviceName, moistureLevel);

      // Check upcoming weather forecast for rain

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

/// Save the current settings to EEPROM
void saveDefaultSettings() {
  EEPROM.put(settingsEEPROMAddress, settings);
}

/// Update the default settings based on the provided
/// new settings and save to EEPROM
int updateSettings(String data) {
  JsonParser jsonParser;
	jsonParser.addString(data);

  if (jsonParser.parse()) {

    int startHour = -1;
    int endHour = 25;

    jsonParser.getOuterValueByKey("startHour", startHour);
    jsonParser.getOuterValueByKey("endHour", endHour);

    if (startHour >= 0 && startHour <= 24
        && endHour >= 0 && endHour <= 24) {

      MotherBirdSettings newSettings = {0, startHour, endHour};
      settings = newSettings;
      saveDefaultSettings();
    }
  }
  return 1;
}

/// Try to read and set settings from EEPROM.  If not available
/// use default.
void setDefaultSettings() {
  EEPROM.get(settingsEEPROMAddress, settings);

  if(settings.version != 0) {
    // EEPROM was empty -> initialize settings
    settings = { 0, defaultStartHour, defaultEndHour };
    saveDefaultSettings();
  }
}

// setup() runs once, when the device is first turned on.
void setup() {

  setDefaultSettings();

  if (Particle.connected) {
    Particle.function("updateSettings", updateSettings);
  }

  Mesh.subscribe("moisture-check", reviewMoistureLevel);
}

// loop() runs over and over again, as quickly as it can execute.
void loop() {
}
