#include "P1.h"
#include <string.h>

namespace GateWay
{
	P1::P1()
	{
		this->Name = "P1";
		this->baudRate = BaudRate::B_115200;
		this->dataBits = NumDataBits::EIGHT;
		this->parity = Parity::NONE;
		this->stopBits = NumStopBits::ONE;
		this->timeOut = 2000;

		// Set the maximum timeouts before the device is marked as offline.
		// to 50times, which is 2 * 30 seconds is one minute. (we have missed already 6 telegrams).
		this->maximumTimeOuts = 2;


		// Set the maximum timeouts to 30, as the P1 port is readed every second,
		// but the P1 port is only updated every 10 seconds.
		this->timeOutsConsecutive = 30;
		this->timeOutConsecutive = 0;
	}

	P1::~P1()
	{
		// Nothing to do here.
	}

	void P1::Read()
	{
		string newData;
		this->serialPort.Read(newData);

		if (newData.length() > 0)
		{
			this->data += newData;
			int len = this->data.length();
			if (len > 500)
			{
				// We have a valid telegram. Parse this.
				if (len > P1_MAXLINELENGTH)
				{
					std::cout << "P1 - Telegram too long. Resetting data." << std::endl;
					this->data = "";
					this->serialPort.ClearBuffer();
					return;
				}

				// Copy the data (string) to char array
				strncpy(telegram, this->data.c_str(), len);
				decode_telegram(len);
				this->data = "";
				//this->PrintValues();
				this->StoreValues();
			}
		}

		else
		{
			this->timeOutConsecutive++;
			if (this->timeOutConsecutive > this->timeOutsConsecutive)
			{
				this->UpdateTimeout(false);
				timeOutConsecutive = 0;
			}
		}

		this->Sleep(1000);
	}

	void P1::CreateDatalist()
	{
		this->DataList.push_back(DataValue(Float, aConsumedLowTariff, "Consumed low tariff"));
		this->DataList.push_back(DataValue(Float, aConsumedHighTariff, "Consumed high tariff"));
		this->DataList.push_back(DataValue(Float, aReturnedLowTariff, "Returned low tariff"));
		this->DataList.push_back(DataValue(Float, aReturnedHighTariff, "Returned high tariff"));
		this->DataList.push_back(DataValue(Word, aShortPowerFailureCnt, "Short power failure count"));
		this->DataList.push_back(DataValue(Word, aLongPowerFailureCnt, "Long power failure count"));
		this->DataList.push_back(DataValue(Word, aShortPowerFailureDuration, "Short power failure duration"));
		this->DataList.push_back(DataValue(Word, aLongPowerFailureDuration, "Long power failure duration"));
		this->DataList.push_back(DataValue(Float, aGas, "Gas"));
		this->DataList.push_back(DataValue(Float, aActualConsumption, "Actual consumption", 0.001f));	// To get it in Watts, as it's received in kW's.
		this->DataList.push_back(DataValue(Float, aActualReturn, "Actual return", 0.001f));  // To get it in Watts, as it's received in kW's.
		this->DataList.push_back(DataValue(Float, aActualConsumptionL1, "Actual consumption L1", 0.001f));  // To get it in Watts, as it's received in kW's.
		this->DataList.push_back(DataValue(Float, aActualConsumptionL2, "Actual consumption L2", 0.001f));  // To get it in Watts, as it's received in kW's.
		this->DataList.push_back(DataValue(Float, aActualConsumptionL3, "Actual consumption L3", 0.001f));  // To get it in Watts, as it's received in kW's.
		this->DataList.push_back(DataValue(Float, aActualReturnL1, "Actual return L1", 0.001f)); // To get it in Watts, as it's received in kW's.
		this->DataList.push_back(DataValue(Float, aActualReturnL2, "Actual return L2", 0.001f)); // To get it in Watts, as it's received in kW's.
		this->DataList.push_back(DataValue(Float, aActualReturnL3, "Actual return L3", 0.001f)); // To get it in kW's, as it's received in kW's.
		this->DataList.push_back(DataValue(Float, aCurrentL1, "Current L1"));
		this->DataList.push_back(DataValue(Float, aCurrentL2, "Current L2"));
		this->DataList.push_back(DataValue(Float, aCurrentL3, "Current L3"));
		this->DataList.push_back(DataValue(Float, aVoltageL1, "Voltage L1"));
		this->DataList.push_back(DataValue(Float, aVoltageL2, "Voltage L2"));
		this->DataList.push_back(DataValue(Float, aVoltageL3, "Voltage L3"));
		this->DataList.push_back(DataValue(Word, aTariff, "Tariff"));
	}

