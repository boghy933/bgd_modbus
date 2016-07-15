#include "modbus_bgd.h"

void Modbus_bgd::begin_listening(std::string port) {
	std::thread (Modbus_bgd::listening_function,port).detach();
}

void Modbus_bgd::listening_function(std::string port) {
	Modbus_bgd bgd_listening;
	bgd_listening.init_registers();
	bgd_listening.listening(port);
}

void Modbus_bgd::init_registers() {
	for (int i = 0; i < SERVER_SLAVE; ++i) {
		for (int y = 0; y  < NUMBER_OF_REGISTERS; ++y) {
			MB_MEM[i][y] = 0x0000;
		}
	}
}

bool Modbus_bgd::listening(std::string port) {
	int status;
	
	int yes = 1;
	struct addrinfo host_info;
	struct addrinfo *host_info_list;

	memset(&host_info, 0, sizeof host_info);

	host_info.ai_family = AF_UNSPEC;
	host_info.ai_socktype = SOCK_STREAM;
	host_info.ai_flags = AI_PASSIVE;

	status = getaddrinfo(NULL, port.c_str(), &host_info, &host_info_list);
	if(status == -1)
		std::cout << "Error, unable to listen on the port " << port << std::endl;

	int socketfd;
	socketfd = socket(host_info_list->ai_family,host_info_list->ai_socktype, host_info_list->ai_protocol);
	if(socketfd == -1)
		std::cout << "Socket error!" << std::endl;

	status = setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
	status = bind(socketfd, host_info_list->ai_addr, host_info_list->ai_addrlen);
	if(status == -1)
		std::cout << "Bind error!" << std::endl;

	std::cout << "Listening for connections on port " << port << std::endl;
	status = listen(socketfd, MAX_CON);
		if(status == -1)
			std::cout << "Error, listening failed on port " << port << std::endl;

	int new_sd;
	struct sockaddr_storage their_addr;
	socklen_t addr_size = sizeof(their_addr);

	while((new_sd = accept(socketfd, (struct sockaddr *)&their_addr, &addr_size)) > 0) {
		if(new_sd == -1) {
			std::cout << "Error, unable to accept connection!" << std::endl;
		} else {
			std::cout << "Connection accepted. [ socketfd: " << new_sd << " ]" << std::endl;
			std::thread(socket_thread, new_sd).detach();
		}
	}
}

void Modbus_bgd::socket_thread(int new_sd) {
	Modbus_bgd bind_bgd;
	bind_bgd.listening_socket(new_sd);
}

void Modbus_bgd::listening_socket(int new_sd) {
	ssize_t bytes_received;
	uint8_t incoming_data_buffer[100];

	//std::cout << "Waiting to receive data..." << std::endl;
	do {
		bytes_received = recv(new_sd, incoming_data_buffer,100,0);
		if(bytes_received == 0){
			std::cout << "Host shut down." << std::endl;
			break;
		}
		if(bytes_received == -1){
			std::cout << "Receive error!!" << std::endl;
			break;
		}
		if(!Modbus_bgd::request(incoming_data_buffer,bytes_received, new_sd)){
			std::cout << "Request error!!" << std::endl;
		}
	} while(true);
	close(new_sd);
}

