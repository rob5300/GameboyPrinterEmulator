#include "GBoyPrinter.h"

GBoyPrinter::GBoyPrinter(int clockpin, int in, int out)
{
	if (gpioInitialise() >= 0)
	{
		std::cout << "Start!" << std::endl;

		gpioSetMode(clockpin, PI_INPUT);
		gpioSetMode(in, PI_INPUT);
		gpioSetMode(out, PI_OUTPUT);
		while(true){
			int lastClockRead = 0;
			int lastInRead = 0;
			std::cout << "Press enter to begin recording for 0.5 seconds." << std::endl;

			std::string a;
			std::cin >> a;

			//Loop to check for magic bytes to begin.
			std::chrono::time_point begin = std::chrono::high_resolution_clock::now();
			unsigned count = 0;
			while (CountSeconds(begin) < 500){
				int clockpinread = gpioRead(clockpin);
				int inpinread = gpioRead(in);

				if (clockpinread != lastClockRead) {
					lastClockRead = clockpinread;
					if (clockpinread != 0)
					{
						std::cout << "[" << count << "] " << "In pin was " << inpinread << std::endl;
						if (ClockHigh_MagicBytesCheck(inpinread)) {
							history.clear();
							break;
						}
						//Magic bytes not found, do nothing.
					}
				}
				count++;
			}
			std::cout << "Finished reading! Press enter to restart" << std::endl;
			std::string b;
			std::cin >> b;
			std::cout.clear();
		}
	}
	else {
		std::cout << "Failed to start gpio!" << std::endl;
	}
}

//Called when the clock goes high
bool GBoyPrinter::ClockHigh_MagicBytesCheck(int in)
{
	history.push_back(in);
	if (history.size() > historyMax) {
		history.erase(history.begin());
	}

	//Check contents of history for magic bytes
	if(history.size() == historyMax){
		int matches = 0;
		int matches2 = 0;
		for (size_t i = 0; i < history.size(); i++)
		{
			if(history[i] == MagicBytesCompare[i]) matches++;
			if (history[i] == MagicBytesCompareBackwards[i]) matches2++;
		}
		if (matches == historyMax) {
			std::cout << "Magic Bytes read!";
			return true;
		}

		if (matches2 == historyMax) {
			std::cout << "Magic Bytes read for backwards!";
			return true;
		}
	}

	return false;
}

double GBoyPrinter::CountSeconds(std::chrono::time_point<std::chrono::high_resolution_clock> begin)
{
	std::chrono::duration<double, std::milli> seconds = std::chrono::high_resolution_clock::now() - begin;
	return seconds.count();
}
