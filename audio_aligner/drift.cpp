#include <boost/program_options.hpp>

#include <cstdlib>
#include <chrono>
#include <thread>
#include <string>
#include <sstream>
#include <vector>
#include <iostream>
#include <iomanip>
#include <cmath>

#include <Mlt.h>

#include "AudioEnvelopeFFT.h"


static bool use_percent;
static bool print_debug;
static bool script_output;
static std::string profile_name;
static std::string reference_file;
static std::vector<std::string> input_files;
static double approximate_drift;
static double drift_range;
static int final_precision;
static int print_precision;
static std::string field_separator;

static std::ostream nullout(nullptr);
std::ostream& debug_out (void) { return (print_debug) ? std::cerr : nullout; }
std::ostream& human_out (void) { return (!script_output) ? std::cout : nullout; }
std::ostream& script_out (void) { return (script_output) ? std::cout : nullout; }
namespace po = boost::program_options;

po::variables_map process_args(int argc, const char* argv[])
{
  po::options_description desc("Options");
  desc.add_options()
    ("help,h",
        "help message")
    ("script,s",
        po::bool_switch(&script_output)->default_value(false),
        "output one line with <un-drift speed> <lag direction> <lag frame count>")
    ("profile,p",
        po::value<std::string>(&profile_name)->default_value("atsc_1080p_60"),
        "MLT profile")
    ("percent",
        po::bool_switch(&use_percent)->default_value(false),
        "use percentage (i.e.: %) in reports")
    ("debug",
        po::bool_switch(&print_debug)->default_value(false),
        "print debug output")
    ("field-separator,d",
        po::value<std::string>(&field_separator)->default_value(";"),
        "field separator for script output (default ';'): speed;lag file number;lag")
    ("approximate-drift,a",
        po::value<double>(&approximate_drift)->default_value(1.0),
        "initial drift estimation")
    ("drift-range,r",
        po::value<double>(&drift_range)->default_value(0.01),
        "initial drift estimation")
    ("print-precision,m",
        po::value<int>(&print_precision)->default_value(6),
        "number of precision digits")
    ("precision",
        po::value<int>(&final_precision)->default_value(9),
        "number of precision digits")
    ("base-file,b",
        po::value<std::string>(&reference_file)->required(),
        "reference to align to")
    ("input-file,i",
        po::value<std::vector<std::string>>(&input_files)->required(),
        "file to be un-drifted and aligned")
  ;

  po::positional_options_description p;
  po::variables_map vm;

  try
  {
    p.add("base-file", 1);
    p.add("input-file", 1);
    p.add("input-file", -1);

    po::store(po::command_line_parser(argc, argv).
      options(desc).positional(p).run(), vm);

    if(vm.count("help"))
    {
      std::cerr << desc << std::endl;
      std::exit(0);
    }

    po::notify(vm);
  }
  catch(std::exception& e)
  {
      std::cerr << "Error: " << e.what() << std::endl;
      std::exit(1);
  }
  catch(...)
  {
      std::cerr << "Unknown error!" << std::endl;
      std::exit(2);
  }

  return vm;
}

std::string format_percent (double value, int precision)
{
  int after_dot = (use_percent) ? (precision - 2) : precision;
  value = (use_percent) ? (value * 100) : value;

  // Would be nice to use std::format.
  std::ostringstream result;
  if(value < 0)
  {
    result << "-";
    value = -value;
  }

  result << std::to_string((int) std::floor(value));
  if(after_dot > 0)
  {
    result << ".";
    result << std::to_string((int) std::floor( (value - std::floor(value)) * std::pow(10.0,after_dot) ));
  }
  if(use_percent) result << "%";
  return result.str();
}


using namespace Mlt;

void drift_score_and_lag (FFTInplaceArray& x, FFTInplaceArray& y, double drift, double* score, int* lag)
{
  FFTInplaceArray drifted_y = y.clone(drift);
  *lag = x.get_lag_from(drifted_y, score);
}


void get_drift_and_lag_sub(FFTInplaceArray& x, FFTInplaceArray& y, double drift_range, double* drift_about, const int precision, int* lag)
{
  double drift_step = std::pow(10.0, -precision);

  human_out() << "Precision: " << format_percent(drift_step, precision) << "." << std::endl;
  human_out() << "Range: (" << format_percent(*drift_about-drift_range, precision)
              << ", " << format_percent(*drift_about+drift_range, precision) << ")." << std::endl;

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


  if(precision < final_precision)
  {
    human_out() << "Drift up to now: " << format_percent(best_drift, precision) << "."
                << " Lag: " << *lag << " frames." << std::endl;
    get_drift_and_lag_sub(x, y, drift_range/10, drift_about, precision+1, lag);
  }
}


void get_drift(Profile& profile, const char *f1, const char *f2, double drift_range, double drift_about, int precision = 4)
{
  Producer producer_x(profile, f1);
  Producer producer_y(profile, f2);

  int max = std::max(producer_x.get_playtime(), producer_y.get_playtime());

  auto start_time = std::chrono::steady_clock::now();
  human_out() << "| Start extracting audio..." << std::endl;
  AudioEnvelopeFFT envelope_x(producer_x, max);

  auto finished_x_time = std::chrono::steady_clock::now();
  human_out() << "| Finished extracting first producer's audio."
              << " Elapsed: " << (finished_x_time - start_time).count()/1000000 << " million ticks."
              << std::endl;
  AudioEnvelopeFFT envelope_y(producer_y, max);
  auto finished_y_time = std::chrono::steady_clock::now();
  human_out() << "| Finished extracting second producer's audio."
              << " Elapsed: " << (finished_y_time - finished_x_time).count()/1000000 << " million ticks."
              << std::endl;

  human_out() << "Calculating speed change for the file " << f2 << "." << std::endl;

  int lag = 0;
  get_drift_and_lag_sub(envelope_x, envelope_y, drift_range, &drift_about, precision, &lag);

  script_out() << format_percent(drift_about, print_precision) << field_separator
               << ((lag < 0)?("1"):("2")) << field_separator
               << ((lag < 0)?(-lag):(lag)) << std::endl;

  human_out() << "Adjust the speed of the second clip to "
              << format_percent(drift_about, print_precision) << "." << std::endl;

  human_out() << "AFTER adjusting the drift, adjust clip relative position." << std::endl;

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

  human_out() << "Calculated lag: " << sign << lag << " frames. "
              << ((float)lag) / fps << " seconds." << std::endl;

  human_out() << sign << hours << "h"
              << minutes << "m"
              << seconds << "s (" << sign
              << frames << " frames) (fps = " << fps << ")." << std::endl;
}


int main( int argc, const char *argv[] )
{
  auto vm = process_args(argc, argv);

  Factory::init();

  Profile profile(profile_name.c_str());
  for(auto& drifted_file: input_files)
  {
    human_out() << "/=== PROFILE: " << profile_name << " ===" << std::endl;
    get_drift(profile, reference_file.c_str(), drifted_file.c_str(), drift_range, approximate_drift);
  }

  Factory::close();
  return 0;
}

