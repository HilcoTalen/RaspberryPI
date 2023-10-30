//!
//! @file 			SerialPort.cpp
//! @author 		Geoffrey Hunter <gbmhunter@gmail.com> (www.mbedded.ninja)
//! @created		2014-01-07
//! @last-modified 	2017-11-27
//! @brief			The main serial port class.
//! @details
//!					See README.rst in repo root dir for more info.

// System includes
#include <iostream>
#include <sstream>
#include <cassert>
#include <stdio.h>   	// Standard input/output definitions
#include <string.h>  	// String function definitions
#include <unistd.h>  	// UNIX standard function definitions
#include <fcntl.h>   	// File control definitions
#include <errno.h>   	// Error number definitions
//#include <termios.h> 	// POSIX terminal control definitions (struct termios)
#include <system_error>	// For throwing std::system_error
#include <sys/ioctl.h> // Used for TCGETS2, which is required for custom baud rates
#include <cassert>
//#include <asm/termios.h> // Terminal control definitions (struct termios)
#include <asm/ioctls.h>
// #include <asm/termbits.h>
#include <algorithm>
#include <iterator>
#include <wiringPi.h>

// User includes
#include "Exception.h"
#include "SerialPort.h"
#include <thread>
//#include <termios.h>

#define    BOTHER 0010000

namespace GateWay
{
	using namespace std;

	SerialPort::SerialPort() {
		echo_ = false;
		timeout_ms_ = defaultTimeout_ms_;
		baudRateType_ = BaudRateType::STANDARD;
		baudRateStandard_ = defaultBaudRate_;
		readBufferSize_B_ = defaultReadBufferSize_B_;
		readBuffer_.reserve(readBufferSize_B_);
		state_ = State::CLOSED;
	}

	SerialPort& SerialPort::operator= (const SerialPort& source)
	{
		this->device_ = source.device_;
		this->baudRateType_ = source.baudRateType_;
		this->baudRateStandard_ = source.baudRateStandard_;
		this->numDataBits_ = source.numDataBits_;
		this->parity_ = source.parity_;
		this->numStopBits_ = source.numStopBits_;
		this->fileDesc_ = source.fileDesc_;
		this->timeout_ms_ = source.timeout_ms_;
		this->state_ = source.state_;
	}

	SerialPort::SerialPort(const SerialPort& source)
	{
		this->device_ = source.device_;
		this->baudRateType_ = source.baudRateType_;
		this->baudRateStandard_ = source.baudRateStandard_;
		this->baudRateCustom_ = source.baudRateCustom_;
		this->numDataBits_ = source.numDataBits_;
		this->parity_ = source.parity_;
		this->numStopBits_ = source.numStopBits_;
		this->fileDesc_ = source.fileDesc_;
		this->timeout_ms_ = source.timeout_ms_;
		this->state_ = source.state_;
	}

	SerialPort::SerialPort(const std::string& device, BaudRate baudRate) :
		SerialPort() {
		device_ = device;
		baudRateType_ = BaudRateType::STANDARD;
		baudRateStandard_ = baudRate;
	}

	SerialPort::SerialPort(const std::string& device, speed_t baudRate) :
		SerialPort() {
		device_ = device;
		baudRateType_ = BaudRateType::CUSTOM;
		baudRateCustom_ = baudRate;
	}

	SerialPort::SerialPort(const std::string& device, BaudRate baudRate, NumDataBits numDataBits, Parity parity, NumStopBits numStopBits) :
		SerialPort() {
		device_ = device;
		baudRateType_ = BaudRateType::STANDARD;
		baudRateStandard_ = baudRate;
		numDataBits_ = numDataBits;
		parity_ = parity;
		numStopBits_ = numStopBits;
	}

	SerialPort::~SerialPort() {
		try {
			// Close();
		}
		catch (...) {
			// We can't do anything about this!
			// But we don't want to throw within destructor, so swallow
		}
	}

