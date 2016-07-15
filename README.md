# bgd_modbus V1.0
# A simple modbus slave
  Website: bgddev.com
# 
After download you can compile the main file to test it 
  g++ main-test.cpp -o modbus_bgd -std=c++11 -pthread


Supports the following functions:

 ReadCoilStatus FC=01
 ReadHoldingRegisters FC=03
 WriteSingleCoil FC=05
 WriteSingleRegister FC=06
 ForecMultipleCoils FC=15
 WriteMultipleRegisters FC=16


To read and write data inside the registers you have to use MB_MEM, is a static uint16_t two-dimensional array, and this is how it works:
 MB_MEM[slave id][register id]