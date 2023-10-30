#ifndef P1_H
#define P1_H

#include <SerialPort.h>
#include <CommunicationStats.h>

namespace GateWay
{
	using namespace std;

	class P1 : public CommunicationStats
	{
	public:
		P1();
		~P1();
		void Read();
		void CreateDatalist();

	private:
		unsigned int CRC16(unsigned int crc, unsigned char* buf, int len);
		bool isNumber(char* res, int len);
		int FindCharInArrayRev(char array[], char c, int len);
		double GetValue(char* buffer, int maxlen, char startchar, char endchar);
		bool decode_telegram(int len);
		bool IsItem(string line, string item);
		uint16_t timeOutsConsecutive;
		uint16_t timeOutConsecutive;

		uint16_t calculateCRC16IBM(const char* message, size_t length);
		void CheckAndParseLine(char* line, string item, char startChar, char endChar, int address);

		// * Max telegram length
#define P1_MAXLINELENGTH 1050
#define BAUD_RATE 115200

// * Set to store received telegram
		char telegram[P1_MAXLINELENGTH];
		// * Set during CRC checking
		unsigned int currentCRC = 0;

		static const int aConsumedLowTariff = 2;
		static const int aConsumedHighTariff = 4;
		static const int aReturnedLowTariff = 6;
		static const int aReturnedHighTariff = 8;
		static const int aShortPowerFailureCnt = 10;
		static const int aLongPowerFailureCnt = 12;
		static const int aShortPowerFailureDuration = 14;
		static const int aLongPowerFailureDuration = 16;
		static const int aGas = 18;
		static const int aActualConsumption = 20;
		static const int aActualReturn = 22;
		static const int aActualConsumptionL1 = 24;
		static const int aActualConsumptionL2 = 26;
		static const int aActualConsumptionL3 = 28;
		static const int aActualReturnL1 = 30;
		static const int aActualReturnL2 = 32;
		static const int aActualReturnL3 = 34;
		static const int aCurrentL1 = 36;
		static const int aCurrentL2 = 38;
		static const int aCurrentL3 = 40;
		static const int aVoltageL1 = 42;
		static const int aVoltageL2 = 44;
		static const int aVoltageL3 = 46;
		static const int aTariff = 48;

		string data;
	};
}
#endif
