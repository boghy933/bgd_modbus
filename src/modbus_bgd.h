/*
	# Maximum number of simultaneous connections
*/
#define MAX_CON 8


/*
	# Number of slaves and registers per slave
*/
#define SERVER_SLAVE 255
#define NUMBER_OF_REGISTERS 65535 

/*
	# Function code
*/

#define MODBUS_BGD_READ_COILS 			  	0x01
#define MODBUS_BGD_READ_HOLDING_REGISTERS 	0x03
#define MODBUS_BGD_WRITE_SINGLE_COIL 	  	0X05
#define MODBUS_BGD_WRITE_SINGLE_REGISTER  	0x06
#define MODBUS_BGD_WRITE_MULTIPLE_COILS   	0x0F
#define MODBUS_BGD_WRITE_MULTIPLE_REGISTERS 0X10


/* 
	# Max number of registers to read in one query 
*/
#define MAX_REGISTERS 150 


/*
	# Modbus registers
*/
static uint16_t MB_MEM[SERVER_SLAVE][NUMBER_OF_REGISTERS];

/*

	# Modbus protocol
		# 12 bytes recieved :

		# 0 -> [0] //transaction id
		# 1 -> [7] //transaction id

		# 2 -> [0] //protocol id
		# 3 -> [0] //protocol id

		# 4 -> [0] //message length
		# 5 -> [6] //message length

		# 6 -> [11] //unit identifier / slave

		# 7 -> [1] //function code

		# 8 -> [ff] //data addres of first register
		# 9 -> [dc] //data addres of first register

		# 10 -> [0]  //number of read registers
		# 11 -> [14] //number of read registers
*/
/*
	#	Structure used to handle requests
*/
struct tcp_request {
	int transaction_id;
	int protocol_id;
	int message_length;
	int unit_id;
	int function_code;
	int start_register;
	int number_requested;
	uint8_t values[500];
};


class Modbus_bgd {
public:
	/*
		# Call this member function in your program to start the modbus slave.
		@Param: number of listening port
		@Default = 502
	*/
	void begin_listening(std::string port = "502");

	/*
		# This member function is used to bind togheder the begin_listening and listening member function.
		# I use in this way to not make the listening member function static due to the thread function that require a static member.
		# Maybe there are better ways, but i don't know how.
		@Param: number of listening port
	*/
	static void listening_function(std::string port);

	/*
		# Member function used to set all registers to 0
	*/
	void init_registers();

	/*
		# Waiting for connection and open new socket thread for each one.

		@Param: number of listening port

		# This member function is used by the begin_listening, 
		# no need to call it again.
	*/
	bool listening(std::string port);

	/*
		# Used to bind togheder listening and listening_socket, 
		# I use it for the same reason i use one upper.
		@Param: socked fd
	
	*/
	static void socket_thread(int new_sd);


	/*
		# Listening for connections on the socket
		@Param: socked fd
	*/
	void listening_socket(int new_sd);
	
	/*
		# Once we get data on the socket this function is called to analyze data.
		@Params: data buffer, number of bytes received, socket fd
	*/
	bool request(uint8_t incoming_data_buffer[100], ssize_t bytes_received, int new_sd);

	/*
		# Used to select the right modbus function
		@Params: query data, socket fd
	*/
	bool query_function(tcp_request info_data, int new_sd);

	/*
		# Member function to read coil status
		# FC = 01
		@Params: query data, socket fd
	*/
	void ReadCoilStatus(tcp_request info_data, int new_sd); 

	/*
		# Member function to read holding registers
		# FC = 03
		@Params: query data, socket fd
	*/
	void ReadHoldingRegisters(tcp_request info_data, int new_sd);

	/*
		# Member function to Write single coil
		# FC = 05
		@Params: query data, socket fd
	*/
	void ForceSingleCoil(tcp_request info_data, int new_sd);

	/*
		# Member function to Write single register
		# FC = 06
		@Params: query data, socket fd
	*/
	void WriteSingleRegister(tcp_request info_data, int new_sd);

	/*
		# Member function to Write multiple coils
		# FC = 15
		@Params: query data, socket fd
	*/
	void ForceMultipleCoils(tcp_request info_data, int new_sd);

	/*
		# Member function to Write multiple registers
		# FC = 16
		@Params: query data, socket fd
	*/
	void WriteMultipleRegisters(tcp_request info_data, int new_sd);

	/*
		# Member function used to send data buffer
		@Params: socket fd, buffer
	*/
	void send_reply(int new_sd, uint8_t sending_data_buffer[12]);
};