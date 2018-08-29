#if defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
#include <TinyWireM.h>
#define i2cBegin TinyWireM.begin
#define i2cBeginTransmission TinyWireM.beginTransmission
#define i2cEndTransmission TinyWireM.endTransmission
#define i2cRequestFrom TinyWireM.requestFrom
#define i2cRead TinyWireM.receive
#define i2cWrite TinyWireM.send
#elif ARDUINO >= 100
#include <Wire.h>
#define i2cBegin Wire.begin
#define i2cBeginTransmission Wire.beginTransmission
#define i2cEndTransmission Wire.endTransmission
#define i2cRequestFrom Wire.requestFrom
#define i2cRead Wire.read
#define i2cWrite Wire.write
#else
#include <Wire.h>
#define i2cBegin Wire.begin
#define i2cBeginTransmission Wire.beginTransmission
#define i2cEndTransmission Wire.endTransmission
#define i2cRequestFrom Wire.requestFrom
#define i2cRead Wire.receive
#define i2cWrite Wire.send
#endif

// DS3232 I2C Address
#define RTC_ADDR 0x68

// DS3232 Register Addresses
#define RTC_SECONDS 0x00
#define RTC_MINUTES 0x01
#define RTC_HOURS 0x02
#define RTC_DAY 0x03
#define RTC_DATE 0x04
#define RTC_MONTH 0x05
#define RTC_YEAR 0x06
#define ALM1_SECONDS 0x07
#define ALM1_MINUTES 0x08
#define ALM1_HOURS 0x09
#define ALM1_DAYDATE 0x0A
#define ALM2_MINUTES 0x0B
#define ALM2_HOURS 0x0C
#define ALM2_DAYDATE 0x0D
#define RTC_CONTROL 0x0E
#define RTC_STATUS 0x0F
#define RTC_AGING 0x10
#define RTC_TEMP_MSB 0x11
#define RTC_TEMP_LSB 0x12
#define SRAM_START_ADDR 0x14    // first SRAM address
#define SRAM_SIZE 236           // number of bytes of SRAM

// Alarm mask bits
#define A1M1 7
#define A1M2 7
#define A1M3 7
#define A1M4 7
#define A2M2 7
#define A2M3 7
#define A2M4 7

// Control register bits
#define EOSC 7
#define BBSQW 6
#define CONV 5
#define RS2 4
#define RS1 3
#define INTCN 2
#define A2IE 1
#define A1IE 0

// Status register bits
#define OSF 7
#define BB32KHZ 6
#define CRATE1 5
#define CRATE0 4
#define EN32KHZ 3
#define BSY 2
#define A2F 1
#define A1F 0

// Other
#define DS1307_CH 7                // for DS1307 compatibility, Clock Halt bit in Seconds register
#define HR1224 6                   // Hours register 12 or 24 hour mode (24 hour mode==0)
#define CENTURY 7                  // Century bit in Month register
#define DYDT 6                     // Day/Date flag bit in alarm Day/Date registers



#include <TimeLib.h>        //https://github.com/PaulStoffregen/Time
#include "steppoRTC.h"

byte steppoRTC::errCode;           // for debug 

steppoRTC::steppoRTC(bool initI2C)
{
  if (initI2C) {
    i2cBegin();
  }
}

void steppoRTC::begin()
{
  i2cBegin();
}

byte steppoRTC::readRTC(byte addr)
{
  byte b;
  readRTC(addr, &b, 1);
  return b;
}

byte steppoRTC::readRTC(byte addr, byte *values, byte nBytes)
{
  i2cBeginTransmission(RTC_ADDR);
  i2cWrite(addr);
  if (byte e = i2cEndTransmission()) return e;
  i2cRequestFrom((uint8_t)RTC_ADDR, nBytes);
  for (byte i = 0; i < nBytes; i++) values[i] = i2cRead();
  return 0;
}

float steppoRTC::temperature()
{
  union int16_byte {
    int i;
    byte b[2];
  } rtcTemp;
  rtcTemp.b[0] = readRTC(RTC_TEMP_LSB);
  rtcTemp.b[1] = readRTC(RTC_TEMP_MSB);
  return ((rtcTemp.i / 64) / 4.);
}

// Read the current time from the RTC and return it in a tmElements_t
// structure. Returns the I2C status (zero if successful).
byte steppoRTC::read(tmElements_t &tm)
{
    i2cBeginTransmission(RTC_ADDR);
    i2cWrite((uint8_t)RTC_SECONDS);
    if ( byte e = i2cEndTransmission() ) { errCode = e; return e; }
    // request 7 bytes (secs, min, hr, dow, date, mth, yr)
    i2cRequestFrom(RTC_ADDR, tmNbrFields);
    tm.Second = bcdToDec(i2cRead() & ~_BV(DS1307_CH));
    tm.Minute = bcdToDec(i2cRead());
    tm.Hour = bcdToDec(i2cRead() & ~_BV(HR1224));    // assumes 24hr clock
    tm.Wday = i2cRead();
    tm.Day = bcdToDec(i2cRead());
    tm.Month = bcdToDec(i2cRead() & ~_BV(CENTURY));  // don't use the Century bit
    tm.Year = y2kYearToTm(bcdToDec(i2cRead()));
    return 0;
}

// Read the current time from the RTC and return it as a time_t
// value. Returns a zero value if an I2C error occurred (e.g. RTC
// not present).
time_t steppoRTC::get()
{
    tmElements_t tm;

    if ( read(tm) ) return 0;
    return( makeTime(tm) );
}

// Decimal-to-BCD conversion
uint8_t steppoRTC::decToBcd(uint8_t n)
{
    return n + 6 * (n / 10);
}

// BCD-to-Decimal conversion
uint8_t __attribute__ ((noinline)) steppoRTC::bcdToDec(uint8_t n)
{
    return n - 6 * (n >> 4);
}

#ifdef ARUINO_ARCH_AVR
extern STEPPORTC RTC;
#endif
