#include <avr/interrupt.h>
#include <avr/io.h>
#include <SPI.h>
#include <SdFat.h>

#define SAMPLE_RATE 8000
#define BUFFER_SIZE 256
#define WAV_FILE_START_INDEX 44

SdFat SD;
int speakerPin = 3; // This is the OC2B PWM pin.

char * files_to_play[] = {"gabby.wav", "charlie.wav", "gabby.wav", "charlie.wav"};
uint8_t index_of_current_file_being_read = 0;
boolean more_data_to_be_read = true;
uint16_t bytes_read_from_current_file = 0;
uint8_t sample_buffer_a[BUFFER_SIZE];
uint8_t sample_buffer_b[BUFFER_SIZE];
boolean sample_buffer_not_being_read_is_filled = true;
volatile uint16_t sample_buffer_playback_index = 0;
uint8_t * sample_buffer_being_read = sample_buffer_a;
uint8_t * sample_buffer_not_being_read = sample_buffer_b;

void fill_unused_buffer(){
  uint8_t buffer_sample_index = 0;
  more_data_to_be_read = false;
  while(index_of_current_file_being_read < sizeof(files_to_play)){
    File file = SD.open(files_to_play[index_of_current_file_being_read]);
    file.seek(WAV_FILE_START_INDEX + bytes_read_from_current_file);
    while (file.available() && buffer_sample_index < BUFFER_SIZE) {
      uint8_t sample = file.read();
      sample_buffer_not_being_read[buffer_sample_index] = sample;
	  bytes_read_from_current_file++;
      buffer_sample_index++;
    }
    if(buffer_sample_index >= BUFFER_SIZE){
      more_data_to_be_read = true;
	  file.close();
      break;
    } 
    else{
      bytes_read_from_current_file = 0;
	  index_of_current_file_being_read++;
	  file.close();
    }
  }
  sample_buffer_not_being_read_is_filled = true;
}

void swap_buffers(){
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

void stop_playback(){
  TIMSK1 &= ~_BV(OCIE1A); // Disable playback per-sample interrupt.
  TCCR1B &= ~_BV(CS10); // Disable the per-sample timer completely.
  TCCR2B &= ~_BV(CS10); // Disable the PWM timer.
  digitalWrite(speakerPin, LOW);
}

ISR(TIMER1_COMPA_vect) 
{
  if(sample_buffer_playback_index >= BUFFER_SIZE){
    if(sample_buffer_not_being_read_is_filled == true){
      swap_buffers();
    }
    else{
      stop_playback();
	  return;
    }
  }
  
  uint8_t sample_to_play = sample_buffer_being_read[sample_buffer_playback_index];
  OCR2B = sample_to_play;   
  sample_buffer_playback_index++;
}

void start_playback(){
  pinMode(speakerPin, OUTPUT);
  
  // Set up Timer 2 to do pulse width modulation on the speaker pin. //
  // Use internal clock [page 164]
  ASSR &= ~(_BV(EXCLK) | _BV(AS2));
  // Set fast PWM mode  [page 160]
  TCCR2A |= _BV(WGM21) | _BV(WGM20);
  TCCR2B &= ~_BV(WGM22);
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
  OCR1A = F_CPU / SAMPLE_RATE; // 16e6 (16Mhz) / 8000 = 2000
  // Enable interrupt when TCNT1 == OCR1A [page 139]
  TIMSK1 |= _BV(OCIE1A);
  sei();
}

void setup(){
  Serial.begin(9600);
  pinMode(10, OUTPUT); // Pin 10 must be left as an output or the SD library functions will not work.

  if (!SD.begin()){
    Serial.println("SD init failed.");
  }
  
  fill_unused_buffer();
  swap_buffers();
  start_playback();
}

void loop(){
  if(more_data_to_be_read == true && sample_buffer_not_being_read_is_filled == false){
	fill_unused_buffer();
  }
}