bool Modbus_bgd::request(uint8_t incoming_data_buffer[100], ssize_t bytes_received, int new_sd) {

	tcp_request info_data;

	//std::cout << bytes_received << " bytes received :" << std::endl;
	incoming_data_buffer[bytes_received] = '\0';
	
	/*Some debug code*//* 
	for(int i = 0; i<bytes_received;i++) {
		printf("%d -> [%x]\n",i,incoming_data_buffer[i] );
	}
	/**/

	/*
		# Save values for write multiple registers... all the values are for sure after the first 12 bytes
	*/
		info_data.values[0] = '\0'; //cleaning from the old buffer
 		for(int i=13; i< bytes_received;i++) {
			info_data.values[i-13] = incoming_data_buffer[i];
			info_data.values[i-12] = '\0'; //cleaning from the old buffer 
		}
    


	/*Get packet informazion*/
	info_data.transaction_id = (incoming_data_buffer[0] << 8) + incoming_data_buffer[1];
	info_data.protocol_id = (incoming_data_buffer[2] << 8) + incoming_data_buffer[3];
	info_data.message_length = (incoming_data_buffer[4] << 8) + incoming_data_buffer[5];
	info_data.unit_id = incoming_data_buffer[6];
	info_data.function_code = incoming_data_buffer[7];
	info_data.start_register = (incoming_data_buffer[8] << 8) + incoming_data_buffer[9];
	info_data.number_requested = (incoming_data_buffer[10] << 8) + incoming_data_buffer[11];

	/* Debug info
	printf("transaction_id: %d\n", info_data.transaction_id);
	printf("protocol_id: %d\n", info_data.protocol_id);
	printf("message_length: %d\n", info_data.message_length);
	printf("unit_id: %d\n", info_data.unit_id);
	printf("function_code: %d\n", info_data.function_code);
	printf("start_register: %d\n", info_data.start_register);	
	printf("number_requested: %d\n", info_data.number_requested);
	printf("[%d] [%d] [%d] [%d] [%d] [%d] [%d]\n", info_data.transaction_id, info_data.protocol_id, info_data.message_length, info_data.unit_id, info_data.function_code, info_data.start_register, info_data.number_requested);
	*/
	return Modbus_bgd::query_function(info_data, new_sd);
}

bool Modbus_bgd::query_function(tcp_request info_data, int new_sd) {

	switch(info_data.function_code) {
		case MODBUS_BGD_READ_COILS:
			//std::cout << "Read coils" << std::endl;
			Modbus_bgd::ReadCoilStatus(info_data,new_sd);
		break;

		case MODBUS_BGD_READ_HOLDING_REGISTERS:
			//std::cout << "Read holding registers" << std::endl;
			Modbus_bgd::ReadHoldingRegisters(info_data,new_sd);
		break;

		case MODBUS_BGD_WRITE_SINGLE_COIL:
			//std::cout << "Write single coil" << std::endl;
			Modbus_bgd::ForceSingleCoil(info_data,new_sd);
		break;

		case MODBUS_BGD_WRITE_SINGLE_REGISTER:
			//std::cout << "Write single register" << std::endl;
			Modbus_bgd::WriteSingleRegister(info_data,new_sd);
		break;

		case MODBUS_BGD_WRITE_MULTIPLE_COILS:
			//std::cout << "Write multiple coils" << std::endl;
			Modbus_bgd::ForceMultipleCoils(info_data,new_sd);
		break;

		case MODBUS_BGD_WRITE_MULTIPLE_REGISTERS:
			//std::cout << "Write multiple registers" << std::endl;
			Modbus_bgd::WriteMultipleRegisters(info_data,new_sd);
		break;

		default:
			//std::cout << "Function: " << info_data.function_code << ". Not supported." << std::endl;
			return false;
		break;		
	}
	return true;
}


void Modbus_bgd::ReadCoilStatus(tcp_request info_data, int new_sd) {

	uint8_t reply[MAX_REGISTERS+3];
	/*
		# Modbus protocol 
	*/
	reply[0] = info_data.transaction_id >> 8;
	reply[1] = info_data.transaction_id & 0xff;
	reply[2] = 0x00;
	reply[3] = 0x00;
	reply[4] = 0x00;
	reply[5] = 0x03; //number of registers + the other 3 below
	reply[6] = info_data.unit_id;
	reply[7] = info_data.function_code;
	reply[8] = 0x00; //number of registers

	/*Data*/
	int byte_size = 9;
	int i,counter=0;
	
	int group_by = 8;

	int actual, shift, total_bytes;
	actual = 0;
	shift = 1;
	for(i = info_data.start_register; i < info_data.start_register + info_data.number_requested; i++ ) {
		reply[byte_size] = 0;
		byte_size++;
	}
	byte_size = 9;
	for(i = info_data.start_register; i < info_data.start_register + info_data.number_requested; i++ ) {
		if(MB_MEM[info_data.unit_id][i] == 0xff) {
			reply[byte_size] += 1 << actual;
		} else {
			reply[byte_size] += 0 << actual;
		}
		actual++; 
		if( (actual) == group_by) {
			actual = 0;
			shift = 1;
			byte_size++;
			counter++;
		}
	}
	if(actual != 0)
		counter++;
	reply[5] += ((counter*1)); //size
	reply[8] +=	((counter*1));

	Modbus_bgd::send_reply(new_sd,reply);
}

