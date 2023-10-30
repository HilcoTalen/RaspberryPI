#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "SerialPort.h"
#include <Config.h>
#include <Hewalex.h>
#include <Intergas.h>
#include <ModbusServer.h>
#include <P1.h>
#include <wiringPi.h>
#include <map>

// LED Pin - wiringPi pin 0 is BCM_GPIO 17.
// we have to use BCM numbering when initializing with wiringPiSetupSys

// when choosing a different pin number please use the BCM numbering, also
// update the Property Pages - Build Events - Remote Post-Build Event command
// which uses gpio export for setup for wiringPiSetupSys
#define	LED	17

using namespace chrono;

GateWay::Intergas intergas;
GateWay::Hewalex hewalex;
GateWay::ModbusServer modBus;
GateWay::P1 p1;
GateWay::Config config;
bool isSwitchingPorts = false;

static void readIntergas()
{
	while (true)
	{
		intergas.Read();
	}
}

static void readHewalex()
{
	while (true)
	{
		if (!isSwitchingPorts)
		{
			hewalex.Read();
		}
	}
}

static void readModbus()
{
	while (true)
	{
		if (!isSwitchingPorts)
		{
			modBus.Read();
		}
	}
}

static void readP1()
{
	while (true)
	{
		p1.Read();
	}
}

static void CheckOnlineStatus()
{
	while (true)
	{
		uint8_t secondsWaited = 0;
		if (hewalex.online || modBus.online)
		{
			// The Hewalex AND OR the ModBUS is online. We cannot switch the USB ports.
			// We switch only the USB ports, when both are offline, as that indicates that the
			// USB converters <> device paths are being switched.
			// So we can sleep for a while.
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			secondsWaited = 0;
		}
		else
		{
			bool offline = true;
			uint8_t secondsToWait = 10;
			while (true)
			{
				std::this_thread::sleep_for(std::chrono::seconds(1));
				if (hewalex.online || modBus.online)
				{
					offline = false;
					break;
				}
				else
				{
					std::cout << "Hewalex and ModBUS are both offline for " << secondsWaited << " seconds." << std::endl;
					secondsWaited++;
					if (secondsToWait == secondsWaited)
					{
						break;
					}
				}
			}

			if (offline)
			{
				isSwitchingPorts = true;

				// The Hewalex AND the ModBUS are both offline. This can be a sign that the USB converters
				// are switched. We should check this.
				// We are gonna switch the device paths, for these two threads.

				std::cout << "Hewalex and ModBUS are both offline. Switching USB ports." << std::endl;

				string currentHewalexPort = hewalex.GetSerialPort();
				string currentModbusPort = modBus.GetSerialPort();

				// Close the serial port.
				hewalex.CloseSerialPort();
				modBus.CloseSerialPort();

				// Open again, with the switch device paths.
				hewalex.OpenSerialPort(currentModbusPort);
				modBus.OpenSerialPort(currentHewalexPort);
				isSwitchingPorts = false;
			}
		}
	}
}

int main(void)
{
	// Runs the config script first. This one will create the usbConfig.ini file.
	// In that file, the link between the USB serial description and the device paths are created.
	// As for the P1 reader, a FTDI chip is used, and for the Intergas a Serial to USB converter,
	// we can just open the device path directly, and let the P1 thread and the Intergas thread running.
	config.RunUsbScript();

	// Open Intergas initial port.
	intergas.OpenSerialPort(config.GetIntergasComPort());
	intergas.CreateDatalist();
	std::thread IntergasThread(readIntergas);

	// Open P1 initial port.
	p1.OpenSerialPort(config.GetP1ComPort());
	p1.CreateDatalist();
	std::thread P1Thread(readP1);

	// Open Hewalex initial port.
	hewalex.OpenSerialPort(config.GetHewalexComPort());
	hewalex.CreateDatalist();
	std::thread HewalexThread(readHewalex);

	// Open ModBUS initial port.
	modBus.OpenSerialPort(config.GetModbusComPort());
	modBus.CreateDatalist();
	std::thread ModbusThread(readModbus);

	// When the USB Serial > RS485 are being switched (and they will be both offline),
	// this will be seen by the thread 'CheckOnlineStatus'.
	std::thread CheckOnlineStatusThread(CheckOnlineStatus);

	modBus.hewalex = &hewalex;
	modBus.intergas = &intergas;
	modBus.p1 = &p1;

	int i = 0;
	while (true)
	{
		i++;
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	return 1;
}