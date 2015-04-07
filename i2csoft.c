/**********************************************************

Software I2C Library for AVR Devices.

Copyright 2008-2012
eXtreme Electronics, India
www.eXtremeElectronics.co.in
**********************************************************/
 
#include <avr/io.h>
#include <util/delay.h>
#include <stdbool.h>

#include "i2csoft.h"

#define Q_DEL _delay_loop_2(3)
#define H_DEL _delay_loop_2(5)

void SoftI2CInit()
{
	SDAPORT&=(1<<SDA);
	SCLPORT&=(1<<SCL);
	
	SOFT_I2C_SDA_HIGH;	
	SOFT_I2C_SCL_HIGH;	
		
}
void SoftI2CStart()
{
	SOFT_I2C_SCL_HIGH;
	H_DEL;
	
	SOFT_I2C_SDA_LOW;	
	H_DEL;  	
}

void SoftI2CStop()
{
	 SOFT_I2C_SDA_LOW;
	 H_DEL;
	 SOFT_I2C_SCL_HIGH;
	 Q_DEL;
	 SOFT_I2C_SDA_HIGH;
	 H_DEL;
}

uint8_t SoftI2CWriteByte(uint8_t data)
{
	 
	 uint8_t i;
	 	
	 for(i=0;i<8;i++)
	 {
		SOFT_I2C_SCL_LOW;
		Q_DEL;
		
		if(data & 0x80)
			SOFT_I2C_SDA_HIGH;
		else
			SOFT_I2C_SDA_LOW;	
		
		H_DEL;
		
		SOFT_I2C_SCL_HIGH;
		H_DEL;
		
		while((SCLPIN & (1<<SCL))==0);
			
		data=data<<1;
	}
	 
	//The 9th clock (ACK Phase)
	SOFT_I2C_SCL_LOW;
	Q_DEL;
		
	SOFT_I2C_SDA_HIGH;		
	H_DEL;
		
	SOFT_I2C_SCL_HIGH;
	H_DEL;	
	
	uint8_t ack=!(SDAPIN & (1<<SDA));
	
	SOFT_I2C_SCL_LOW;
	H_DEL;
	
	return ack;
	 
}
 
 
uint8_t SoftI2CReadByte(uint8_t ack)
{
	uint8_t data=0x00;
	uint8_t i;
			
	for(i=0;i<8;i++)
	{
			
		SOFT_I2C_SCL_LOW;
		H_DEL;
		SOFT_I2C_SCL_HIGH;
		H_DEL;
			
		while((SCLPIN & (1<<SCL))==0);
		
		if(SDAPIN &(1<<SDA))
			data|=(0x80>>i);
			
	}
		
	SOFT_I2C_SCL_LOW;
	Q_DEL;						//Soft_I2C_Put_Ack
	
	if(ack)
	{
		SOFT_I2C_SDA_LOW;	
	}
	else
	{
		SOFT_I2C_SDA_HIGH;
	}	
	H_DEL;
	
	SOFT_I2C_SCL_HIGH;
	H_DEL;
	
	SOFT_I2C_SCL_LOW;
	H_DEL;
			
	return data;
	
}



/***************************************************

Function To Read Internal Registers of device connected via software I2C
---------------------------------------------

address : Address of the register
data: value of register is copied to this.


Returns:
0= Failure
1= Success
***************************************************/

bool SoftI2CRead(uint8_t device_address, uint8_t address,uint8_t *data)
{
   uint8_t res;   //result

   //Start

   SoftI2CStart();

   //SLA+W (for dummy write to set register pointer)
   // res=SoftI2CWriteByte(M41T56M6_SLA_W); //M41T56M6 address + W
   res=SoftI2CWriteByte(device_address & 0xfe); //device address + W, 0xfe = 11111110

   //Error
   if(!res) return false;

   //Now send the address of required register

   res=SoftI2CWriteByte(address);

   //Error
   if(!res) return false;

   //Repeat Start
   SoftI2CStart();

   //SLA + R
   // res=SoftI2CWriteByte(M41T56M6_SLA_R); //M41T56M6 Address + R
   res=SoftI2CWriteByte(device_address | 1); //device address + R

   //Error
   if(!res) return false;

   //Now read the value with NACK
   *data=SoftI2CReadByte(0);

   //Error

   if(!res) return false;

   //STOP
   SoftI2CStop();

   // return TRUE;
   return true;
}

/***************************************************

Function To Write Internal Registers of device connected via software I2C

---------------------------------------------

address : Address of the register
data: value to write.


Returns:
0= Failure
1= Success
***************************************************/

bool SoftI2CWrite(uint8_t device_address, uint8_t address, uint8_t data)
{
   uint8_t res;   //result

   //Start
   SoftI2CStart();

   //SLA+W
   // res=SoftI2CWriteByte(M41T56M6_SLA_W); //M41T56M6 address + W
   res=SoftI2CWriteByte(device_address & 0xfe); //device address + W, 0xfe = 11111110

   //Error
   if(!res) return false;

   //Now send the address of required register
   res=SoftI2CWriteByte(address);

   //Error
   if(!res) return false;

   //Now write the value

   res=SoftI2CWriteByte(data);

   //Error
   if(!res) return false;

   //STOP
   SoftI2CStop();

   return true;
}

// TODO: write functions reading and writing strings of multiple bytes

