#include <chrono>
#include <iostream>
#include <cmath>

#include <Mlt.h>

#include "AudioEnvelopeFFT.h"


using namespace Mlt;

double drift_score (AudioEnvelopeFFT& x, AudioEnvelopeFFT& y, float drift)
{
  FFTInplaceArray drifted_y = y.clone(drift);
  double quality = 0.0;
  x.get_lag_from(drifted_y, &quality);
  return quality;
}

void get_drift(const char *f1, const char *f2, const char* profile_id = nullptr)
{
  std::cout << "/=== PROFILE: " << profile_id << " ===" << std::endl;
  if(nullptr == profile_id)
  {
    profile_id = "atsc_1080p_60";
  }
  Profile profile(profile_id);

  Producer producer_x(profile, f1);
  Producer producer_y(profile, f2);

  int max = std::max(producer_x.get_playtime(), producer_y.get_playtime());

  auto start_time = std::chrono::steady_clock::now();
  std::cout << "| Start extracting audio..." << std::endl;
  AudioEnvelopeFFT envelope_x(producer_x, max);

  auto finished_x_time = std::chrono::steady_clock::now();
  std::cout << "| Finished extracting first producer's audio."
            << " Elapsed: " << (finished_x_time - start_time).count()/1000000 << " million ticks."
            << std::endl;
  AudioEnvelopeFFT envelope_y(producer_y, max);
  auto finished_y_time = std::chrono::steady_clock::now();
  std::cout << "| Finished extracting second producer's audio."
            << " Elapsed: " << (finished_y_time - finished_x_time).count()/1000000 << " million ticks."
            << std::endl;

  double max_score = 0.0;
  double best_drift = 1.;
  for(double i = -0.1; i <= +0.1; i += 0.0001)
  {
    double score = drift_score(envelope_x, envelope_y, i);
    if(score > max_score)
    {
      max_score = score;
      best_drift = i;
    }
  }

  std::cout << "Determined drift: " << best_drift * 100 << "%." << std::endl;
}


void usage (int return_value = 0)
{
  auto& stream = (0 == return_value) ? std::cout : std::cerr;
  stream << "Usage:" << std::endl;
  stream << "\tdrift <file_1> <file_2> [<profile> [...]]." << std::endl;
}

int main( int argc, const char *argv[] )
{
  if(argc <= 2)
  {
      usage(1);
      return 1;
  }

  Factory::init();
  if(argc > 3)
  {
    for(int i = 3; i < argc; ++i)
    {
      get_drift(argv[1], argv[2], argv[i]);
    }
  }
  else
  {
    get_drift(argv[1], argv[2]);
  }
  Factory::close();
  return 0;
}
