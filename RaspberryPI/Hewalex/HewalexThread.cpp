#include <string.h>
#include <map>
#include <cassert>
#include "HewalexThread.h"
#include <valarray>

namespace GateWay
{
	using namespace std;

	HewalexThread::HewalexThread()
	{
		this->startRegister = 999;
	}

	HewalexThread::~HewalexThread()
	{
	}

	bool HewalexThread::Initialize()
	{
		try
		{
			// Create serial port object and open serial port at 57600 buad, 8 data bits, no parity bit, and one stop bit (8n1)
			SerialPort serialPort("/dev/ttyUSB1", BaudRate::B_38400, NumDataBits::EIGHT, Parity::NONE, NumStopBits::ONE);
			serialPortHewalex = serialPort;
			// Use SerialPort serialPort("/dev/ttyACM0", 13000); instead if you want to provide a custom baud rate
			serialPortHewalex.SetTimeout(1000); // Block when reading until any data is received
			serialPortHewalex.Open();

			this->FillRegisters();

			return true;
		}
		catch (Exception)
		{
			return false;
		}
	}

	void HewalexThread::Read()
	{
		if (this->startRegister > 250)
		{
			this->startRegister = 100;
		}
		else
		{
			this->startRegister += 50;
		}

		vector<uint8_t> command = CreatePacket(this->sourceAddress, this->destinationAddress, this->startRegister, this->amountOfRegisters);

		serialPortHewalex.WriteBinary(command);
		this->UpdateSendedData((uint16_t)command.size());

		// Read some data back (will block until at least 1 byte is received due to the SetTimeout(-1) call above)
		std::string readData;
		std::vector<uint8_t> data;

		serialPortHewalex.ReadBinary(data);
		if (data.size() != 0)
		{
			this->UpdateReceivedData((uint16_t)data.size());
			this->Parse(data);
			data.clear();
		}
		else
		{
			this->timeOuts++;
		}
		//this->PrintRegisters();
		std::this_thread::sleep_for(std::chrono::milliseconds(2000));
	}

	vector<uint8_t> HewalexThread::CreatePacket(uint8_t sender, uint8_t target, uint16_t startRegister, uint8_t amountOfRegisters)
	{
		vector<uint8_t> header;
		vector<uint8_t> payload;
		header.push_back(0x69); // Packet header
		header.push_back(target); // destination address
		header.push_back(sender); // source address
		header.push_back(0x84); // Unknown byte
		header.push_back(0x00); // Unknown byte
		header.push_back(0x00); // Unknown byte
		header.push_back(0x0c); // Payload

		// Calculate the CRC for the header.
		header.push_back(crc8_dvb_s2(header, (uint8_t)header.size()));

		// Start fillin the payload.
		payload.push_back(target);
		payload.push_back(0x00); // Unknown byte
		payload.push_back(sender);
		payload.push_back(0x00); // Unknown byte
		payload.push_back(0x40); // Function ID
		payload.push_back(0x80); // Unknown byte
		payload.push_back(0x00); // Unknown byte
		payload.push_back(amountOfRegisters); // Amount of memory register bytes
		payload.push_back((uint8_t)startRegister);
		payload.push_back(0x00); // Unknown byte

		// Calculate the CRC for the payload.
		uint16_t crcPayload = this->crc16_xmodem(payload, (uint8_t)payload.size());
		payload.push_back((uint8_t)(crcPayload >> 8));
		payload.push_back((uint8_t)crcPayload);

		// Merge the header and payload together.
		header.insert(header.end(), payload.begin(), payload.end());
		return header;
	}

	/// <summary>
	/// CRC16/XMODEM calculation.
	/// Based on https://www.acooke.org/cute/16bitCRCAl0.html
	/// </summary>
	/// <param name="data">The message to calculate the CRC about.</param>
	/// <param name="length">The number of bytes to process.</param>
	/// <returns>The calculated CRC.</returns>
	uint16_t HewalexThread::crc16_xmodem(vector<uint8_t> data, uint8_t length)
	{
		uint16_t polynomial = 0x1021;
		uint16_t crc = 0x00;
		for (uint8_t byte = 0; byte < length; ++byte)
		{
			crc ^= ((data[byte]) << (uint8_t)8);
			for (uint8_t bit = 8; bit > 0; --bit)
			{
				if (crc & 0x8000)
				{
					crc = (crc << (uint8_t)1) ^ polynomial;
				}
				else
				{
					crc = (crc << 1);
				}
			}
		}
		return (crc);
	}

