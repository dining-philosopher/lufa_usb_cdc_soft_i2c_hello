/*
             LUFA Library
     Copyright (C) Dean Camera, 2014.

  dean [at] fourwalledcubicle [dot] com
           www.lufa-lib.org
*/

/*
  Copyright 2014  Dean Camera (dean [at] fourwalledcubicle [dot] com)

  Permission to use, copy, modify, distribute, and sell this
  software and its documentation for any purpose is hereby granted
  without fee, provided that the above copyright notice appear in
  all copies and that both that the copyright notice and this
  permission notice and warranty disclaimer appear in supporting
  documentation, and that the name of the author not be used in
  advertising or publicity pertaining to distribution of the
  software without specific, written prior permission.

  The author disclaims all warranties with regard to this
  software, including all implied warranties of merchantability
  and fitness.  In no event shall the author be liable for any
  special, indirect or consequential damages or any damages
  whatsoever resulting from loss of use, data or profits, whether
  in an action of contract, negligence or other tortious action,
  arising out of or in connection with the use or performance of
  this software.
*/

/** \file
 *
 *  Main source file for the VirtualSerial demo. This file contains the main tasks of
 *  the demo and is responsible for the initial application hardware configuration.
 */

#include "VirtualSerial.h"


#include "i2csoft.h"


/** LUFA CDC Class driver interface configuration and state information. This structure is
 *  passed to all CDC Class driver functions, so that multiple instances of the same class
 *  within a device can be differentiated from one another.
 */
USB_ClassInfo_CDC_Device_t VirtualSerial_CDC_Interface =
	{
		.Config =
			{
				.ControlInterfaceNumber   = INTERFACE_ID_CDC_CCI,
				.DataINEndpoint           =
					{
						.Address          = CDC_TX_EPADDR,
						.Size             = CDC_TXRX_EPSIZE,
						.Banks            = 1,
					},
				.DataOUTEndpoint =
					{
						.Address          = CDC_RX_EPADDR,
						.Size             = CDC_TXRX_EPSIZE,
						.Banks            = 1,
					},
				.NotificationEndpoint =
					{
						.Address          = CDC_NOTIFICATION_EPADDR,
						.Size             = CDC_NOTIFICATION_EPSIZE,
						.Banks            = 1,
					},
			},
	};

/** Standard file stream for the CDC interface when set up, so that the virtual CDC COM port can be
 *  used like any regular character stream in the C APIs.
 */
static FILE USBSerialStream;

static FILE * USBin; // usb command input buffer

int usb_get(FILE * f){
    int recv = -1;
    while ((recv < 0) | (recv > 255)) recv = CDC_Device_ReceiveByte(&VirtualSerial_CDC_Interface);
    return recv;
}


/** Main program entry point. This routine contains the overall program flow, including initial
 *  setup of all components and the main program loop.
 */
int main(void)
{
	SetupHardware();

	/* Create a regular character stream for the interface so that it can be used with the stdio.h functions */
	// CDC_Device_CreateStream(&VirtualSerial_CDC_Interface, &USBSerialStream);
	CDC_Device_CreateBlockingStream(&VirtualSerial_CDC_Interface, &USBSerialStream);
        
        // USBin = fdevopen(NULL, usb_get); // open usb command input buffer

	// LEDs_SetAllLEDs(LEDMASK_USB_NOTREADY);
	GlobalInterruptEnable();

	for (;;)
	{
		MainFunction();

		/* Must throw away unused bytes from the host, or it will lock up while waiting for the device */
		// CDC_Device_ReceiveByte(&VirtualSerial_CDC_Interface);

		CDC_Device_USBTask(&VirtualSerial_CDC_Interface);
		USB_USBTask();
	}
}

