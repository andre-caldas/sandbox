#include <chrono>
#include <thread>
#include <string>
#include <iostream>
#include <iomanip>
#include <cmath>

#include <Mlt.h>

#include "AudioEnvelopeFFT.h"


using namespace Mlt;

void usage (int return_value = 0)
{
  auto& stream = (0 == return_value) ? std::cout : std::cerr;
  stream << "Usage:" << std::endl;
  stream << "\tdrift <file_1> <file_2> [<drift_range> [<profile> [...]]]." << std::endl;
  stream << "Look for speed correction for <file_2> between (1.0 - <drift_range>) and (1.0 + <drift_range>)." << std::endl;
}


void drift_score_and_lag (FFTInplaceArray& x, FFTInplaceArray& y, double drift, double* score, int* lag)
{
  FFTInplaceArray drifted_y = y.clone(drift);
  *lag = x.get_lag_from(drifted_y, score);
}


void get_drift_and_lag_sub(FFTInplaceArray& x, FFTInplaceArray& y, double drift_range, double* drift_about, int precision, int* lag)
{
  if(precision >= 2)
  {
    std::cout << std::setprecision(precision - 2) << std::fixed;
  }
  double drift_step = std::pow(10.0, -precision);

  std::cout << "Precision: " << drift_step * 100 << "%." << std::endl;
  std::cout << "Range: (" << (*drift_about-drift_range) * 100
            << "%, " << (*drift_about+drift_range) * 100 << "%)." << std::endl;

  double max_score = *drift_about;
  double best_drift = 1.0;
  for(double i = *drift_about-drift_range; i <= *drift_about+drift_range; i += drift_step)
  {
    double score = 0.0;
    drift_score_and_lag(x, y, i, &score, lag);
    if(score > max_score)
    {
      max_score = score;
      best_drift = i;
    }
  }
  *drift_about = best_drift;


  if(precision < 9)
  {
    std::cout << "Drift up to now: " << best_drift * 100 << "%."
              << " Lag: " << *lag << " frames." << std::endl;
    get_drift_and_lag_sub(x, y, drift_range/10, drift_about, precision+1, lag);
  }
}


void get_drift(Profile& profile, const char *f1, const char *f2, double drift_range, double drift_about = 1.0, int precision = 4)
{
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

  std::cout << "Calculating speed change for the file " << f2 << "." << std::endl;

  int lag = 0;
  get_drift_and_lag_sub(envelope_x, envelope_y, drift_range, &drift_about, precision, &lag);


  std::cout << "Adjust the speed of the second clip"
            << " to " << drift_about * 100 << "%." << std::endl;

  std::cout << "AFTER adjusting the drift, adjust clip relative position." << std::endl;

  char sign = '+';
  if(lag < 0)
  {
    sign = '-';
    lag *= -1;
  }

  float fps = profile.fps();
  int hours = std::floor(lag/(60*60 * fps));
  int minutes = std::floor(lag/(60 * fps)) - 60*hours;
  int seconds = std::floor(lag/(fps)) - 60*60*hours - 60*minutes;
  int frames = std::floor(lag) - 60*60*fps*hours - 60*fps*minutes - fps*seconds;

  std::cout << "Calculated lag: " << sign << lag << " frames. " << ((float)lag) / fps << " seconds." << std::endl;

  std::cout << sign << hours << "h"
            << minutes << "m"
            << seconds << "s (" << sign
            << frames << " frames) (fps = " << fps << ")." << std::endl;
}


int main( int argc, const char *argv[] )
{
  if(argc <= 2)
  {
      usage(1);
      return 1;
  }

  double drift_range = 0.01; // 99% to 101%.
  if(argc > 3)
  {
    drift_range = std::stof(argv[3]);
  }

  Factory::init();
  if(argc > 4)
  {
    for(int i = 4; i < argc; ++i)
    {
      std::cout << "/=== PROFILE: " << argv[i] << " ===" << std::endl;
      Profile profile(argv[i]);
      get_drift(profile, argv[1], argv[2], drift_range);
    }
  }
  else
  {
    std::cout << "/=== PROFILE: " << "atsc_1080p_60" << " ===" << std::endl;
    Profile profile("atsc_1080p_60");
    get_drift(profile, argv[1], argv[2], drift_range);
  }
  Factory::close();
  return 0;
}
