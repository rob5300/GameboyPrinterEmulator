#include "GBoyPrinter.h"
#include <exception>

using namespace std;

GBoyPrinter::GBoyPrinter(int clockpin, int in, int out)
{	//17,22,27
	if (gpioInitialise() >= 0)
	{
		Print("GameBoy Printer Emulator Start!");

		gpioSetMode(clockpin, PI_INPUT);//17
		gpioSetMode(in, PI_INPUT);//22
		gpioSetMode(out, PI_OUTPUT);//27

		state = PrinterCommand;

		while(true){
			int lastClockRead = 0;
			int lastInRead = 0;

			bitsLeft = ByteLength;

			vector<int> readBytesBuffer;
						
			while (true)
			{
				int clockpinread = gpioRead(clockpin);
				int inpinread = gpioRead(in);

				//Keep checking the clock pin and only execute if the pin changes state.
				if (clockpinread != lastClockRead)
				{
					lastClockRead = clockpinread;
					//If the pin state is not 0 then we can react to the current input value.
					if (clockpinread == 1)
					{
						//lastHighPinReadTime = &chrono::high_resolution_clock::now();
						if(!recievedMagicBytes)
						{
							PreMagicBytesLoop(inpinread);
						}
						else
						{
							MainPacketStateLoop(inpinread, readBytesBuffer);
						}

						//Send any bits if we have any
						if (outputBuffer.size() != 0)
						{
							Print("# [" + to_string(outputBuffer.size()) + "] Out Write was: " + to_string(outputBuffer[0]));
							gpioWrite(out, outputBuffer[0]);
							outputBuffer.erase(outputBuffer.begin() + 0);
						}
						else
						{
							//Write nothing.
							gpioWrite(out, 0);
						}
					}
				}
			}
		}
	}
	else {
		Print("Failed to start gpio!");
	}
}

void GBoyPrinter::PreMagicBytesLoop(int& in)
{
	//Keep looking for the magic bytes that signals the start of data from the Gameboy Camera.
	cout << "In pin was: " << in << endl;
	recievedMagicBytes = ClockHigh_MagicBytesCheck(in);

	if (recievedMagicBytes) {
		//Prepare to read 1 byte for the PrinterCommand state.
		state = PrinterCommand;
		SetBytesToRead(1);
		bytesRead = 0;
		currentByteBuffer = 0;
	}
}

void GBoyPrinter::MainPacketStateLoop(int& in, vector<int>& readBuffer)
{
	//Magic bytes were found soo keep reading bits, create int's out of them and give data to states.
	//Add and left shift in the bits read, when we finish we should have an 8bit number in an unsigned int.
	cout << "# In pin was: " << in << endl;
	currentByteBuffer += in;
	bitsLeft--;
	if (bitsLeft > 0)
	{
		currentByteBuffer << 1;
	}
	else
	{
		//We have read a whole byte!
		readBuffer.push_back(currentByteBuffer);
		bytesRead++;
		if (bytesRead == bytesToRead)
		{
			//Send all the bytes read to be processed.
			Print("Attempting to process state using buffered data.");
			ProcessBufferForState(state, readBuffer);
			readBuffer.clear();
			bytesRead = 0;
		}
		else
		{
			//Reset for reading another byte
			bitsLeft = ByteLength;
		}
		currentByteBuffer = 0;
	}
}

//React to the recieved data depending on the current state.
void GBoyPrinter::ProcessBufferForState(PrinterState& state, vector<int>& data)
{
	switch (state)
	{
		case PrinterCommand:
			PrinterCommandState(data);
			break;
		case CompressionFlag:
			CompressionFlagState(data);
			break;
		case PacketDataLength:
			PacketDataLengthState(data);
			break;
		case PacketData:
			PacketDataState(data);
			break;
		case PacketChecksum:
			PacketChecksumState(data);
			break;
		case Keepalive:
			KeepaliveState(data);
			break;
		case CurrentPrinterStatus:
			CurrentPrinterStatusState(data);
			break;
	}
}

//Process the data for the PrinterCommand state.
void GBoyPrinter::PrinterCommandState(vector<int>& data)
{
	Print("PrinterCommand State checking...");
	int command = data[0];
	switch (command) {
		case 1:
			//Buffer Clear
			Print("PrinterCommand: Buffer Clear");
			mainBuffer.clear();
			break;
		case 2:
			//Print
			Print("PrinterCommand: Print");
			break;
		case 4:
			//Dot data send
			Print("PrinterCommand: DotData send");
			//We are gonna get dot data!.
			break;
		case 16:
			//Status reply
			Print("PrinterCommand: Status Reply");
			//Gameboy wants our status, lets reply with it.
			break;
		default:
			string message = "Unknown command for printer command state: " + to_string(command);
			Print(message);
			break;
	}
	//Advance to next state.
	state = CompressionFlag;
	SetBytesToRead(1);
}

