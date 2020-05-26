#include "cmdinc.h"
#include "ChipDefine.h"
#include "swdprotocol.h"

uint32_t flash_code[512] =
{
#if BLUE_NRG
#include "c:\\FlashRam_BlueNrg\\FlashRam.txt"
#else
#include "c:\\FlashRam\\FlashRam.txt"
#endif
};

uint16_t g_FlashSize, g_RamSize, g_PageSize, g_UserDataSize, g_ProductionID;
unsigned char g_family;

volatile uint32_t mscWriteCtrl;
volatile uint32_t mscWriteCmd;
volatile uint32_t mscAddrb;
volatile uint32_t mscWdata;
volatile uint32_t mscStatusReg;
volatile uint32_t mscMassLock;
volatile uint32_t mscLock;

volatile uint32_t flashctrltype;

BOOL LoadFlashRam( void )
{
	int i;
	printf("LoadFlashRam");

	g_protocol_error = SW_ACK_OK;
	for( i = 0; i < sizeof(flash_code); i+=4 )
	{
		writeMem(0x20004000 + i , flash_code[i>>2]);
		if( SW_ACK_OK != g_protocol_error )
		{
			return false;
		}
	}

	writeCpuReg(13, flash_code[0]);
	writeCpuReg(15, flash_code[1]);
	printf(" finished\n");
	return true;
}

BOOL CheckBootloader( void )
{
	uint32_t i;

	readMem( REMOTE_bebugcounter );
	i = io_word.Long;
	printf("FlashRam counter T0 = %08lx, ", io_word.Long);

	readMem( REMOTE_bebugcounter );
	printf("T1 = %08lx, ", io_word.Long);

	readMem( REMOTE_bebugcounter );
	printf("T2 = %08lx, ", io_word.Long);

	if( i == io_word.Long )
	{
		printf("is not running\n");
		return false;
	}
	else
	{
		printf("is running\n");
	}

	return true;
}

/* MSC register base address and offset */
#define MSCBASE_ADDR_P1         0x400c0000  // Gecko, GG, HG, LG, TG, WG, ZG, EFR32
#define MSCBASE_ADDR_P2         0x400e0000  // JP1, JG12, PG1, PG12, xG12, xG13, xG14
#define MSCBASE_ADDR_P3         0x40000000  // GG11, TG11
#define WRITECTRL_OFFSET        0x08
#define WRITECMD_OFFSET         0x0c
#define ADDRB_OFFSET            0x10
#define WDATA_OFFSET            0x18
#define STATUS_OFFSET           0x1c
#define MSCLOCK_OFFSET          0x40
#define MASSLOCK_OFFSET         0x54

#define MSCBASE_ADDR_S2         0x40030000  // XG21
#define WRITECTRL_OFFSET_S2     0x0C
#define WRITECMD_OFFSET_S2      0x10
#define ADDRB_OFFSET_S2         0x14
#define WDATA_OFFSET_S2         0x18
#define STATUS_OFFSET_S2        0x1c
#define MSCLOCK_OFFSET_S2       0x3c
#define MISCLOCKWORD_OFFSET_S2  0x40

#define MSC_WRITECTRL_WREN       (0x1UL << 0)   /**< Enable Write/Erase Controller  */
#define MSC_WRITECMD_ERASEPAGE   (0x1UL << 1)   /**< Erase Page */
#define MSC_WRITECMD_LADDRIM     (0x1UL << 0)   /**< Load MSC_ADDRB into ADDR */
#define MSC_STATUS_BUSY          (0x1UL << 0)   /**< Erase/Write Busy */
#define MSC_WRITECMD_WRITEONCE   (0x1UL << 3)   /**< Word Write-Once Trigger */
#define MSC_STATUS_WDATAREADY    (0x1UL << 3)   /**< WDATA Write Ready */
#define MSC_WRITECMD_ERASEMAIN0  (0x1UL << 8)   /**< Mass erase region 0 */
#define MSC_WRITECMD_ERASEMAIN1  (0x1UL << 9)   /**< Mass erase region 1 */

/* AAP registers */
#define AAP_CMD    0
#define AAP_CMDKEY 1

/* Key which must be written to AAP_CMDKEY before writing to AAP_CMD */
#define AAP_UNLOCK_KEY 0xcfacc118
/* AAP bit fields */
#define AAP_CMD_DEVICEERASE 1