	void SerialPort::SetDevice(const std::string& device) {
		device_ = device;
		if (state_ == State::OPEN)
			ConfigureTermios();
	}

	void SerialPort::SetBaudRate(BaudRate baudRate) {
		baudRateType_ = BaudRateType::STANDARD;
		baudRateStandard_ = baudRate;
		if (state_ == State::OPEN)
			ConfigureTermios();
	}

	void SerialPort::SetBaudRate(speed_t baudRate) {
		baudRateType_ = BaudRateType::CUSTOM;
		baudRateCustom_ = baudRate;
		if (state_ == State::OPEN)
			ConfigureTermios();
	}

	void SerialPort::SetNumDataBits(NumDataBits numDataBits) {
		numDataBits_ = numDataBits;
		if (state_ == State::OPEN)
			ConfigureTermios();
	}

	void SerialPort::SetParity(Parity parity) {
		parity_ = parity;
		if (state_ == State::OPEN)
			ConfigureTermios();
	}

	void SerialPort::SetNumStopBits(NumStopBits numStopBits) {
		numStopBits_ = numStopBits;
		if (state_ == State::OPEN)
			ConfigureTermios();
	}

	void SerialPort::ClearBuffer()
	{
		if (tcflush(this->fileDesc_, TCIOFLUSH) != 0) {
			std::cerr << "Buffer cleared failed." << std::endl;
		}
		std::cout << device_ << " - Buffer cleared." << std::endl;
	}

	void SerialPort::Open()
	{
		// std::cout << "Attempting to open COM port \"" << device_ << "\"." << std::endl;

		if (device_.empty()) {
			THROW_EXCEPT("Attempted to open file when file path has not been assigned to.");
		}

		// Attempt to open file
		//this->fileDesc = open(this->filePath, O_RDWR | O_NOCTTY | O_NDELAY);

		// O_RDONLY for read-only, O_WRONLY for write only, O_RDWR for both read/write access
		// 3rd, optional parameter is mode_t mode
		fileDesc_ = open(device_.c_str(), O_RDWR);

		// Check status
		if (fileDesc_ == -1) {
			THROW_EXCEPT("Could not open device " + device_ + ". Is the device name correct and do you have read/write permission?");
		}

		// Set the file descriptor non blocking
		//int flags = fcntl(fileDesc_, F_GETFL, 0);
		//fcntl(fileDesc_, F_SETFL, flags | O_NONBLOCK);

		ConfigureTermios();

		std::cout << "COM port opened successfully." << std::endl;
		state_ = State::OPEN;
	}

	void SerialPort::SetEcho(bool value) {
		echo_ = value;
		ConfigureTermios();
	}

