#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <pigpio.h>

class GBoyPrinter
{
	public:
		GBoyPrinter(int clockpin, int in, int out);

	private:
		bool ClockHigh_MagicBytesCheck(int in);

		std::vector<int> history;
		int historyMax = 16;

		//0x88 , 136
		//0x33, 51
		//These are backwards due to backwards input
		std::vector<int> MagicBytesCompare{
			0,0,0,1,0,0,0,1, 
			1,1,0,0,1,1,0,0
		}
};

