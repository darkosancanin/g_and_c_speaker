#include <Arduino.h>
#include <avr/io.h>
#include <SPI.h>
#include <SdFat.h>

#define SPEAKER_PIN 3 // This is the OC2B PWM pin and cant be changed.
#define SD_CS_PIN 4 
#define SAMPLE_BUFFER_SIZE 128
#define WAV_FILE_SAMPLE_RATE 8000
#define WAV_FILE_START_INDEX 44

class WavPlayer {
  private:
    SdFat sd;
    static const char * files_to_play[4];
    int number_of_files_to_play = 4;
    uint8_t index_of_current_file_being_read = 0;
    boolean more_data_to_be_read = true;
    uint32_t bytes_read_from_current_file = 0;
    char sample_buffer_a[SAMPLE_BUFFER_SIZE];
    char sample_buffer_b[SAMPLE_BUFFER_SIZE];
    boolean sample_buffer_not_being_read_is_filled = true;
    volatile uint16_t sample_buffer_playback_index = 0;
    char * sample_buffer_being_read = sample_buffer_a;
    char * sample_buffer_not_being_read = sample_buffer_b;
    void fill_unused_buffer (void);
    void start_playback (void);
    void stop_playback (void);
    void swap_buffers (void);
  public:
    void initialize (void);
    void handle_interrupt (void);
    void check_if_unused_buffer_needs_to_be_filled (void);
    void play_temperature (void);
};
