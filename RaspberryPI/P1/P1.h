#ifndef P1_H
#define P1_H

#include <SerialPort.h>
#include <CommunicationStats.h>

namespace GateWay
{
	using namespace std;

	class P1 : public CommunicationStats
	{
	public:
		P1();
		~P1();
		void Read();
		void CreateDatalist();

	private:
	};
}
#endif
