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
  // Ramp up at the start from 0 through to the first value in the sample to stop the pop at the start of the playback.
  if(have_ramped_up == false){
    if(last_sample_value >= sample_buffer_being_read[0]){
       have_ramped_up = true;
     }
     else{
        last_sample_value++;
        OCR2B = last_sample_value; 
        return true;
     }
  }
  if(sample_buffer_playback_index >= SAMPLE_BUFFER_SIZE){
    if(sample_buffer_not_being_read_is_filled == true){
      swap_buffers();
    }
    else{
      if(more_data_to_be_read == false){
        // Check if we have ramped down to 0 otherwise incrementally ramp down to stop the pop at the end of the playback.
		if(last_sample_value == 0){
          stop_playback();
          return false;
        }
        else{
          last_sample_value--; 
          OCR2B = last_sample_value;    
        }
      }
      return true;
    }
  }
  byte sample_to_play = sample_buffer_being_read[sample_buffer_playback_index];
  OCR2B = sample_to_play;   
  last_sample_value = sample_to_play;
  sample_buffer_playback_index++;
  return true;
}

void WavPlayer::play_temperature(float temperature){
  initialize_state_variables();
  int significant_number = (int)temperature;
  files_to_play[0] = 0; 
  files_to_play[1] = significant_number + 1; 
  char decimal_buffer[4];
  dtostrf(temperature, 4, 1, decimal_buffer);
  files_to_play[2] = String(decimal_buffer[3]).toInt() + 61; 
  open_file(0);
  fill_unused_buffer();
  swap_buffers();
  start_playback();
}

void WavPlayer::play_current_time (uint8_t the_hour, uint8_t the_minute, boolean daylight_savings_enabled){
  initialize_state_variables();
  if(daylight_savings_enabled == true){ 
    the_hour++;
  }
  if(the_hour == 24 || the_hour == 0){ 
    the_hour = 12;
  }
  else if(the_hour > 12){
    the_hour -= 12;
  }
  files_to_play[0] = 1; 
  files_to_play[1] = the_hour + 1;
  if(the_minute == 0){
    files_to_play[2] = 79; 
  }
  else if(the_minute < 10){
    files_to_play[2] = the_minute + 71; 
  }
  else{
    files_to_play[2] = the_minute + 1;
  }
  open_file(0);
  fill_unused_buffer();
  swap_buffers();
  start_playback();
}

void WavPlayer::initialize_state_variables(){
  last_sample_value = 0;
  have_ramped_up = false;
  number_of_files_to_play = 3;
  index_of_current_file_being_read = 0;
  more_data_to_be_read = true;
  sample_buffer_not_being_read_is_filled = true;
  sample_buffer_playback_index = 0;
}

void WavPlayer::open_file(int index_of_file){
  uint8_t file_to_play = files_to_play[index_of_file];
  char filename_buffer[12];
  strcpy_P(filename_buffer, (char*)pgm_read_word(&(filename_lookup_table[file_to_play])));
  File current_file_being_played = sd->open(filename_buffer);
  current_file_being_played.seek(WAV_FILE_START_INDEX);
}

void WavPlayer::fill_unused_buffer(){
  int bytes_filled_in_buffer = 0;
  while(index_of_current_file_being_read < number_of_files_to_play){
    int bytes_left_in_buffer = SAMPLE_BUFFER_SIZE - bytes_filled_in_buffer;
    int bytes_filled = current_file_being_played.read(&sample_buffer_not_being_read[bytes_filled_in_buffer], bytes_left_in_buffer);
    bytes_filled_in_buffer += bytes_filled;
	// If the buffer is full then just break until the next buffer is ready.
    if(bytes_filled_in_buffer >= SAMPLE_BUFFER_SIZE){
      break;
    } 
    else {
	  current_file_being_played.close();
	  index_of_current_file_being_read++;
	  // If there are no more files then break otherwise open the next file for reading.
	  if(index_of_current_file_being_read == number_of_files_to_play){
		more_data_to_be_read = false;
        break;
      }
	  else{
	    open_file(index_of_current_file_being_read);
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
