#include "Intergas.h"
#include <string.h>
#include <cassert>
#include <list>

namespace GateWay
{
	using namespace std;

	/// <summary>
	/// Creates the Intergas thread object.
	/// </summary>
	Intergas::Intergas()
	{
		this->portName = "/dev/ttyUSB0";
		this->baudRate = BaudRate::B_9600;
		this->dataBits = NumDataBits::EIGHT;
		this->parity = Parity::NONE;
		this->stopBits = NumStopBits::ONE;
	}

	/// <summary>
	/// Reads the Intergas module.
	/// </summary>
	void Intergas::Read()
	{
		MessageType messageType = GetMessageType();

		std::string command;
		switch (messageType)
		{
		case NONE:
			command = "";
			break;
		case PARAMETERS:
			command = "V?\r";
			break;
		case PARAMETERSEXT:
			command = "V1\r";
			break;
		case DATA:
			command = "S?\r";
			break;
		case STATISTICS:
			command = "HN\r";
			break;
		case REVISION:
			command = "REV";
			break;
		case PRODUCTIONCODE:
			command = "B?\r";
			break;
		case ERROR_CODES:
			command = "EN\r";
			break;
		}

		// Write some ASCII data
		this->UpdateSendedData((uint16_t)command.size());

		this->serialPort.Write(command);

		// Read the data.
		std::string readData;
		std::vector<uint8_t> data;

		this->serialPort.ReadBinary(data);
		if (data.size() == 0)
		{
			timeOuts++;
		}
		else
		{
			this->UpdateReceivedData((uint16_t)data.size());
		}

		if (data.size() == 32)
		{
			Parse(data, messageType);
			PrintValues();
		}
		else
		{
			std::cout << "Received incorrect message" << std::endl;
		}

		// Wait one second for next request.
		this->Sleep(1000);
	}

