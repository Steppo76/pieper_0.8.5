// led.h

#ifndef _LED_h
#define _LED_h

#if defined(ARDUINO) && ARDUINO >= 100
  #include "arduino.h"
#else
  #include "WProgram.h"
#endif

class led
{
  protected:
      int ledPin;
      long OnTime;
      long OffTime;
      int ledState;
      unsigned long previousMillis;

  public:
      led(int pin);
      led(int pin, long on);
      led(int pin, long on, long off);
      void UpdateLED();
      void onLED();
      void offLED();
};

#endif

