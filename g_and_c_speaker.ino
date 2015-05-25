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

#define SD_CS_PIN 4 
#define ONE_WIRE_PIN 2
#define IR_RECEIVER_PIN 8
#define MODE_IR_LISTENING 0
#define MODE_PLAYING_WAV 1

SdFat SD;
WavPlayer WP(&SD);
OneWire oneWire(ONE_WIRE_PIN);
DallasTemperature DT(&oneWire);
IRrecv IR(IR_RECEIVER_PIN);
decode_results ir_decode_results;
volatile char mode = MODE_IR_LISTENING;

ISR(TIMER1_COMPA_vect) 
{
  if(mode == MODE_PLAYING_WAV){
    boolean more_data_to_be_played = WP.update_sample_value_being_played();
    if(more_data_to_be_played == false){
      start_listening_to_ir_receiver();
    }
  }
}

void start_listening_to_ir_receiver(){
  mode = MODE_IR_LISTENING;
  IR.enableIRIn();
  IR.resume();
}

void stop_listening_to_ir_receiver(){
  TIMSK2 = 0;
}

void setup(){
  Serial.begin(9600);
  pinMode(10, OUTPUT); // Pin 10 must be left as an output for the SD library.
  if (!SD.begin(SD_CS_PIN, SPI_HALF_SPEED)){
    SD.initErrorHalt();
    Serial.println("SD initialization failed.");
  }
  DT.begin();
  start_listening_to_ir_receiver();
}

void loop(){
  if(mode == MODE_IR_LISTENING){
    check_remote_control_receiver_data(); 
  }
  else if(mode == MODE_PLAYING_WAV){
    WP.check_if_unused_buffer_needs_to_be_filled();
  }
}

void check_remote_control_receiver_data(){
  if (IR.decode(&ir_decode_results)) {
    if(ir_decode_results.decode_type == 1){
      if(ir_decode_results.value == 16582903){
        stop_listening_to_ir_receiver();
        play_temperature();
      }
      else{
        stop_listening_to_ir_receiver();
        play_current_time();
      }
    }
  } 
}

void play_temperature(){
  mode = MODE_PLAYING_WAV;
  DT.requestTemperatures();
  float temperature = DT.getTempCByIndex(0);
  Serial.print("Temp is: ");
  Serial.println(temperature);
  //WP.play_temperature(temperature);
  WP.play_temperature(22.5);
}

void play_current_time(){
  tmElements_t tm;
  if (RTC.read(tm)) {
    WP.play_current_time(tm.Hour, tm.Minute);
    Serial.print("Time: ");
    Serial.print(tm.Hour);
    Serial.print(":");
    Serial.println(tm.Minute);
  }
  else{
    if (RTC.chipPresent()) {
      Serial.println("DS1307 stopped.");
    } else {
      Serial.println("DS1307 error.");
      Serial.println();
    }
  }
  start_listening_to_ir_receiver();
}
