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

WavPlayer WP;
#define ONE_WIRE_BUS 2 // Data wire is plugged into pin 2 on the Arduino

// Setup a oneWire instance to communicate with any OneWire devices 
// (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

#define DECODE_NEC 

int RECV_PIN = 8;
IRrecv irrecv(RECV_PIN);
decode_results results;



ISR(TIMER1_COMPA_vect) 
{
  WP.handle_interrupt();
}

void setup(){
  Serial.begin(9600);
  WP.initialize();
  // Start up the library
  sensors.begin();
  WP.play_temperature();
  //irrecv.enableIRIn();
}

void loop(){
  //check_for_remote_control();
  WP.check_if_unused_buffer_needs_to_be_filled();
  //get_time();
}

void check_for_remote_control(){
  if (irrecv.decode(&results)) {
    Serial.println(results.value, HEX);
    Serial.println(results.decode_type);
    if(results.decode_type == 1){
      WP.play_temperature();
      Serial.println("NEC received.");
    }
  irrecv.resume(); // Receive the next value
  } 
  else{
    //Serial.println("No IR."); 
  }
}

void get_time(){
  tmElements_t tm;

  if (RTC.read(tm)) {
    Serial.print("Time = ");
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
      Serial.println("DS1307 stopped.");
      Serial.println();
    } else {
      Serial.println("DS1307 error!");
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
  // call sensors.requestTemperatures() to issue a global temperature request to all devices on the bus
  sensors.requestTemperatures(); // Send the command to get temperatures
  Serial.print("Temp is: ");
  Serial.print(sensors.getTempCByIndex(0)); // Why "byIndex"? 
  // You can have more than one IC on the same bus. 0 refers to the first IC on the wire
}
