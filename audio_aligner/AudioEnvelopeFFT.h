#ifndef AUDIOENVELOPE_H
#define AUDIOENVELOPE_H

#include <map>

#include <complex.h>
#include <fftw3.h>

#include <MltProducer.h>

#include "FFTInplaceArray.h"


class AudioEnvelopeFFT : public FFTInplaceArray
{
  public:
    AudioEnvelopeFFT(Mlt::Producer &producer, size_t n_frames = 0);
    ~AudioEnvelopeFFT(void) {};
};
#endif

