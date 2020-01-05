#include <iostream>
#include <pigpio.h>
#include <string>

int main(int argc, char* argv[])
{
	int clockpin = 17;
	int in = 22;
	int out = 27;

	if (gpioInitialise() >= 0)
	{
		std::cout << "Start!" << std::endl;

		gpioSetMode(clockpin, PI_INPUT);
		gpioSetMode(in, PI_INPUT);
		gpioSetMode(out, PI_OUTPUT);

		while (true) {
			int clockpinread = gpioRead(clockpin);
			std::cout << clockpinread << std::endl;
		}
	}
	else {
		std::cout << "Failed to start gpio!" << std::endl;
	}
	return 0;
}