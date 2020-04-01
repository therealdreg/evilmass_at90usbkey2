/*
David Reguera Garcia aka Dreg - dreg@fr33project.org - https://github.com/David-Reguera-Garcia-Dreg - http://www.fr33project.org

ported from LUFA_091223 http://www.fourwalledcubicle.com/LUFA.php
to lufa-LUFA-170418

ported from Mathieu Sonet lufa-sdcard-mass-storage-demo USB Mass storage on SD card with a Teensy2/ATMEGA32U4 https://bitbucket.org/elasticsheep/lufa-sdcard-mass-storage-demo/src/default/
to AT90USBKEY2 at90usb1287

ported from http://www.roland-riegel.de/sd-reader/index.html MMC/SD/SDHC card reader library 
to FatFS - Generic FAT Filesystem Module - http://elm-chan.org/fsw/ff/00index_e.html
*/

/*-----------------------------------------------------------------------
/  Low level disk interface modlue include file   (C)ChaN, 2016
/-----------------------------------------------------------------------*/

#ifndef _MMC_DEFINED
#define _MMC_DEFINED

#include "diskio.h"

#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------------------*/
/* Prototypes for disk control functions */

DSTATUS mmc_disk_initialize (void);
DSTATUS mmc_disk_status (void);
DRESULT mmc_disk_read (BYTE* buff, DWORD sector, UINT count);
DRESULT mmc_disk_write (const BYTE* buff, DWORD sector, UINT count);
DRESULT mmc_disk_ioctl (BYTE cmd, void* buff);
void mmc_disk_timerproc (void);

#ifdef __cplusplus
}
#endif

#endif
