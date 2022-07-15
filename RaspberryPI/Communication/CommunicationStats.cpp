#include "CommunicationStats.h"
#include <iostream>
#include <thread>

namespace GateWay
{
	/// <summary>
	/// Initiates the communication stats class.
	/// Set all values to 0.
	/// </summary>
	CommunicationStats::CommunicationStats()
	{
		messagesReceived = 0;
		bytesReceived = 0;
		messagesSended = 0;
		bytesSended = 0;
		timeOuts = 0;
		invalidCrc = 0;
	}

	/// <summary>
	/// Destructor.
	/// </summary>
	CommunicationStats::~CommunicationStats()
	{
		// delete serialPort;
	}

	void CommunicationStats::Sleep(int milliSeconds)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(milliSeconds));
	}

	void CommunicationStats::UpdateTimeout(bool validDataReceived)
	{
		if (validDataReceived)
		{
			this->online = true;
			this->consecutiveTimeouts = 0;
		}
		else
		{
			this->timeOut++;
			this->consecutiveTimeouts++;
			if (this->consecutiveTimeouts > 5)
			{
				this->online = false;
			}

			// Prevent overflow
			if (this->consecutiveTimeouts > 250)
			{
				this->consecutiveTimeouts = 6;
			}
		}
	}

	bool CommunicationStats::OpenSerialPort(string ttyInterface)
	{
		this->portName = ttyInterface;

		std::cout << this->Name << ": Try to open port: " << ttyInterface << std::endl;

		try
		{
			// Create serial port object and open serial port.
			this->serialPort = SerialPort();
			this->serialPort.SetDevice(this->portName);
			this->serialPort.SetBaudRate(this->baudRate);
			this->serialPort.SetNumDataBits(this->dataBits);
			this->serialPort.SetParity(this->parity);
			this->serialPort.SetNumStopBits(this->stopBits);
			this->serialPort.SetTimeout(this->timeOut);
			this->serialPort.Open();

			// Reset message sended
			this->messagesSended = 0;
			this->messagesReceived = 0;

			return true;
		}
		catch (Exception)
		{
			return false;
		}
	}

	uint16_t CommunicationStats::GetTimeouts()
	{
		return this->timeOuts;
	}

	/// <summary>
	/// Checks if the data list contains given address.
	/// </summary>
	/// <param name="address">The address to check.</param>
	/// <returns>True when the address exists, otherwise false.</returns>
	bool CommunicationStats::HasRegister(uint16_t address)
	{
		for (DataValue& dataValue : this->DataList)
		{
			if (dataValue.address == address)
			{
				return true;
			}
		}
		return false;
	}

	void CommunicationStats::SwitchComport(string ttyInterface)
	{
		while (this->serialPort.Busy())
		{
			// just wait
			this->Sleep(100);
		}
		std::cout << this->Name << ": Close port: " << std::endl;
		this->serialPort.Close();
		this->OpenSerialPort(ttyInterface);
	}

	/// <summary>
	/// Print all the values in the data list.
	/// </summary>
	void CommunicationStats::PrintValues()
	{
		for (DataValue& dataValue : this->DataList)
		{
			cout << dataValue.ToString() << endl;
		}
	}

	/// <summary>
	/// Returns the register, based on the given address.
	/// </summary>
	/// <param name="address">The requested address.</param>
	/// <returns>The requested register.</returns>
	DataValue& CommunicationStats::GetRegister(uint16_t address)
	{
		for (DataValue& dataValue : this->DataList)
		{
			if (dataValue.address == address)
			{
				return dataValue;
			}
		}

		throw ("Cannot find the register " + address);
	}

	/// <summary>
	/// Updates the statisticss with the sended bytes.
	/// </summary>
	/// <param name="bytes">The bytes which have been sended.</param>
	void CommunicationStats::UpdateSendedData(uint16_t bytes)
	{
		bytesSended = bytesSended + bytes;
		messagesSended++;
	}

	/// <summary>
	/// Updates the received data statistics.
	/// </summary>
	/// <param name="bytes">The amount of bytes which are received.</param>
	void CommunicationStats::UpdateReceivedData(uint16_t bytes)
	{
		bytesReceived = bytesReceived + bytes;
		messagesReceived++;
	}
}