/** Configures the board hardware and chip peripherals for the demo's functionality. */
void SetupHardware(void)
{
#if (ARCH == ARCH_AVR8)
	/* Disable watchdog if enabled by bootloader/fuses */
	MCUSR &= ~(1 << WDRF);
	wdt_disable();

	/* Disable clock division */
	clock_prescale_set(clock_div_1);
#elif (ARCH == ARCH_XMEGA)
	/* Start the PLL to multiply the 2MHz RC oscillator to 32MHz and switch the CPU core to run from it */
	XMEGACLK_StartPLL(CLOCK_SRC_INT_RC2MHZ, 2000000, F_CPU);
	XMEGACLK_SetCPUClockSource(CLOCK_SRC_PLL);

	/* Start the 32MHz internal RC oscillator and start the DFLL to increase it to 48MHz using the USB SOF as a reference */
	XMEGACLK_StartInternalOscillator(CLOCK_SRC_INT_RC32MHZ);
	XMEGACLK_StartDFLL(CLOCK_SRC_INT_RC32MHZ, DFLL_REF_INT_USBSOF, F_USB);

	PMIC.CTRL = PMIC_LOLVLEN_bm | PMIC_MEDLVLEN_bm | PMIC_HILVLEN_bm;
#endif

	/* Hardware Initialization */
        SoftI2CInit(); // real time clock M41T56M6
        
	USB_Init();
}




// M41T56M6 real time clock definitions

#define M41T56M6_SLA_W 0xD0
#define M41T56M6_SLA_R 0xD1

/*
0 SECONDS
1 MINUTES
2 CENTURY/HOURS
3 DAY
4 DATE
5 MONTH
6 YEAR
7 CONTROL
8 RAM 56*8
*/

uint8_t bcd2bin(uint8_t a) { // binary coded decimal conversion
    return (a & 15) + ((a >> 4) * 10);
}

uint8_t bin2bcd(uint8_t a) { // binary coded decimal conversion
    return (a % 10) | ((a / 10) << 4);
}

bool M41T56M6Read(uint8_t address, uint8_t * data) {
    return SoftI2CRead(M41T56M6_SLA_R, address, data);
}

bool M41T56M6Write(uint8_t address,uint8_t data) {
    return SoftI2CWrite(M41T56M6_SLA_W, address, data);
}

// time operations

bool rtc_get_seconds(uint8_t * data) {
    bool res = M41T56M6Read(0, data);
    if (res) *data = bcd2bin(*data);
    return res;
}

bool rtc_get_minutes(uint8_t * data) {
    bool res = M41T56M6Read(1, data);
    if (res) *data = bcd2bin(*data);
    return res;
}

bool rtc_set_seconds(uint8_t data) {
    return M41T56M6Write(0, bin2bcd(data));
}

bool rtc_set_minutes(uint8_t data) {
    return M41T56M6Write(1, bin2bcd(data));
}


// end of M41T56M6 real time clock definitions






