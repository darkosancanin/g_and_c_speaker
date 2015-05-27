#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/wdt.h>
#include <SPI.h>
#include <SdFat.h>
#include "WavPlayer.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include <IRremote.h>
#include <Wire.h>
#include <Time.h>
#include <DS1307RTC.h>
#include "MemoryFree.h"

#define DEBUG 
#define SD_CS_PIN 4 
#define ONE_WIRE_PIN 2
#define IR_RECEIVER_PIN 8
#define DAYLIGHT_SAVINGS_ENABLED_PIN 5
#define MODE_IR_LISTENING 0
#define MODE_PLAYING_WAV 1
#define IR_CODE_TEMPERATURE 16582903
#define IR_CODE_TIME 16615543

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
  delay(500);
}

void stop_listening_to_ir_receiver(){
  TIMSK2 = 0;
}

void setup(){
  wdt_disable();
  Serial.begin(9600);
  Serial.println(F("Initializing application."));
  pinMode(DAYLIGHT_SAVINGS_ENABLED_PIN, INPUT); 
  digitalWrite(DAYLIGHT_SAVINGS_ENABLED_PIN, HIGH);
  pinMode(10, OUTPUT); // Pin 10 must be left as an output for the SD library.
  if (!SD.begin(SD_CS_PIN, SPI_HALF_SPEED)){
    SD.initErrorHalt();
    #ifdef DEBUG
    Serial.println(F("SD failed to initialize."));
    #endif
  }
  DT.begin();
  start_listening_to_ir_receiver();
  wdt_enable(WDTO_8S);
}

void loop(){
  wdt_reset();
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
      if(ir_decode_results.value == IR_CODE_TEMPERATURE){
        stop_listening_to_ir_receiver();
        play_temperature();
      }
      else if(ir_decode_results.value == IR_CODE_TIME){
        stop_listening_to_ir_receiver();
        play_current_time();
      }
    }
    IR.resume();
  } 
  
  #ifdef DEBUG
  //Serial.print(F("Free Memory: "));
  //Serial.println(freeMemory());
  #endif
  
  delay(100);
}

void play_temperature(){
  mode = MODE_PLAYING_WAV;
  DT.requestTemperatures();
  float temperature = DT.getTempCByIndex(0);
  #ifdef DEBUG
  Serial.print(F("The temperature is: "));
  Serial.println(temperature);
  #endif
  WP.play_temperature(temperature);
}

void play_current_time(){
  mode = MODE_PLAYING_WAV;
  tmElements_t tm;
  if (RTC.read(tm)) {
    #ifdef DEBUG
    Serial.print(F("The time is: "));
    Serial.print(tm.Hour);
    Serial.print(F(":"));
    Serial.println(tm.Minute);
    #endif
    WP.play_current_time(tm.Hour, tm.Minute, digitalRead(DAYLIGHT_SAVINGS_ENABLED_PIN) == LOW);
  }
  else{
    start_listening_to_ir_receiver();
  }
}
