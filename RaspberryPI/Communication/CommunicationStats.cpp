#include "CommunicationStats.h"
#include <iostream>
#include <thread>
#include <iomanip>

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
			if (this->online == false)
			{
				std::cout << this->Name << ": Online again." << std::endl;
			}
			this->online = true;
			this->consecutiveTimeouts = 0;
		}
		else
		{
			this->timeOuts++;
			this->consecutiveTimeouts++;
			if (this->consecutiveTimeouts > this->maximumTimeOuts)
			{
				if (this->online == true)
				{
					std::cout << this->Name << ": Timeout." << std::endl;
				}
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
			this->serialPort.ClearBuffer();

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

	/// <summary>
	/// Gets the serial port.
	/// </summary>
	/// <returns>There serial port.</returns>
	string CommunicationStats::GetSerialPort()
	{
		return this->portName;
	}

	void CommunicationStats::CloseSerialPort()
	{
		this->online = false;
		this->timeOuts = 0;
		this->serialPort.Close();
	}

	uint16_t CommunicationStats::GetTimeouts()
	{
		return this->timeOuts;
	}

	void CommunicationStats::PrintHex(const std::vector<uint8_t>& data)
	{
		for (const uint8_t& value : data) {
			std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<unsigned int>(value) << " ";
		}
		std::cout << std::dec << std::endl; // Reset to decimal mode
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
		std::cout << this->Name << ": Closing port: " << std::endl;
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
	/// Stores the value details into a file.
	/// The file is in '/var/log/gateway/<name or the communication part>_values.txt
	/// The file is created when not exists.
	/// The folder /var/log/gateway should be a RAM drive, as this file is written every second.
	/// </summary>
	void CommunicationStats::StoreValues()
	{
		// Create and open a text file
		string filePathLog = "/var/log/gateway/" + this->Name + "_values.txt";
		string filePathCommFile = "/var/log/gateway/" + this->Name + "_comm.txt";
		ofstream logFile(filePathLog);
		ofstream commFile(filePathCommFile);

		for (DataValue& dataValue : this->DataList)
		{
			logFile << dataValue.ToString() << "  Updated: " << dataValue.GetLatestUpdateTime() << " Readed ModBUS: " << dataValue.GetLatestReadTime() << " (" << static_cast<int>(dataValue.readOuts) << " x ), Offline Reads: " << static_cast<int>(dataValue.offlineReadOuts) << " x " << std::endl;
			commFile << static_cast<int>(dataValue.address) << "\t " << dataValue.ToString() << " \t " << dataValue.GetLatestUpdateTime() << " \t" << dataValue.readOuts << " \t" << dataValue.GetLatestReadTime() << " \t" << static_cast<int>(dataValue.readOutsModBUS) << " \t" << static_cast<int>(dataValue.offlineReadOuts) << std::endl;
		}

		// Close the file
		logFile.close();
		commFile.close();
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