void Modbus_bgd::ReadHoldingRegisters(tcp_request info_data, int new_sd) {
	//00 03 00 00 00 05 19 03  02 00 00 

	uint8_t reply[MAX_REGISTERS+3];
	/*
		# Modbus protocol 
	*/
	
		reply[0] = info_data.transaction_id >> 8;
		reply[1] = info_data.transaction_id & 0xff;
		reply[2] = 0x00;
		reply[3] = 0x00;
		reply[4] = 0x00;
		reply[5] = 0x03; //number of registers + the other 3 below
		reply[6] = info_data.unit_id;
		reply[7] = info_data.function_code;
		reply[8] = 0x00; //number of registers
	
	
	/*
		#Data
	*/
	int byte_size = 9;
	int i,counter=0;

	for(i = info_data.start_register; i < info_data.start_register + info_data.number_requested; i++ ) {
		reply[byte_size]= (MB_MEM[info_data.unit_id][i] >> 8);
		byte_size++;
		reply[byte_size]= (MB_MEM[info_data.unit_id][i] & 0xff);
		byte_size++;
		counter++;
	}

	reply[5] += ((counter*2)); //size
	reply[8] +=	((counter*2));

	Modbus_bgd::send_reply(new_sd,reply);
}

void Modbus_bgd::ForceSingleCoil(tcp_request info_data, int new_sd) {

	std::stringstream stream;

	stream << std::hex << info_data.number_requested ;
	uint16_t new_value;

	stream >> new_value;
	if(new_value == 0xff00)
		MB_MEM[info_data.unit_id][info_data.start_register] = 0xff;
	else
		MB_MEM[info_data.unit_id][info_data.start_register] = 0x0;

	uint8_t reply[MAX_REGISTERS+3];
	reply[0] = 0x00;
	reply[1] = info_data.transaction_id;
	reply[2] = 0x00;
	reply[3] = 0x00;
	reply[4] = 0x00;

	reply[5] = 0x06; //number of registers + the other 3 below
	reply[6] = info_data.unit_id;
	reply[7] = info_data.function_code;
	reply[8] = 0x00;
	reply[9] = 0x00; //number of registers
	reply[10] = (MB_MEM[info_data.unit_id][info_data.start_register] & 0xff);
	reply[11] = (MB_MEM[info_data.unit_id][info_data.start_register] >> 8);
	
	Modbus_bgd::send_reply(new_sd,reply);
}


void Modbus_bgd::WriteSingleRegister(tcp_request info_data, int new_sd) {

	/*
		# 00000000  00 01 00 00 00 06 01 06  00 01 01 45             ........ ...E
    	# 00000000  00 01 00 00 00 06 01 06  00 01 01 45             ........ ...E
	*/
	std::stringstream stream;
	stream << std::hex << info_data.number_requested;
	stream >> MB_MEM[info_data.unit_id][info_data.start_register];
		
	uint8_t reply[MAX_REGISTERS+3];
	reply[0] = 0x00;
	reply[1] = info_data.transaction_id;
	reply[2] = 0x00;
	reply[3] = 0x00;
	reply[4] = 0x00;

	reply[5] = 0x06; //number of registers + the other 3 below
	reply[6] = info_data.unit_id;
	reply[7] = info_data.function_code;
	reply[8] = 0x00;
	reply[9] = 0x00; //number of registers
	reply[10] = (MB_MEM[info_data.unit_id][info_data.start_register] >> 8);
	reply[11] = (MB_MEM[info_data.unit_id][info_data.start_register] & 0xff);

	Modbus_bgd::send_reply(new_sd,reply);
}


