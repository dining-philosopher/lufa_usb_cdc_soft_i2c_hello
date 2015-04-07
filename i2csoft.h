/**********************************************************

Software I2C Library for AVR Devices.

Copyright 2008-2012
eXtreme Electronics, India
www.eXtremeElectronics.co.in

modified by wormball https://github.com/dining-philosopher

**********************************************************/


#ifndef _I2CSOFT_H
#define _I2CSOFT_H

/* 
I/O Configuration 
*/

#define SOFT_I2C_SDA_PORT F
#define SOFT_I2C_SCL_PORT F
#define SOFT_I2C_SDA_LEG  0
#define SOFT_I2C_SCL_LEG  1

/*
Ports and pins should be defined. For example this is what we should define if we want PF0 leg to be SDA and PF1 leg to be SCL:

#define SOFT_I2C_SDA_PORT F
#define SOFT_I2C_SCL_PORT F
#define SOFT_I2C_SDA_LEG  0
#define SOFT_I2C_SCL_LEG  1
*/

// A subtle side effect of the ## operator is that it inhibits expansion.
// from https://github.com/pfultz2/Cloak/wiki/C-Preprocessor-tricks,-tips,-and-idioms
#define CAT(a, ...) PRIMITIVE_CAT(a, __VA_ARGS__)
#define PRIMITIVE_CAT(a, ...) a ## __VA_ARGS__

#define SCLPORT	CAT(PORT,SOFT_I2C_SCL_PORT)
#define SCLDDR	CAT(DDR, SOFT_I2C_SCL_PORT)

#define SDAPORT	CAT(PORT, SOFT_I2C_SDA_PORT)
#define SDADDR	CAT(DDR, SOFT_I2C_SDA_PORT)

#define SDAPIN	CAT(PIN, SOFT_I2C_SDA_PORT)
#define SCLPIN	CAT(PIN, SOFT_I2C_SCL_PORT)

#define SCL	CAT(P, CAT(SOFT_I2C_SCL_PORT, SOFT_I2C_SCL_LEG))		//PORTD.0 PIN AS SCL PIN
#define SDA	CAT(P, CAT(SOFT_I2C_SDA_PORT, SOFT_I2C_SDA_LEG))		//PORTD.1 PIN AS SDA PIN

/*
#define SCLPORT	PORTF	//TAKE PORTF as SCL OUTPUT WRITE
#define SCLDDR	DDRF	//TAKE DDRF as SCL INPUT/OUTPUT configure

#define SDAPORT	PORTF	//TAKE PORTF as SDA OUTPUT WRITE
#define SDADDR	DDRF	//TAKE PORTF as SDA INPUT configure

#define SDAPIN	PINF	//TAKE PORTF TO READ DATA
#define SCLPIN	PINF	//TAKE PORTF TO READ DATA

#define SCL	PF1		//PORTF.0 PIN AS SCL PIN
#define SDA	PF0		//PORTF.1 PIN AS SDA PIN
*/

#define SOFT_I2C_SDA_LOW	SDADDR|=((1<<SDA))
#define SOFT_I2C_SDA_HIGH	SDADDR&=(~(1<<SDA))

#define SOFT_I2C_SCL_LOW	SCLDDR|=((1<<SCL))
#define SOFT_I2C_SCL_HIGH	SCLDDR&=(~(1<<SCL))


/**********************************************************
SoftI2CInit()

Description:
	Initializes the Soft I2C Engine.
	Must be called before using any other lib functions.
	
Arguments:
	NONE
	
Returns:
	Nothing

**********************************************************/
void SoftI2CInit();	

/**********************************************************
SoftI2CStart()

Description:
	Generates a START(S) condition on the bus.
	NOTE: Can also be used for generating repeat start(Sr)
	condition too.
	
Arguments:
	NONE
	
Returns:
	Nothing

**********************************************************/
void SoftI2CStart();

/**********************************************************
SoftI2CStop()

Description:
	Generates a STOP(P) condition on the bus.
	NOTE: Can also be used for generating repeat start
	condition too.
	
Arguments:
	NONE
	
Returns:
	Nothing

**********************************************************/
void SoftI2CStop();

/**********************************************************
SoftI2CWriteByte()

Description:
	Sends a Byte to the slave.
	
Arguments:
	8 bit date to send to the slave.
	
Returns:
	non zero if slave acknowledge the data receipt.
	zero other wise.

**********************************************************/
uint8_t SoftI2CWriteByte(uint8_t data);

/**********************************************************
SoftI2CReadByte()

Description:
	Reads a byte from slave.
	
Arguments:
	1 if you want to acknowledge the receipt to slave.
	0 if you don't want to acknowledge the receipt to slave.
	
Returns:
	The 8 bit data read from the slave.

**********************************************************/
uint8_t SoftI2CReadByte(uint8_t ack);



/***************************************************

Function To Read Internal Register of device connected via software I2C
---------------------------------------------

address : Address of the register
data: value of register is copied to this.


Returns:
0= Failure
1= Success
***************************************************/

bool SoftI2CRead(uint8_t device_address, uint8_t address,uint8_t *data);


/***************************************************

Function To Write Internal Register of device connected via software I2C

---------------------------------------------

address : Address of the register
data: value to write.


Returns:
0= Failure
1= Success
***************************************************/

bool SoftI2CWrite(uint8_t device_address, uint8_t address, uint8_t data);

#endif 