	uint8_t HewalexThread::crc8_dvb_s2(vector<uint8_t> data, uint8_t length)
	{
		uint8_t crc = 0x00;
		for (uint8_t i = 0; i < length; i++)
		{
			crc ^= data[i];
			for (int ii = 0; ii < 8; ++ii) {
				if (crc & 0x80) {
					crc = (crc << 1) ^ (uint8_t)0xD5;
				}
				else {
					crc = crc << 1;
				}
			}
		}
		return crc;
	}

	void HewalexThread::Parse(std::vector<uint8_t> data)
	{
		UpdateReceivedData((uint16_t)data.size());
		if (data[0] != 0x69)
		{
			// Start of packet is 0x69. Anything else is wrong.
			return;
		}

		// Check first the CRC of the header.
		if (crc8_dvb_s2(data, 7) != data[7])
		{
			// The payload CRC is invalid, abort parsing.
			return;
		}

		// Header CRC is correct; check the total length of the package.
		uint8_t payloadBytes = data[6];

		if (payloadBytes + 8 != (uint8_t)data.size())
		{
			// The amount of bytes is invalid. abort parsing.
			return;
		}

		// Creates the payload vector.
		vector<uint8_t> payload;
		for (int i = 0; i < payloadBytes; i++)
		{
			payload.push_back(data[i + 8]);
		}

		// Check the payload CRC
		uint16_t payloadCRC = ((uint16_t)payload[(uint8_t)payload.size() - 1]) + (((uint16_t)payload[payload.size() - 2]) << 8);
		uint16_t calculatedCRC = crc16_xmodem(payload, payloadBytes - 2); // Without the CRC itself.
		if (payloadCRC != calculatedCRC)
		{
			// Invalid CRC of the payload. Abort parsing.
			return;
		}

		// CRC's are ok; continue with parsing

		if (payload[4] == 0x50)
		{
			uint8_t amountRegisters = payload[7];
			uint8_t startRegister = payload[8];
			for (uint16_t g = 0; g < amountRegisters; g = g + 2)
			{
				uint16_t registerNumber = startRegister + g;
				if (this->HasRegister(registerNumber))
				{
					this->UpdateRegister(registerNumber, &payload[g + 10]);
				}
			}
		}
	}

	void HewalexThread::UpdateRegister(uint16_t registerNumber, uint8_t* data)
	{
		for (uint16_t i = 0; i < this->DataList.size(); i++)
		{
			if (this->DataList.at(i).address == registerNumber)
			{
				this->DataList.at(i).SetValue(data);
			}
		}
	}


