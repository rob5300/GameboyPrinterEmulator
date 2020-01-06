#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <pigpio.h>
#include <chrono>

class GBoyPrinter
{
	public:
		GBoyPrinter(int clockpin, int in, int out);

	private:
		bool ClockHigh_MagicBytesCheck(int in);
		double CountSeconds(std::chrono::time_point<std::chrono::high_resolution_clock> begin);

		std::vector<int> history;
		int historyMax = 16;

		//0x88 , 136
		//0x33, 51
		//These are backwards due to msb first
		std::vector<int> MagicBytesCompare{
			0,0,0,1,0,0,0,1, 
			1,1,0,0,1,1,0,0
		};

		//0x33 then 0x88
		std::vector<int> MagicBytesCompareBackwards{
			1,1,0,0,1,1,0,0,
			0,0,0,1,0,0,0,1
		};
};