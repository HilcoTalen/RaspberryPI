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

	bool CommunicationStats::OpenSerialPort()
	{
		try
		{
			// Create serial port object and open serial port at 57600 buad, 8 data bits, no parity bit, and one stop bit (8n1)
			this->serialPort = SerialPort();
			this->serialPort.SetDevice(this->portName);
			this->serialPort.SetBaudRate(this->baudRate);
			this->serialPort.SetNumDataBits(this->dataBits);
			this->serialPort.SetParity(this->parity);
			this->serialPort.SetNumStopBits(this->stopBits);
			this->serialPort.SetTimeout(this->timeOut);
			this->serialPort.Open();
			return true;
		}
		catch (Exception)
		{
			return false;
		}
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