	void SerialPort::ConfigureTermios()
	{
		// std::cout << "Configuring COM port \"" << device_ << "\"." << std::endl;

		//================== CONFIGURE ==================//

		// termios tty = GetTermios();
		termios tty = GetTermios();

		//================= (.c_cflag) ===============//

		// Set num. data bits
		// See https://man7.org/linux/man-pages/man3/tcflush.3.html
		tty.c_cflag &= ~CSIZE;			// CSIZE is a mask for the number of bits per character
		switch (numDataBits_) {
		case NumDataBits::FIVE:
			tty.c_cflag |= CS5;
			break;
		case NumDataBits::SIX:
			tty.c_cflag |= CS6;
			break;
		case NumDataBits::SEVEN:
			tty.c_cflag |= CS7;
			break;
		case NumDataBits::EIGHT:
			tty.c_cflag |= CS8;
			break;
		default:
			THROW_EXCEPT("numDataBits_ value not supported!");
		}

		// Set parity
		// See https://man7.org/linux/man-pages/man3/tcflush.3.html
		switch (parity_) {
		case Parity::NONE:
			tty.c_cflag &= ~PARENB;
			break;
		case Parity::EVEN:
			tty.c_cflag |= PARENB;
			tty.c_cflag &= ~PARODD; // Clearing PARODD makes the parity even
			break;
		case Parity::ODD:
			tty.c_cflag |= PARENB;
			tty.c_cflag |= PARODD;
			break;
		default:
			THROW_EXCEPT("parity_ value not supported!");
		}

		// Set num. stop bits
		switch (numStopBits_) {
		case NumStopBits::ONE:
			tty.c_cflag &= ~CSTOPB;
			break;
		case NumStopBits::TWO:
			tty.c_cflag |= CSTOPB;
			break;
		default:
			THROW_EXCEPT("numStopBits_ value not supported!");
		}

		tty.c_cflag &= ~CRTSCTS;       // Disable hadrware flow control (RTS/CTS)
		tty.c_cflag |= CREAD | CLOCAL;     				// Turn on READ & ignore ctrl lines (CLOCAL = 1)

		//===================== BAUD RATE =================//

		// We used to use cfsetispeed() and cfsetospeed() with the B... macros, but this didn't allow
		// us to set custom baud rates. So now to support both standard and custom baud rates lets
		// just make everything "custom". This giant switch statement could be replaced with a map/lookup
		// in the future
		if (baudRateType_ == BaudRateType::STANDARD) {
			tty.c_cflag &= ~CBAUD;
			tty.c_cflag |= CBAUDEX;
			switch (baudRateStandard_) {
			case BaudRate::B_0:
				cfsetispeed(&tty, B0);
				cfsetospeed(&tty, B0);

				break;
			case BaudRate::B_50:
				cfsetispeed(&tty, B50);
				cfsetospeed(&tty, B50);
				break;
			case BaudRate::B_75:
				cfsetispeed(&tty, B75);
				cfsetospeed(&tty, B75);
				break;
			case BaudRate::B_110:
				cfsetispeed(&tty, B110);
				cfsetospeed(&tty, B110);
				break;
			case BaudRate::B_134:
				cfsetispeed(&tty, B134);
				cfsetospeed(&tty, B134);
				break;
			case BaudRate::B_150:
				cfsetispeed(&tty, B150);
				cfsetospeed(&tty, B150);
				break;
			case BaudRate::B_200:
				cfsetispeed(&tty, B200);
				cfsetospeed(&tty, B200);
				break;
			case BaudRate::B_300:
				cfsetispeed(&tty, B300);
				cfsetospeed(&tty, B300);
				break;
			case BaudRate::B_600:
				cfsetispeed(&tty, B600);
				cfsetospeed(&tty, B600);
				break;
			case BaudRate::B_1200:
				cfsetispeed(&tty, B1200);
				cfsetospeed(&tty, B1200);
				break;
			case BaudRate::B_1800:
				cfsetispeed(&tty, B1800);
				cfsetospeed(&tty, B1800);
				break;
			case BaudRate::B_2400:
				cfsetispeed(&tty, B2400);
				cfsetospeed(&tty, B2400);
				break;
			case BaudRate::B_4800:
				cfsetispeed(&tty, B4800);
				cfsetospeed(&tty, B4800);
				break;
			case BaudRate::B_9600:
				cfsetispeed(&tty, B9600);
				cfsetospeed(&tty, B9600);
				break;
			case BaudRate::B_19200:
				cfsetispeed(&tty, B19200);
				cfsetospeed(&tty, B19200);
				break;
			case BaudRate::B_38400:
				cfsetispeed(&tty, B38400);
				cfsetospeed(&tty, B38400);
				break;
			case BaudRate::B_57600:
				cfsetispeed(&tty, B57600);
				cfsetospeed(&tty, B57600);
				break;
			case BaudRate::B_115200:
				cfsetispeed(&tty, B115200);
				cfsetospeed(&tty, B115200);
				break;
			case BaudRate::B_230400:
				cfsetispeed(&tty, B230400);
				cfsetospeed(&tty, B230400);
				break;
			case BaudRate::B_460800:
				cfsetispeed(&tty, B460800);
				cfsetospeed(&tty, B460800);
				break;
			default:
				throw std::runtime_error(std::string() + "baudRate passed to " + __PRETTY_FUNCTION__ + " unrecognized.");
			}
		}
		else
		{
			// Should never get here, bug in this libraries code!
			assert(false);
		}

		//===================== (.c_oflag) =================//

		tty.c_oflag = 0;              // No remapping, no delays
		tty.c_oflag &= ~OPOST;			// Make raw      (not processed by the system) (see: https://stackoverflow.com/questions/1798511/how-to-avoid-press-enter-with-any-getchar)

		//================= CONTROL CHARACTERS (.c_cc[]) ==================//

		// c_cc[VTIME] sets the inter-character timer, in units of 0.1s.
		// Only meaningful when port is set to non-canonical mode
		// VMIN = 0, VTIME = 0: No blocking, return immediately with what is available
		// VMIN > 0, VTIME = 0: read() waits for VMIN bytes, could block indefinitely
		// VMIN = 0, VTIME > 0: Block until any amount of data is available, OR timeout occurs
		// VMIN > 0, VTIME > 0: Block until either VMIN characters have been received, or VTIME
		//                      after first character has elapsed
		// c_cc[WMIN] sets the number of characters to block (wait) for when read() is called.
		// Set to 0 if you don't want read to block. Only meaningful when port set to non-canonical mode

		if (timeout_ms_ == -1) {
			// Always wait for at least one byte, this could
			// block indefinitely
			tty.c_cc[VTIME] = 0;
			tty.c_cc[VMIN] = 1;
		}
		else if (timeout_ms_ == 0) {
			// Setting both to 0 will give a non-blocking read
			tty.c_cc[VTIME] = 0;
			tty.c_cc[VMIN] = 0;
		}
		else if (timeout_ms_ > 0) {
			tty.c_cc[VTIME] = (cc_t)(timeout_ms_ / 100);    // 0.5 seconds read timeout
			tty.c_cc[VMIN] = 0;
		}

		//======================== (.c_iflag) ====================//

		tty.c_iflag &= ~(IXON | IXOFF | IXANY);			// Turn off s/w flow ctrl
		tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL);