BOOL unlockChip(void)
{
#if 0
	uint16_t i;

    P0MDOUT   = 0x1E;
    XBR0      = 0x01;
    SWCLK_Out = 0; nSRST_Out = 0; SWDIO_Out = 0;

	nSRST_Out = 0;nSRST_Out = 0;nSRST_Out = 0;nSRST_Out = 0;
    for( i = 0; i < 10; i++ );
	SWCLK_Out = 1;SWCLK_Out = 1;SWCLK_Out = 1;SWCLK_Out = 1;
    for( i = 0; i < 10; i++ );

	for ( i=0; i<4; i++ )
	{
		SWDIO_Out = 1; SWDIO_Out = 1;SWDIO_Out = 1; SWDIO_Out = 1;
		SWDIO_Out = 0; SWDIO_Out = 0;SWDIO_Out = 0; SWDIO_Out = 0;
	}

	SWCLK_Out = 0;SWCLK_Out = 0;SWCLK_Out = 0;SWCLK_Out = 0;

	for ( i=0; i<4; i++ )
	{
		SWDIO_Out = 1; SWDIO_Out = 1;SWDIO_Out = 1; SWDIO_Out = 1;
		SWDIO_Out = 0; SWDIO_Out = 0;SWDIO_Out = 0; SWDIO_Out = 0;
	}

	SWCLK_Out = 0; SWCLK_Out = 0;SWCLK_Out = 0; SWCLK_Out = 0;
    for( i = 0; i < 10; i++ );
	nSRST_Out = 1; nSRST_Out = 1; nSRST_Out = 1; nSRST_Out = 1;
	for( i = 0; i < 10; i++ );

	/* Connect to SW-DP */
	SW_ShiftReset();
	SW_ShiftByteOut(0x9E);
	SW_ShiftByteOut(0xE7);

	SW_ShiftReset();
	SW_ShiftByteOut(0xB6);
	SW_ShiftByteOut(0xED);

	// Complete SWD reset sequence (50 cycles high followed by 2 or more idle cycles)
	SW_ShiftReset();
	SW_ShiftByteOut(0);
	SW_ShiftByteOut(0);

	// Now read the DPIDR register to move the SWD out of reset
	SW_ShiftPacket(SW_IDCODE_RD, 0);
	printf("\n***DPID = 0x%08lx\n", io_word.Long);

	io_word.Long = 0x1e;
	SW_ShiftPacket(0x81,0);
	SW_ShiftByteOut(0);

	io_word.Long = 0;
	SW_ShiftPacket(SW_CTRLSTAT_WR,0);

	io_word.Long = 0x1e;
	SW_ShiftPacket(0x81,0);
	SW_ShiftByteOut(0);

	/* Powerup request and acknowledge bits in CTRL/STAT */
	#define DP_CTRL_CDBGPWRUPREQ  ((unsigned long)1 << 28)
	#define DP_CTRL_CDBGPWRUPACK  ((unsigned long)1 << 29)
	#define DP_CTRL_CSYSPWRUPREQ  ((unsigned long)1 << 30)
	#define DP_CTRL_CSYSPWRUPACK  ((unsigned long)1 << 31)

	io_word.Long = DP_CTRL_CSYSPWRUPREQ | DP_CTRL_CDBGPWRUPREQ;
	SW_ShiftPacket(SW_CTRLSTAT_WR,0);

	/* Wait until we receive powerup ACK */
	for( i = 0; i < 3; i++ )
	{
		SW_ShiftPacket(SW_CTRLSTAT_RD,0);

		if ( (io_word.Long & (DP_CTRL_CDBGPWRUPACK | DP_CTRL_CSYSPWRUPACK))
				 == (DP_CTRL_CDBGPWRUPACK | DP_CTRL_CSYSPWRUPACK) )
		{
			//printf("Power = 0x%08lx\n", io_word.Long);
			break;
		}
	}

	/* Throw error if we failed to power up the debug interface */
	if ( (io_word.Long & (DP_CTRL_CDBGPWRUPACK | DP_CTRL_CSYSPWRUPACK))
				!= (DP_CTRL_CDBGPWRUPACK | DP_CTRL_CSYSPWRUPACK) )
	{
		printf("SWD_ERROR_DEBUG_POWER\n");
		//return false;
	}

	/* Select last AP bank */
	io_word.Long = 0x000000F0;
	SW_ShiftPacket(SW_SELECT_WR, 0);

	io_word.Long = 0x03;
	SW_ShiftPacket(0x8B,0);
	SW_ShiftByteOut(0);

	/* read AP ID */
	SW_ShiftPacket(0x9f,0);
	SW_ShiftByteOut(0);
	SW_ShiftPacket(SW_RDBUFF_RD, 0);
	printf("***APID = 0x%08lx\n", io_word.Long);

	io_word.Long = 0x00000000;
	SW_ShiftPacket(SW_SELECT_WR, 0);

	/* Start a Device Erase (Debug Unlock) operation */
	// writeAP(AAP_CMDKEY, AAP_UNLOCK_KEY);
	io_word.Long = AAP_UNLOCK_KEY;
	SW_ShiftPacket(0x8B, 0);
	io_word.Long = AAP_UNLOCK_KEY;
	SW_ShiftPacket(0x8B, 0);

	// writeAP(AAP_CMD, AAP_CMD_DEVICEERASE);
	io_word.Long = AAP_CMD_DEVICEERASE;
	SW_ShiftPacket(0xA3, 0);
	io_word.Long = AAP_CMD_DEVICEERASE;
	SW_ShiftPacket(0xA3, 0);

	// writeAP(AAP_CMDKEY, 0);
	io_word.Long = 0;
	SW_ShiftPacket(0x8B, 0);
	io_word.Long = 0;
	SW_ShiftPacket(0x8B, 0);
#if 0
	for( i = 0; i < 100; i++ )
	{
		SW_ShiftPacket(0xB7, 0);
		SW_ShiftPacket(SW_RDBUFF_RD, 0);
		printf("***STATUS = 0x%08lx\n", io_word.Long);
	}
#endif
	delayms(100);

	printf("***Unlock finished");
#endif
	return true;
}

