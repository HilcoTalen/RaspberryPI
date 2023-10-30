#ifndef DATAVALUE_H
#define DATAVALUE_H

#include <cstdint>
#include <ctime>
#include <vector>
#include <string>

namespace GateWay
{
	using namespace std;

	enum DataType
	{
		Word,
		DWord,
		Float,
		Date,
		Time,
	};

	class DataValue
	{
		enum Function
		{
			DiscreteInput,
			Coil,
			InputRegister,
			HoldingRegister,
		};

	public:
		DataValue(DataType dataType, uint16_t address, string name);
		DataValue(DataType dataType, uint16_t address, string name, float divider);
		~DataValue();
		DataValue(const DataValue& source);
		string GetName();
		void SetValue(float value);
		float GetDivider();
		void SetValue(uint8_t value);
		void SetValue(int32_t value);
		void SetValue(uint8_t* data);
		bool DataReceived;
		string GetLatestUpdateTime();
		string GetLatestReadTime();
		std::string ToString();
		vector<uint8_t> getBytes();
		uint16_t address;
		uint16_t readOuts;
		uint16_t readOutsModBUS;
		uint16_t offlineReadOuts;

	private:
		void UpdateLatestUpdateTime();
		time_t updateTime;
		time_t readTime;
		DataType dataType;
		int16_t int16Value;
		int32_t int32Value;
		void Fill(string& str, int length);
		float floatValue;
		tm dateTimeValue;
		float divider;
		string name;
	};
}
#endif
