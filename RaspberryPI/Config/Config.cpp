#include "Config.h"
#include <vector>
#include <map>

namespace GateWay
{
	/// <summary>
	/// The constructor of the Config class.
	/// It sets the default values for the devices.
	/// The hewalex an the modbus device are the same, because they are both
	/// RS-485 converters. The intergas device is a RS-232 converter.
	/// The P1 device is the FTDI USB to serial converter.
	/// </summary>
	Config::Config()
	{
		this->intergasDevice = "USB-Serial Controller";
		this->hewalexDevice = "USB2.0-Ser!";
		this->modbusDevice = "USB2.0-Ser!";
		this->p1Device = "FT232R USB UART - FT232R USB UART";
	}

	Config::~Config()
	{
		// No need to do anything here
	}

	string Config::findPortDeviceByDescription(const std::string& description, int no)
	{
		std::ifstream file(this->filePath);
		if (!file.is_open()) {
			std::cerr << "Error: Could not open the file." << std::endl;
			return "";
		}

		std::string line;
		std::vector<std::string> descriptions;
		std::vector<std::string> devicePaths;

		while (std::getline(file, line)) {
			size_t colonPos = line.find(':');
			if (colonPos != std::string::npos) {
				std::string desc = line.substr(0, colonPos);
				std::string devicePath = line.substr(colonPos + 1);

				// Remove leading and trailing spaces from description and devicePath
				desc.erase(0, desc.find_first_not_of(" \t"));
				desc.erase(desc.find_last_not_of(" \t") + 1);
				devicePath.erase(0, devicePath.find_first_not_of(" \t"));
				devicePath.erase(devicePath.find_last_not_of(" \t") + 1);

				descriptions.push_back(desc);
				devicePaths.push_back(devicePath);
			}
		}

		int occurence = 0;
		for (int i = 0; i < descriptions.size(); i++)
		{
			if (descriptions[i] == description)
			{
				if (occurence == no)
				{
					return devicePaths[i];
				}
				occurence++;
			}
		}

		return ""; // Return an empty string if no match is found
	}

	void Config::RunUsbScript()
	{
		// Replace "your_script.sh" with the actual path to your shell script
		const char* scriptPath = "python3 /home/pi/lsserial.py > /home/pi/usbConfig.ini";

		int returnValue = system(scriptPath);

		if (returnValue == 0) {
			std::cout << "USB Script executed successfully." << std::endl;
		}
		else {
			std::cout << "USB Script execution failed with return code. We try it with the previous one. " << returnValue << std::endl;
		}
	}

	string Config::ReadConfig(string description, int occurence)
	{
		std::string result = findPortDeviceByDescription(description, occurence);
		if (!result.empty()) {
			std::cout << "Matching device: " << result << " for " << description << std::endl;
		}
		else {
			std::cout << "No matching device path found for: " << description << std::endl;
		}
		return result;
	}
	string Config::GetIntergasComPort()
	{
		return ReadConfig(this->intergasDevice, 0);
	}
	string Config::GetHewalexComPort()
	{
		return ReadConfig(this->hewalexDevice, 0);
	}
	string Config::GetModbusComPort()
	{
		return ReadConfig(this->modbusDevice, 1);
	}
	string Config::GetP1ComPort()
	{
		return ReadConfig(this->p1Device, 0);
	}
}