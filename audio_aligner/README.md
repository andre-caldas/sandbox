# Automatic Audio Aligner


Did it... :-)
Based on Kdenlive's audio aligner.


## Description

The program `compare` tells you how many frames
(in an "almost-Shotcut" notation HH:MM:SS:FF, where FF is frames)
you need to shift the second audio in order to align it to the first one.
Actually, `compare` prints +HH:MM:SS (+FF). Or "minus".

From the debug output, you can see that what takes time is not the computation.
The program spends most of its time extracting the audio data, frame by frame.


## Complie

> $ g++ -fPIC -I /usr/include/mlt-7 -I /usr/include/mlt-7/mlt++ -o compare compare.cpp FFTInplaceArray.cpp AudioEnvelopeFFT.cpp -Wall -Wextra -lpthread -L /usr/lib/x86_64-linux-gnu/mlt-7/ -l 'mlt++-7' -lmlt-7 -lfftw3


## Execute

> ./compare <audio_file1> <audio_file2> [<mlt_format> [...]]

If you don't want debug output:
> ./compare <audio_file1> <audio_file2> [<mlt_format> [...]] 2>/dev/null


## How to test it
1. Compare the audio files and take note of the output "x".
2. Open Shotcut with two tracks.
3. Put one file on each track. Both at time 0.
4. If "x" is positive, move the second track to the indicated place.
I type the value in the 'Project Clip' dialog. And use the "snap".
5. If "x" is negative, do the same as "4", but using the first track.
