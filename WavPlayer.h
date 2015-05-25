#include <Arduino.h>
#include <avr/io.h>
#include <SPI.h>
#include <SdFat.h>

#define SPEAKER_PIN 3 // This is the OC2B PWM pin and cant be changed.
#define SAMPLE_BUFFER_SIZE 128
#define WAV_FILE_SAMPLE_RATE 8000
#define WAV_FILE_START_INDEX 44

const char file_temperature_is[] PROGMEM = "TEMPIS.WAV";
const char file_time_is[] PROGMEM = "TIMEIS.WAV";
const char file_1[] PROGMEM = "1.WAV";
const char file_2[] PROGMEM = "2.WAV";
const char file_3[] PROGMEM = "3.WAV";
const char file_4[] PROGMEM = "4.WAV";
const char file_5[] PROGMEM = "5.WAV";
const char file_6[] PROGMEM = "6.WAV";
const char file_7[] PROGMEM = "7.WAV";
const char file_8[] PROGMEM = "8.WAV";
const char file_9[] PROGMEM = "9.WAV";
const char file_10[] PROGMEM = "10.WAV";
const char file_11[] PROGMEM = "11.WAV";
const char file_12[] PROGMEM = "12.WAV";
const char file_13[] PROGMEM = "13.WAV";
const char file_14[] PROGMEM = "14.WAV";
const char file_15[] PROGMEM = "15.WAV";
const char file_16[] PROGMEM = "16.WAV";
const char file_17[] PROGMEM = "17.WAV";
const char file_18[] PROGMEM = "18.WAV";
const char file_19[] PROGMEM = "19.WAV";
const char file_20[] PROGMEM = "20.WAV";
const char file_21[] PROGMEM = "21.WAV";
const char file_22[] PROGMEM = "22.WAV";
const char file_23[] PROGMEM = "23.WAV";
const char file_24[] PROGMEM = "24.WAV";
const char file_25[] PROGMEM = "25.WAV";
const char file_26[] PROGMEM = "26.WAV";
const char file_27[] PROGMEM = "27.WAV";
const char file_28[] PROGMEM = "28.WAV";
const char file_29[] PROGMEM = "29.WAV";
const char file_30[] PROGMEM = "30.WAV";
const char file_31[] PROGMEM = "31.WAV";
const char file_32[] PROGMEM = "32.WAV";
const char file_33[] PROGMEM = "33.WAV";
const char file_34[] PROGMEM = "34.WAV";
const char file_35[] PROGMEM = "35.WAV";
const char file_36[] PROGMEM = "36.WAV";
const char file_37[] PROGMEM = "37.WAV";
const char file_38[] PROGMEM = "38.WAV";
const char file_39[] PROGMEM = "39.WAV";
const char file_40[] PROGMEM = "40.WAV";
const char file_41[] PROGMEM = "41.WAV";
const char file_42[] PROGMEM = "42.WAV";
const char file_43[] PROGMEM = "43.WAV";
const char file_44[] PROGMEM = "44.WAV";
const char file_45[] PROGMEM = "45.WAV";
const char file_46[] PROGMEM = "46.WAV";
const char file_47[] PROGMEM = "47.WAV";
const char file_48[] PROGMEM = "48.WAV";
const char file_49[] PROGMEM = "49.WAV";
const char file_50[] PROGMEM = "50.WAV";
const char file_51[] PROGMEM = "51.WAV";
const char file_52[] PROGMEM = "52.WAV";
const char file_53[] PROGMEM = "53.WAV";
const char file_54[] PROGMEM = "54.WAV";
const char file_55[] PROGMEM = "55.WAV";
const char file_56[] PROGMEM = "56.WAV";
const char file_57[] PROGMEM = "57.WAV";
const char file_58[] PROGMEM = "58.WAV";
const char file_59[] PROGMEM = "59.WAV";
const char file_point_0_degrees[] PROGMEM = "POINT0.WAV";
const char file_point_1_degrees[] PROGMEM = "POINT1.WAV";
const char file_point_2_degrees[] PROGMEM = "POINT2.WAV";
const char file_point_3_degrees[] PROGMEM = "POINT3.WAV";
const char file_point_4_degrees[] PROGMEM = "POINT4.WAV";
const char file_point_5_degrees[] PROGMEM = "POINT5.WAV";
const char file_point_6_degrees[] PROGMEM = "POINT6.WAV";
const char file_point_7_degrees[] PROGMEM = "POINT7.WAV";
const char file_point_8_degrees[] PROGMEM = "POINT8.WAV";
const char file_point_9_degrees[] PROGMEM = "POINT9.WAV";
const char file_01[] PROGMEM = "01.WAV";
const char file_02[] PROGMEM = "02.WAV";
const char file_03[] PROGMEM = "03.WAV";
const char file_04[] PROGMEM = "04.WAV";
const char file_05[] PROGMEM = "05.WAV";
const char file_06[] PROGMEM = "06.WAV";
const char file_07[] PROGMEM = "07.WAV";
const char file_08[] PROGMEM = "08.WAV";
const char file_09[] PROGMEM = "09.WAV";
const char file_oclock[] PROGMEM = "OCLOCK.WAV";

const char* const filename_lookup_table[] PROGMEM = {file_temperature_is, file_time_is, file_1, file_2, file_3, file_4, file_5, file_6, file_7, file_8, file_9, file_10, file_11, file_12, file_13, file_14, file_15, file_16, file_17, file_18, file_19, file_20, file_21, file_22, file_23, file_24, file_25, file_26, file_27, file_28, file_29, file_30, file_31, file_32, file_33, file_34, file_35, file_36, file_37, file_38, file_39, file_40, file_41, file_42, file_43, file_44, file_45, file_46, file_47, file_48, file_49, file_50, file_51, file_52, file_53, file_54, file_55, file_56, file_57, file_58, file_59, file_point_0_degrees, file_point_1_degrees, file_point_2_degrees, file_point_3_degrees, file_point_4_degrees, file_point_5_degrees, file_point_6_degrees, file_point_7_degrees, file_point_8_degrees, file_point_9_degrees, file_01, file_02, file_03, file_04, file_05, file_06, file_07, file_08, file_09, file_oclock};

class WavPlayer {
  private:
    SdFat* sd;
    int files_to_play[3];
    int number_of_files_to_play = 3;
    int index_of_current_file_being_read = 0;
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
    void reset_variables();
  public:
    WavPlayer(SdFat*);
    boolean update_sample_value_being_played (void);
    void check_if_unused_buffer_needs_to_be_filled (void);
    void play_current_time (uint8_t, uint8_t);
    void play_temperature (float);
};
