#ifndef CONFIG_H
#define CONFIG_H

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm> // Include this for std::remove

namespace GateWay
{
	using namespace std;

	class Config
	{
	public:
		Config();
		~Config();

		void RunUsbScript();
		string ReadConfig(string description, int occurence);

		string GetIntergasComPort();
		string GetHewalexComPort();
		string GetModbusComPort();
		string GetP1ComPort();

	private:
		string findPortDeviceByDescription(const std::string& description, int no);
		string filePath = "/home/pi/usbConfig.ini";

		string intergasDevice;
		string hewalexDevice;
		string modbusDevice;
		string p1Device;
	};
}

#endif
