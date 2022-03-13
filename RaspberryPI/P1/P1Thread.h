#ifndef P1_H
#define P1_H

#include <SerialPort.h>
#include <CommunicationStats.h>

namespace GateWay
{
	using namespace std;

	class P1Thread : public CommunicationStats
	{
	public:
		P1Thread();
		~P1Thread();
		bool Initialize();
		void Read();

	private:
	};
}
#endif
