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

	class Hewalex : public CommunicationStats
	{
	public:
		Hewalex();
		~Hewalex();
		void CreateDatalist();
		void Read();

	private:
		const uint8_t sourceAddress = 1;
		const uint8_t destinationAddress = 2;
		const uint8_t amountOfRegisters = 50;

		uint16_t startRegister;
		void Parse(std::vector<uint8_t> data);

		void UpdateRegister(uint16_t registerNumber, uint8_t* data);
		uint8_t crc8_dvb_s2(vector<uint8_t> data, uint8_t length);
		uint16_t crc16_xmodem(vector<uint8_t> data, uint8_t length);
		vector<uint8_t> CreatePacket(uint8_t sender, uint8_t target, uint16_t startRegister, uint8_t amountOfRegisters);
	};
}

#endif
