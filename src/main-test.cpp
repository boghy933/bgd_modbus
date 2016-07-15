#include <iostream>
#include <string>

#include <cstring> //Nedded for memset
#include <sstream> //string stream for hex conversion
#include <iomanip> // cout << hex << "0xff"
#include <thread> //thread class for socket -std=c++0x -pthread

#include <unistd.h> //fopen fclose close...

#include <sys/types.h>
#include <sys/socket.h>
//#include <netinet/in.h>
#include <netdb.h>

#include "modbus_bgd.cpp"

/*
	# how to compile
	# g++ main-test.cpp -o modbus_bgd -std=c++11 -pthread
*/

int main(int argc, char *argv[]) {

	std::cout<< "Starting modbus...on port 502" <<std::endl;

	Modbus_bgd modbus;
	modbus.begin_listening("5002");
	/*
		# To start modbus on different port
		# modbus.begin_listening("5002");
	*/



	/*
	//	# How to use modbus
		MB_MEM[125][1].value = 0x000f;
		MB_MEM[125][1].value = 0x00f0;
		MB_MEM[125][1].value = 0x0f00;
		MB_MEM[125][1].value = 0xf000;
	*/



	MB_MEM[1][1] = 0xff;
	MB_MEM[1][2] = 0xff;
	MB_MEM[1][3] = 0x0;
	MB_MEM[1][4] = 0x0;
	MB_MEM[1][5] = 0xff;
	MB_MEM[1][6] = 0x0;
	MB_MEM[1][7] = 0xff;
	MB_MEM[1][8] = 0xff;
	MB_MEM[1][9] = 0x0;

	//MB_MEM[125][3] = 0x0f00;
	//MB_MEM[125][4] = 0xf000;

	std::cout << "Starting loop" << std::endl;

	while(true) {

	}

	return 0;
}
