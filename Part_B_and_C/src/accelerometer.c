#include "SPI.h"
#include "accelerometer.h"
#include <stdio.h>

void accWrite(uint8_t addr, uint8_t val){
	// TODO access SPI_Transfer_Data
	// Write Bit = 0, MB Bit = 0
	// To make addr 6 bits
	uint8_t addr_6bit = addr << 2;
	addr_6bit = addr_6bit >> 2;
	
	//uint16_t write_data1 = 0b00 << 14;
	//uint16_t write_data2 = addr << 8;
	//uint16_t write_data3 = val;
	//uint16_t write_data = write_data1 | write_data2 | write_data3;
	uint16_t write_data = ((uint16_t)addr_6bit << 8);
	// write_data &= 0xFF00;
	write_data = write_data | val;
	// send data byte
	SPI_Transfer_Data(write_data);
}

uint8_t accRead(uint8_t addr){
	// access SPI_Transfer_Data
	// Read Bit = 1, MB Bit = 0
	// To make addr 6 bits
	// TODO: Is this shift good?
	uint8_t addr_6bit = addr << 2;
	addr_6bit = addr_6bit >> 2;
	
	//uint16_t write_data1 = 0b10 << 14;
	//uint16_t write_data2 = addr_6bit << 8;
	//uint16_t write_data3 = 0;
	//uint16_t write_data = write_data1 | write_data2 | write_data3;
	uint16_t write_data = ((uint16_t)0b10) << 14;
	write_data = write_data | ((uint16_t)addr_6bit << 8);
	//write_data &= 0xFF00;
	// send data byte
	uint16_t read_data = SPI_Transfer_Data(write_data);
	//read_data |= 0x00FF;
	uint8_t read_data1 = read_data;
	return read_data1; // TODO
}

void initAcc(void){
	// set 100Hz output data rate
	accWrite(0x2C, 0x0A);
	// set full resolution mode
	accWrite(0x31, 0x08);
	// enable measurement
	accWrite(0x2D, 0x08);
}

void readValues(double* x, double* y, double* z){
	// TODO
	// find scaler from data sheet
	double scaler = 3.9 / 1000;
	// read values into x,y,z using accRead
	int16_t x0 = accRead(0x32);
	int16_t x1 = accRead(0x33);
	int16_t y0 = accRead(0x34);
	int16_t y1 = accRead(0x35);
	int16_t z0 = accRead(0x36);
	int16_t z1 = accRead(0x37);
	

	int16_t x_o = (x1 << 8) | x0;
	int16_t y_o = (y1 << 8) | y0;
	int16_t z_o = (z1 << 8) | z0;
	
	*x = scaler * x_o;
	*y = scaler * y_o;
	*z = scaler * z_o;
}