/* Main loop. */
void MainFunction(void)
{
	// uint8_t     JoyStatus_LCL = Joystick_GetStatus();
	char*       ReportString  = NULL;
	char        buf[127];
	static bool ActionSent    = false; // not needed???????????????
/*
	if (JoyStatus_LCL & JOY_UP)
	  ReportString = "Joystick Up\r\n";
	else if (JoyStatus_LCL & JOY_DOWN)
	  ReportString = "Joystick Down\r\n";
	else if (JoyStatus_LCL & JOY_LEFT)
	  ReportString = "Joystick Left\r\n";
	else if (JoyStatus_LCL & JOY_RIGHT)
	  ReportString = "Joystick Right\r\n";
	else if (JoyStatus_LCL & JOY_PRESS)
	  ReportString = "Joystick Pressed\r\n";
	else
	  ActionSent = false;
*/
        /*
        if (CDC_Device_ReceiveByte(&VirtualSerial_CDC_Interface) > 0){
            ReportString = "echo!\n";
            ActionSent = false;
        };
        */
        
        int addr = 0; // read or write address
        int data = 0;
        char * token = NULL;

        ReportString  = fgets(buf, sizeof(buf), &USBSerialStream); // read command from host
        // ReportString  = fgets(buf, sizeof(buf), USBin);
        
        token = strtok (ReportString, " \r\n");
        
        // switch (ReportString[0]) {
        // switch (token) {
            // case "r":
            if (strcmp(token, "r") == 0) {
                addr = atoi(strtok (NULL, " \r\n"));
                // addr = atoi(&ReportString[1]);
                if (M41T56M6Read(addr, &data))
                    sprintf(ReportString, "clock [%d] = %d\r\n", addr, data);
                // ReportString = itoa(data, buf, 10);
                else
                    sprintf(ReportString, "Failure reading clock [%d]\r\n", addr);
                // break;
            // case "w":
            } else if (strcmp(token, "w") == 0) {
                token = strtok (NULL, " \r\n");
                // token = strtok (&ReportString[1], " ");
                addr = atoi(token);
                token = strtok (NULL, " \r\n");
                data = atoi(token);
                if ((token != NULL) && M41T56M6Write(addr, data))
                    // ReportString = "Success";
                    sprintf(ReportString, "Success writing %d into clock [%d]\r\n", data, addr);
                else
                    // ReportString = "Failure";
                    if (token != NULL)
                        sprintf(ReportString, "Failure writing %d into clock [%d]\r\n", data, addr);
                    else
                        sprintf(ReportString, "You forgot address or data!\r\n");
                // break;
            // case "sec":
            } else if (strcmp(token, "sec") == 0) {
                token = strtok (NULL, " \r\n");
                data = atoi(token);
                if (token != NULL)
                    if (rtc_set_seconds(data))
                        sprintf(ReportString, "Seconds are now set to %d\r\n", data);
                    else
                        sprintf(ReportString, "Error setting seconds");
                else
                    if (rtc_get_seconds(&data))
                        sprintf(ReportString, "It's %d seconds o'clock\r\n", data);
                    else
                        sprintf(ReportString, "Unable to get seconds");
                // break;
            // case "min":
            } else if (strcmp(token, "min") == 0) {
                token = strtok (NULL, " \r\n");
                data = atoi(token);
                if (token != NULL)
                    if (rtc_set_minutes(data))
                        fprintf(&USBSerialStream, "Seconds are now set to %d\r\n", data); // works too!
                        // sprintf(ReportString, "Seconds are now set to %d\r\n", data);
                    else
                        sprintf(ReportString, "Error setting minutes");
                else
                    if (rtc_get_minutes(&data))
                        sprintf(ReportString, "It's %d minutes o'clock\r\n", data);
                    else
                        sprintf(ReportString, "Unable to get minutes");
                // break;
            // default:
            } else {
                
            }    
                // break;
        // }

        ActionSent = false;
        
	if ((ReportString != NULL) && (ActionSent == false))
	{
		ActionSent = true;

		/* Write the string to the virtual COM port via the created character stream */
		fputs(ReportString, &USBSerialStream);

		/* Alternatively, without the stream: */
		// CDC_Device_SendString(&VirtualSerial_CDC_Interface, ReportString);
	}
}

/** Event handler for the library USB Connection event. */
void EVENT_USB_Device_Connect(void)
{
	// LEDs_SetAllLEDs(LEDMASK_USB_ENUMERATING);
}

/** Event handler for the library USB Disconnection event. */
void EVENT_USB_Device_Disconnect(void)
{
	// LEDs_SetAllLEDs(LEDMASK_USB_NOTREADY);
}

/** Event handler for the library USB Configuration Changed event. */
void EVENT_USB_Device_ConfigurationChanged(void)
{
	bool ConfigSuccess = true;

	ConfigSuccess &= CDC_Device_ConfigureEndpoints(&VirtualSerial_CDC_Interface);

	// LEDs_SetAllLEDs(ConfigSuccess ? LEDMASK_USB_READY : LEDMASK_USB_ERROR);
}

/** Event handler for the library USB Control Request reception event. */
void EVENT_USB_Device_ControlRequest(void)
{
	CDC_Device_ProcessControlRequest(&VirtualSerial_CDC_Interface);
}