BOOL eraseChip(void)
{
#if BLUE_NRG
	writeMem( 0x4010001c, 0x40100000 );
	writeMem( 0x40100020, 0x1002FFFC );
	writeMem( 0x40100008, 0xFFFFFFFF );
	writeMem( 0x200000C0, 0x0 );
	writeMem( 0x40100008, 0xFFFFFFFF );
	writeMem( 0x40100000, 0x22 );
	delayms(20);
	printf("Erase chip(BLUE_NRG) finished\n" );
	return true;
#else
	uint16_t timeOut;
	g_protocol_error = SW_ACK_OK;

	writeMem(mscLock, 0x1B71 );
	writeMem(mscMassLock, 0x631A); // Gecko and Tiny Gecko do not need this
	writeMem(mscWriteCtrl, MSC_WRITECTRL_WREN);

	/* Start page erase operation */
	if( flashctrltype == FLASH_CONTROL_ADDR_T4 )
	{
		writeMem(mscWriteCmd, MSC_WRITECMD_ERASEMAIN0 ); // GG11
	}
	else
	{
		writeMem(mscWriteCmd, MSC_WRITECMD_ERASEMAIN0 );
	}

	/* Wait until erase is complete */
	for( timeOut = 0; timeOut < 500; timeOut++ )
	{
		if( SW_ACK_OK != g_protocol_error )
		{
			printf("Erase chip failed\n" );
			return false;
		}

		delayms(1);
		readMem(mscStatusReg);
		if((io_word.Long & MSC_STATUS_BUSY) == 0)
		{
			printf("Erase chip finished\n" );
			return true;
		}
	}

	printf("Erase chip timeout\n" );
	return false;
#endif
}

BOOL erasePage( uint32_t address )
{
	uint8_t timeOut = 100;

	g_protocol_error = SW_ACK_OK;

	/* Enable access to MSC */
	writeMem(mscLock, 0x1B71 );

	/* Enable write in MSC */
	writeMem(mscWriteCtrl, MSC_WRITECTRL_WREN);

	/* Enter the start address of the page */
	writeMem(mscAddrb, address);

	/* Load address into internal register */
	writeMem(mscWriteCmd, MSC_WRITECMD_LADDRIM);

	/* Start page erase operation */
	writeMem(mscWriteCmd, MSC_WRITECMD_ERASEPAGE);

	/* Wait until erase is complete */
	for( timeOut = 0; timeOut < 200; timeOut++ )
	{
		if( SW_ACK_OK != g_protocol_error )
		{
			goto ERASEP_ERR;
		}

		delayms(1);
		readMem(mscStatusReg);
		if((io_word.Long & MSC_STATUS_BUSY) == 0)
		{
			printf("Erase page finished\n" );
			return true;
		}
	}
ERASEP_ERR:
	printf("Erase page failed\n" );
	return false;
}

#if BLUE_NRG
#define NVM_REG_COMMAND         (uint32_t)(0x40100000 + 0x00)    // 0x00
#define NVM_REG_IRQSTAT         (uint32_t)(0x40100000 + 0x08)    // 0x08
#define NVM_REG_ADDRESS         (uint32_t)(0x40100000 + 0x18)    // 0x18
#define NVM_REG_IRQRAW          (uint32_t)(0x40100000 + 0x10)    // 0x10
#define NVM_REG_DATA            (uint32_t)(0x40100000 + 0x40)    // 0x40
#define NVM_REG_UNLOCKM         (uint32_t)(0x40100000 + 0x1C)    // 0x1C
#define NVM_REG_UNLOCKL         (uint32_t)(0x40100000 + 0x20)    // 0x20
#define NVM_IRQ_CMDDONE         0x01
#define NVM_CMD_ERASE           0x11
#define NVM_CMD_WRITE           0x33
#define NVM_CMDSTART            0x00000002
#define NVM_CMDDONE             0x00000001

