#include <cmath>
#include <iostream>
#include <memory>

#include "AudioEnvelopeFFT.h"

/*
* An immutable vector representing the Fourier Transform
* for the audio in producer.
*/
AudioEnvelopeFFT::AudioEnvelopeFFT(Mlt::Producer& producer, size_t n_frames)
{
    // In theory, we could want 0 frames.
    // Should we use -1 instead?
    if (0 == n_frames)
    {
        n_frames = producer.get_playtime();
    }

    size_t play_frames = producer.get_playtime();
    play_frames = std::min(n_frames, play_frames);

    init(n_frames);

    std::cerr << "Extracting audio...";
    int frequency = 5 * producer.get_fps();
    int channels = 1;
    mlt_audio_format format = mlt_audio_s16;

    double total = 0;
    for (size_t i = 0; i < play_frames; ++i)
    {
        std::unique_ptr<Mlt::Frame> frame(producer.get_frame(i));
        mlt_position position = mlt_frame_get_position(frame->get_frame());
        int samples = mlt_audio_calculate_frame_samples(float(producer.get_fps()), frequency, position);
        auto* data = static_cast<int16_t*>(frame->get_audio(format, frequency, channels, samples));

        double samples_value = 0;
        for(int k = 0; k < samples; ++k)
        {
            samples_value += std::abs(data[k]);
        }
        samples_value = samples_value / samples;
        envelope(i) = samples_value;
        total += samples_value;
    }

    double mean = total / play_frames;
    std::cerr << "Mean: " << mean << "." << std::endl;
    for (size_t i = 0; i < play_frames; ++i)
    {
        envelope(i) = envelope(i) - mean;
    }

    std::cerr << " Done." << std::endl;
}

