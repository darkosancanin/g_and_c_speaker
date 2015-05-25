#include <Arduino.h>
#include "WavPlayer.h"
#include <SPI.h>
#include <SdFat.h>

WavPlayer::WavPlayer(SdFat* sdfat){
  sd = sdfat;
}

void WavPlayer::check_if_unused_buffer_needs_to_be_filled(){
  if(more_data_to_be_read == true && sample_buffer_not_being_read_is_filled == false){
    fill_unused_buffer();
  }
}

boolean WavPlayer::update_sample_value_being_played(){
  if(sample_buffer_playback_index >= SAMPLE_BUFFER_SIZE){
    if(sample_buffer_not_being_read_is_filled == true){
      swap_buffers();
    }
    else{
      if(more_data_to_be_read == false){
        stop_playback();
        return false;
      }
      return true;
    }
  }
  char sample_to_play = sample_buffer_being_read[sample_buffer_playback_index];
  OCR2B = sample_to_play;   
  sample_buffer_playback_index++;
  return true;
}

void WavPlayer::play_temperature(float temperature){
  reset_variables();
  int significant_number = (int)temperature;
  files_to_play[0] = 0; // First file to play is 'temperature is'.
  files_to_play[1] = significant_number + 1; // Second file is the significant number, the first number in the lookup table starts x entries in. 
  char decimal_buffer[4];
  dtostrf(temperature, 4, 1, decimal_buffer);
  files_to_play[2] = String(decimal_buffer[3]).toInt() + 60; // The third file to play is the 'point x degrees', the first number in the lookup table starts x entries in.
  fill_unused_buffer();
  swap_buffers();
  start_playback();
}

void WavPlayer::play_current_time (uint8_t hour, uint8_t minute){
  
}

void WavPlayer::reset_variables(){
  number_of_files_to_play = 3;
  index_of_current_file_being_read = 0;
  more_data_to_be_read = true;
  bytes_read_from_current_file = 0;
  sample_buffer_not_being_read_is_filled = true;
  sample_buffer_playback_index = 0;
}

void WavPlayer::fill_unused_buffer(){
  int bytes_filled_in_buffer = 0;
  while(index_of_current_file_being_read < number_of_files_to_play){
    uint8_t file_to_play = files_to_play[index_of_current_file_being_read];
    char filename_buffer[12];
    strcpy_P(filename_buffer, (char*)pgm_read_word(&(filename_lookup_table[file_to_play])));
    File file = sd->open(filename_buffer);
    file.seek(WAV_FILE_START_INDEX + bytes_read_from_current_file);
    int bytes_left_in_buffer = SAMPLE_BUFFER_SIZE - bytes_filled_in_buffer;
    int bytes_filled = file.read(&sample_buffer_not_being_read[bytes_filled_in_buffer], bytes_left_in_buffer);
    bytes_read_from_current_file += bytes_filled;
    bytes_filled_in_buffer += bytes_filled;
    file.close();
    if(bytes_filled_in_buffer >= SAMPLE_BUFFER_SIZE){
      break;
    } 
    else{
      index_of_current_file_being_read++;
      bytes_read_from_current_file = 0;
      if(index_of_current_file_being_read == number_of_files_to_play){
        more_data_to_be_read = false;
        break;
      }
    }
  }
  sample_buffer_not_being_read_is_filled = true;
}

void WavPlayer::swap_buffers(){
  if(sample_buffer_being_read == sample_buffer_a){
    sample_buffer_being_read = sample_buffer_b;
    sample_buffer_not_being_read = sample_buffer_a;
  }
  else{
    sample_buffer_being_read = sample_buffer_a;
    sample_buffer_not_being_read = sample_buffer_b;
  }
  sample_buffer_playback_index = 0;
  sample_buffer_not_being_read_is_filled = false;
}

void WavPlayer::start_playback() {
  pinMode(SPEAKER_PIN, OUTPUT);
  
  // Set up Timer 2 to do pulse width modulation on the speaker pin. //
  // Use internal clock [page 164]
  ASSR &= ~(_BV(EXCLK) | _BV(AS2));
  // Set fast PWM mode  [page 160]
  TCCR2A = _BV(WGM21) | _BV(WGM20);
  TCCR2B = ~_BV(WGM22);
  // Do non-inverting PWM on pin OC2B [page 159]
  TCCR2A = (TCCR2A | _BV(COM2B1)) & ~_BV(COM2B0);
  TCCR2A &= ~(_BV(COM2A1) | _BV(COM2A0));
  // No prescaler [page 162]
  TCCR2B = (TCCR2B & ~(_BV(CS22) | _BV(CS21))) | _BV(CS20);

  // Set up Timer 1 to send a sample every interrupt. //
  // OCR1A is a 16-bit register, so we have to do this with interrupts disabled to be safe.
  cli();
  // Set CTC mode (Clear Timer on Compare Match) [page 136]
  TCCR1B = (TCCR1B & ~_BV(WGM13)) | _BV(WGM12);
  TCCR1A = TCCR1A & ~(_BV(WGM11) | _BV(WGM10));
  // No prescaler [page 137]
  TCCR1B = (TCCR1B & ~(_BV(CS12) | _BV(CS11))) | _BV(CS10);
  // Set the compare register (OCR1A).
  OCR1A = F_CPU / WAV_FILE_SAMPLE_RATE; // 16e6 (16Mhz) / 8000 = 2000
  // Enable interrupt when TCNT1 == OCR1A [page 139]
  TIMSK1 = _BV(OCIE1A);
  sei();
}

void WavPlayer::stop_playback(){
  TIMSK1 &= ~_BV(OCIE1A); // Disable playback per-sample interrupt.
  TCCR1B &= ~_BV(CS10); // Disable the per-sample timer completely.
  TCCR2B &= ~_BV(CS10); // Disable the PWM timer.
  digitalWrite(SPEAKER_PIN, LOW);
}
