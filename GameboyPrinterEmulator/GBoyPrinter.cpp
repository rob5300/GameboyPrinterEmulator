#include "GBoyPrinter.h"

GBoyPrinter::GBoyPrinter(int clockpin, int in, int out)
{
	if (gpioInitialise() >= 0)
	{
		std::cout << "Start!" << std::endl;

		gpioSetMode(clockpin, PI_INPUT);
		gpioSetMode(in, PI_INPUT);
		gpioSetMode(out, PI_OUTPUT);

		int lastClockRead = 0;
		int lastInRead = 0;
		std::cout << "Reading pins..." << std::endl;

		//Loop to check for magic bytes to begin.
		while (true) {

			int clockpinread = gpioRead(clockpin);
			int inpinread = gpioRead(in);

			if (clockpinread != lastClockRead) {
				lastClockRead = clockpinread;
				if (clockpinread == 1)
				{
					if (ClockHigh_MagicBytesCheck(inpinread)) {
						history.clear();
						break;
					}
					//Magic bytes not found, do nothing.
				}
			}
		}


		/*while (true) {
			
		}*/
	}
	else {
		std::cout << "Failed to start gpio!" << std::endl;
	}
}

//Called when the clock goes high
bool GBoyPrinter::ClockHigh_MagicBytesCheck(int in)
{
	history.push_back(in);
	if (history.size > historyMax) {
		history.erase(history.begin());
	}

	//Check contents of history for magic bytes
	if (history == MagicBytesCompare) {
		std::cout << "Magic Bytes read!";
		return true;
	}

	return false;
}
