/*
 
Kyle Hudson
Jan 2024 
Arduio code for DHT11 sensor and liquid crystal display 
live laugh love 

*/

#include <dht_nonblocking.h>  // sensor library
#include <LiquidCrystal.h>    // LCD library
#define DHT_SENSOR_TYPE DHT_TYPE_11

static const int DHT_SENSOR_PIN = 2;  // sensor input pin
DHT_nonblocking dht_sensor(DHT_SENSOR_PIN, DHT_SENSOR_TYPE);

//                BS E  D4 D5  D6  D7
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);  // match arduio pins to lcd

const int buttonPin = 13;                // Digital pin connected to the pushbutton
bool showFahrenheit = false;             // Tracks whether the display is showing °C (false) or °F (true)
bool lastButtonState = HIGH;             // Stores the most recent *raw* button reading for debounce comparison
unsigned long lastDebounceTime = 0;      // Timestamp of the last time the button reading changed (for debouncing)
const unsigned long debounceDelay = 50;  // How long the button reading must stay stable before we trust it (ms)

// setup runs once
void setup() {
  lcd.begin(16, 2);
  pinMode(buttonPin, INPUT_PULLUP);
  lcd.clear(); 
  lcd.setCursor(0, 0);
  lcd.print("live laugh love");
  delay(1500);  // show message for 1.5 seconds
  lcd.clear();  // wipe before starting normal display
}

void updateButton() {
  int reading = digitalRead(buttonPin);  // Read the raw button state (HIGH = not pressed, LOW = pressed)

  if (reading != lastButtonState) {  // If the reading changed from the last loop, the button is bouncing
    lastDebounceTime = millis();     // Reset the debounce timer so we wait for it to settle
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {  // Only act on the button if the reading has been stable
    static bool buttonState = HIGH;                     // Track the *debounced* button state separately from the raw reading
    if (reading != buttonState) {                       // If the stable state has changed, update it
      buttonState = reading;
      if (buttonState == LOW) {
        showFahrenheit = !showFahrenheit;  // Toggle the temperature mode here
      }
    }
  }
  lastButtonState = reading;  // Save the raw reading for the next loop
}

// Poll for a measurement, keeping the state machine alive
static bool measure_environment(float *temperature, float *humidity) {
  static unsigned long measurement_timestamp = millis();
  if (millis() - measurement_timestamp > 2000ul) {  // Measure once every two seconds
    if (dht_sensor.measure(temperature, humidity) == true) {
      measurement_timestamp = millis();
      return (true);  // true if a measurement is available
    }
  }
  return (false);  // false if no measurement available
}

// Main program (display) loop
void loop() {
  updateButton();  // call function

  float displayTemp;  // declare all variables we'll use
  float temperature;
  float humidity;

  /* Measure temperature and humidity.  If the functions returns
     true, then a measurement is available. */
  if (measure_environment(&temperature, &humidity) == true) {
    lcd.clear();          // clear old lcd characters
    lcd.setCursor(0, 0);  // start beginning of first line
    if (showFahrenheit) {
      displayTemp = temperature * 9.0 / 5.0 + 32.0;
      lcd.print(displayTemp, 0);  // no decimal places
      lcd.print((char)223);       // degree symbol
      lcd.print("F");
    } else {
      displayTemp = temperature;
      lcd.print(displayTemp, 0);  // no decimal places
      lcd.print((char)223);       // degree symbol
      lcd.print("C");
    }
    lcd.setCursor(7, 0);  // first line 8th cell start printing
    lcd.print(humidity, 0);
    lcd.print("%");
  }
}