void GBoyPrinter::CompressionFlagState(vector<int>& data)
{
	compressionFlag = data[0];
	Print("Compression flag state: Read flag as: " + to_string(compressionFlag));
	state = PacketDataLength;
	SetBytesToRead(2);
}

void GBoyPrinter::PacketDataLengthState(vector<int>& data)
{
	try{
		Print("PacketDataLength state...");
		Print("Data input array length is: " + to_string(data.size()));
		//Then the 2 data bytes into a 16 bit number and reverse its bits.
		short int packetLength = 0;
		packetLength += data[0];
		packetLength << 8;
		packetLength += data[1];

		dataPacketLength = reverseBits(packetLength);
		Print("Packet Length Read and reversed to be: " + to_string(dataPacketLength));
		if (dataPacketLength != 0) {
			state = PacketData;
			SetBytesToRead(dataPacketLength);
		}
		else
		{
			//Skip the data packet part?
			state = PacketChecksum;
			SetBytesToRead(2);
		}
	}
	catch (exception e) {
	    Print("PacketDataLength read failed due to: ");
		Print(e.what());
	}
}

void GBoyPrinter::PacketDataState(vector<int>& data)
{
	Print("Packet Data State. Will store bytes of length: " + to_string(data.size()));
	mainBuffer = data;
	state = PacketChecksum;
	SetBytesToRead(2);
}

void GBoyPrinter::PacketChecksumState(vector<int>& data)
{
	Print("Packet Checksum check. This is skipped for now but still has to exist to accept the data.");
	state = Keepalive;
	SetBytesToRead(1);
	Print("Set bytes to send for keep alive state that is upcoming now.");
	outputBuffer = { 1,0,0,0, 0,0,0,1 }; //0x81
}

void GBoyPrinter::KeepaliveState(vector<int>& data)
{
	Print("Keep alive state");
	//Queue bits to send!
	SetBytesToRead(1);
	//Set printer status data now.
	outputBuffer = { 0,0,0,1, 0,0,0,0 };
	state = CurrentPrinterStatus;
}

void GBoyPrinter::CurrentPrinterStatusState(vector<int>& data)
{
	//State from: https://gbdev.gg8.se/wiki/articles/Gameboy_Printer#Status_byte
	//Bit 7 	Low Battery 			Set when the voltage is below threshold
	//Bit 6 	Other error
	//Bit 5 	Paper jam 				Set when the encoder gives no pulses when the motor is powered
	//Bit 4 	Packet error
	//Bit 3 	Unprocessed data 		Set when there's unprocessed data in memory - AKA ready to print
	//Bit 2 	Image data full
	//Bit 1 	Currently printing 		Set as long as the printer's burnin' paper
	//Bit 0		Checksum error 			Set when the calculated checksum doesn't match the received one
	
	Print("Printer Status send state. Will currently send 0");
	//We now need to go back to waiting for magic bytes and start over.
	recievedMagicBytes = false;
}

void inline GBoyPrinter::SetBytesToRead(int num)
{
	bytesToRead = num;
	bitsLeft = ByteLength;
}

//Input this input to the history and also check for the magic bytes.
bool GBoyPrinter::ClockHigh_MagicBytesCheck(int in)
{
	history.push_back(in);
	if (history.size() > historyMax) {
		history.erase(history.begin());
	}

	//Check contents of history for magic bytes
	if(history.size() == historyMax){
		int matches = 0;

		for (size_t i = 0; i < history.size(); i++)
		{
			if(history[i] == MagicBytesCompare[i]) matches++;
		}
		if (matches == historyMax) {
			Print("Magic Bytes read!");
			return true;
		}
	}

	return false;
}

double GBoyPrinter::CountSeconds(chrono::time_point<chrono::high_resolution_clock> begin)
{
	chrono::duration<double, milli> seconds = chrono::high_resolution_clock::now() - begin;
	return seconds.count();
}

//Print to the standard output.
void GBoyPrinter::Print(string toPrint)
{
	//Exists as I hate writing this...
	cout << toPrint << endl;
}

short int GBoyPrinter::reverseBits(short int& num)
{
	unsigned int count = 16;
	short int reverse_num = num;

	num >>= 1;
	while (num)
	{
		reverse_num <<= 1;
		reverse_num |= num & 1;
		num >>= 1;
		count--;
	}
	reverse_num <<= count;
	return reverse_num;
}
