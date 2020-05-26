#include "cmdinc.h"

#if EFM8BB3
void LcmPutStr(uint8_t col,uint8_t page,uint8_t *puts)
{
	 col = page = *puts;
}
#endif

void FLASH_CmpProcess(void)
{
	uint32_t i = 0;

	if( 0 == strcmp( cmd_buf, "reset" ))
	{
		nSRST_Out = 0;
		delayms(1);
		nSRST_Out = 1;
	}
	else if( 0 == strcmp( cmd_buf, "unlock" ))
	{
		unlockChip();
	}
	else if( 0 == strcmp( cmd_buf, "copy" ))
	{
		g_varnum = sscanf( cmd, (const char*)"%s 0x%lx 0x%lx 0x%lx", cmd_buf, &g_var1, &g_var2, &g_var3 );
		if( g_varnum != 4 )
		{
			printf("copy 0x0 0x10 0x20     : copy from SPI 0x0 to flash 0x10, size 0x20\n");
		}
		else
		{
			CopySPIToFlash( g_var1, g_var2, g_var3 );
		}
	}
	else if( 0 == strcmp( cmd_buf, "conn" ))
	{
		if( 2 == SWD_Connect()) // locked chips return 2
		{
			//continue;
		}

		if( true != getdeviceinfo() )
		{
			return;
		}
	}
	else if( 0 == strcmp( cmd_buf, "connect" ))
	{
		if( 2 == SWD_Connect()) // locked chips return 2
		{
			//continue;
		}

		if( true != getdeviceinfo() )
		{
			return;
		}

		if( true != HaltCore( true ))
		{
			//continue;
		}

		resetAndHaltCore();
		LoadFlashRam();
		HaltCore( false );
		if( true == CheckBootloader())
		{
#if BLUE_NRG
#else
			Cmd_SetFlashCtrl();
#endif
			printf("Ram loader OK\n");
		}
		else
		{
			printf("Ram loader failed\n");
		}
	}
	else if( 0 == strcmp( cmd_buf, "deviceinfo" ))
	{
		getdeviceinfo();
	}
	else if( 0 == strcmp( cmd_buf, "loader" ))
	{
		if( true != HaltCore( true ))
		{
			//continue;
		}

		resetAndHaltCore();
		LoadFlashRam();
		HaltCore( false );
		if( true == CheckBootloader())
		{
#if BLUE_NRG
#else
			Cmd_SetFlashCtrl();
#endif
			printf("Ram loader OK\n");
		}
		else
		{
			printf("Ram loader failed\n");
		}
	}
	else if( 0 == strcmp( cmd_buf, "debug" ))
	{
		DisplayDebug();
	}
	else if( 0 == strcmp( cmd_buf, "read" ))
	{
		if( g_varnum > 1 )
		{
			if( g_varnum == 2 )
			{
				g_var2 = 0x20;
			}
			readDisplay( g_var1, g_var2 );
		}
	}
	else if( 0 == strcmp( cmd_buf, "write" ))
	{
		if( g_varnum == 3 )
		{
			printf("write flash addr 0x%lx, data 0x%lx\n", g_var1, g_var2 );
			writeWordToFlash( g_var1, g_var2 );
		}
	}
	else if( 0 == strcmp( cmd_buf, "writemem" ))
	{
		if( g_varnum == 3 )
		{
			printf("writemem addr 0x%lx, data 0x%lx\n", g_var1, g_var2 );
			writeMem( g_var1, g_var2 );
		}
	}
#if 0
	else if( 0 == strcmp( cmd_buf, "write" ))
	{
#if BLUE_NRG
		g_PageSize = 1024;
		for( i = 0; i < g_FlashSize/(g_PageSize/1024); i++ )
		{
			//printf("writepage addr 0x%lx\n", i*g_PageSize/4 );
			if( true != Cmd_writeInOnePage( i*g_PageSize, g_PageSize, NO_WAIT_FINISH, AUTO_INC ))
			{
				break;
			}
		}
#else
		//printf("write addr 0x%lx, data 0x%lx\n", g_var1, g_var2 );
		//Cmd_writeInOnePage( 0, 1024, WAIT_FINISH, NO_AUTO_INC );
		g_PageSize = 1024;
		for( i = 0; i < g_FlashSize/(g_PageSize/1024); i++ )
		{
			if( true != Cmd_writeInOnePage( i*g_PageSize, g_PageSize, WAIT_FINISH, AUTO_INC ))
			{
				break;
			}
		}
#endif
		printf("\nFinished\n");
	}
#endif
	else if( 0 == strcmp( cmd_buf, "erasechip" ))
	{
		eraseChip();
	}
	else if( 0 == strcmp( cmd_buf, "cmderasechip" ))
	{
		Cmd_eraseChip();
	}
	else if( 0 == strcmp( cmd_buf, "erasepage" ))
	{
		//if( varnum > 1 )
		//{
			erasePage( g_var1 );
		//}
	}
	else if( 0 == strcmp( cmd_buf, "cmderasepage" ))
	{
		//if( varnum > 1 )
		//{
			Cmd_erasePage( g_var1 );
		//}
	}
	else if( 0 == strcmp( cmd_buf, "blank" ))
	{
		//if( varnum == 3 )
		//{
			Cmd_blankcheck( g_var1, g_var2 );
		//}
	}
	else if( 0 == strcmp( cmd_buf, "blankchip" ))
	{
#if BLUE_NRG
		Cmd_blankcheck( 0x10040000, 0x28000 );
#else
		Cmd_blankcheck( 0, (uint32_t)g_FlashSize*1024 );
#endif
	}
	else if( 0 == strcmp( cmd_buf, "checksum" ))
	{
		//if( varnum == 3 )
		//{
			Cmd_checksum( g_var1, g_var2, &g_var3 );
		//}
	}
	else if( 0 == strcmp( cmd_buf, "checksumchip" ))
	{
#if BLUE_NRG
		Cmd_checksum( 0x10040000, 0x27000, &g_var3 );
#else
		Cmd_checksum( 0, (uint32_t)g_FlashSize*1024, &g_var3 );
#endif
	}
	else if( 0 == strcmp( cmd_buf, "auto" ))
	{
		LED1_BLUE = LED_ON;
		LcmPutStr(0,0,"Connect");
		SWD_Connect();
		LED1_BLUE = !LED1_BLUE;
		if( true != getdeviceinfo())
		{
			LED1_BLUE = LED_OFF;	LED2_RED = LED_ON;
			return;
		}
		LcmPutStr(0,0,cmd);
		LED1_BLUE = !LED1_BLUE;
#if BLUE_NRG
#else
		LcmPutStr(0,0,"Erase");
		eraseChip();
#endif
		LED1_BLUE = !LED1_BLUE;
		if( true != HaltCore( true ))
		{
		}

		resetAndHaltCore();
		LED1_BLUE = !LED1_BLUE;
		LoadFlashRam();
		LED1_BLUE = !LED1_BLUE;
		HaltCore( false );
		if( true == CheckBootloader())
		{
#if BLUE_NRG
#else
			Cmd_SetFlashCtrl();
#endif
			printf("Ram loader OK\n");
			LcmPutStr(0,0,"Ram loader OK");
		}
		else
		{
			LcmPutStr(0,0,"Ram loader failed");
			printf("Ram loader failed\n");
			LED1_BLUE = LED_OFF;	LED2_RED = LED_ON;
			return;
		}

		LcmPutStr(0,0,"Copying");
#if BLUE_NRG
		CopySPIToFlash( 0, (uint32_t)0x10040000, (uint32_t)g_FlashSize*1024 );
#else
		CopySPIToFlash( 0, 0, (uint32_t)g_FlashSize*1024 );
#endif
	}
}