	/// <summary>
	/// Parses the received data.
	/// </summary>
	/// <param name="data">The data.</param>
	/// <param name="messageType">The message type of the received data.</param>
	void Intergas::Parse(std::vector<uint8_t> data, MessageType messageType)
	{
		if (data.size() == 32)
		{
			std::string str(data.begin(), data.end());

			switch (messageType)
			{
			case NONE:
				break;

			case DATA:
				messageNumberLastData = messagesReceived;

				this->GetRegister(aTempExhaust).SetValue(ConvertBytesToFloat(data[0], data[1]));
				this->GetRegister(aTempSource).SetValue(ConvertBytesToFloat(data[2], data[3]));
				this->GetRegister(aTempReturn).SetValue(ConvertBytesToFloat(data[4], data[5]));
				this->GetRegister(aTempHotWater).SetValue(ConvertBytesToFloat(data[6], data[7]));
				this->GetRegister(aTempBoiler).SetValue(ConvertBytesToFloat(data[8], data[9]));
				this->GetRegister(aTempOutside).SetValue(ConvertBytesToFloat(data[10], data[11]));
				this->GetRegister(aPressure).SetValue(ConvertBytesToFloat(data[12], data[13]));
				this->GetRegister(aTempSetpoint).SetValue(ConvertBytesToFloat(data[14], data[15]));
				this->GetRegister(aFanSpeedSetpoint).SetValue(ConvertBytesToFloat(data[16], data[17]));
				this->GetRegister(aFanSpeed).SetValue(ConvertBytesToFloat(data[18], data[19]));
				this->GetRegister(aFanPwm).SetValue(ConvertBytesToFloat(data[20], data[21]));
				this->GetRegister(aIoCurrent).SetValue(ConvertBytesToFloat(data[22], data[23]));
				this->GetRegister(aStatusCode).SetValue(data[24]);
				this->GetRegister(aStatusByte1).SetValue(data[26]);
				this->GetRegister(aStatusByte2).SetValue(data[28]);
				break;

			case REVISION:

				messageNumberLastRevision = messagesReceived;

				hardwareRelease = str.substr(0, 18);
				softwareRelease = str.substr(18, 5);
				break;

			case STATISTICS:
				messageNumberLastStatistics = messagesReceived;

				this->GetRegister(aLinePowerConnectedHours).SetValue(ConvertBytesToUint16(data[0], data[1]));
				this->GetRegister(aLinePowerConnectedTimes).SetValue(ConvertBytesToUint16(data[2], data[3]));
				this->GetRegister(aHeatingHours).SetValue(ConvertBytesToUint16(data[4], data[5]));
				this->GetRegister(aHotWaterHours).SetValue(ConvertBytesToUint16(data[6], data[7]));
				this->GetRegister(aBurnerStarts).SetValue(ConvertBytesToUint16(data[8], data[9]));
				this->GetRegister(aIgnitionFailed).SetValue(ConvertBytesToUint16(data[10], data[11]));
				this->GetRegister(aFlameLost).SetValue(ConvertBytesToUint16(data[12], data[13]));
				this->GetRegister(aResets).SetValue(ConvertBytesToUint16(data[14], data[15]));
				this->GetRegister(aGasmeterHeating).SetValue((int32_t)ConvertBytesToUint32(data[16], data[17], data[18], data[19]));
				this->GetRegister(aGasmeterHotWater).SetValue((int32_t)ConvertBytesToUint32(data[20], data[21], data[22], data[23]));
				this->GetRegister(aWaterMeter).SetValue((int32_t)ConvertBytesToUint32(data[25], data[26], data[29], 0x00));
				this->GetRegister(aBurnerstartsHeating).SetValue(ConvertBytesToUint16(data[27], data[28]));
				break;

			case PARAMETERS:
				messageNumberLastParameters = messagesReceived;
				this->GetRegister(aHeaterOn).SetValue(data[0]);
				this->GetRegister(aComfortMode).SetValue(data[1]);
				this->GetRegister(aCentralHeatingMaxSp).SetValue(data[2]);
				this->GetRegister(aHotWaterSetpoint).SetValue(data[3]);
				this->GetRegister(aEcoDays).SetValue(data[4]);
				this->GetRegister(aComfortSetpoint).SetValue(data[5]);
				this->GetRegister(aHotWaterNight).SetValue(data[6]);
				this->GetRegister(aCentralHeatingNight).SetValue(data[7]);
				this->GetRegister(aParameter1).SetValue(data[8]);
				this->GetRegister(aParameter2).SetValue(data[9]);
				this->GetRegister(aParameter3).SetValue(data[10]);
				this->GetRegister(aParameter4).SetValue(data[11]);
				this->GetRegister(aParameter5).SetValue(data[12]);
				this->GetRegister(aParameter6).SetValue(data[13]);
				this->GetRegister(aParameter7).SetValue(data[14]);
				this->GetRegister(aParameter8).SetValue(data[15]);
				this->GetRegister(aParameter9).SetValue(data[16]);
				this->GetRegister(aParameterA).SetValue(data[17]);
				this->GetRegister(aParameterb).SetValue(data[18]);
				this->GetRegister(aParameterC).SetValue(data[19]);
				this->GetRegister(aParameterc).SetValue(data[20]);
				this->GetRegister(aParameterd).SetValue(data[21]);
				this->GetRegister(aParameterE).SetValue(data[22]);
				this->GetRegister(aParameterEdot).SetValue(data[23]);
				this->GetRegister(aParameterF).SetValue(data[24]);
				this->GetRegister(aParameterh).SetValue(data[25]);
				this->GetRegister(aParametern).SetValue(data[26]);
				this->GetRegister(aParametero).SetValue(data[27]);
				this->GetRegister(aParameterP).SetValue(data[28]);
				this->GetRegister(aParameterr).SetValue(data[29]);
				this->GetRegister(aParameterFdot).SetValue(data[30]);
				break;

			case PARAMETERSEXT:
				messageNumberLastParametersExt = messagesReceived;
				this->GetRegister(aBetaCentralHeatingFlow).SetValue(data[0]);
				this->GetRegister(aParameterOdot).SetValue(data[1]);
				this->GetRegister(aParameter5dot).SetValue(data[3]);
				this->GetRegister(aParametercdot).SetValue(data[4]);
				this->GetRegister(aParameter3dot).SetValue(data[5]);
				this->GetRegister(aParameterPdot).SetValue(data[6]);
				this->GetRegister(aParameterq).SetValue(data[7]);
				this->GetRegister(aParameterL).SetValue(data[8]);
				break;

			case ERROR_CODES:
				messageNumberLastErrorCodes = messagesReceived;
				this->GetRegister(aFault0Times).SetValue(data[0]);
				this->GetRegister(aFault1Times).SetValue(data[1]);
				this->GetRegister(aFault2Times).SetValue(data[2]);
				this->GetRegister(aFault3Times).SetValue(data[3]);
				this->GetRegister(aFault4Times).SetValue(data[4]);
				this->GetRegister(aFault5Times).SetValue(data[5]);
				this->GetRegister(aFault6Times).SetValue(data[6]);
				this->GetRegister(aFault7Times).SetValue(data[7]);
				this->GetRegister(aFault8Times).SetValue(data[8]);
				this->GetRegister(aFault9Times).SetValue(data[9]);
				this->GetRegister(aFault10Times).SetValue(data[10]);
				this->GetRegister(aFault11Times).SetValue(data[11]);
				this->GetRegister(aFault12Times).SetValue(data[12]);
				this->GetRegister(aFault13Times).SetValue(data[13]);
				this->GetRegister(aFault14Times).SetValue(data[14]);
				this->GetRegister(aFault15Times).SetValue(data[15]);
				this->GetRegister(aFault16Times).SetValue(data[16]);
				this->GetRegister(aFault17Times).SetValue(data[17]);
				this->GetRegister(aFault18Times).SetValue(data[18]);
				this->GetRegister(aFault19Times).SetValue(data[19]);
				this->GetRegister(aFault20Times).SetValue(data[20]);
				this->GetRegister(aFault21Times).SetValue(data[21]);
				this->GetRegister(aFault22Times).SetValue(data[22]);
				this->GetRegister(aFault23Times).SetValue(data[23]);
				this->GetRegister(aFault24Times).SetValue(data[24]);
				this->GetRegister(aFault25Times).SetValue(data[25]);
				this->GetRegister(aFault26Times).SetValue(data[26]);
				this->GetRegister(aFault27Times).SetValue(data[27]);
				this->GetRegister(aFault28Times).SetValue(data[28]);
				this->GetRegister(aFault29Times).SetValue(data[29]);
				this->GetRegister(aFault30Times).SetValue(data[30]);
				this->GetRegister(aFault31Times).SetValue(data[31]);
				break;

			case PRODUCTIONCODE:
				messageNumberLastProductionCode = messagesReceived;
				break;
			}
		}
		else
		{
			cout << "Invalid bytes received: " << data.size() << endl;
		}
	}

