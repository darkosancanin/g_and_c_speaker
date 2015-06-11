## Overview 
ATmega328 micro controller based project that connects to audio speakers to read out and play the current temperature or the current time.  A remote control is used to select what to play. The playback is done with voice recordings from my daughter that are read from the micro SD card.

## Implementation Overview 
A DS1307 real time clock is used and read to obtain the current time. A Dallas DS18S20 temperature sensor is used and read to obtain the current temperature. The IR receiver listens for the remote control command to determine what data to play (I programmed some of my unused TV remote buttons so that I did not need a separate remote control to turn it on).  I recorded my daughters voice and encoded in 8bit, 8000Hz WAV format and stored it on a micro SD card. The voice data is stored in separate files that are played one after the other to read the relevant data out. e.g. If it is is 22.1 degrees, then the following files are read in order from the micro SD card: {"TEMPIS.WAV", "22.WAV", "POINT1.WAV"} which will say "The temperature is 22 point 1 degrees.".

The audio playback is encapsulated in the WavPlayer class that I wrote and is implemented using Pulse Width Modulation (PWM) output, with the PWM value being updated 8000 times per second via interrupts to match the encoding of the voice data in the WAV files. A double buffer is used, the unused buffer is filled between interrupts and swapped out when the first buffer is read. The relatively low (compared to CD quality) 8000Hz sample rate allows for more than enough time to read from the SD card and fill the unused buffer between the interrupts.

Human voice generally ranges in frequency from 300Hz to 3000Hz (http://en.wikipedia.org/wiki/Voice_frequency). Taking into account the  sampling theorem (http://en.wikipedia.org/wiki/Nyquist%E2%80%93Shannon_sampling_theorem), a sufficient sample-rate is therefore 2 B(hz) samples/second, or anything larger. Thus the 8000Hz sample rate is adequate.

The following libraries are used to interface with the external components:  
DS1307 - DS1307RTC - https://www.pjrc.com/teensy/td_libs_DS1307RTC.html  
DS18S20 - Arduino Library for Dallas Temperature ICs - https://github.com/milesburton/Arduino-Temperature-Control-Library  
TSOP34338SS1F IR Receiver - Arduino-IRremote - https://github.com/shirriff/Arduino-IRremote  
Micro SD - SdFat - https://github.com/greiman/SdFat  

### Schematic
![Schematic](https://raw.githubusercontent.com/darkosancanin/g_and_c_speaker/master/images/schematic.png)