BOOL writeWordToFlash(uint32_t addr, uint32_t wdata)
{
	uint8_t timeOut = 100;

	g_protocol_error = SW_ACK_OK;

	writeMem( NVM_REG_IRQSTAT, 0x3f );
	writeMem( NVM_REG_ADDRESS, addr/4 );
	writeMem( NVM_REG_DATA, wdata );
	writeMem( NVM_REG_COMMAND, NVM_CMD_WRITE );

	/* Wait for write to complete */
	for( timeOut = 0; timeOut < 100; timeOut++ )
	{
		if( SW_ACK_OK != g_protocol_error )
		{
			return false;
		}

		readMem(NVM_REG_IRQRAW);
		//if((io_word.Long & MSC_STATUS_WDATAREADY) == 1)
		if((io_word.Long & NVM_IRQ_CMDDONE) == 1)
		{
			return true;
		}
	}
	return false;
}
#else
BOOL writeWordToFlash(uint32_t addr, uint32_t wdata)
{
	uint8_t timeOut = 100;

	g_protocol_error = SW_ACK_OK;

	/* Enable access to MSC */
	writeMem(mscLock, 0x1B71 );

	/* Enable write in MSC */
	writeMem(mscWriteCtrl, MSC_WRITECTRL_WREN);

	/* Load address */
	writeMem(mscAddrb, addr);

	/* Load address into internal register */
	writeMem(mscWriteCmd, MSC_WRITECMD_LADDRIM);

	/* Write word to WDATA */
	writeMem(mscWdata, wdata);

	/* Start flash write */
	writeMem(mscWriteCmd, MSC_WRITECMD_WRITEONCE);

	/* Wait for write to complete */
	for( timeOut = 0; timeOut < 100; timeOut++ )
	{
		if( SW_ACK_OK != g_protocol_error )
		{
			return false;
		}

		readMem(mscStatusReg);
		//if((io_word.Long & MSC_STATUS_WDATAREADY) == 1)
		if((io_word.Long & MSC_STATUS_BUSY) == 0)
		{
			return true;
		}
	}

	return false;
}
#endif

#if BLUE_GR55
BOOL getdeviceinfo(void)
{
}

#elif BLUE_NRG
BOOL getdeviceinfo(void)
{
	uint32_t i;

	#define NVM_REG_FLASH_SIZE  ((uint32_t)0x40100000 + 0x14)
	#define NVM_REG_DIE_ID      ((uint32_t)0x40900000 + 0x1c)

	i = readMem( NVM_REG_DIE_ID );
	printf("Die ID = 0x%08lx, ", i );

	if( i == 0x113 )
	{
		printf("BlueNRG-1\n");
	}
	else if( i == 0x100 )
	{
		printf("BlueNRG-2\n");
	}
	else
	{
		printf("Unknown\n");
	}

	i = readMem( NVM_REG_FLASH_SIZE );
	printf("Flash size = 0x%08lx, ", i );
	switch( i )
	{
		case 0x00009fff:
			g_FlashSize = 160;
			g_PageSize = 2048;
			printf("Flash start 0x10040000, 0x28000 bytes\n");
			break;
		case 0x0000FFFF:
			g_FlashSize = 255;
			g_PageSize = 2048;
			printf("Flash start 0x10040000, 0x40000 bytes\n");
			break;
		default:
			printf("Flash size unknown");
			return false;
	}

	return true;
}
#else
// 0x0FE081FC PART_NUMBER [15:0]: EFM32 part number as uint16_teger (eg. 230).
// 0x0FE081FE PART_FAMILY [7:0]: EFM32 part family number (Gecko = 71, Giant Gecko = 72, Tiny Gecko = 73, Leopard Gecko=74, Wonder Gecko=75).
// 0x0FE081FF PROD_REV [7:0]: EFM32 Production ID.
#define S01_PART_NUMBER_FAMILY_REV 		0x0FE081FC
#define S01_MEMORY_INFO_FLASH_RAM 		0x0FE081F8
#define S01_MEM_INFO_PAGE_SIZE          0x0FE081E4

