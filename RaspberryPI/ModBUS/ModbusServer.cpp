#include "ModbusServer.h"
#include <iomanip>

namespace GateWay
{
	ModbusServer::ModbusServer()
	{
		this->DataList.push_back(DataValue(DWord, 1, "Intergas Sent"));
		this->DataList.push_back(DataValue(DWord, 2, "Intergas Received"));
		this->DataList.push_back(DataValue(DWord, 3, "Intergas Timeouts"));
		this->DataList.push_back(DataValue(DWord, 4, "Hewalex Sent"));
		this->DataList.push_back(DataValue(DWord, 5, "Hewalex Received"));
		this->DataList.push_back(DataValue(DWord, 6, "Hewalex Timeouts"));
		this->DataList.push_back(DataValue(DWord, 7, "P1 Sent"));
		this->DataList.push_back(DataValue(DWord, 8, "P1 Received"));
		this->DataList.push_back(DataValue(DWord, 9, "P1 Timeout"));
		this->DataList.push_back(DataValue(DWord, 10, "tmp"));
	}

	ModbusServer::~ModbusServer()
	{
	}

	void ModbusServer::UpdateComStatistics()
	{
		this->GetRegister(1).SetValue(this->Intergas->messagesSended);
		this->GetRegister(2).SetValue(this->Intergas->messagesReceived);
		this->GetRegister(3).SetValue(this->Intergas->timeOuts);
		this->GetRegister(4).SetValue(this->Hewalex->messagesSended);
		this->GetRegister(5).SetValue(this->Hewalex->messagesSended);
		this->GetRegister(6).SetValue(this->Hewalex->timeOuts);
		this->GetRegister(7).SetValue(this->P1->messagesSended);
		this->GetRegister(8).SetValue(this->P1->messagesSended);
		this->GetRegister(9).SetValue(this->P1->timeOuts);
	}

	bool ModbusServer::Initialize()
	{
		try
		{
			// Create serial port object and open serial port at 57600 buad, 8 data bits, no parity bit, and one stop bit (8n1)
			SerialPort serialPort("/dev/ttyUSB2", BaudRate::B_38400, NumDataBits::EIGHT, Parity::EVEN, NumStopBits::ONE);
			serialPortModbus = serialPort;
			serialPortModbus.SetTimeout(100); // Block when reading until any data is received
			serialPortModbus.Open();

			return true;
		}
		catch (Exception)
		{
			return false;
		}
	}

	void ModbusServer::Read()
	{
		// Read the data.
		std::string readData;
		std::vector<uint8_t> data;
		serialPortModbus.bytesReceived = 0;
		serialPortModbus.ReadBinary(data);
		if (data.size() != 0)
		{
			this->Parse(data);
		}

		// Update the statistics
		this->UpdateComStatistics();
	}

	void ModbusServer::returnData(FunctionCodes functionCode, uint8_t slaveAddress, uint16_t startAddress, vector<uint8_t> returnData)
	{
		vector<uint8_t> data;
		data.push_back(slaveAddress);
		data.push_back(functionCode);
		data.push_back((uint8_t)returnData.size());
		for (uint16_t i = 0; i < returnData.size(); i++)
		{
			data.push_back(returnData.at(i));
		}
		uint16_t crc = this->Crc(data, (uint8_t)data.size());
		data.push_back((uint8_t)(crc & 0xFF));
		data.push_back((uint8_t)(crc >> 8));
		this->serialPortModbus.WriteBinary(data);
	}

	void ModbusServer::returnErrorCode(ErrorCodes errorCode, FunctionCodes functionCode, uint8_t slaveAddress, uint16_t startingAddress, uint16_t quantity)
	{
		vector<uint8_t> data;
		data.push_back(slaveAddress);
		data.push_back(functionCode | 0x80);
		data.push_back(errorCode);
		uint16_t crc = this->Crc(data, (uint8_t)data.size());
		data.push_back((uint8_t)(crc << 8));
		data.push_back((uint8_t)(crc & 0xFF));
		this->serialPortModbus.WriteBinary(data);
		std::cout << "Wrong request, FC: " << functionCode << " Register: " << startingAddress << " Amount: " << quantity << std::endl;
	}

	uint16_t ModbusServer::Crc(vector<uint8_t> data, uint8_t length)
	{
		uint16_t crc = 0xFFFF;

		for (int pos = 0; pos < length; pos++) {
			crc ^= (uint16_t)data[pos];          // XOR byte into least sig. byte of crc

			for (int i = 8; i != 0; i--) {    // Loop over each bit
				if ((crc & 0x0001) != 0) {      // If the LSB is set
					crc >>= 1;                    // Shift right and XOR 0xA001
					crc ^= 0xA001;
				}
				else                            // Else LSB is not set
					crc >>= 1;                    // Just shift right
			}
		}
		// Note, this number has low and high bytes swapped, so use it accordingly (or swap bytes)
		return crc;
	}

	void ModbusServer::RetrieveRegisterData(uint8_t slaveAddress, FunctionCodes functionCode, uint16_t startingAddress, uint16_t quantity)
	{
		CommunicationStats* device;
		switch (slaveAddress)
		{
		case 1:
			device = this;
			break;
		case 2:
			device = Intergas;
			break;
		case 3:
			device = Hewalex;
			break;
		case 4:
			device = P1;
			break;
		default:
			return;
		}

		// The return data.
		vector<uint8_t> returnData;

		switch (functionCode)
		{
		case ReadCoilStatus:
			this->returnErrorCode(IllegalFunction, functionCode, slaveAddress, startingAddress, quantity);
			return;
		case ReadInputStatus:
			if (quantity == 1)
			{
				if (device->HasRegister(startingAddress))
				{
					returnData = device->GetRegister(startingAddress).getBytes();
				}
			}
			break;
		case ReadInputRegisters:
		case ReadHoldingRegisters:
			if ((quantity == 1) || (quantity == 2))
			{
				if (device->HasRegister(startingAddress))
				{
					returnData = device->GetRegister(startingAddress).getBytes();
				}
			}
			break;
		default:
			this->returnErrorCode(IllegalFunction, functionCode, slaveAddress, startingAddress, quantity);
			return;
		}

		if (returnData.size() == 0)
		{
			this->returnErrorCode(IllegalDataAddress, functionCode, slaveAddress, startingAddress, quantity);
		}
		else
		{
			this->returnData(functionCode, slaveAddress, startingAddress, returnData);
		}
	}

	void ModbusServer::Parse(vector<uint8_t> data)
	{
		if (data.size() < 8)
		{
			// Invalid ModBUS message, just return, send no reply.
		}

		// Parse the stuff
		uint16_t crcReceived = data[data.size() - 2] | (uint16_t)data[data.size() - 1] << 8;
		if (this->Crc(data, (uint8_t)data.size() - 2) != crcReceived)
		{
			return;
			// Invalid CRC, return without sending reply.
		}

		uint8_t slaveAddress = data[0];
		FunctionCodes functionCode = (FunctionCodes)data[1];
		uint16_t startingAddress = data[3] | data[2] << 8;
		uint16_t quantity = data[5] | data[4] << 8;

		this->RetrieveRegisterData(slaveAddress, functionCode, startingAddress, quantity);
	}
}