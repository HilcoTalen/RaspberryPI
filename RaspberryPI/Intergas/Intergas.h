#ifndef INTERGASTHREAD_H
#define INTERGASTHREAD_H

#include <thread>
#include <string>
#include "SerialPort.h"
#include <DataValue.h>
#include <CommunicationStats.h>
namespace GateWay
{
	using namespace std;

	enum MessageType
	{
		NONE,
		DATA,
		PARAMETERS,
		PARAMETERSEXT,
		REVISION,
		ERROR_CODES,
		STATISTICS,
		PRODUCTIONCODE,
	};

	class Intergas : public CommunicationStats
	{
	public:

		Intergas();
		void Read();
		void CreateDatalist();
		~Intergas();

	private:
		int32_t messageNumberLastData = 0;
		int32_t messageNumberLastParameters = 0;
		int32_t messageNumberLastParametersExt = 0;
		int32_t messageNumberLastErrorCodes = 0;
		int32_t messageNumberLastStatistics = 0;
		int32_t messageNumberLastProductionCode = 0;
		int32_t messageNumberLastRevision = 0;

		static const int intervalRevision = 250;
		static const int intervalData = 1;
		static const int intervalParameters = 50;
		static const int intervalExtParameters = 50;
		static const int intervalErrorCodes = 25;
		static const int intervalCounters = 30;
		static const int intervalProductionCode = 250;

		void Parse(std::vector<uint8_t> data, MessageType messageType);
		float ConvertBytesToFloat(uint8_t lsb, uint8_t msb);
		uint16_t ConvertBytesToUint16(uint8_t lsb, uint8_t msb);
		uint32_t ConvertBytesToUint32(uint8_t a, uint8_t b, uint8_t c, uint8_t d);
		MessageType GetMessageType();

		string hardwareRelease;
		string softwareRelease;

		// Addresses
		// as the Intergas data has no address (it's just a block of 32 bytes of data),
		// so we make our own addresses.

		// The data part.
		const uint8_t aTempExhaust = 2;
		const uint8_t aTempSource = 4;
		const uint8_t aTempReturn = 6;
		const uint8_t aTempHotWater = 8;
		const uint8_t aTempBoiler = 10;
		const uint8_t aTempOutside = 12;
		const uint8_t aPressure = 14;
		const uint8_t aTempSetpoint = 16;
		const uint8_t aFanSpeedSetpoint = 18;
		const uint8_t aFanSpeed = 20;
		const uint8_t aFanPwm = 22;
		const uint8_t aIoCurrent = 24;
		const uint8_t aStatusCode = 26;
		const uint8_t aStatusByte1 = 28;
		const uint8_t aStatusByte2 = 30;

		// The revision
		const uint8_t aHardwareVersion = 32;
		const uint8_t aSoftwareVersion = 34;

		// The statistics
		const uint8_t aLinePowerConnectedHours = 36;
		const uint8_t aLinePowerConnectedTimes = 38;
		const uint8_t aHeatingHours = 40;
		const uint8_t aHotWaterHours = 42;
		const uint8_t aBurnerStarts = 44;
		const uint8_t aIgnitionFailed = 46;
		const uint8_t aFlameLost = 48;
		const uint8_t aResets = 50;
		const uint8_t aGasmeterHeating = 52;
		const uint8_t aGasmeterHotWater = 54;
		const uint8_t aWaterMeter = 56;
		const uint8_t aBurnerstartsHeating = 58;

		// The parameters
		const uint8_t aHeaterOn = 60;
		const uint8_t aComfortMode = 61;
		const uint8_t aCentralHeatingMaxSp = 62;
		const uint8_t aHotWaterSetpoint = 63;
		const uint8_t aEcoDays = 64;
		const uint8_t aComfortSetpoint = 65;
		const uint8_t aHotWaterNight = 66;
		const uint8_t aCentralHeatingNight = 67;
		const uint8_t aParameter1 = 68;
		const uint8_t aParameter2 = 69;
		const uint8_t aParameter3 = 70;
		const uint8_t aParameter4 = 71;
		const uint8_t aParameter5 = 72;
		const uint8_t aParameter6 = 73;
		const uint8_t aParameter7 = 74;
		const uint8_t aParameter8 = 75;
		const uint8_t aParameter9 = 76;
		const uint8_t aParameterA = 77;
		const uint8_t aParameterb = 78;
		const uint8_t aParameterC = 79;
		const uint8_t aParameterc = 80;
		const uint8_t aParameterd = 81;
		const uint8_t aParameterE = 82;
		const uint8_t aParameterEdot = 83;
		const uint8_t aParameterF = 84;
		const uint8_t aParameterh = 85;
		const uint8_t aParametern = 86;
		const uint8_t aParametero = 87;
		const uint8_t aParameterP = 88;
		const uint8_t aParameterr = 89;
		const uint8_t aParameterFdot = 90;
		const uint8_t aBetaCentralHeatingFlow = 91;
		const uint8_t aParameterOdot = 92;
		const uint8_t aCascadeReaction = 93;
		const uint8_t aParameter5dot = 94;
		const uint8_t aParametercdot = 95;
		const uint8_t aParameter3dot = 96;
		const uint8_t aParameterPdot = 97;
		const uint8_t aParameterq = 98;
		const uint8_t aParameterL = 99;

		// The fault codes
		const uint8_t aFault0Times = 100;
		const uint8_t aFault1Times = 101;
		const uint8_t aFault2Times = 102;
		const uint8_t aFault3Times = 103;
		const uint8_t aFault4Times = 104;
		const uint8_t aFault5Times = 105;
		const uint8_t aFault6Times = 106;
		const uint8_t aFault7Times = 107;
		const uint8_t aFault8Times = 108;
		const uint8_t aFault9Times = 109;
		const uint8_t aFault10Times = 110;
		const uint8_t aFault11Times = 111;
		const uint8_t aFault12Times = 112;
		const uint8_t aFault13Times = 113;
		const uint8_t aFault14Times = 114;
		const uint8_t aFault15Times = 115;
		const uint8_t aFault16Times = 116;
		const uint8_t aFault17Times = 117;
		const uint8_t aFault18Times = 118;
		const uint8_t aFault19Times = 119;
		const uint8_t aFault20Times = 120;
		const uint8_t aFault21Times = 121;
		const uint8_t aFault22Times = 122;
		const uint8_t aFault23Times = 123;
		const uint8_t aFault24Times = 124;
		const uint8_t aFault25Times = 125;
		const uint8_t aFault26Times = 126;
		const uint8_t aFault27Times = 127;
		const uint8_t aFault28Times = 128;
		const uint8_t aFault29Times = 129;
		const uint8_t aFault30Times = 130;
		const uint8_t aFault31Times = 131;
	};
}
#endif
