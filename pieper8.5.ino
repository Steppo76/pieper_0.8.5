#include <Arduino.h>
#include <Wire.h>
#include <TFT.h>
#include <SPI.h>
#include <SD.h>
#include <MsTimer2.h>
#include <Time.h>
#include "led.h"
#include "steppoRTC.h"

#define CS 10
#define DC  9
#define RESET 8

#define pieperPin 5
#define buttonPin  2
#define ledPin 3
//const byte buzzerPin = 13;

volatile int TasterStatus = 0;    //Taster Anschluss an Pin 2 entspricht Vektor 0
                                  //Taster Anschluss an Pin 3 entspricht Vektor 1 (hier)
volatile unsigned long alteZeit = 0;
volatile unsigned long entprellZeit = 200;

unsigned short sekunden = 0;
unsigned short sekStrom = 0;
unsigned short minuten = 0;
unsigned short stunden = 0;

byte modus = 1;
unsigned int i = 1;

time_t lastTime = 0;
time_t timeArray[4] = {0};

bool pieperState = LOW;
bool buttonState = LOW;

String pieperVer = "0.8.5";

led myLed(ledPin, 100, 100);
//led myLed1(ledPin, 70, 70);

steppoRTC myRTC(false);
TFT screen = TFT(CS, DC, RESET);

float temp = 0;
float tempPrev = 0;

//Sd2Card card;
//SdVolume volume;
//SdFile root;

const int chipSelect = 10;

File dataFile;

void setup()
{
  Wire.begin();
  Serial.begin(9600);
  Serial.print("Initializing SD Card ...");
  if (!SD.begin(4))
  {
    Serial.println("Initialization failed!");
  } else
  {
    Serial.println("Initialization done.");
  }

  dataFile = SD.open("LogData.txt", FILE_WRITE);
  if(dataFile)
  {
    Serial.println("File OK");
  } else
  {
    Serial.println("Error Opening File");
  }
  dataFile.close();
  Serial.println("File Close");

  pinMode(pieperPin, INPUT);
  pinMode(buttonPin, INPUT);
  //pinMode(buzzerPin, OUTPUT);

  attachInterrupt(0, TasterUnterbricht, HIGH);

  screenInit();

  MsTimer2::set(1000, flash);
  MsTimer2::start();
}


void loop()
{
   if (digitalRead(pieperPin) && pieperState == LOW)
   {
     pieperState = HIGH;
   }
   else if ((digitalRead(pieperPin)) && (pieperState == HIGH) && ((myRTC.get() - lastTime) >= 15))
   {
     TasterUnterbricht();
     i++;
   }
   else if ((digitalRead(pieperPin) == LOW) && (pieperState == LOW))
   {
     i = 1;
   }


  switch (modus)
  {
    case 0:
      if (pieperState == HIGH)
	    {
        myLed.UpdateLED();
      }
      break;
    case 1:
      if (pieperState == HIGH)
	    {
        myLed.UpdateLED();
      }
      break;
  }

  //buttonState = digitalRead(buttonPin);

  if (pieperState == HIGH)
  {
    if (lastTime == 0)
	  {
      lastTime = myRTC.get();

      timeArray[i-1] = lastTime;

      dataFile = SD.open("LogData.txt", FILE_WRITE);
      if (dataFile)
      {
        Serial.println("file open");
        dataFile.print(i);
        Serial.print(i);
        dataFile.print("  ");
        Serial.print("  ");
        dataFile.print(printDay(lastTime));
        Serial.print(printDay(lastTime));
        dataFile.print(digit(day(lastTime)));
        Serial.print(digit(day(lastTime)));
        dataFile.print(".");
        Serial.print(".");
        dataFile.print(digit(month(lastTime)));
        Serial.print(digit(month(lastTime)));
        dataFile.print(".");
        Serial.print(".");
        if (year(lastTime) >= 1000)
        {
          dataFile.print(2000 + (CalendarYrToTm(year(lastTime))-30));
          Serial.print(2000 + (CalendarYrToTm(year(lastTime))-30));
        } else
        {
          dataFile.print(2000 + (y2kYearToTm(year(lastTime))-30));
          Serial.print(2000 + (y2kYearToTm(year(lastTime))-30));
        }
        dataFile.print("  ");
        Serial.print("  ");
        dataFile.print(digit(hour(lastTime)));
        Serial.print(digit(hour(lastTime)));
        dataFile.print(":");
        Serial.print(":");
        dataFile.print(digit(minute(lastTime)));
        Serial.print(digit(minute(lastTime)));
        dataFile.print(":");
        Serial.print(":");
        dataFile.println(digit(second(lastTime)));
        Serial.println(digit(second(lastTime)));
      }
      dataFile.close();
      Serial.println("File close");
    }
    if (sekStrom < 60) // Stromsparmodus
	  {
      screen.fill(0, 0, 0);
      screen.noStroke();
      screen.rect(0, 40, screen.width(), 21, 0);

      screen.setTextSize(3);
      screen.setCursor(2, 40);

      screen.print(stunden);
      screen.print(":");

      screenDigit(minuten);
      screen.print(":");

      screenDigit(sekunden);

      screen.setTextSize(2);
      screen.setCursor(2,90);
      screen.println("Alarmzeit:");
      screen.setTextSize(1);
      screen.println();

      screen.fill(0, 0, 0);
      screen.noStroke();
      screen.rect(0, 115, screen.width(), screen.height());

      for (int counter = 0; counter < i; counter++)
      {
        screen.print(counter+ 1 );
        screen.print("  ");
        screenTime(timeArray[counter]);
        screen.print(" ");
        screenDate(timeArray[counter]);
        screen.println();
      }

      delay(450);
      screen.invertDisplay(true);
      delay(450);
      screen.invertDisplay(false);

      if ((sekunden) > 59)
	    {
        minuten++;
        sekunden = 0;
        if (minuten > 59)
		    {
          stunden++;
          minuten = 0;
        }
      }
      //display.clearDisplay();
      sekStrom = 0;
    }
  } else if (pieperState == LOW)
  {
    myLed.offLED();
    sekunden = 0;
    minuten = 0;
    stunden = 0;
    lastTime = 0;
  }

  temp = myRTC.temperature();
  if (temp != tempPrev)
  {
    screen.fill(0, 0, 0);
    screen.noStroke();
    screen.rect(70, 11, screen.width(), 16, 0);

    screen.setTextSize(1);
    screen.stroke(255, 255, 255);
    screen.text("T:", 70, 11);

    screen.print(temp);
    screen.print((char)247);
    screen.print("C");
  }
  tempPrev = temp;
}

