// steppoRTC.h

#ifndef _STEPPORTC_H_INCLUDED
#define _STEPPORTC_H_INCLUDED

#include <Arduino.h>

class steppoRTC
{
  public:
      steppoRTC(bool initI2C = true);
      void begin();
      float temperature(void);
      static time_t steppoRTC::get();
      static byte read(tmElements_t &tm);
      byte readRTC(byte addr);
      byte readRTC(byte addr, byte *values, byte nBytes);
  private:
      uint8_t decToBcd(uint8_t n);
      static uint8_t bcdToDec(uint8_t n);
      static byte errCode;
  protected:
};

#ifdef ARUINO_ARCH_AVR
extern STEPPORTC RTC;
#endif

#endif
