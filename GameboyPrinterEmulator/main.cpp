#include <iostream>
#include <pigpio.h>
#include <string>

int main(int argc, char* argv[])
{
	//g++ -Wall -pthread -o foobar foobar.cpp -lpigpio -lrt

	int clockpin = 17;
	int in = 22;
	int out = 27;

	if (gpioInitialise() >= 0)
	{
		std::cout << "Start!" << std::endl;

		gpioSetMode(clockpin, PI_INPUT);
		gpioSetMode(in, PI_INPUT);
		gpioSetMode(out, PI_OUTPUT);

		int lastClockRead = 0;
		int lastInRead = 0;
		std::cout << "Reading pins...";

		while (true) {
			
			int clockpinread = gpioRead(clockpin);
			int inpinread = gpioRead(in);

			if (clockpinread != lastClockRead) {
				std::cout << "Clockpin: " << clockpinread << std::endl;
				lastClockRead = clockpinread;
			}

			if (inpinread != lastInRead) {
				std::cout << "InRead: " << inpinread << std::endl;
				lastInRead = inpinread;
			}


		}
	}
	else {
		std::cout << "Failed to start gpio!" << std::endl;
	}
	return 0;
}