	/// <summary>
	/// Converts given bytes to an 32 bit value.
	/// </summary>
	/// <param name="a">Byte a.</param>
	/// <param name="b">Byte b.</param>
	/// <param name="c">Byte c.</param>
	/// <param name="d">Byte d.</param>
	/// <returns>The 32 bit value.</returns>
	uint32_t Intergas::ConvertBytesToUint32(uint8_t a, uint8_t b, uint8_t c, uint8_t d)
	{
		// UINT32 Little Endian (DCBA)
		return ((uint32_t)d << 24) + ((uint32_t)c << 16) + ((uint32_t)b << 8) + ((uint32_t)a);
	}

	/// <summary>
	/// Convert the bytes to an uint16 value.
	/// </summary>
	/// <param name="msb">The most significant byte.</param>
	/// <param name="lsb">The least significant byte.</param>
	/// <returns></returns>
	uint16_t Intergas::ConvertBytesToUint16(uint8_t msb, uint8_t lsb)
	{
		return ((uint16_t)lsb << 8) + msb;
	}

	/// <summary>
	/// Converts the given bytes to a float value.
	/// </summary>
	/// <param name="lsb"></param>
	/// <param name="msb"></param>
	/// <returns></returns>
	float Intergas::ConvertBytesToFloat(uint8_t lsb, uint8_t msb)
	{
		float f;
		if (msb > 127)
		{
			f = -(float(msb ^ 255) + 1) * 256 - lsb / 100;
		}
		else
		{
			f = float(msb * 265 + lsb) / 100;
		}
		return f;
	}

