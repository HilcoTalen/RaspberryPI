#ifndef COMMUNICATIONSTATS_H
#define COMMUNICATIONSTATS_H

#include <cstdint>
#include <DataValue.h>

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
		bool HasRegister(uint16_t address);
		uint16_t timeOuts;
		uint16_t invalidCrc;
		uint16_t messagesSended;
		uint16_t messagesReceived;

	protected:
		vector<DataValue> DataList;
	private:
		void PrintValues();
		uint16_t bytesReceived;
		uint16_t bytesSended;

		
	};
}
#endif // !COMMUNICATIONSTATS_H
