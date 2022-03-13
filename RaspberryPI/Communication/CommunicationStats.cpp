#include "CommunicationStats.h"
#include <iostream>

namespace GateWay
{
	CommunicationStats::CommunicationStats()
	{
		messagesReceived = 0;
		bytesReceived = 0;
		messagesSended = 0;
		bytesSended = 0;
		timeOuts = 0;
		invalidCrc = 0;
	}

	CommunicationStats::~CommunicationStats()
	{
	}

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

	void CommunicationStats::PrintValues()
	{
		for (DataValue& dataValue : this->DataList)
		{
			cout << dataValue.ToString() << endl;
		}
	}

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

	void CommunicationStats::UpdateSendedData(uint16_t bytes)
	{
		bytesSended = bytesSended + bytes;
		messagesSended++;
	}
	void CommunicationStats::UpdateReceivedData(uint16_t bytes)
	{
		bytesReceived = bytesReceived + bytes;
		messagesReceived++;
	}
}