	unsigned int P1::CRC16(unsigned int crc, unsigned char* buf, int len)
	{
		for (int pos = 0; pos < len; pos++)
		{
			crc ^= (unsigned int)buf[pos];    // * XOR byte into least sig. byte of crc
			// * Loop over each bit
			for (int i = 8; i != 0; i--)
			{
				// * If the LSB is set
				if ((crc & 0x0001) != 0)
				{
					// * Shift right and XOR 0xA001
					crc >>= 1;
					crc ^= 0xA001;
				}
				// * Else LSB is not set
				else
					// * Just shift right
					crc >>= 1;
			}
		}
		return crc;
	}

	bool P1::isNumber(char* res, int len)
	{
		for (int i = 0; i < len; i++)
		{
			if (((res[i] < '0') || (res[i] > '9')) && (res[i] != '.' && res[i] != 0))
				return false;
		}
		return true;
	}

	int P1::FindCharInArrayRev(char array[], char c, int len)
	{
		for (int i = len - 1; i >= 0; i--)
		{
			if (array[i] == c)
				return i;
		}
		return -1;
	}

	double P1::GetValue(char* buffer, int maxlen, char startchar, char endchar)
	{
		int s = FindCharInArrayRev(buffer, startchar, maxlen);
		int l = FindCharInArrayRev(buffer, endchar, maxlen) - s - 1;

		if (l > 0)
		{
			char res[16];
			memset(res, 0, sizeof(res));

			if (strncpy(res, buffer + s + 1, l))
			{
				if (endchar == '*' || endchar == ')')
				{
					if (isNumber(res, l))
					{
						return atof(res);
					}
				}
			}
		}
		return 0;
	}

	bool P1::IsItem(string line, string item)
	{
		return strncmp(line.c_str(), item.c_str(), strlen(item.c_str())) == 0;
	}

	void P1::CheckAndParseLine(char* line, string registerText, char startChar, char endChar, int address)
	{
		if (IsItem(line, registerText))
		{
			float value = (float)GetValue(line, strlen(line), startChar, endChar);
			float divider = this->GetRegister(address).GetDivider();
			this->GetRegister(address).SetValue(value);
			// std::cout << this->GetRegister(address).GetName() << ": " << value << std::endl;
		}
	}

	// CRC-16-IBM parameters
	const uint16_t crc16_ibm_poly = 0xA001;
	const uint16_t crc16_ibm_initial = 0xFFFF;

	// Function to calculate CRC-16-IBM
	uint16_t P1::calculateCRC16IBM(const char* message, size_t length) {
		uint16_t crc = crc16_ibm_initial;

		for (size_t i = 0; i < length; ++i) {
			crc ^= static_cast<uint16_t>(message[i]);

			for (int j = 0; j < 8; ++j) {
				if (crc & 1) {
					crc = (crc >> 1) ^ crc16_ibm_poly;
				}
				else {
					crc >>= 1;
				}
			}
		}

		return crc;
	}

