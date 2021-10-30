# Automatic Audio Aligner and "Undrifter"


Did it... :-)
Based on Kdenlive's audio aligner.


## Description

The program `drift` first detemines if the two producers have a <i>drift</i>.
Some times, one audio is slightly faster/slower then the other.
The program determines how the speed of the second audio should be changed
in order to eliminate the drift.

Then,
it tells you how many frames
(in an "almost-Shotcut" notation HH:MM:SS:FF, where FF is frames)
you need to shift the second audio in order to align it to the first one.
Actually, `compare` prints +HHhMMmSSs (+FF). Or "minus".

From the debug output, you can see that what takes time is not the computation.
The program spends most of its time extracting the audio data, frame by frame.
I think that this is slow when the audio is converted to the frequency of 48000Hz.


## Complie

> $ g++ -fPIC -I /usr/include/mlt-7 -I /usr/include/mlt-7/mlt++ -o drift drift.cpp FFTInplaceArray.cpp AudioEnvelopeFFT.cpp -Wall -Wextra -lpthread -L /usr/lib/x86_64-linux-gnu/mlt-7/ -l 'mlt++-7' -lmlt-7 -lfftw3


## Execute

> ./drift <audio_file1> <audio_file2> [<drift_range> [<mlt_format> [...]]

If you don't want debug output:
> ./drift <audio_file1> <audio_file2> [<drift_range> [<mlt_format> [...]]] 2>/dev/null


## How to test it
1. Compare the audio files and take note of the output "x".
2. Open Shotcut with two tracks.
3. Put one file on each track. Both at time 0.
4. Adjust the speed of the second track according to the value determined by the program.
5. If "x" is positive, move the second track to the indicated place.
I type the value in the 'Project Clip' dialog. And use the "snap".
6. If "x" is negative, do the same as "5", but using the first track.
