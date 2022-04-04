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

    const int fraction_of_a_second = 240;
    init(std::ceil(double(fraction_of_a_second * n_frames) / producer.get_fps()));

    std::cerr << "Extracting audio...";

    int frequency = 48000;
    // I don't really know how to properly handle "channels".
    // So, let's use channels = 1.
    int channels = 1;
    mlt_audio_format format = mlt_audio_s16;

    // I hope frame->get_audio() does not change the value of "frequency".
    // I believe the correct thing to do would probably be:
    // - Consider a different frequency for each frame!
    // But we are assuming the frequency is equal to 48000 for every frame.
    // A better approach would be to assume the frequency is constant,
    // but not necessarily equal to 48000. But then, if it is not a multiple
    // of "fraction_of_a_second", we would have a problem! :-(
    const int samples_per_fraction = frequency / fraction_of_a_second;
    int current_sample_set = 0;
    int samples_to_add = samples_per_fraction;
    assert(samples_to_add > 0);

    double total = 0;
    for (size_t i = 0; i < play_frames; ++i)
    {
        std::unique_ptr<Mlt::Frame> frame(producer.get_frame(i));
        mlt_position position = mlt_frame_get_position(frame->get_frame());
        int samples = mlt_audio_calculate_frame_samples(float(producer.get_fps()), frequency, position);
        auto* data = static_cast<int16_t*>(frame->get_audio(format, frequency, channels, samples));
        assert(48000 == frequency);

        for(int k = 0; k < samples; ++k)
        {
            if (samples_to_add <= 0)
            {
              ++current_sample_set;
              envelope(current_sample_set) = 0;
              samples_to_add = samples_per_fraction;
            }
            envelope(current_sample_set) += std::abs(data[k]);
            total += std::abs(data[k]);
            --samples_to_add;
        }
    }

    double mean = total / (current_sample_set + 1);
    std::cerr << "Mean: " << mean << "." << std::endl;
    for (int i = 0; i <= current_sample_set; ++i)
    {
        envelope(i) = envelope(i) - mean;
    }
}