BOOL getdeviceinfo(void)
{
	uint32_t i;
	uint16_t partNum;
	//if( HaltCore( 1 ) != 0 )
	//{
	//	return 1;
	//}
	readMem(S01_PART_NUMBER_FAMILY_REV);
	partNum = io_word.Long & 0xffff;
	g_family = (io_word.Long >> 16) & 0xff;
	g_ProductionID = io_word.Long >> 24;

	if( g_family != 128 ) // S0/S1
	{
		printf("Series 0/1 chip\n");
		printf("S0/1_PART_INFO = 0x%08lx\n", io_word.Long );

		// 0x0FE081F8 MEM_INFO_FLASH [15:0]: Flash size, kbyte count as uint16_teger
		// 0x0FE081FA MEM_INFO_RAM [15:0]: Ram size, kbyte count as uint16_teger
		/* Flash addresses to retrieve the device unique id */
		readMem(S01_MEMORY_INFO_FLASH_RAM);
		g_FlashSize = io_word.Long & 0xffff;
		g_RamSize = io_word.Long >> 16;
		printf("S0/1_MEMORY_INFO = 0x%08lx\n", io_word.Long );
		g_UserDataSize = 512;
		//printf("Flash = %dK Bytes, RAM = %dK Bytes\n", g_FlashSize, g_RamSize );
		//printf("FlashChipInfo = 0x%08lx, family = %d, part = %d, PID = %d\n", io_word.Long, (uint16_t)g_family, (uint16_t)g_partNum, (uint16_t)g_ProductionID );

		for( i = 0; i < TOTAL_CHIP_NUM; i++)
		{
			if( g_ChipDefine[ i ].familycode == g_family )
			{
				printf("Part number = %s%dF%d\n", g_ChipDefine[ i ].pChipName, partNum, g_FlashSize );
				sprintf( cmd, "%s%dF%d", g_ChipDefine[ i ].pChipName, partNum, g_FlashSize );
				//printf("%s", cmd );

				readMem(S01_MEM_INFO_PAGE_SIZE);
				g_PageSize = 1<<((int)io_word.Byte[0] + 10);
				//readMem(UNIQUE_ID_HIGH_ADDR); // [63:32] Unique number.
				//printf("UniqueID = 0x%08lx", io_word.Long );
				//readMem(UNIQUE_ID_LOW_ADDR); // [31:0] Unique number.
				//printf("%08lx\n", io_word.Long );

				switch( g_ChipDefine[ i ].MscAddr )
				{
				case 0:
					mscWriteCtrl = (MSCBASE_ADDR_P1 + WRITECTRL_OFFSET);
					mscWriteCmd = (MSCBASE_ADDR_P1 + WRITECMD_OFFSET);
					mscAddrb = (MSCBASE_ADDR_P1 + ADDRB_OFFSET);
					mscWdata = (MSCBASE_ADDR_P1 + WDATA_OFFSET);
					mscStatusReg = (MSCBASE_ADDR_P1 + STATUS_OFFSET);
					mscMassLock = (MSCBASE_ADDR_P1 + MASSLOCK_OFFSET);
					mscLock = (MSCBASE_ADDR_P1 + MSCLOCK_OFFSET);
					flashctrltype = FLASH_CONTROL_ADDR_T1;
					break;
				case 1:
					mscWriteCtrl = (MSCBASE_ADDR_P2 + WRITECTRL_OFFSET);
					mscWriteCmd = (MSCBASE_ADDR_P2 + WRITECMD_OFFSET);
					mscAddrb = (MSCBASE_ADDR_P2 + ADDRB_OFFSET);
					mscWdata = (MSCBASE_ADDR_P2 + WDATA_OFFSET);
					mscStatusReg = (MSCBASE_ADDR_P2 + STATUS_OFFSET);
					mscMassLock = (MSCBASE_ADDR_P2 + MASSLOCK_OFFSET);
					mscLock = (MSCBASE_ADDR_P2 + MSCLOCK_OFFSET);
					flashctrltype = FLASH_CONTROL_ADDR_T2;
					break;
				case 2: //GG11, TG11
					mscWriteCtrl = (MSCBASE_ADDR_P3 + WRITECTRL_OFFSET);
					mscWriteCmd = (MSCBASE_ADDR_P3 + WRITECMD_OFFSET);
					mscAddrb = (MSCBASE_ADDR_P3 + ADDRB_OFFSET);
					mscWdata = (MSCBASE_ADDR_P3 + WDATA_OFFSET);
					mscStatusReg = (MSCBASE_ADDR_P3 + STATUS_OFFSET);
					mscMassLock = (MSCBASE_ADDR_P3 + MASSLOCK_OFFSET);
					mscLock = (MSCBASE_ADDR_P3 + MSCLOCK_OFFSET);
					flashctrltype = FLASH_CONTROL_ADDR_T4;
					break;
				}

				break;
			}
		}
		if( i == TOTAL_CHIP_NUM )
		{
			printf("Get device info failed\n");
			sprintf( cmd, "Get device info failed");
			//printf("%s", cmd );

			return false;
		}
	}
	else // S2
	{
#define S2_FLASH_DEVINFO 0x0fe08000

#define S2_DEVINFO_INFO 		( S2_FLASH_DEVINFO + 0x000 )
#define S2_DEVINFO_PART 		( S2_FLASH_DEVINFO + 0x004 )
#define S2_DEVINFO_MEMINFO 		( S2_FLASH_DEVINFO + 0x008 )
#define S2_DEVINFO_MSIZE 		( S2_FLASH_DEVINFO + 0x00C )
#define S2_DEVINFO_PKGINFO 		( S2_FLASH_DEVINFO + 0x010 )
#define S2_DEVINFO_CUSTOMINFO  	( S2_FLASH_DEVINFO + 0x014 )
		code char ttemper[4] = { 'G', 'I', 'I', ' ' };
		const char *strefr32fg = "EFR32FG";
		const char *strefr32mg = "EFR32MG";
		const char *strefr32bg = "EFR32BG";
		bit isxg22 = 0;

		printf("Series 2 chip\n");

		readMem( S2_DEVINFO_INFO );
		printf("S2_DEVINFO_INFO (0x%08lx) = 0x%08lx\n", S2_DEVINFO_INFO, io_word.Long );
		g_ProductionID = io_word.Byte[1];
		readMem( S2_DEVINFO_PART );
		printf("S2_DEVINFO_PART (0x%08lx) = 0x%08lx\n", S2_DEVINFO_PART, io_word.Long );
		readMem( S2_DEVINFO_MEMINFO );
		printf("S2_DEVINFO_MEMINFO (0x%08lx) = 0x%08lx\n", S2_DEVINFO_MEMINFO, io_word.Long );
		g_PageSize = 1<<((io_word.Byte[3] +10) & 0xFF);
		g_UserDataSize = io_word.Byte[2] * g_PageSize;
		readMem( S2_DEVINFO_MSIZE );
		printf("S2_DEVINFO_MSIZE (0x%08lx) = 0x%08lx\n", S2_DEVINFO_MSIZE, io_word.Long );
		g_FlashSize = (int)(io_word.Int[1]);
		g_RamSize = (int)(io_word.Int[0]&0x3ff);
		readMem( S2_DEVINFO_PKGINFO );
		printf("S2_DEVINFO_PKGINFO (0x%08lx) = 0x%08lx\n", S2_DEVINFO_PKGINFO, io_word.Long );
		readMem( S2_DEVINFO_CUSTOMINFO );
		printf("S2_DEVINFO_CUSTOMINFO (0x%08lx) = 0x%08lx\n", S2_DEVINFO_CUSTOMINFO, io_word.Long );

		readMem( S2_DEVINFO_PART );
		printf("Part number = ");
		switch( io_word.Byte[0]&0x3f )
		{
		case 0:
			printf( strefr32fg );
			strcpy( cmd, strefr32fg );
			break;
		case 1:
			printf( strefr32mg );
			strcpy( cmd, strefr32mg );
			break;
		case 2:
			printf( strefr32bg );
			strcpy( cmd, strefr32bg );
			break;
		default:
			break;
		}

		printf("%02d", (int)(io_word.Byte[1]&0x3f) );

		if( (int)(io_word.Byte[1]&0x3f) == 22 )
		{
			isxg22 = 1;
		}

		printf("%c", (char)(io_word.Int[1]/1000 + 'A' ));
		printf("%03d", io_word.Int[1]%1000 );

		sprintf( &cmd[strlen(cmd)], "%02d", (int)(io_word.Byte[1]&0x3f) );
		sprintf( &cmd[strlen(cmd)], "%c", (char)(io_word.Int[1]/1000 + 'A' ));
		sprintf( &cmd[strlen(cmd)], "%03d", io_word.Int[1]%1000 );

		readMem(S2_DEVINFO_MSIZE);
		printf("F%d", (int)(io_word.Long&0xffff) );
		sprintf( &cmd[strlen(cmd)], "F%d", (int)(io_word.Long&0xffff) );

		readMem( S2_DEVINFO_MEMINFO );
		readMem( S2_DEVINFO_PKGINFO );
		printf("%c", ttemper[io_word.Byte[3]]);
		sprintf( &cmd[strlen(cmd)], "%c", ttemper[io_word.Byte[3]]);

	    switch( io_word.Byte[2] )
	    {
	        case 77:
	        	printf("%c", 'M');
	        	sprintf( &cmd[strlen(cmd)], "%c", 'M');
	            break;
	        case 74:
	        case 76:
	        case 81:
	        	printf("%c", 'x');
	        	sprintf( &cmd[strlen(cmd)], "%c", 'x');
	            break;
	    }

	    printf("%02d", (int)(io_word.Byte[1]) );
	    sprintf( &cmd[strlen(cmd)], "%02d", (int)(io_word.Byte[1]) );
	    readMem( S2_DEVINFO_INFO );
	    printf("-%c\n", 'A' + io_word.Byte[1] );
	    sprintf( &cmd[strlen(cmd)], "-%c", 'A' + io_word.Byte[1] );
	    //printf("%s", cmd );

	    mscWriteCtrl = (MSCBASE_ADDR_S2 + WRITECTRL_OFFSET_S2);
	    mscWriteCmd = (MSCBASE_ADDR_S2 + WRITECMD_OFFSET_S2);
	    mscAddrb = (MSCBASE_ADDR_S2 + ADDRB_OFFSET_S2);
	    mscWdata = (MSCBASE_ADDR_S2 + WDATA_OFFSET_S2);
	    mscLock = (MSCBASE_ADDR_S2 + MSCLOCK_OFFSET_S2);
	    mscMassLock = (MSCBASE_ADDR_S2 + MISCLOCKWORD_OFFSET_S2);
	    mscStatusReg = (MSCBASE_ADDR_S2 + STATUS_OFFSET_S2);
	    flashctrltype = FLASH_CONTROL_ADDR_T3;

	    if( isxg22 == 1 )
	    {
	    	writeMem(0x40008068, 0x20000); // Open IMEM clock
	    }
	}

	printf("Family = %d, Flash = %dK Bytes, RAM = %dK Bytes, PageSize = %d Bytes, User = %d Bytes, ProductionID = %d\n", (int)g_family, g_FlashSize, g_RamSize, g_PageSize, g_UserDataSize, g_ProductionID );

	printf("mscWriteCtrl = 0x%08lx, mscWriteCmd  = 0x%08lx, mscAddrb = 0x%08lx\n", mscWriteCtrl, mscWriteCmd, mscAddrb );
	printf("    mscWdata = 0x%08lx, mscStatusReg = 0x%08lx, mscLock  = 0x%08lx, mscMassLock = 0x%08lx\n", mscWdata, mscStatusReg, mscLock, mscMassLock );

	return true;
}
#endif

