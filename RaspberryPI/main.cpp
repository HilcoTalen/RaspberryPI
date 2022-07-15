#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "SerialPort.h"
#include <Hewalex.h>
#include <Intergas.h>
#include <ModbusServer.h>
#include <P1.h>
#include <wiringPi.h>

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
uint8_t currentComPortUsed;
//static GateWay::P1 p1;

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
		hewalex.Read();
	}
}

static void readModbus()
{
	while (true)
	{
		modBus.Read();
	}
}

static void readP1()
{
	while (true)
	{
		//p1.Read();
		std::this_thread::sleep_for(std::chrono::milliseconds(200));
	}
}

string GetNextComPort()
{

}


int main(void)
{
	vector<string> ComPortList;
	ComPortList.push_back("/dev/ttyUSB0");
	ComPortList.push_back("/dev/ttyUSB1");
	ComPortList.push_back("/dev/ttyUSB2");

	// Open Intergas initial port.
	intergas.OpenSerialPort(ComPortList[currentComPortUsed]);
	intergas.CreateDatalist();

	// Start read thread.
	std::thread IntergasThread(readIntergas);

	// Start stopwatch
	auto start = high_resolution_clock::now();

	// Iterate through the COM ports, till the device is alive.
	while (!intergas.online)
	{
		auto stop = high_resolution_clock::now();
		double elapsed = duration_cast<milliseconds>(stop - start).count() / 1000.0;

		if (elapsed > 10)
		{
			// We should try another COM port.
			currentComPortUsed++;
			if (currentComPortUsed >= ComPortList.size())
			{
				std::cout << "Cannot connect Intergas module." << std::endl;
				break;
			}
			intergas.SwitchComport(ComPortList[currentComPortUsed]);
			start = high_resolution_clock::now();

		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}

	if (intergas.online)
	{
		ComPortList.erase(ComPortList.begin() + currentComPortUsed);
	}

	// Reset current COM port used.
	currentComPortUsed = 0;

	// Open Hewalex initial port.
	hewalex.OpenSerialPort(ComPortList[currentComPortUsed]);
	hewalex.CreateDatalist();

	// Start read thread.
	std::thread HewalexThread(readHewalex);

	// Start stopwatch
	start = high_resolution_clock::now();

	// Iterate through the COM ports, till the device is alive.
	while (!hewalex.online)
	{
		auto stop = high_resolution_clock::now();
		double elapsed = duration_cast<milliseconds>(stop - start).count() / 1000.0;

		if (elapsed > 10)
		{
			// We should try another COM port.
			currentComPortUsed++;
			if (currentComPortUsed >= ComPortList.size())
			{
				std::cout << "Cannot connect Hewalex module." << std::endl;
				break;
			}
			hewalex.SwitchComport(ComPortList[currentComPortUsed]);
			start = high_resolution_clock::now();

		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}

	if (hewalex.online)
	{
		ComPortList.erase(ComPortList.begin() + currentComPortUsed);
	}


	// Only 1 COM port should be left for the ModBUS communication. Use this one directly.
	modBus.OpenSerialPort(ComPortList[0]);
	modBus.CreateDatalist();

	//p1.OpenSerialPort();
	//p1.CreateDatalist();


	std::thread ModbusThread(readModbus);
	//std::thread draadjeP1(readP1);

	modBus.hewalex = &hewalex;
	modBus.intergas = &intergas;
	//modBus.p1 = &p1;

	int i = 0;
	while (true)
	{
		i++;
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	return 1;
}