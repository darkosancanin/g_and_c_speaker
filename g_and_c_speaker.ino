#include <avr/interrupt.h>
#include <avr/io.h>
#include <SPI.h>
#include <SdFat.h>
#include "WavPlayer.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include <IRremote.h>
#include <Wire.h>
#include <Time.h>
#include <DS1307RTC.h>

#define ONE_WIRE_BUS 2 // Data wire is plugged into pin 2 on the Arduino
WavPlayer WP;

// Setup a oneWire instance to communicate with any OneWire devices 
// (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

ISR(TIMER1_COMPA_vect) 
{
  WP.handle_interrupt();
}

void setup(){
  WP.initialize();
  // Start up the library
  sensors.begin();
  
  WP.play_temperature();
}

void loop(){
  WP.check_if_unused_buffer_needs_to_be_filled();
  //get_time();
}

void get_time(){
  tmElements_t tm;

  if (RTC.read(tm)) {
    Serial.print("Ok, Time = ");
    print2digits(tm.Hour);
    Serial.write(':');
    print2digits(tm.Minute);
    Serial.write(':');
    print2digits(tm.Second);
    Serial.print(", Date (D/M/Y) = ");
    Serial.print(tm.Day);
    Serial.write('/');
    Serial.print(tm.Month);
    Serial.write('/');
    Serial.print(tmYearToCalendar(tm.Year));
    Serial.println();
  } else {
    if (RTC.chipPresent()) {
      Serial.println("The DS1307 is stopped.  Please run the SetTime");
      Serial.println("example to initialize the time and begin running.");
      Serial.println();
    } else {
      Serial.println("DS1307 read error!  Please check the circuitry.");
      Serial.println();
    }
    delay(9000);
  }
  delay(1000);
}

void print2digits(int number) {
  if (number >= 0 && number < 10) {
    Serial.write('0');
  }
  Serial.print(number);
}

void get_temperature(){
  // call sensors.requestTemperatures() to issue a global temperature
  // request to all devices on the bus
  //--Serial.print(" Requesting temperatures...");
  //--sensors.requestTemperatures(); // Send the command to get temperatures
  //--Serial.println("DONE");

  //--Serial.print("Temperature for Device 1 is: ");
  //--Serial.print(sensors.getTempCByIndex(0)); // Why "byIndex"? 
  //--  // You can have more than one IC on the same bus. 
  //--  // 0 refers to the first IC on the wire
}
