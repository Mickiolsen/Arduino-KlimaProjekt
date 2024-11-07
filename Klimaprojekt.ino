//#include <Arduino.h>
#include <DHT_Async.h>
#include <Bounce2.h>
#include <EEPROM.h>

// Definer sensor-type til DHT11
#define DHT_SENSOR_TYPE DHT_TYPE_11
static const int DHT_SENSOR_PIN = 8;
DHT_Async dht_sensor(DHT_SENSOR_PIN, DHT_SENSOR_TYPE);  // Opsætning af DHT11 sensor

// Konfiguration for knap og LEDs
Bounce2::Button button = Bounce2::Button();
int offbtn = 22;
int led_green = 12;
int led_red = 3;
int led_blue = 11;
int led_yellow = 2;

bool offButton;
int tempSet;
bool ledStateYellow = HIGH;

/**
 * @brief Læs termostatens tilstand fra EEPROM.
 */
void readThermostatStateFromEEPROM() {
  offButton = EEPROM.read(0);
}

/**
 * @brief Gem termostatens tilstand til EEPROM.
 */
void saveThermostatStateToEEPROM() {
  // EEPROM.write(0, offButton);
}

/**
 * @brief Initialiserer opsætningen.
 */
void setup() {
  Serial.begin(9600);

  // Knap opsætning
  button.attach(offbtn, INPUT_PULLUP);
  button.interval(10);
  button.setPressedState(LOW);

  // Læs termostatens tilstand fra EEPROM
  readThermostatStateFromEEPROM();
  ledStateYellow = offButton ? HIGH : LOW;

  // LED opsætning
  pinMode(led_green, OUTPUT);
  pinMode(led_red, OUTPUT);
  pinMode(led_blue, OUTPUT);
  pinMode(led_yellow, OUTPUT);

  digitalWrite(led_yellow, ledStateYellow);
  digitalWrite(led_green, LOW);
  digitalWrite(led_red, LOW);
  digitalWrite(led_blue, LOW);
}

/**
 * @brief Håndter knaptryk og skift termostatens tilstand.
 */
void handleButtonPress() {
  if (button.pressed()) {
    Serial.println("Knappen blev trykket");
    // delay(100);

    offButton = !offButton;
    saveThermostatStateToEEPROM();
    ledStateYellow = !ledStateYellow;
    digitalWrite(led_yellow, ledStateYellow);

    // Sluk for LED'er, når termostaten er inaktiv
    if (offButton) {
      digitalWrite(led_green, LOW);
      digitalWrite(led_red, LOW);
      digitalWrite(led_blue, LOW);
    }
  }
}

/**
 * @brief Læs og konverter potentiometer-værdi til temperaturinterval.
 * @return Den ønskede temperatur fra potentiometeret.
 */
int readTemperatureSetting() {
  int temp = analogRead(A0);
  return map(temp, 0, 50, 0, 2);
}

/**
 * @brief Læs temperaturen fra DHT-sensoren på en ikke-blokerende måde.
 * @param temperature Reference til temperaturværdien.
 * @return True, hvis temperaturen er blevet opdateret; ellers false.
 */
bool measure_environment(float *temperature, float *humidity) {
  static unsigned long measurement_timestamp = millis();

  // Måler en gang hver fire sekunder
  if (millis() - measurement_timestamp > 200ul) {
    if (dht_sensor.measure(temperature, humidity)) {
      measurement_timestamp = millis();
      return true;
    }
  }
  return false;
}

/**
 * @brief Aflæs temperaturen fra sensoren og styr LED'erne baseret på forskel til forvalgt temperatur.
 */
void handleTemperatureControl() {
  float temperature, humidity;

  if (measure_environment(&temperature, &humidity)) {
    Serial.print("Aktuel Temperatur: ");
    Serial.print(temperature, 1);
    Serial.println("°C");

    if (abs(tempSet - temperature) <= 1) {
      // Grøn LED tændes, når temperaturen er tæt på forvalgt værdi
      digitalWrite(led_green, HIGH);
      digitalWrite(led_red, LOW);
      digitalWrite(led_blue, LOW);
      digitalWrite(led_yellow, LOW);
    } 
    else if (tempSet > temperature) {
      // Rød LED tændes, når temperaturen er under forvalgt værdi
      digitalWrite(led_red, HIGH);
      digitalWrite(led_green, LOW);
      digitalWrite(led_blue, LOW);
      digitalWrite(led_yellow, LOW);
    } 
    else {
      // Blå LED tændes, når temperaturen er over forvalgt værdi
      digitalWrite(led_blue, HIGH);
      digitalWrite(led_green, LOW);
      digitalWrite(led_red, LOW);
      digitalWrite(led_yellow, LOW);
    }
  }
}

void loop() {
  button.update();
  handleButtonPress();

  tempSet = readTemperatureSetting();

  if (!offButton) {
    handleTemperatureControl();
  } else {
    Serial.println("Klimaanlægget er slukket");
    // delay(1000);
  }
}
