#ifndef COMMUNICATIONSTATS_H
#define COMMUNICATIONSTATS_H

#include <cstdint>
#include <DataValue.h>
#include <SerialPort.h>

namespace GateWay
{
	class CommunicationStats
	{
	public:
		CommunicationStats();
		~CommunicationStats();

		void UpdateSendedData(uint16_t bytes);
		void UpdateReceivedData(uint16_t bytes);
		DataValue& GetRegister(uint16_t address);
		bool OpenSerialPort(string serialPort);
		uint16_t GetTimeouts();
		bool HasRegister(uint16_t address);
		void SwitchComport(string ttyInterface);
		bool online;
		uint16_t invalidCrc;
		int16_t messagesSended;
		uint16_t messagesReceived;

	protected:
		// The data list.
		vector<DataValue> DataList;
		void PrintValues();
		void Sleep(int milliSeconds);
		void UpdateTimeout(bool validDataReceived);
		// Serial port settings.
		SerialPort serialPort;
		string portName;
		BaudRate baudRate;
		NumDataBits dataBits;
		Parity parity;
		NumStopBits stopBits;
		int timeOut = 1000;
		string Name;

	private:
		virtual void CreateDatalist() = 0;
		uint16_t timeOuts;
		uint16_t bytesReceived;
		uint16_t bytesSended;
		uint8_t consecutiveTimeouts;
	};
}
#endif // !COMMUNICATIONSTATS_H
