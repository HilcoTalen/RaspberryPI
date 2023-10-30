#include "ModbusServer.h"
#include <iomanip>

namespace GateWay
{
	/// <summary>
	/// Initiates the ModBUS server.
	/// Adds the data values (statistics) to the list.
	/// </summary>
	ModbusServer::ModbusServer()
	{
		this->Name = "ModBUS";
		this->baudRate = BaudRate::B_38400;
		this->dataBits = NumDataBits::EIGHT;
		this->parity = Parity::EVEN;
		this->stopBits = NumStopBits::ONE;
		this->timeOut = 5;
		this->ticker = 0;
	}

	void ModbusServer::CreateDatalist()
	{
		this->DataList.push_back(DataValue(DWord, 1, "Intergas Sent"));
		this->DataList.push_back(DataValue(DWord, 2, "Intergas Received"));
		this->DataList.push_back(DataValue(DWord, 3, "Intergas Timeouts"));
		this->DataList.push_back(DataValue(DWord, 4, "Hewalex Sent"));
		this->DataList.push_back(DataValue(DWord, 5, "Hewalex Received"));
		this->DataList.push_back(DataValue(DWord, 6, "Hewalex Timeouts"));
		this->DataList.push_back(DataValue(DWord, 7, "P1 Sent (not used)"));
		this->DataList.push_back(DataValue(DWord, 8, "P1 Received"));
		this->DataList.push_back(DataValue(DWord, 9, "P1 Timeout"));
		this->DataList.push_back(DataValue(DWord, 10, "Status"));
	}

	/// <summary>
	/// Destructor.
	/// </summary>
	ModbusServer::~ModbusServer()
	{
		// Nothing to destruct.
		// Serial port is closed in base class.
	}

	/// <summary>
	/// Updates the communication statistics of all devices connected.
	/// </summary>
	void ModbusServer::UpdateComStatistics()
	{
		this->GetRegister(1).SetValue(this->intergas->messagesSended);
		this->GetRegister(2).SetValue(this->intergas->messagesReceived);
		this->GetRegister(3).SetValue(this->intergas->GetTimeouts());
		this->GetRegister(4).SetValue(this->hewalex->messagesSended);
		this->GetRegister(5).SetValue(this->hewalex->messagesReceived);
		this->GetRegister(6).SetValue(this->hewalex->GetTimeouts());
		this->GetRegister(7).SetValue(this->p1->messagesSended);	// We do not send messages.
		this->GetRegister(8).SetValue(this->p1->messagesReceived);
		this->GetRegister(9).SetValue(this->p1->GetTimeouts());

		// Merge the online status of the devices.
		int status = this->intergas->online ? 1 : 0;
		status |= this->hewalex->online ? 2 : 0;
		status |= this->p1->online ? 4 : 0;
		this->GetRegister(10).SetValue(status);
	}

	/// <summary>
	/// Reads the serial port, and parses the received data.
	/// </summary>
	void ModbusServer::Read()
	{
		// Read the data.
		std::string readData;
		std::vector<uint8_t> data;
		serialPort.bytesReceived = 0;
		while (serialPort.BytesReceiving())
		{
			this->Sleep(1);
		}

		serialPort.ReadBinary(data);
		if (data.size() != 0)
		{
			this->Parse(data);
		}

		// Update the statistics
		this->UpdateComStatistics();
		this->Sleep(1);

		// Save some info.
		this->ticker++;
		if (this->ticker == 100)
		{
			this->StoreValues();
			this->ticker = 0;
		}
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
		this->serialPort.WriteBinary(data);
	}

	/// <summary>
	/// Sends an error code to the ModBUS client.
	/// </summary>
	/// <param name="errorCode">The error code.</param>
	/// <param name="functionCode">The original function code.</param>
	/// <param name="slaveAddress">The slave (client) address.</param>
	/// <param name="startingAddress">The starting address (only for the debug message).</param>
	/// <param name="quantity">The quantity (only for the debug message).</param>
	void ModbusServer::returnErrorCode(ErrorCodes errorCode, FunctionCodes functionCode, uint8_t slaveAddress, uint16_t startingAddress, uint16_t quantity)
	{
		vector<uint8_t> data;
		data.push_back(slaveAddress);
		data.push_back(functionCode | 0x80);
		data.push_back(errorCode);
		uint16_t crc = this->Crc(data, (uint8_t)data.size());
		data.push_back((uint8_t)(crc << 8));
		data.push_back((uint8_t)(crc & 0xFF));
		this->serialPort.WriteBinary(data);

		// If the error code is not a slave device failure, we should print a debug message.
		// The SlaveDeviceFailure will happen when the data is old, but it's not an error.
		if (errorCode != SlaveDeviceFailure)
		{
			std::cout << dec << "Wrong request (" << unsigned(errorCode) << "), Address: " << unsigned(slaveAddress) << " FunctionCode : " << unsigned(functionCode) << " Register : " << unsigned(startingAddress) << " Amount : " << unsigned(quantity) << std::endl;
		}
	}

	/// <summary>
	/// Calculates the CRC code for the given data, with given length.
	/// </summary>
	/// <param name="data">The data.</param>
	/// <param name="length">The length of the data, to calculate the CRC over.</param>
	/// <returns>The calculated CRC code.</returns>
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

