#ifndef MODBUSSERVER_H
#define MODBUSSERVER_H

#include <DataValue.h>
#include <SerialPort.h>
#include <Hewalex.h>
#include <Intergas.h>
#include <P1.h>

using namespace std;

namespace GateWay
{
	enum FunctionCodes
	{
		ReadCoilStatus = 1,
		ReadInputStatus = 2,
		ReadHoldingRegisters = 3,
		ReadInputRegisters = 4,
		ForceSingleCoil = 5,
		ForceSingleRegister = 6,
		ForceMultipleCoils = 15,
		PresetMultipleRegisters = 16,
	};

	enum ErrorCodes
	{
		IllegalFunction = 1,
		IllegalDataAddress = 2,
		IllegalDataValue = 3,
		SlaveDeviceFailure = 4,
		Acknowledge = 5,
		SlaveDeviceBusy = 6,
		NegativeAcknowledge = 7,
		MemoryParityError = 8,
		GatewayPathUnavailable = 10,
		GatewayTargetDeviceFailedToRespond = 11,
	};

	class ModbusServer : public CommunicationStats
	{
		const uint8_t addressGateway = 1;
		const uint8_t addressIntergas = 2;
		const uint8_t addressHewalex = 3;
		const uint8_t addressP1 = 4;

	public:
		ModbusServer();
		~ModbusServer();
		void Read();
		void Parse(vector<uint8_t> data);
		void CreateDatalist();

		Hewalex* hewalex;
		Intergas* intergas;
		P1* p1;

	private:

		uint16_t ticker;
		uint16_t Crc(vector<uint8_t> data, uint8_t length);
		void UpdateComStatistics();
		void returnErrorCode(ErrorCodes errorCode, FunctionCodes functionCode, uint8_t slaveAddress, uint16_t startingAddress, uint16_t quantity);
		void returnData(FunctionCodes functionCode, uint8_t slaveAddress, uint16_t startAddress, vector<uint8_t> returnData);
		void RetrieveRegisterData(uint8_t slaveAddress, FunctionCodes functionCode, uint16_t startingAddress, uint16_t quantity);
	};
}

#endif // MODBUSSERVER_H