	void HewalexThread::FillRegisters()
	{
		this->DataList.push_back(DataValue(Date, 120, "Date"));								// Date
		this->DataList.push_back(DataValue(Time, 124, "Time"));								// Time
		this->DataList.push_back(DataValue(Float, 128, "T1"));							// T1 (Collectors temp)
		this->DataList.push_back(DataValue(Float, 130, "T2"));							// T2 (Tank bottom temp)
		this->DataList.push_back(DataValue(Float, 132, "T3"));							// T3 (Air separator temp)
		this->DataList.push_back(DataValue(Float, 134, "T4"));							// T4 (Tank top temp)
		this->DataList.push_back(DataValue(Float, 136, "T5"));							// T5 (Boiler outlet temp)
		this->DataList.push_back(DataValue(Float, 138, "T6"));							// T6
		this->DataList.push_back(DataValue(Word, 144, "Collector Power (W)"));			// Collector Power (W)
		this->DataList.push_back(DataValue(Float, 148, "Consumption", 10));						// Consumption (W)
		this->DataList.push_back(DataValue(Word, 150, "Collector active"));					// Collector Active (True/False)
		this->DataList.push_back(DataValue(Float, 152, "Flow Rate", 10));						// Flow Rate (l/min)
		this->DataList.push_back(DataValue(Word, 154, "Pumps"));						// Collector Pump (P) ON (True/False)
		this->DataList.push_back(DataValue(Word, 156, "CollectorPumpSpeed"));			// Collector Pump Speed (0-15)
		this->DataList.push_back(DataValue(Float, 166, "TotalEnergy", 10));						// Total Energy (kWh)
		this->DataList.push_back(DataValue(Word, 170, "InstallationScheme"));			// Installation Scheme (1-19)
		this->DataList.push_back(DataValue(Word, 172, "DisplayTimeout"));				// Display Timeout (1-10 min)
		this->DataList.push_back(DataValue(Word, 174, "DisplayBrightness"));			// Display Brightness (1-10)
		this->DataList.push_back(DataValue(Word, 176, "AlarmSoundEnabled"));			// Alarm Sound Enabled (True/False)
		this->DataList.push_back(DataValue(Word, 178, "KeySoundEnabled"));				// Key Sound Enabled (True/False)
		this->DataList.push_back(DataValue(Word, 180, "DisplayLanguage"));				// Display Language (0=PL, 1=EN, 2=DE, 3=FR, 4=PT, 5=ES, 6=NL, 7=IT, 8=CZ, 9=SL, ...)
		this->DataList.push_back(DataValue(Float, 182, "FluidFreezingTemp", 10));			// Fluid Freezing Temp
		this->DataList.push_back(DataValue(Float, 186, "FlowRateNominal", 10));					// Flow Rate Nominal (l/min)
		this->DataList.push_back(DataValue(Word, 188, "FlowRateMeasurement"));			// Flow Rate Measurement (0=Rotameter, 1=Electronic G916, 2=Electronic)
		this->DataList.push_back(DataValue(Float, 190, "FlowRateWeight", 100));					// Flow Rate Weight (imp/l)
		this->DataList.push_back(DataValue(Word, 192, "HolidayEnabled"));				// Holiday Enabled (True/False)
		this->DataList.push_back(DataValue(Word, 194, "HolidayStartDay"));				// Holiday Start Day
		this->DataList.push_back(DataValue(Word, 196, "HolidayStartMonth"));			// Holiday Start Month
		this->DataList.push_back(DataValue(Word, 198, "HolidayStartYear"));				// Holiday Start Year
		this->DataList.push_back(DataValue(Word, 200, "HolidayEndDay"));				// Holiday End Day
		this->DataList.push_back(DataValue(Word, 202, "HolidayEndMonth"));				// Holiday End Month
		this->DataList.push_back(DataValue(Word, 204, "HolidayEndYear"));				// Holiday End Year
		this->DataList.push_back(DataValue(Word, 206, "CollectorType"));				// Collector Type (0=Flat, 1=Tube)
		this->DataList.push_back(DataValue(Float, 208, "CollectorPumpHysteresis"));		// Collector Pump Hysteresis (Difference between T1 and T2 to turn on collector pump)
		this->DataList.push_back(DataValue(Float, 210, "ExtraPumpHysteresis"));			// Extra Pump Hysteresis (Temp difference to turn on extra pump)
		this->DataList.push_back(DataValue(Float, 212, "CollectorPumpMaxTemp"));			// Collector Pump Max Temp (Maximum T2 temp to turn off collector pump)
		this->DataList.push_back(DataValue(Word, 214, "BoilerPumpMinTemp"));			// Boiler Pump Min Temp (Minimum T5 temp to turn on boiler pump)
		this->DataList.push_back(DataValue(Word, 218, "HeatSourceMaxTemp"));			// Heat Source Max Temp (Maximum T4 temp to turn off heat sources)
		this->DataList.push_back(DataValue(Word, 220, "BoilerPumpMaxTemp"));			// Boiler Pump Max Temp (Maximum T4 temp to turn off boiler pump)
		this->DataList.push_back(DataValue(Word, 222, "PumpRegulationEnabled"));		// Pump Regulation Enabled (True/False)
		this->DataList.push_back(DataValue(Word, 226, "HeatSourceMaxCollectorPower"));	// Heat Source Max Collector Power (Maximum collector power to turn off heat sources) (100-9900W)
		this->DataList.push_back(DataValue(Word, 228, "CollectorOverheatProtEnabled"));	// Collector Overheat Protection Enabled (True/False)
		this->DataList.push_back(DataValue(Float, 230, "CollectorOverheatProtMaxTemp"));	// Collector Overheat Protection Max Temp (Maximum T2 temp for overheat protection)
		this->DataList.push_back(DataValue(Word, 232, "CollectorFreezingProtEnabled"));	// Collector Freezing Protection Enabled (True/False)
		this->DataList.push_back(DataValue(Word, 234, "HeatingPriority"));				// Heating Priority
		this->DataList.push_back(DataValue(Word, 236, "LegionellaProtEnabled"));		// Legionella Protection Enabled (True/False)
		this->DataList.push_back(DataValue(Word, 238, "LockBoilerKWithBoilerC"));		// Lock Boiler K With Boiler C (True/False)
		this->DataList.push_back(DataValue(Word, 240, "NightCoolingEnabled"));			// Night Cooling Enabled (True/False)
		this->DataList.push_back(DataValue(Float, 242, "NightCoolingStartTemp"));		// Night Cooling Start Temp
		this->DataList.push_back(DataValue(Float, 244, "NightCoolingStopTemp"));			// Night Cooling Stop Temp
		this->DataList.push_back(DataValue(Word, 246, "NightCoolingStopTime"));			// Night Cooling Stop Time (hr)
		this->DataList.push_back(DataValue(DWord, 248, "TimeProgramCM-F"));				// Time Program C M-F (True/False per hour of the day)
		this->DataList.push_back(DataValue(DWord, 252, "TimeProgramCSat"));				// Time Program C Sat (True/False per hour of the day)
		this->DataList.push_back(DataValue(DWord, 256, "TimeProgramCSun"));				// Time Program C Sun (True/False per hour of the day)
		this->DataList.push_back(DataValue(DWord, 260, "TimeProgramKM-F"));				// Time Program K M-F (True/False per hour of the day)
		this->DataList.push_back(DataValue(DWord, 264, "TimeProgramKSat"));				// Time Program K Sat (True/False per hour of the day)
		this->DataList.push_back(DataValue(DWord, 268, "TimeProgramKSun"));				// Time Program K Sun (True/False per hour of the day)
		this->DataList.push_back(DataValue(Word, 278, "CollectorPumpMinRev"));			// Collector Pump Min Rev (rev/min)
		this->DataList.push_back(DataValue(Word, 280, "CollectorPumpMaxRev"));			// Collector Pump Max Rev (rev/min)
		this->DataList.push_back(DataValue(Word, 282, "CollectorPumpMinIncTime"));		// Collector Pump Min Increase Time (s)
		this->DataList.push_back(DataValue(Word, 284, "CollectorPumpMinDecTime"));		// Collector Pump Min Decrease Time (s)
		this->DataList.push_back(DataValue(Word, 286, "CollectorPumpStartupSpeed"));	// Collector Pump Startup Speed (1-15)
		this->DataList.push_back(DataValue(Word, 288, "PressureSwitchEnabled"));		// Pressure Switch Enabled (True/False)
		this->DataList.push_back(DataValue(Word, 290, "TankOverheatProtEnabled"));		// Tank Overheat Protection Enabled (True/False)
		this->DataList.push_back(DataValue(Word, 322, "CirculationPumpEnabled"));		// Circulation Pump Enabled (True/False)
		this->DataList.push_back(DataValue(Word, 324, "CirculationPumpMode"));			// Circulation Pump Mode (0=Discontinuous, 1=Continuous)
		this->DataList.push_back(DataValue(Float, 326, "CirculationPumpMinTemp"));		// Circulation Pump Min Temp (Minimum T4 temp to turn on circulation pump)
		this->DataList.push_back(DataValue(Word, 328, "CirculationPumpONTime"));		// Circulation Pump ON Time (1-59 min)
		this->DataList.push_back(DataValue(Word, 330, "CirculationPumpOFFTime"));		// Circulation Pump OFF Time (1-59 min)

		// Weird DataList
		this->DataList.push_back(DataValue(DWord, 312, "TotalOperationTime"));			// Total Operation Time (min) - lives in config space but is status register
		this->DataList.push_back(DataValue(Word, 320, "Reg320"));						// Unknown register - value changes constantly
	}
}