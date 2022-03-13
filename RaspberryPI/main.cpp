#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "SerialPort.h"
#include <HewalexThread.h>
#include <IntergasThread.h>
#include <ModbusServer.h>
#include <P1Thread.h>
#include <wiringPi.h>

// LED Pin - wiringPi pin 0 is BCM_GPIO 17.
// we have to use BCM numbering when initializing with wiringPiSetupSys

// when choosing a different pin number please use the BCM numbering, also
// update the Property Pages - Build Events - Remote Post-Build Event command
// which uses gpio export for setup for wiringPiSetupSys
#define	LED	17

static GateWay::IntergasThread intergas;
static GateWay::HewalexThread hewalex;
static GateWay::ModbusServer modBus;
static GateWay::P1Thread p1;

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
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
	}
}

static void readP1()
{
	while (true)
	{
		p1.Read();
		std::this_thread::sleep_for(std::chrono::milliseconds(200));
	}
}

int main(void)
{
	intergas.Initialize();

	hewalex.Initialize();

	modBus.Initialize();

	p1.Initialize();

	std::thread draadjeIntergas(readIntergas);
	std::thread draadjeHewalex(readHewalex);
	std::thread draadjeModbus(readModbus);
	//std::thread draadjeP1(readP1);

	modBus.Hewalex = &hewalex;
	modBus.Intergas = &intergas;
	modBus.P1 = &p1;

	int i = 0;
	while (true)
	{
		i++;
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	return 1;
}