Did it... :-)

Based on Kdenlive's audio aligner.

You need an audio of more then 4150 frames to test.

Complie:
> $ g++ -fPIC -I /usr/include/mlt-7 -I /usr/include/mlt-7/mlt++ -o play play.cpp FFTInplaceArray.cpp AudioEnvelopeFFT.cpp -Wall -Wextra -lpthread -L /usr/lib/x86_64-linux-gnu/mlt-7/ -l 'mlt++-7' -lmlt-7 -lfftw3

Execute:
> ./play <audio_file>

If you don't want debug output:
> ./play <audio_file> 2>/dev/null
