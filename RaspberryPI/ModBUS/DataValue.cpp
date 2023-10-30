#include "DataValue.h"
#include <string.h>
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>

namespace GateWay
{
	using namespace std;

	/// Fill the given string with spaces, until the string has the given length.
	/// <param name="str">The string to fill.</param>
	/// <param name="length">The length of the string.</param>
	void DataValue::Fill(string& str, int length)
	{
		while (str.length() < length)
		{
			str.append(" ");
		}
	}

	// Create a tm structure initialized to the epoch date (January 1, 1970, 00:00:00 UTC)
	std::tm epoch_time = { 0, 0, 0, 1, 0, 70, 4, 0, 0 };

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
		this->Fill(this->name, 40);
		this->floatValue = 0.0f;
		this->int16Value = 0;
		this->int32Value = 0;
		this->divider = 1;

		// Initialize the time stamp (Unix timestamp)
		this->dateTimeValue = epoch_time;
		this->readOuts = 0;
		this->offlineReadOuts = 0;
		this->readTime = 0;
		this->updateTime = 0;
		this->readOutsModBUS = 0;
	}

	/// <summary>
	/// Initiates a new data value, including the divider.
	/// </summary>
	/// <param name="dataType">The data value type.</param>
	/// <param name="address">The data value address.</param>
	/// <param name="name">The data value name. (Only used for printing).</param>
	/// <param name="divider">The data value divider.</param>
	DataValue::DataValue(DataType dataType, uint16_t address, string name, float divider)
	{
		this->dataType = dataType;
		this->address = address;
		this->name = name;
		this->Fill(this->name, 40);
		this->divider = divider;
		this->floatValue = 0.0f;
		this->int16Value = 0;
		this->int32Value = 0;
		this->readOuts = 0;
		this->offlineReadOuts = 0;

		// INitialize the time stamp. (Unix timestamp)
		this->dateTimeValue = epoch_time;
		this->readTime = 0;
		this->updateTime = 0;
		this->readOutsModBUS = 0;
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
		this->updateTime = source.updateTime;
		this->readTime = source.readTime;
		this->readOuts = source.readOuts;
		this->DataReceived = source.DataReceived;
		this->dateTimeValue = source.dateTimeValue;
		this->readOutsModBUS = source.readOutsModBUS;
		this->offlineReadOuts = source.offlineReadOuts;
	}

	/// <summary>
	/// Gets the name of the data value.
	/// </summary>
	/// <returns></returns>
	string DataValue::GetName()
	{
		return this->name;
	}

	/// <summary>
	/// Sets a float value.
	/// </summary>
	/// <param name="value">The new value.</param>
	void DataValue::SetValue(float value)
	{
		this->floatValue = value / this->divider;
		this->UpdateLatestUpdateTime();
	}

	/// <summary>
	/// Gets the divider.
	/// </summary>
	/// <returns>The divider.</returns>
	float DataValue::GetDivider()
	{
		return this->divider;
	}

	/// <summary>
	/// Sets a 8 bit value.
	/// </summary>
	/// <param name="value">The new value.</param>
	void DataValue::SetValue(uint8_t value)
	{
		this->int16Value = static_cast<int8_t>(value);
		this->UpdateLatestUpdateTime();
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
		this->UpdateLatestUpdateTime();
	}

	/// <summary>
	/// Sets a 32 bit value.
	/// </summary>
	/// <param name="value"></param>
	void DataValue::SetValue(int32_t value)
	{
		this->int32Value = value;
		this->UpdateLatestUpdateTime();
	}

	/// <summary>
	/// Gets the latest update time, of the value in format YYYY-MM-DD hh:mm
	/// </summary>
	/// <returns></returns>
	string DataValue::GetLatestUpdateTime()
	{
		// convert now to string form, and remove the trailing newline
		char* dateTimeLocal = ctime(&updateTime);
		if (dateTimeLocal[strlen(dateTimeLocal) - 1] == '\n')
		{
			dateTimeLocal[strlen(dateTimeLocal) - 1] = '\0';
		}
		string tmp(dateTimeLocal);

		return tmp;
	}

	string DataValue::GetLatestReadTime()
	{
		// convert now to string form and remove the trailing newline
		char* dateTimeLocal = ctime(&readTime);
		if (dateTimeLocal[strlen(dateTimeLocal) - 1] == '\n')
		{
			dateTimeLocal[strlen(dateTimeLocal) - 1] = '\0';
		}
		string tmp(dateTimeLocal);
		return tmp;
	}

	/// <summary>
	/// Creates a string containing the main information of the data value.
	/// </summary>
	/// <returns></returns>
	string DataValue::ToString()
	{
		string tmp(this->name);
		this->Fill(tmp, 30);
		tmp.append(":\t ");

		string val;
		switch (this->dataType)
		{
		case Word:
			val = std::to_string(int16Value);
			break;
		case DWord:
			val = std::to_string(int32Value);
			break;
		case Float:
		{
			std::ostringstream ss{};
			ss << std::fixed << std::setprecision(2) << floatValue;
			val = ss.str();
			break;
		}
		case Date:
			val = std::to_string(dateTimeValue.tm_year);
			val.append("-");
			val.append(std::to_string(dateTimeValue.tm_mon));
			val.append("-");
			val.append(std::to_string(dateTimeValue.tm_mday));
			break;
		case Time:
			val = std::to_string(dateTimeValue.tm_hour);
			val.append(":");
			val.append(std::to_string(dateTimeValue.tm_min));
			val.append(":");
			val.append(std::to_string(dateTimeValue.tm_sec));
			break;
		}

		this->Fill(val, 10);
		tmp.append(val);
		return tmp;
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

		// Update the latest read time.
		this->readTime = time(0);

		this->readOutsModBUS++;
		return data;
	}

	/// <summary>
	/// Updates the latest update time, of the data value.
	/// </summary>
	void DataValue::UpdateLatestUpdateTime()
	{
		this->DataReceived = true;
		this->readOuts++;
		this->updateTime = time(0);
	}
}