	/// <summary>
	/// Retrieves the requested data of the devices.
	/// </summary>
	/// <param name="slaveAddress">The slave address.</param>
	/// <param name="functionCode">The function code.</param>
	/// <param name="startingAddress">The starting address.</param>
	/// <param name="quantity">The quantity of the registers.</param>
	void ModbusServer::RetrieveRegisterData(uint8_t slaveAddress, FunctionCodes functionCode, uint16_t startingAddress, uint16_t quantity)
	{
		CommunicationStats* device;
		switch (slaveAddress)
		{
		case 1:
			device = this;
			break;
		case 2:
			device = intergas;
			break;
		case 3:
			device = hewalex;
			break;
		case 4:
			device = p1;
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
					DataValue& dataValue = device->GetRegister(startingAddress);
					if (dataValue.DataReceived)
					{
						// We have the data received from the device, lets continue.
						returnData = device->GetRegister(startingAddress).getBytes();
					}
					else
					{
						this->returnErrorCode(SlaveDeviceFailure, functionCode, slaveAddress, startingAddress, quantity);
						dataValue.offlineReadOuts++;
						return;
					}
				}
			}
			break;
		case ReadInputRegisters:
		case ReadHoldingRegisters:
			if (device->HasRegister(startingAddress))
			{
				DataValue& dataValue = device->GetRegister(startingAddress);
				if (dataValue.DataReceived)
				{
					// We have the data received from the device, lets continue.
					returnData = device->GetRegister(startingAddress).getBytes();
					uint8_t dataSizeOneRegister = returnData.size();
					if (quantity == dataSizeOneRegister)
					{
						// It's just a call for one register. So no extra bytes needed.
					}
					else
					{
						// We have a call for multiple registers.
						// First check if the consecutive registers are avaialable.
						uint8_t realRegistersRequested = quantity / dataSizeOneRegister;

						vector<uint8_t> extraBytes;

						bool consecutiveRegistersReadible = true;
						// Start register is one higher, as the first register is already set in the returnData.
						for (uint16_t registerAddress = startingAddress + 1; registerAddress < startingAddress + realRegistersRequested; registerAddress++)
						{
							// Check if the register exists.
							if (device->HasRegister(registerAddress))
							{
								vector<uint8_t> tmp = device->GetRegister(registerAddress).getBytes();
								// Check if the data size of the register is the same size as the first one.
								if (tmp.size() == dataSizeOneRegister)
								{
									// Add this to the return data.
									extraBytes.insert(extraBytes.end(), tmp.begin(), tmp.end());
								}
								else
								{
									consecutiveRegistersReadible = false;
									std::cout << "Multiple registers: The size of register address " << registerAddress << " differs from first address (" << startingAddress << " , Size: " << dataSizeOneRegister << std::endl;
									break;
								}
							}
							else
							{
								consecutiveRegistersReadible = false;
								std::cout << "Multiple registers: Register address " << registerAddress << " not found" << std::endl;
								break;
							}
						}

						if (consecutiveRegistersReadible)
						{
							returnData.insert(returnData.end(), extraBytes.begin(), extraBytes.end());
						}
						else
						{
							this->returnErrorCode(IllegalDataAddress, functionCode, slaveAddress, startingAddress, quantity);
							return;
						}
					}
				}
				else
				{
					this->returnErrorCode(SlaveDeviceFailure, functionCode, slaveAddress, startingAddress, quantity);
					dataValue.offlineReadOuts++;
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

	static void PrintHex(vector < uint8_t> data)
	{
		for (int i = 0; i < data.size(); i++)
			cout << hex << setfill('0') << setw(2) << data[i] << " ";
		cout << endl;
	}

	/// <summary>
	/// Parses the readed data.
	/// </summary>
	/// <param name="data">The data to parse.</param>
	void ModbusServer::Parse(vector<uint8_t> data)
	{
		if (data.size() < 7)
		{
			this->UpdateTimeout(false);
			std::cout << "Invalid ModBUS message received (" << data.size() << " bytes). ";
			PrintHex(data);
			return;
			// Invalid ModBUS message, just return, send no reply.
		}

		// Parse the stuff
		uint16_t crcReceived = data[data.size() - 2] | (uint16_t)data[data.size() - 1] << 8;
		uint16_t crcCalculated = this->Crc(data, (uint8_t)data.size() - 2);
		if (crcCalculated != crcReceived)
		{
			std::cout << "Invalid ModBUS CRC received (" << crcReceived << "), expected: " << crcCalculated << " ";
			this->invalidCrc++;
			this->UpdateTimeout(false);
			PrintHex(data);
			return;
			// Invalid CRC, return without sending reply.
		}

		this->UpdateTimeout(true);
		uint8_t slaveAddress = data[0];
		FunctionCodes functionCode = (FunctionCodes)data[1];
		uint16_t startingAddress = data[3] | data[2] << 8;
		uint16_t quantity = data[5] | data[4] << 8;

		this->RetrieveRegisterData(slaveAddress, functionCode, startingAddress, quantity);
	}
}