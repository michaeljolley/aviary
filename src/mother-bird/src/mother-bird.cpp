#include <ctime>
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

struct DailyWeatherForecast {
  int date;
  //HourlyWeatherForecast[] forecasts;
};

struct HourlyWeatherForecast {
  int hour;
  float precipIntensity;
  float precipProbability;
  float temperatureC;
  float cloudCover;
};

int defaultStartHour = 11; // 11AM UTC = 6AM CDT
int defaultEndHour = 23; // 11PM UTC = 6PM CDT

MotherBirdSettings settings;
int settingsEEPROMAddress = 0;

DailyWeatherForecast forecast;

/// Calculates whether the current hour is within our
/// allowed window for watering.
bool insideWateringWindow(String deviceName, int moistureLevel) {
  bool response = false;

  int currentHour = Time.hour();

  if (settings.endHour > settings.startHour) {
    if (currentHour < settings.startHour || currentHour > settings.endHour) {
      response = false;
    }
    else {
      response = true;
    }
  }
  else {
    if (currentHour < settings.startHour && currentHour > settings.endHour) {
      response = false;
    }
    else {
      response = true;
    }
  }

  if (!response) {
    if (Particle.connected) {
      String message = deviceName + " reported: " + String(moistureLevel) + " at " + String(currentHour) + ". (Outside hours of " + String(settings.startHour) + " and " + String(settings.endHour) + ")";
      Particle.publish("hydration-prevented", message, PRIVATE);
    }
  }

  return response;
}

// Consider the rain forecast for today in determining whether
// to water the plants.
bool reviewWeatherForecast() {

  // If we don't have a forecast and don't have a connection
  // to particle we can't get the forecast, so we'll assume
  // it's okay to water.
  if (forecast.date == 0 && !Particle.connected) {
    return true;
  }

  int currentDay = Time.day();

  // Either we have no forecast or it isn't for today
  if (forecast.date == 0 ||
      forecast.date != currentDay) {

    // If we have access to Particle, send request
    // to update weather forecast for today
    if (Particle.connected) {

      String data = "{ \"day\":" + String(Time.now()) + " }";

      Particle.publish("weather-forecast", data, PRIVATE);
      delay(30000);
    }
  }

  bool response = false;

  // compare forecast info
  //int currentHour = Time.hour();



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
    if (moistureLevel <= 400) {
      hydrationNeeded = true;
    }

    // Check to ensure we're within our approved watering hours
    hydrationNeeded = hydrationNeeded && insideWateringWindow(deviceName, moistureLevel);

    // Check upcoming weather forecast for rain
    hydrationNeeded = hydrationNeeded && reviewWeatherForecast();

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

    if (Particle.connected) {
      Particle.publish("hydration-needed", deviceName + " (" + moistureLevel + "): should hydrate(" + shouldWeHydrate + ")", PRIVATE);
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

      if (Particle.connected) {
        Particle.publish("settings-updated", data, PRIVATE);
      }
    }
  }
  return 1;
}

/// Updates the forecast property so we can determine
/// whether to water based on rain forecast
void updateForecast(const char *event, const char *data) {


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
    Particle.subscribe("hook-response/weather-forecast", updateForecast, MY_DEVICES);
  }

  Mesh.subscribe("moisture-check", reviewMoistureLevel);
}

// loop() runs over and over again, as quickly as it can execute.
void loop() {
}
