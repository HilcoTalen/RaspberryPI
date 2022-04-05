#include "DataValue.h"
#include <string.h>

namespace GateWay
{
	using namespace std;

	/// <summary>
	/// Initiates a new data value.
	/// </summary>
	/// <param name="dataType">The data value type.</param>
	/// <param name="address">The data value address.</param>
	/// <param name="name">The data value name. (Only used for printing).</param>
	DataValue::DataValue(DataType dataType, uint16_t address, string name)
	{
		this->dataType = dataType;
		this->address = address;
		this->name = name;
		this->floatValue = 0.0f;
		this->int16Value = 0;
		this->int32Value = 0;
		this->divider = 1;
	}

	/// <summary>
	/// Initiates a new data value, including the divider.
	/// </summary>
	/// <param name="dataType">The data value type.</param>
	/// <param name="address">The data value address.</param>
	/// <param name="name">The data value name. (Only used for printing).</param>
	/// <param name="divider">The data value divider.</param>
	DataValue::DataValue(DataType dataType, uint16_t address, string name, int divider)
	{
		this->dataType = dataType;
		this->address = address;
		this->name = name;
		this->divider = divider;
		this->floatValue = 0.0f;
		this->int16Value = 0;
		this->int32Value = 0;
	}

	/// <summary>
	/// The destructor.
	/// </summary>
	DataValue::~DataValue()
	{
	}

	/// <summary>
	/// The copy constructor
	/// </summary>
	/// <param name="source"></param>
	DataValue::DataValue(const DataValue& source)
	{
		this->dataType = source.dataType;
		this->address = source.address;
		this->floatValue = source.floatValue;
		this->int16Value = source.int16Value;
		this->int32Value = source.int32Value;
		this->divider = source.divider;
		this->name = source.name;
	}

	/// <summary>
	/// Sets a float value.
	/// </summary>
	/// <param name="value">The new value.</param>
	void DataValue::SetValue(float value)
	{
		this->floatValue = value;
	}

	/// <summary>
	/// Sets a 8 bit value.
	/// </summary>
	/// <param name="value">The new value.</param>
	void DataValue::SetValue(uint8_t value)
	{
		this->int16Value = (int16_t)value;
	}

	/// <summary>
	/// Sets a new value.
	/// </summary>
	/// <param name="value">The new value.</param>
	void DataValue::SetValue(uint8_t* data)
	{
		switch (this->dataType)
		{
		case Word:
			this->int16Value = data[0] | ((uint16_t)data[1] << (uint16_t)8);
			break;
		case DWord:
			this->int32Value = data[0] | ((int32_t)data[1] << 8) | ((int32_t)data[2] << 16) | ((int32_t)data[3] << 24);
			break;
		case Float:
		{
			uint16_t tmpValue = (uint16_t)data[0] | ((uint16_t)data[1] << (uint16_t)8);
			this->floatValue = (float)static_cast<int16_t>(tmpValue) / this->divider;
			break;
		}
		case Date:
			this->dateTimeValue.tm_year = data[0] + 2000;
			this->dateTimeValue.tm_mon = data[1];
			this->dateTimeValue.tm_mday = data[2];
			break;
		case Time:
			this->dateTimeValue.tm_hour = data[0];
			this->dateTimeValue.tm_min = data[1];
			this->dateTimeValue.tm_sec = data[2];
			break;
		}
	}

	/// <summary>
	/// Sets a 32 bit value.
	/// </summary>
	/// <param name="value"></param>
	void DataValue::SetValue(int32_t value)
	{
		this->int32Value = value;
	}

	/// <summary>
	/// Creates a string containing the main information of the data value.
	/// </summary>
	/// <returns></returns>
	string DataValue::ToString()
	{
		string tmp(this->name);
		tmp.append(":\t ");
		switch (this->dataType)
		{
		case Word:
			return tmp.append(std::to_string(int16Value));
		case DWord:
			return tmp.append(std::to_string(int32Value));
		case Float:
			return tmp.append(std::to_string(floatValue));
		case Date:
			tmp.append(std::to_string(dateTimeValue.tm_year));
			tmp.append("-");
			tmp.append(std::to_string(dateTimeValue.tm_mon));
			tmp.append("-");
			tmp.append(std::to_string(dateTimeValue.tm_mday));
			tmp.append("-");
			return tmp;
		case Time:
			tmp.append(std::to_string(dateTimeValue.tm_hour));
			tmp.append(":");
			tmp.append(std::to_string(dateTimeValue.tm_min));
			tmp.append(":");
			tmp.append(std::to_string(dateTimeValue.tm_sec));
			tmp.append(":");
			return tmp;
		default:
			return string("");
		}
	}

	/// <summary>
	/// Returns the bytes, containing the data value.
	/// </summary>
	/// <returns>The bytes containing the data value.</returns>
	vector<uint8_t> DataValue::getBytes()
	{
		vector<uint8_t> data;
		switch (this->dataType)
		{
		case Word:
			data.push_back((uint8_t)(this->int16Value >> 8));
			data.push_back((uint8_t)this->int16Value);
			break;
		case DWord:
			data.push_back((uint8_t)(this->int32Value >> 24));
			data.push_back((uint8_t)(this->int32Value >> 16));
			data.push_back((uint8_t)(this->int32Value >> 8));
			data.push_back((uint8_t)this->int32Value);
			break;
		case Date:
			data.push_back((uint8_t)(this->dateTimeValue.tm_year - 2000));
			data.push_back((uint8_t)this->dateTimeValue.tm_mon);
			data.push_back((uint8_t)this->dateTimeValue.tm_mday);
			data.push_back(0x00);
		case Time:
			data.push_back((uint8_t)this->dateTimeValue.tm_hour);
			data.push_back((uint8_t)this->dateTimeValue.tm_min);
			data.push_back((uint8_t)this->dateTimeValue.tm_sec);
			data.push_back(0x00);
		case Float:
			uint8_t byteArray[sizeof(float)];
			memcpy(byteArray, &this->floatValue, sizeof(float));
			data.push_back(byteArray[3]);
			data.push_back(byteArray[2]);
			data.push_back(byteArray[1]);
			data.push_back(byteArray[0]);
		}
		return data;
	}
}