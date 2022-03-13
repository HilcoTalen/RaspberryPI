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
		DataValue(DataType dataType, uint16_t address, string name, int divider);
		~DataValue();
		DataValue(const DataValue& source);

		void SetValue(float value);
		void SetValue(uint8_t value);
		void SetValue(int16_t value);
		void SetValue(int32_t value);
		void SetValue(tm value);
		void SetValue(uint8_t* data);
		std::string ToString();
		vector<uint8_t> getBytes();
		uint16_t address;

	private:
		DataType dataType;
		int16_t int16Value;
		int32_t int32Value;
		float floatValue;
		tm dateTimeValue;
		int divider;
		string name;
	};
}
#endif