BOOL Cmd_SetFlashCtrl( void )
{
	int i;

	g_protocol_error = SW_ACK_OK;

	printf("Set MSC control addr(T%lx) ", flashctrltype );
	writeMem( REMOTE_flashctrltype, flashctrltype );
	writeMem( REMOTE_command, CMD_FLASH_CTRL_ADDR );

	for( i=0; i < 100; i++ )
	{
		if( SW_ACK_OK != g_protocol_error )
		{
			return false;
		}

		readMem( REMOTE_command );
		if( io_word.Long == CMD_FINISHED )
		{
			break;
		}
	}

	if( io_word.Long == CMD_FINISHED )
	{
		printf("0x%08lx\n", mscWriteCtrl );
	}
	else
	{
		printf("Error\n");
	}

	readMem( REMOTE_cmdresult );
	switch(io_word.Long)
	{
	case CMD_RESULT_NONE:
		printf(" None\n" );
		break;
	}

	return true;
}

BOOL Cmd_erasePage( uint32_t addr )
{
	int i;

	printf("Erase page at 0x%08lx ", addr);

	g_protocol_error = SW_ACK_OK;
	writeMem( REMOTE_buffer1_start_addr, addr );
	writeMem( REMOTE_cmdresult, CMD_RESULT_NONE );
	writeMem( REMOTE_command, CMD_PAGE_ERASE );
	for( i = 0; i < 50; i++ )
	{
		if( SW_ACK_OK != g_protocol_error )
		{
			return false;
		}

		delayms(1);
		readMem( REMOTE_command );
		//printf("%08lx", io_word.Long);
		if( io_word.Long == CMD_FINISHED )
		{
			break;
		}
	}

	if( io_word.Long == CMD_FINISHED )
	{
		printf("finished\n");
	}
	else
	{
		printf("error\n");
	}

	return true;
}

