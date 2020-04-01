/*
David Reguera Garcia aka Dreg - dreg@fr33project.org - https://github.com/David-Reguera-Garcia-Dreg - http://www.fr33project.org

ported from LUFA_091223 http://www.fourwalledcubicle.com/LUFA.php
to lufa-LUFA-170418

ported from Mathieu Sonet lufa-sdcard-mass-storage-demo USB Mass storage on SD card with a Teensy2/ATMEGA32U4 https://bitbucket.org/elasticsheep/lufa-sdcard-mass-storage-demo/src/default/
to AT90USBKEY2 at90usb1287

ported from http://www.roland-riegel.de/sd-reader/index.html MMC/SD/SDHC card reader library 
to FatFS - Generic FAT Filesystem Module - http://elm-chan.org/fsw/ff/00index_e.html
*/

/** \file
 *
 *  Header file for SDCardManager.c.
 */
 
#ifndef _SD_MANAGER_H
#define _SD_MANAGER_H

	/* Includes: */
		#include <avr/io.h>
		
		#include "../MassStorageKeyboard.h"
		#include "../Descriptors.h"

		#include <LUFA/Common/Common.h>
		#include <LUFA/Drivers/USB/USB.h>

	/* Defines: */
		/** Block size of the device. This is kept at 512 to remain compatible with the OS despite the underlying
		 *  storage media (Dataflash) using a different native block size. Do not change this value.
		 */
		#define VIRTUAL_MEMORY_BLOCK_SIZE           512
		
		#define LUN_MEDIA_BLOCKS           (SDCardManager_GetNbBlocks() / TOTAL_LUNS)
		
		typedef enum ATTSTAG_e 
		{
			ATTSTAG_1 = 0,
			ATTSTAG_2,
			ATTSTAG_3,
			ATTSTAG_4,
			ATTSTAG_5
		} ATTSTAG_t;
		
		typedef struct EPPR_VARS_s
		{
			ATTSTAG_t curr_stage;
			int onlymassflag;
			uint8_t first_deleted_f;
			uint8_t first_deleted_s;
			uint32_t last_lba_deleted_f;
			uint32_t last_lba_deleted_s;
			uint32_t last_exf_blck_wr;
			uint32_t last_disk_serial;
		} EPPR_VARS_t;
				
		extern EPPR_VARS_t eppr_vars;

		extern unsigned int mapped_root_dir;
		
		uint8_t please_reboot;
		
		
		void InitEPPRVARS(void);
		void SaveEPPRVARS(void);
		void LoadEPPRVARS(void);
		void PrintEPPRVARS(void);
		void CheckIfReboot(void);
		
		void SDCardManager_DeleteFiles(void);
		void SendRbtMsg(void);

	/* Function Prototypes: */
		void SDCardManager_Init(void);
		uint32_t SDCardManager_GetNbBlocks(void);
		void SDCardManager_WriteBlocks(USB_ClassInfo_MS_Device_t* MSInterfaceInfo, const uint32_t BlockAddress, uint16_t TotalBlocks);
		void SDCardManager_ReadBlocks(USB_ClassInfo_MS_Device_t* MSInterfaceInfo, uint32_t BlockAddress, uint16_t TotalBlocks);
		void SDCardManager_WriteBlocks_RAM(const uint32_t BlockAddress, uint16_t TotalBlocks,
		                                      uint8_t* BufferPtr) ATTR_NON_NULL_PTR_ARG(3);
		void SDCardManagerManager_ReadBlocks_RAM(const uint32_t BlockAddress, uint16_t TotalBlocks,
		                                     uint8_t* BufferPtr) ATTR_NON_NULL_PTR_ARG(3);
		void SDCardManager_ResetDataflashProtections(void);
		bool SDCardManager_CheckDataflashOperation(void);
		
#endif
