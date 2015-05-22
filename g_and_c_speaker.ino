#include <avr/interrupt.h>
#include <avr/io.h>
#include <SPI.h>
#include <SdFat.h>
#include "WavPlayer.h"

WavPlayer WP;

ISR(TIMER1_COMPA_vect) 
{
  WP.handle_interrupt();
}

void setup(){
  WP.initialize();
  WP.play_temperature();
}

void loop(){
  WP.check_if_unused_buffer_needs_to_be_filled();
}