	/// <summary>
	/// Gets the message type to send.
	/// </summary>
	/// <returns></returns>
	MessageType Intergas::GetMessageType()
	{
		if (((messagesSended - messageNumberLastRevision) > intervalRevision) || messageNumberLastRevision == 0)
		{
			return REVISION;
		}
		if (((messagesSended - messageNumberLastStatistics) > intervalCounters) || messageNumberLastStatistics == 0)
		{
			return STATISTICS;
		}
		else if (((messagesSended - messageNumberLastParameters) > intervalParameters) || messageNumberLastParameters == 0)
		{
			return PARAMETERS;
		}
		else if (((messagesSended - messageNumberLastParametersExt) > intervalExtParameters) || messageNumberLastParametersExt == 0)
		{
			return PARAMETERSEXT;
		}
		else if (((messagesSended - messageNumberLastProductionCode) > intervalProductionCode) || messageNumberLastProductionCode == 0)
		{
			return PRODUCTIONCODE;
		}
		else if (((messagesSended - messageNumberLastErrorCodes) > intervalErrorCodes) || messageNumberLastErrorCodes == 0)
		{
			return ERROR_CODES;
		}
		return DATA;
	}

	/// <summary>
	/// Creates the data values.
	/// As the Intergas communication does not work with register numbers, we
	/// make the regiseter numbers our selves.
	/// </summary>
	void Intergas::CreateDatalist()
	{
		// The data
		this->DataList.push_back(DataValue(Float, aTempExhaust, "Exhaust temperature"));
		this->DataList.push_back(DataValue(Float, aTempSource, "Source temperature"));
		this->DataList.push_back(DataValue(Float, aTempReturn, "Return temperature"));
		this->DataList.push_back(DataValue(Float, aTempHotWater, "Hot water temperature"));
		this->DataList.push_back(DataValue(Float, aTempBoiler, "Boiler temperature"));
		this->DataList.push_back(DataValue(Float, aTempOutside, "Outside temperature"));
		this->DataList.push_back(DataValue(Float, aPressure, "Pressure"));
		this->DataList.push_back(DataValue(Float, aTempSetpoint, "Temperature setpoint"));
		this->DataList.push_back(DataValue(Float, aFanSpeedSetpoint, "Fan speed setpoint"));
		this->DataList.push_back(DataValue(Float, aFanSpeed, "Fan speed"));
		this->DataList.push_back(DataValue(Float, aFanPwm, "Fan PWM"));
		this->DataList.push_back(DataValue(Float, aIoCurrent, "Ionisation current"));
		this->DataList.push_back(DataValue(Word, aStatusCode, "Status code"));
		this->DataList.push_back(DataValue(Word, aStatusByte1, "Status byte 1"));
		this->DataList.push_back(DataValue(Word, aStatusByte2, "Status byte 2"));

		// The revision part.

		// The statistics.
		this->DataList.push_back(DataValue(DWord, aLinePowerConnectedHours, "Line power connected"));
		this->DataList.push_back(DataValue(DWord, aLinePowerConnectedTimes, "Power connected times"));
		this->DataList.push_back(DataValue(DWord, aHeatingHours, "Heating hours"));
		this->DataList.push_back(DataValue(DWord, aHotWaterHours, "Hot water hours"));
		this->DataList.push_back(DataValue(DWord, aBurnerStarts, "Burner starts"));
		this->DataList.push_back(DataValue(Word, aIgnitionFailed, "Ignition failed"));
		this->DataList.push_back(DataValue(Word, aFlameLost, "Flame losts"));
		this->DataList.push_back(DataValue(Word, aResets, "Resets"));
		this->DataList.push_back(DataValue(DWord, aGasmeterHeating, "Gas meter heating"));
		this->DataList.push_back(DataValue(DWord, aGasmeterHotWater, "Gas meter hot water"));
		this->DataList.push_back(DataValue(DWord, aWaterMeter, "Water meter"));
		this->DataList.push_back(DataValue(DWord, aBurnerstartsHeating, "Burner starts heating"));

		// The parameters
		this->DataList.push_back(DataValue(Word, aHeaterOn, "Heater on"));
		this->DataList.push_back(DataValue(Word, aComfortMode, "Comfort mode"));
		this->DataList.push_back(DataValue(Word, aCentralHeatingMaxSp, "Central heating setpoint"));
		this->DataList.push_back(DataValue(Word, aHotWaterSetpoint, "Hot water setpoint"));
		this->DataList.push_back(DataValue(Word, aEcoDays, "Eco days"));
		this->DataList.push_back(DataValue(Word, aComfortSetpoint, "Comfort setpoint"));
		this->DataList.push_back(DataValue(Word, aHotWaterNight, "Hot water night"));
		this->DataList.push_back(DataValue(Word, aCentralHeatingNight, "Central heating night"));
		this->DataList.push_back(DataValue(Word, aParameter1, "Parameter 1"));
		this->DataList.push_back(DataValue(Word, aParameter2, "Parameter 2"));
		this->DataList.push_back(DataValue(Word, aParameter3, "Parameter 3"));
		this->DataList.push_back(DataValue(Word, aParameter4, "Parameter 4"));
		this->DataList.push_back(DataValue(Word, aParameter5, "Parameter 5"));
		this->DataList.push_back(DataValue(Word, aParameter6, "Parameter 6"));
		this->DataList.push_back(DataValue(Word, aParameter7, "Parameter 7"));
		this->DataList.push_back(DataValue(Word, aParameter8, "Parameter 8"));
		this->DataList.push_back(DataValue(Word, aParameter9, "Parameter 9"));
		this->DataList.push_back(DataValue(Word, aParameterA, "Parameter A"));
		this->DataList.push_back(DataValue(Word, aParameterb, "Parameter b"));
		this->DataList.push_back(DataValue(Word, aParameterC, "Parameter C"));
		this->DataList.push_back(DataValue(Word, aParameterc, "Parameter c"));
		this->DataList.push_back(DataValue(Word, aParameterd, "Parameter d"));
		this->DataList.push_back(DataValue(Word, aParameterE, "Parameter E"));
		this->DataList.push_back(DataValue(Word, aParameterEdot, "Parameter E."));
		this->DataList.push_back(DataValue(Word, aParameterF, "Parameter F"));
		this->DataList.push_back(DataValue(Word, aParameterh, "Parameter h"));
		this->DataList.push_back(DataValue(Word, aParametern, "Parameter n"));
		this->DataList.push_back(DataValue(Word, aParametero, "Parameter o"));
		this->DataList.push_back(DataValue(Word, aParameterP, "Parameter P"));
		this->DataList.push_back(DataValue(Word, aParameterr, "Parameter r"));
		this->DataList.push_back(DataValue(Word, aParameterFdot, "Parameter F."));
		this->DataList.push_back(DataValue(Word, aBetaCentralHeatingFlow, "Parameter heating flow"));
		this->DataList.push_back(DataValue(Word, aParameterOdot, "Parameter O."));
		this->DataList.push_back(DataValue(Word, aCascadeReaction, "Cascade reaction"));
		this->DataList.push_back(DataValue(Word, aParameter5dot, "Parameter 5."));
		this->DataList.push_back(DataValue(Word, aParametercdot, "Parameter c."));
		this->DataList.push_back(DataValue(Word, aParameter3dot, "Parameter 3."));
		this->DataList.push_back(DataValue(Word, aParameterPdot, "Parameter P."));
		this->DataList.push_back(DataValue(Word, aParameterq, "Parameter q"));
		this->DataList.push_back(DataValue(Word, aParameterL, "Parameter L"));

		// The fault list
		this->DataList.push_back(DataValue(Word, aFault0Times, "Fault 1 times"));
		this->DataList.push_back(DataValue(Word, aFault1Times, "Fault 1 times"));
		this->DataList.push_back(DataValue(Word, aFault2Times, "Fault 2 times"));
		this->DataList.push_back(DataValue(Word, aFault3Times, "Fault 3 times"));
		this->DataList.push_back(DataValue(Word, aFault4Times, "Fault 4 times"));
		this->DataList.push_back(DataValue(Word, aFault5Times, "Fault 5 times"));
		this->DataList.push_back(DataValue(Word, aFault6Times, "Fault 6 times"));
		this->DataList.push_back(DataValue(Word, aFault7Times, "Fault 7 times"));
		this->DataList.push_back(DataValue(Word, aFault8Times, "Fault 8 times"));
		this->DataList.push_back(DataValue(Word, aFault9Times, "Fault 9 times"));
		this->DataList.push_back(DataValue(Word, aFault10Times, "Fault 10 times"));
		this->DataList.push_back(DataValue(Word, aFault11Times, "Fault 11 times"));
		this->DataList.push_back(DataValue(Word, aFault12Times, "Fault 12 times"));
		this->DataList.push_back(DataValue(Word, aFault13Times, "Fault 13 times"));
		this->DataList.push_back(DataValue(Word, aFault14Times, "Fault 14 times"));
		this->DataList.push_back(DataValue(Word, aFault15Times, "Fault 15 times"));
		this->DataList.push_back(DataValue(Word, aFault16Times, "Fault 16 times"));
		this->DataList.push_back(DataValue(Word, aFault17Times, "Fault 17 times"));
		this->DataList.push_back(DataValue(Word, aFault18Times, "Fault 18 times"));
		this->DataList.push_back(DataValue(Word, aFault19Times, "Fault 19 times"));
		this->DataList.push_back(DataValue(Word, aFault20Times, "Fault 20 times"));
		this->DataList.push_back(DataValue(Word, aFault21Times, "Fault 21 times"));
		this->DataList.push_back(DataValue(Word, aFault22Times, "Fault 22 times"));
		this->DataList.push_back(DataValue(Word, aFault23Times, "Fault 23 times"));
		this->DataList.push_back(DataValue(Word, aFault24Times, "Fault 24 times"));
		this->DataList.push_back(DataValue(Word, aFault25Times, "Fault 25 times"));
		this->DataList.push_back(DataValue(Word, aFault26Times, "Fault 26 times"));
		this->DataList.push_back(DataValue(Word, aFault27Times, "Fault 27 times"));
		this->DataList.push_back(DataValue(Word, aFault28Times, "Fault 28 times"));
		this->DataList.push_back(DataValue(Word, aFault29Times, "Fault 29 times"));
		this->DataList.push_back(DataValue(Word, aFault30Times, "Fault 30 times"));
		this->DataList.push_back(DataValue(Word, aFault31Times, "Fault 31 times"));
	}

	/// <summary>
	/// The destructor.
	/// </summary>
	Intergas::~Intergas()
	{
		// Nothing to destruct.
		// Serial port is closed in base class.
	}
}