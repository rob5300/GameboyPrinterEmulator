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
		enum PrinterState {PrinterCommand, CompressionFlag, PacketDataLength, PacketData, PacketChecksum, Keepalive, CurrentPrinterStatus};

	private:
		bool ClockHigh_MagicBytesCheck(int in);
		double CountSeconds(std::chrono::time_point<std::chrono::high_resolution_clock> begin);
		void ProcessBufferForState(PrinterState* state, std::vector<int>* data);
		void Print(std::string toPrint);

		std::vector<int> history;
		std::vector<int> mainBuffer;
		const int historyMax = 16;
		const int ByteLength = 8;
		int bytesToRead = 0;
		int compressionFlag = 0;
		PrinterState state;

		//0x88 , 136, 0b 1000 1000
		//0x33, 51, 0b 0011 0011
		//Sent in MSB first.
		std::vector<int> MagicBytesCompare{
			1,0,0,0,1,0,0,0,
			0,0,1,1,0,0,1,1
		};

		//State Methods
		void PrinterCommandState(std::vector<int>* data);
		void PacketDataLengthState(std::vector<int>* data);
};