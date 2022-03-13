#ifndef HEWALEXTHREAD_H
#define HEWALEXTHREAD_H

#include <thread>
#include <string>
#include "SerialPort.h"
#include "DataValue.h"
#include "CommunicationStats.h"

namespace GateWay
{
	using namespace std;

	class HewalexThread : public CommunicationStats
	{
	public:
		HewalexThread();
		~HewalexThread();
		bool Initialize();
		void Read();

	private:
		const uint8_t sourceAddress = 1;
		const uint8_t destinationAddress = 2;
		const uint8_t amountOfRegisters = 50;

		uint16_t startRegister;
		SerialPort serialPortHewalex;
		void Parse(std::vector<uint8_t> data);

		void UpdateRegister(uint16_t registerNumber, uint8_t* data);
		void FillRegisters();
		uint8_t crc8_dvb_s2(vector<uint8_t> data, uint8_t length);
		uint16_t crc16_xmodem(vector<uint8_t> data, uint8_t length);
		vector<uint8_t> CreatePacket(uint8_t sender, uint8_t target, uint16_t startRegister, uint8_t amountOfRegisters);
		uint32_t GetChecksum(uint8_t* packet, uint8_t length, bool header);
	};
}

#endif
