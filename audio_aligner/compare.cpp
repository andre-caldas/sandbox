#include <iostream>
#include <cmath>

#include <Mlt.h>

#include "AudioEnvelopeFFT.h"


using namespace Mlt;

void compare(const char *f1, const char *f2, const char* profile_id = nullptr)
{
  std::cout << "=== PROFILE: " << profile_id << " ===" << std::endl;
  if(nullptr == profile_id)
  {
    profile_id = "atsc_1080p_60";
  }
  Profile profile(profile_id);

  Producer producer_x(profile, f1);
  Producer producer_y(profile, f2);

  int max = std::max(producer_x.get_playtime(), producer_y.get_playtime());

  AudioEnvelopeFFT envelope_x(producer_x, max);
  AudioEnvelopeFFT envelope_y(producer_y, max);

  int calc_lag = envelope_x.get_lag_from(envelope_y);
  char sign = '+';
  if(calc_lag < 0)
  {
    sign = '-';
    calc_lag *= -1;
  }

  float fps = profile.fps();
  int hours = std::floor(calc_lag/(60*60 * fps));
  int minutes = std::floor(calc_lag/(60 * fps)) - 60*hours;
  int seconds = std::floor(calc_lag/(fps)) - 60*60*hours - 60*minutes;
  int frames = std::floor(calc_lag) - 60*60*fps*hours - 60*fps*minutes - fps*seconds;

  std::cout << "=== RESULT (" << profile_id << ") ===" << std::endl;
  std::cout << "Calculated lag: " << sign << calc_lag << " frames. " << ((float)calc_lag) / fps << " seconds." << std::endl;

  std::cout << sign << hours << "h"
            << minutes << "m"
            << seconds << "s (" << sign
            << frames << " frames) (fps = " << fps << ")." << std::endl;
  std::cout << "=== END OF REPORT ===" << std::endl << std::endl;
}

void usage (int return_value = 0)
{
  auto& stream = (0 == return_value) ? std::cout : std::cerr;
  stream << "Usage:" << std::endl;
  stream << "\tcompare <file_1> <file_2> [<profile>]." << std::endl;
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
      compare(argv[1], argv[2], argv[i]);
    }
  }
  else
  {
    compare(argv[1], argv[2]);
  }
  Factory::close();
  return 0;
}