BOOL Cmd_eraseChip( void )
{
	unsigned int i;
	printf("Erase chip ");

	g_protocol_error = SW_ACK_OK;
	writeMem( REMOTE_cmdresult, CMD_RESULT_NONE );
	writeMem( REMOTE_command, CMD_MASS_ERASE );
	for( i = 0; i < 500; i++ )
	{
		if( SW_ACK_OK != g_protocol_error )
		{
			return false;
		}

		delayms(1);
		readMem( REMOTE_command );
		//printf("%08lx", io_word.Long);
		if( io_word.Long == CMD_FINISHED )
		{
			break;
		}
	}

	if( io_word.Long == CMD_FINISHED )
	{
		printf("finished\n");
	}
	else
	{
		printf("error\n");
	}

	return true;
}

BOOL Cmd_WaitWriteFinish( void )
{
	uint16_t i;

	g_protocol_error = SW_ACK_OK;
	for( i = 0; i < 100; i++ )
	{
		if( SW_ACK_OK != g_protocol_error )
		{
			return false;
		}

		readMem( REMOTE_buffer1_rdy );
		if( io_word.Long == BUFFER_STATUS_FREE )
		{
			break;
		}
	}

	for( i = 0; i < 100; i++ )
	{
		if( SW_ACK_OK != g_protocol_error )
		{
			return false;
		}

		readMem( REMOTE_buffer2_rdy );
		if( io_word.Long == BUFFER_STATUS_FREE )
		{
			break;
		}
	}

	return true;
}

