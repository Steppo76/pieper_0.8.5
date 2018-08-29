// led.cpp

#include "led.h"

led::led(int pin) {
  ledPin = pin;
  pinMode(ledPin, OUTPUT);
  OnTime = 500;
  OffTime = 500;
  ledState = LOW;
  previousMillis = 0;
};

led::led(int pin, long on) {
  ledPin = pin;
  pinMode(ledPin, OUTPUT);
  OnTime = on;
  OffTime = on;
  ledState = LOW;
  previousMillis = 0;
};

led::led(int pin, long on, long off) 
{
  ledPin = pin;
  pinMode(ledPin, OUTPUT);
  OnTime = on;
  OffTime = off;
  ledState = LOW;
  previousMillis = 0;
}

void led::UpdateLED()
{
  unsigned long currentMillis = millis();

  if ((ledState == HIGH) && (currentMillis - previousMillis >= OnTime)) {
    ledState = LOW;
    previousMillis = currentMillis;
    digitalWrite(ledPin, ledState);
  }
  else if ((ledState == LOW) && (currentMillis - previousMillis >= OffTime)) {
    ledState = HIGH;
    previousMillis = currentMillis;
    digitalWrite(ledPin, ledState);
  }
}

void led::onLED()
{
	digitalWrite(ledPin, HIGH);
}

void led::offLED()
{
	digitalWrite(ledPin, LOW);
}