void Modbus_bgd::ForceMultipleCoils(tcp_request info_data, int new_sd) {
	
	int i=0;
	int starting_register = info_data.start_register;
	int end_register = info_data.start_register + info_data.number_requested;
	bool flag = false, end = false;
	int reg_position,shifter;
	/*
		#UPDATING REGISTERS
	*/

	/*
		# check if command is to put all values to 0
		# nex version probably will be with the bytes_received
	*/
	if(info_data.values[i] == '\0') {
		flag = true;
	}
	while( (info_data.values[i] != '\0' or flag) and !end) {
		reg_position = 0;
		shifter = 1;
		while(reg_position < 8) {
			if(((info_data.values[i] & shifter)>> reg_position) == 1)
				MB_MEM[info_data.unit_id][starting_register] = 0xff;
			else
				MB_MEM[info_data.unit_id][starting_register] = 0x0;

			if(starting_register == end_register) {
				end = true;
				break;
			}

			starting_register++;
			reg_position++;
			shifter *=2;
		}

		i++;
		

	}
	if(starting_register == end_register) {
		//all ok
		//send feedback
		info_data.function_code = MODBUS_BGD_WRITE_MULTIPLE_COILS;
		
		uint8_t reply[MAX_REGISTERS+3];
		/*
			# Modbus protocol 
	 	*/
	
		reply[0] = info_data.transaction_id >> 8;
		reply[1] = info_data.transaction_id & 0xff;
		reply[2] = 0x00;
		reply[3] = 0x00;
		reply[4] = 0x00;
		reply[5] = 0x06; //number of registers + the other 3 below
		reply[6] = info_data.unit_id;
		reply[7] = info_data.function_code;
		reply[8] = info_data.start_register >> 8;
		reply[9] =  info_data.start_register & 0xff;
		reply[10] = info_data.number_requested >> 8;
		reply[11] = info_data.number_requested & 0xff;
		Modbus_bgd::send_reply(new_sd,reply);
	}  else {
		std::cout << "Something went wrong while writing multiple coils..." << std::endl;
	}
}


void Modbus_bgd::WriteMultipleRegisters(tcp_request info_data, int new_sd) {
	/*
		# start 0 n° 4
 		# 00 01 00 00 00 0f 01 10  00 00 00 04 08 aa aa bb bb cc cc dd dd
		# start 5 n° 2
		# 00 05 00 00 00 0b 01 10  00 05 00 02 04 ff ff ff ff
	*/
	int i=0;
	int starting_register = info_data.start_register;
	int end_register = info_data.start_register + info_data.number_requested;
	bool flag = false;
	/*
	 # UPDATING REGISTERS
	 # for more information about the flags and the if check ForceMultipleCoils
	*/

	if(info_data.values[i] == '\0') {
		flag = true;
	}
	while(info_data.values[i] != '\0' or flag) {
		MB_MEM[info_data.unit_id][starting_register] = (info_data.values[i] << 8) + info_data.values[i+1];
		i+=2;

		if(starting_register == end_register)
			break;

		starting_register++;
	}
	if(starting_register == end_register) {
		//all ok
		//send feedback
		info_data.function_code = MODBUS_BGD_WRITE_MULTIPLE_REGISTERS;
		uint8_t reply[MAX_REGISTERS+3];
		/*
			# Modbus protocol
		*/
		reply[0] = info_data.transaction_id >> 8;
		reply[1] = info_data.transaction_id & 0xff;
		reply[2] = 0x00;
		reply[3] = 0x00;
		reply[4] = 0x00;
		reply[5] = 0x06; //number of registers + the other 3 below
		reply[6] = info_data.unit_id;
		reply[7] = info_data.function_code;
		reply[8] = info_data.start_register >> 8;
		reply[9] =  info_data.start_register & 0xff;
		reply[10] = info_data.number_requested >> 8;
		reply[11] = info_data.number_requested & 0xff;
		Modbus_bgd::send_reply(new_sd,reply);
	} else {
		std::cout << "Something went wrong while writing multiple registers..." << std::endl;
	}
}

void Modbus_bgd::send_reply(int new_sd, uint8_t sending_data_buffer[MAX_REGISTERS+3]) {
	send(new_sd, sending_data_buffer, (sending_data_buffer[5]+6),0); /*not 9 but 6*/
}