BOOL Cmd_checksum( uint32_t addr, uint32_t bytesize, uint32_t *checksum )
{
	printf("Checksum flash Addr = 0x%08lx, 0x%08lx Bytes,", addr, bytesize);
	*checksum = 0;
	g_protocol_error = SW_ACK_OK;
	writeMem( REMOTE_buffer1_start_addr, addr );
	writeMem( REMOTE_buffer1_size, bytesize );
	writeMem( REMOTE_command, CMD_CHECKSUM );
	do{
		if( SW_ACK_OK != g_protocol_error )
		{
			return false;
		}
		readMem( REMOTE_command );
	}while( io_word.Long == CMD_CHECKSUM );

	readMem( REMOTE_checksum );
	*checksum = io_word.Long;
	printf(" 0x%08lx\n", io_word.Long);
	return true;
}

BOOL Cmd_blankcheck( uint32_t addr, uint32_t bytesize )
{
	printf("Blankcheck Addr = 0x%08lx, 0x%08lx Bytes\n", addr, bytesize);
	g_protocol_error = SW_ACK_OK;
	writeMem( REMOTE_buffer1_start_addr, addr );
	writeMem( REMOTE_buffer1_size, bytesize );
	writeMem( REMOTE_command, CMD_BLANK_CHECK );
	do{
		if( SW_ACK_OK != g_protocol_error )
		{
			printf("Failed\n");
			return false;
		}
		readMem( REMOTE_command );
	}while( io_word.Long == CMD_BLANK_CHECK );

	readMem( REMOTE_cmdresult );
	if( io_word.Long != 0xffffffff )
	{
		printf("NOT BLANK at 0x%08lx, read: ", io_word.Long);
		readMem( io_word.Long );
		printf("0x%08lx\n", io_word.Long);
	}
	else
	{
		printf("BLANK\n");
	}

	return true;
}

BOOL readDisplay( uint32_t addr, uint32_t bytesize )
{
	uint32_t i;

	//printf("Read from 0x%08lx for 0x%08lx words:\n", addr, (bytesize+3)&0xfffffffc);
	printf("Read from 0x%08lx for 0x%08lx Bytes:\n", addr, bytesize);
	g_protocol_error = SW_ACK_OK;
	for( i = 0; i < bytesize; i+=4 )
	{
		if( SW_ACK_OK != g_protocol_error )
		{
			return false;
		}
		readMem( addr + i );
		printf("%08lx ", io_word.Long);
	}
	printf("\n");
	return true;
}

void DisplayDebug( void )
{
	readMem( REMOTE_bebugcounter       );	printf("bebugcounter       = %08lx \n", io_word.Long );
	readMem( REMOTE_flashctrltype      );	printf("flashctrltype      = %08lx \n", io_word.Long );
	readMem( REMOTE_flashmain_num      );	printf("flashmain_num      = %08lx \n", io_word.Long );
	readMem( REMOTE_command            );	printf("command            = %08lx \n", io_word.Long );
	readMem( REMOTE_cmdresult          );	printf("cmdresult          = %08lx \n", io_word.Long );
	readMem( REMOTE_checksum           );	printf("checksum           = %08lx \n", io_word.Long );
	readMem( REMOTE_buffer1_rdy        );	printf("buffer1_rdy        = %08lx \n", io_word.Long );
	readMem( REMOTE_buffer2_rdy        );	printf("buffer2_rdy        = %08lx \n", io_word.Long );
	readMem( REMOTE_buffer1_start_addr );	printf("buffer1_start_addr = %08lx \n", io_word.Long );
	readMem( REMOTE_buffer2_start_addr );	printf("buffer2_start_addr = %08lx \n", io_word.Long );
	readMem( REMOTE_buffer1_size       );	printf("buffer1_size       = %08lx \n", io_word.Long );
	readMem( REMOTE_buffer2_size       );	printf("buffer2_size       = %08lx \n", io_word.Long );
}

void delayms( int nms )
{
	int i;
	do
	{
		for( i = 0; i < 1130; i++ );
	}while(nms--);
}
