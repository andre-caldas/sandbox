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
you need to shift the second audio in order to align it to the first one.
The program `drift` prints the output in "human format" but it can generate the output in a format easiear to be used with scripts. Just use the `--script` option.

From the debug output (`--debug`), you can see that what takes time is not the computation.
The program spends most of its time extracting the audio data, frame by frame.
I think that this is slow when the audio is converted to the frequency of 48000Hz.


## Complie

> $ g++ -fPIC -I /usr/include/mlt-7 -I /usr/include/mlt-7/mlt++ -o drift drift.cpp FFTInplaceArray.cpp AudioEnvelopeFFT.cpp -Wall -Wextra -lpthread -L /usr/lib/x86_64-linux-gnu/mlt-7/ -l 'mlt++-7' -lmlt-7 -lfftw3


## Execute

> ./undrifted_and_aligned.sh <video_file> <audio_file> [<mlt_profile>] > output.mlt

This will generate an `output.mlt` file that can be used as a clip in `Shotcut`, and can be also played with `melt`:

> melt output.mlt