void screenInit()
{
  screen.begin();
  screen.setRotation(0);
  screen.background(0, 0, 0);
  screen.stroke(255, 255, 255);
  screen.setTextSize(1);
  //screen.setTextColor(0xFFFF, 0x0000);
  screen.text("Pieper-Alarm-System", 7, 0);
  if (modus == 0)
  {
    screen.text("Modus: ", 0, 11);
    screen.print(modus);
  } else if (modus == 1)
  {
    screen.text("Ver:", 7, 11);
    screen.print(pieperVer);

  }
}

void screenTime(time_t t)
{
  screenDigit(hour(t));
  screen.print(":");
  screenDigit(minute(t));
  screen.print(":");
  screenDigit(second(t));
}

void screenDate(time_t t)
{
  screenDigit(day(t));
  screen.print(".");
  screenDigit(month(t));
  screen.print(".");
  if (year(t) >= 1000)
  {
    screenDigit(CalendarYrToTm(year(t))-30);
  } else
  {
    screenDigit(y2kYearToTm(year(t))-30);
  }
}

void screenDigit(byte zeit)
{
  if (zeit < 10)
  {
    screen.print("0");
  }
  screen.print(zeit, DEC);
}

String digit(byte element)
{
  String result = "";
  if (element < 10)
  {
    result = "0";
    result = result + element;
    return result;
  } else
  {
    return String(element);
  }
}

String printDay(time_t t) // Hier wird wird für den vorher im Code schon verwendeten Befehl printDay eine Bedeutung festgelegt
{
  int day;
  day = weekday(t); // Die Wochentage sollen abhängig vom Datum angezeigt werden.
  if(day == 1){return ("So, ");} // Wenn es sich um Tag 1 handelt soll „So“ usw. angezeigt werden.
  if(day == 2){return ("Mo, ");}
  if(day == 3){return ("Di, ");}
  if(day == 4){return ("Mi, ");}
  if(day == 5){return ("Do, ");}
  if(day == 6){return ("Fr, ");}
  if(day == 7){return ("Sa, ");}
}

void flash()
{
  if (pieperState == HIGH)
  {
    sekunden++;
  } else if (pieperState == LOW)
  {
    //sekStrom++;
  }
}

void TasterUnterbricht()
{
  if ((millis() - alteZeit) > entprellZeit)
  {
    pieperState = LOW;
    //timeArray = {0};
    screen.fill(0, 0, 0);
    screen.noStroke();
    screen.rect(0, 20, screen.width(), screen.height(), 0);
    timeArray[4] = {0};
  }
}