		//=========================== LOCAL MODES (c_lflag) =======================//

		// Canonical input is when read waits for EOL or EOF characters before returning. In non-canonical mode, the rate at which
		// read() returns is instead controlled by c_cc[VMIN] and c_cc[VTIME]
		tty.c_lflag &= ~ICANON;								// Turn off canonical input, which is suitable for pass-through
		// Configure echo depending on echo_ boolean
		if (echo_) {
			tty.c_lflag |= ECHO;
		}
		else {
			tty.c_lflag &= ~(ECHO);
		}
		tty.c_lflag &= ~ECHOE;								// Turn off echo erase (echo erase only relevant if canonical input is active)
		tty.c_lflag &= ~ECHONL;								//
		tty.c_lflag &= ~ISIG;								// Disables recognition of INTR (interrupt), QUIT and SUSP (suspend) characters

		// Try and use raw function call
		//cfmakeraw(&tty);

		// this->SetTermios(tty);
		this->SetTermios(tty);
	}

	void SerialPort::SetRxTxLed(uint8_t ledRx, uint8_t ledTx)
	{
		this->ledNoRx = ledRx;
		this->ledNoTx = ledTx;

		pinMode(ledRx, OUTPUT);
		pinMode(ledTx, OUTPUT);
	}

	void SerialPort::Write(const std::string& data) {
		this->busy = true;
		if (state_ != State::OPEN)
			THROW_EXCEPT(std::string() + __PRETTY_FUNCTION__ + " called but state != OPEN. Please call Open() first.");

		if (fileDesc_ < 0) {
			THROW_EXCEPT(std::string() + __PRETTY_FUNCTION__ + " called but file descriptor < 0, indicating file has not been opened.");
		}

		digitalWrite(ledNoTx, 1);
		int writeResult = write(fileDesc_, data.c_str(), data.size());
		this->bytesReceived = 0;
		digitalWrite(ledNoTx, 0);

		// Check status
		if (writeResult == -1) {
			throw std::system_error(EFAULT, std::system_category());
		}
		this->busy = false;
	}

	void SerialPort::WriteBinary(const std::vector<uint8_t>& data) {
		this->busy = true;
		if (state_ != State::OPEN)
			THROW_EXCEPT(std::string() + __PRETTY_FUNCTION__ + " called but state != OPEN. Please call Open() first.");

		if (fileDesc_ < 0) {
			THROW_EXCEPT(std::string() + __PRETTY_FUNCTION__ + " called but file descriptor < 0, indicating file has not been opened.");
		}

		digitalWrite(ledNoTx, 1);
		int writeResult = write(fileDesc_, data.data(), data.size());
		this->bytesReceived = 0;
		digitalWrite(ledNoTx, 0);

		// Check status
		if (writeResult == -1) {
			throw std::system_error(EFAULT, std::system_category());
		}
		this->busy = false;
	}

	void SerialPort::Read(std::string& data)
	{
		this->busy = true;
		data.clear();

		if (fileDesc_ < 0) {
			//this->sp->PrintError(SmartPrint::Ss() << "Read() was called but file descriptor (fileDesc) was 0, indicating file has not been opened.");
			//return false;
			THROW_EXCEPT("Read() was called but file descriptor (fileDesc) was < 0, indicating file has not been opened.");
		}

		// Allocate memory for read buffer
//		char buf [256];
//		memset (&buf, '\0', sizeof buf);

		// Read from file
		// We provide the underlying raw array from the readBuffer_ vector to this C api.
		// This will work because we do not delete/resize the vector while this method
		// is called
		ssize_t n = read(fileDesc_, &readBuffer_[0], readBufferSize_B_);

		// Error Handling
		if (n < 0) {
			// Read was unsuccessful
			throw std::system_error(EFAULT, std::system_category());
		}

		if (n > 0) {
			//			buf[n] = '\0';
						//printf("%s\r\n", buf);
			//			data.append(buf);
			data = std::string(&readBuffer_[0], n);
			//std::cout << data << " and size of string =" << data.size() << "\r\n";
		}
		this->busy = false;
		// If code reaches here, read must of been successful
	}

	bool SerialPort::Busy()
	{
		return this->busy;
	}

	bool SerialPort::BytesReceiving()
	{
		this->busy = true;

		// Can happen when closing a port while reading.
		if (this->state_ == State::CLOSED)
		{
			return false;
		}

		uint16_t bytesAvailable;
		int retval = ioctl(fileDesc_, FIONREAD, &bytesAvailable);
		if (retval < 0)
		{
			THROW_EXCEPT("IOCTL() was called but return value was < 0, something failed.");
		}

		if (bytesAvailable == 0)
		{
			//std::cout << "Nothing received yet" << std::endl;
			this->receivedMessageCompleted = false;
			this->busy = false;
			return false;
		}
		else if (bytesAvailable > this->bytesReceived)
		{
			if (bytesAvailable > 2048)
			{
				// We are flooded, close this read, as it can become blocking.
				receivedMessageCompleted = true;
				this->busy = false;
				return false;
			}
			//std::cout << "Bytes receiving" << std::endl;
			this->receivedMessageCompleted = false;
			this->bytesReceived = bytesAvailable;
			digitalWrite(ledNoRx, true);
			this->busy = false;
			return true;
		}
		else if (bytesAvailable == this->bytesReceived)
		{
			// No more bytes received; so we are finished.
			 //std::cout << "No more bytes received, total: " << bytesAvailable << std::endl;
			receivedMessageCompleted = true;
			digitalWrite(ledNoRx, false);
			this->busy = false;
			return false;
		}
		this->busy = false;
		return bytesAvailable;
	}

	bool SerialPort::ReceivedMessageComplete()
	{
		return this->receivedMessageCompleted;
	}

	void SerialPort::ReadBinary(std::vector<uint8_t>& data)
	{
		this->busy = true;
		data.clear();

		if (fileDesc_ < 0) {
			//this->sp->PrintError(SmartPrint::Ss() << "Read() was called but file descriptor (fileDesc) was 0, indicating file has not been opened.");
			//return false;
			this->busy = false;
			THROW_EXCEPT("Read() was called but file descriptor (fileDesc) was < 0, indicating file has not been opened.");
		}

		// Set the start value
		std::chrono::system_clock::time_point start = std::chrono::system_clock::now();

		while ((std::chrono::system_clock::now() - start) < std::chrono::milliseconds(this->timeout_ms_))
		{
			while (this->BytesReceiving())
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
			}

			if (this->receivedMessageCompleted)
			{
				break;
			}
			else
			{
				// Wait also when nothing is received yet.
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
			}
		}

		// Read from file
		// We provide the underlying raw array from the readBuffer_ vector to this C api.
		// This will work because we do not delete/resize the vector while this method
		// is called
		ssize_t n = read(fileDesc_, &readBuffer_[0], readBufferSize_B_);

		// Error Handling
		if (n < 0) {
			// Read was unsuccessful
			this->busy = false;
			throw std::system_error(EFAULT, std::system_category());
		}

		if (n > 0) {
			copy(readBuffer_.begin(), readBuffer_.begin() + n, back_inserter(data));
		}
		this->busy = false;
		// If code reaches here, read must of been successful
	}

	


	termios SerialPort::GetTermios() {
		if (fileDesc_ == -1)
			throw std::runtime_error("GetTermios() called but file descriptor was not valid.");

		struct termios tty;
		memset(&tty, 0, sizeof(tty));

		// Get current settings (will be stored in termios structure)
		if (tcgetattr(fileDesc_, &tty) != 0)
		{
			// Error occurred
			std::cout << "Could not get terminal attributes for \"" << device_ << "\" - " << strerror(errno) << std::endl;
			throw std::system_error(EFAULT, std::system_category());
			//return false;
		}

		return tty;
	}

	void SerialPort::SetTermios(termios myTermios)
	{
		// Flush port, then apply attributes
		tcflush(fileDesc_, TCIFLUSH);

		if (tcsetattr(fileDesc_, TCSANOW, &myTermios) != 0)
		{
			// Error occurred
			std::cout << "Could not apply terminal attributes for \"" << device_ << "\" - " << strerror(errno) << std::endl;
			throw std::system_error(EFAULT, std::system_category());

		}

		// Successful!
	}


	void SerialPort::Close() {
		if (fileDesc_ != -1) {
			auto retVal = close(fileDesc_);
			if (retVal != 0)
				THROW_EXCEPT("Tried to close serial port " + device_ + ", but close() failed.");

			fileDesc_ = -1;
		}

		state_ = State::CLOSED;
	}

	void SerialPort::SetTimeout(int32_t timeout_ms) {
		if (timeout_ms < -1)
			THROW_EXCEPT(std::string() + "timeout_ms provided to " + __PRETTY_FUNCTION__ + " was < -1, which is invalid.");
		if (timeout_ms > 25500)
			THROW_EXCEPT(std::string() + "timeout_ms provided to " + __PRETTY_FUNCTION__ + " was > 25500, which is invalid.");
		if (state_ == State::OPEN)
			THROW_EXCEPT(std::string() + __PRETTY_FUNCTION__ + " called while state == OPEN.");
		timeout_ms_ = timeout_ms;
	}
}