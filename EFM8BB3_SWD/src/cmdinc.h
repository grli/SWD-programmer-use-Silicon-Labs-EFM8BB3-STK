#ifndef CMDINC_H
#define CMDINC_H

typedef bit BOOL;
typedef union
{                             // Keil stores longs big-endian.  This allows
   unsigned char Byte[4];     // access to unsigned longs one byte at a time.
   unsigned int Int[2];
   unsigned long Long;
}
BE_LONG;

#if EFM8UB2
#include <SI_EFM8UB2_Register_Enums.h>
#include "si_efm8ub2_defs.h"
#endif

#if EFM8BB3
#include <SI_EFM8BB3_Register_Enums.h>
#include "si_efm8bb3_defs.h"
#endif

#include "stdio.h"
#include "string.h"
#include "devicefun.h"
#include "swdprotocol.h"
#if EFM8UB2
#include "lcd.h"
#endif
extern void delayms( int nms );

#define NO_WAIT_FINISH			0
#define WAIT_FINISH				1

#define NO_AUTO_INC				0
#define AUTO_INC				1

#define TRUE    1
#define FALSE   0

#define COMMAND_LEN  60
extern xdata char cmd_buf[15], cmd[COMMAND_LEN];
extern uint32_t g_var1, g_var2, g_var3, g_copysize, g_varnum;
extern uint16_t g_FlashSize, g_PageSize, g_RamSize, g_UserDataSize, g_ProductionID;
extern uint8_t g_protocol_error;

extern void SPI_writeenable( void );
extern void SPI_chiperase( void );
extern void SPI_readid(void);
extern void SPI_read( uint32_t saddr, uint32_t size );
extern void SPI_writeword( uint32_t saddr, uint32_t dat );
extern void SPI_blank( uint32_t saddr, uint32_t size );
extern BOOL SPI_checksum( uint32_t saddr, uint32_t size, uint32_t *checksum );
extern BOOL SPI_download( uint32_t saddr, uint32_t size );
extern BOOL CopySPIToFlash( uint32_t spisaddr, uint32_t flashaddr, uint32_t size );

#endif
