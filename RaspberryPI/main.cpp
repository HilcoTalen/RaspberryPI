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

GateWay::Intergas intergas;
GateWay::Hewalex hewalex;
GateWay::ModbusServer modBus;
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

int main(void)
{
	intergas.OpenSerialPort();
	intergas.CreateDatalist();

	hewalex.OpenSerialPort();
	hewalex.CreateDatalist();

	modBus.OpenSerialPort();
	modBus.CreateDatalist();

	//p1.OpenSerialPort();
	//p1.CreateDatalist();

	std::thread IntergasThread(readIntergas);
	std::thread HewalexThread(readHewalex);
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