#include <iostream>
#include <Mlt.h>

#include "AudioEnvelopeFFT.h"


using namespace Mlt;

int lags[] = {10, 50, 51, 52, 53, -9, -10, -11, -50, 100, -90, 150};

void play(const char *filename)
{
	Profile profile; // defaults to dv_pal
	for(int lag: lags)
	{
		Producer producer_x(profile, filename);
		Producer producer_y(profile, filename);

		std::cout << "Lag: " << lag << " frames." << std::endl;

		producer_x.set_in_and_out(2000, 4000);
		producer_y.set_in_and_out(2000+lag, 4000+lag);

		AudioEnvelopeFFT envelope_x(producer_x);
		AudioEnvelopeFFT envelope_y(producer_y);

		int calc_lag = envelope_x.get_lag_from(envelope_y);
		std::cout << "Calculated lag: " << calc_lag << std::endl;
	}
}

int main( int, char **argv )
{
	Factory::init();
	play(argv[1]);
	Factory::close();
	return 0;
}