	bool P1::decode_telegram(int len) {
		int startChar = FindCharInArrayRev(telegram, '/', len);
		int endChar = FindCharInArrayRev(telegram, '!', len);

		/* bool validCRCFound = false;

		if (startChar >= 0)
		{
			// * Start found. Reset CRC calculation
			currentCRC = CRC16(0x0000, (unsigned char*)telegram + startChar, len - startChar);
		}
		else if (endChar >= 0)
		{
			// * Add to crc calc
			currentCRC = CRC16(currentCRC, (unsigned char*)telegram + endChar, 1);

			char messageCRC[5];
			strncpy(messageCRC, telegram + endChar + 1, 4);

			messageCRC[4] = 0;   // * Thanks to HarmOtten (issue 5)
			validCRCFound = (strtol(messageCRC, NULL, 16) == currentCRC);

			if (validCRCFound)
				std::cout << "P1 - CRC Valid!" << std::endl;
			else
				std::cout << "P1 - CRC Invalid!" << std::endl;

			currentCRC = 0;
		}
		else
		{
			currentCRC = CRC16(currentCRC, (unsigned char*)telegram, len);
		}

		*/

		// We have only a valid message, when we have a start and end character.
		if (startChar >= 0 && endChar >= 0)
		{
			this->UpdateReceivedData(strlen(telegram));
			this->UpdateTimeout(true);
			char messageCRC[5];
			strncpy(messageCRC, telegram + endChar + 1, 4);

			messageCRC[4] = 0;   // * Thanks to HarmOtten (issue 5)

			// Calculate CRC-16-IBM for the message
			uint16_t crcValue = calculateCRC16IBM(telegram, endChar);

			// Convert the CRC value to a hexadecimal string
			char crcHex[5];
			sprintf(crcHex, "%04X", crcValue);

			//std::cout << "CRC-16-IBM: " << crcHex << " Received: " << messageCRC << std::endl;

			// Split the telegram into lines
			char* line = strtok(telegram, "\r\n");

			// Go over the lines
			while (line != NULL)
			{
				// Print the line to the console.
				// std::cout << "P1 - " << line << std::endl;

				CheckAndParseLine(line, "1-0:1.8.1", '(', '*', aConsumedLowTariff);
				CheckAndParseLine(line, "1-0:1.8.2", '(', '*', aConsumedHighTariff);
				CheckAndParseLine(line, "1-0:2.8.1", '(', '*', aReturnedLowTariff);
				CheckAndParseLine(line, "1-0:2.8.2", '(', '*', aReturnedHighTariff);
				CheckAndParseLine(line, "0-0:96.7.21", '(', ')', aShortPowerFailureCnt);
				CheckAndParseLine(line, "0-0:96.7.9", '(', ')', aLongPowerFailureCnt);
				CheckAndParseLine(line, "1-0:32.32.0", '(', ')', aShortPowerFailureDuration);
				CheckAndParseLine(line, "1-0:32.36.0", '(', ')', aLongPowerFailureDuration);
				CheckAndParseLine(line, "0-1:24.2.1", '(', '*', aGas);
				CheckAndParseLine(line, "1-0:1.7.0", '(', '*', aActualConsumption);
				CheckAndParseLine(line, "1-0:2.7.0", '(', '*', aActualReturn);
				CheckAndParseLine(line, "1-0:21.7.0", '(', '*', aActualConsumptionL1);
				CheckAndParseLine(line, "1-0:41.7.0", '(', '*', aActualConsumptionL2);
				CheckAndParseLine(line, "1-0:61.7.0", '(', '*', aActualConsumptionL3);
				CheckAndParseLine(line, "1-0:22.7.0", '(', '*', aActualReturnL1);
				CheckAndParseLine(line, "1-0:42.7.0", '(', '*', aActualReturnL2);
				CheckAndParseLine(line, "1-0:62.7.0", '(', '*', aActualReturnL3);
				CheckAndParseLine(line, "1-0:31.7.0", '(', '*', aCurrentL1);
				CheckAndParseLine(line, "1-0:51.7.0", '(', '*', aCurrentL2);
				CheckAndParseLine(line, "1-0:71.7.0", '(', '*', aCurrentL3);
				CheckAndParseLine(line, "1-0:32.7.0", '(', '*', aVoltageL1);
				CheckAndParseLine(line, "1-0:52.7.0", '(', '*', aVoltageL2);
				CheckAndParseLine(line, "1-0:72.7.0", '(', '*', aVoltageL3);
				CheckAndParseLine(line, "0-0:96.14.0", '(', ')', aTariff);

				// Go to the next line.
				line = strtok(NULL, "\r\n");
			}
		}
		else
		{
			this->UpdateTimeout(false);
		}
	}
}