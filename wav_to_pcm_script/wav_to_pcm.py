#!/usr/bin/python

import sys
import struct

pcm_data = ''
samples = 0;
f = open(sys.argv[1], "rb")
try:
    f.seek(44);
    data = f.read(1)
    while (data):
        value = struct.unpack('B', data[0])[0]
        pcm_data += str(value) + ", "
        samples = samples + 1
        data = f.read(1)
finally:
    f.close()
	
print "Samples: " + str(samples)
print "PCM Data: " + pcm_data