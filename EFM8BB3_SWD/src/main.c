#include "cmdinc.h"
#include "initdevice.h"

// Resources:
//   UART0  - 115200 baud, 8-N-1
//   Timer1 - UART0 clock source
//   P0.4   - UART0 TX
//   P0.5   - UART0 RX
//   P2.2   - Switch of UART/SPI

void SPI_CmpProcess( void );
void FLASH_CmpProcess(void);

void SiLabs_Startup (void)
{
	 PCA0MD &= ~0x40;
}

void printhelp( void )
{
	printf("SWD: CLK = P2.1; RST = P2.2; SWDIO = P2.3\n");
	printf("SPI: SCK = P0.6; MISO = P0.7; MOSI = P1.0; CS = P1.1\n");
	printf("LCD: CS = P1.4, RS = P1.2, RESET = P1.3\n");

	printf("Command list:\n");
	printf("reset                  : hardware reset chip\n");
	printf("connect                : connect and prepare flashloader\n");
	printf("read 0x0 0x10          : read memory, address size\n" );
	printf("write 0x0 0x10         : write a word 0x10 to addr 0x0\n" );
	printf("erasepage 0x10         : erase a flash page\n");
	printf("cmderasepage 0x10      : erase a flash page\n");
	printf("erasechip              : erase all main flash of a chip\n");
	printf("cmderasechip           : erase all main flash of a chip\n");
	printf("blank 0x0 0x10         : blank check, address size\n");
	printf("blankchip              : blank check a chip\n");
	printf("checksum 0x0 0x10      : get checksum, address size\n");
	printf("checksumchip           : get checksum of a chip\n");

	printf("spiid                  : get device ID\n" );
	printf("spierasechip           : erase nor chip\n" );
	printf("spierasesector 0x1000  : erase a sector\n" );
	printf("spierasepage 0x1000    : erase a sector\n" );
	printf("spiwrite 0x0 0x10      : write a word 0x10 to addr 0x0\n" );
	printf("spiread                : read memory, address size\n" );
	printf("spiblank               : blank check, address size\n");
	printf("spiblankchip           : blank check a chip\n");
	printf("spichecksum 0x0 0x10   : get checksum, address size\n");
	printf("spichecksumchip        : blank check a chip\n");
	printf("download file 0x0 0x10 : download a binary to flash, address size\n");
	printf("copy 0x0 0x10 0x20     : copy from SPI 0x0 to flash 0x10, size 0x20\n");
}

xdata char cmd_buf[15], cmd[COMMAND_LEN];
uint32_t g_var1, g_var2, g_var3, g_varnum;

void main(void)
{
	uint8_t mode = 1;

	enter_DefaultMode_from_RESET();
	SCON0_TI = 1;
#if EFM8UB2
	LcmInit();
	LcmClear( 0 );
	LcmPutStr( 0, 0, "Start" );
	delayms(500);
	LcmPutStr( 0, 1, "OK   " );
#endif
	LED0_GREEN = LED_ON;	LED1_BLUE = LED_ON;		LED2_RED = LED_ON;
	for( g_var1 = 0; g_var1 < 30000; g_var1++ );
	LED0_GREEN = LED_OFF;	LED1_BLUE = LED_OFF;	LED2_RED = LED_OFF;

	for( g_var1 = 0; g_var1 < 30000; g_var1++ )
	{
		if( KEY0 == KEY_PRESSED ||  KEY1 == KEY_PRESSED )
		{
			mode = 1;
			LED0_GREEN = LED_ON;	LED1_BLUE = LED_ON;		LED2_RED = LED_ON;
			break;
		}
	}

	for(;;)
	{
		g_var1 = g_var2 = g_var3 = cmd[0] = cmd_buf[0] = 0;
		if( mode == 1 )
		{
			printf("cmd>");
			gets( cmd, COMMAND_LEN );
			LED0_GREEN = LED_OFF;	LED1_BLUE = LED_OFF;	LED2_RED = LED_OFF;
		}
		else
		{
			for(;;)
			{
				if( KEY0 == KEY_PRESSED || KEY1 == KEY_PRESSED )
				{
					//while( KEY0 == KEY_PRESSED || KEY1 == KEY_PRESSED );
					strcpy( cmd, "auto" );
					LED0_GREEN = LED_OFF;	LED1_BLUE = LED_OFF;	LED2_RED = LED_OFF;
					break;
				}
			}
		}

		LED1_BLUE = LED_ON;
		g_varnum = sscanf( cmd, (const char*)"%s 0x%lx 0x%lx", cmd_buf, &g_var1, &g_var2 );
		FLASH_CmpProcess();
#if EFM8UB2
		LcmPutStr(0,0,cmd);
#endif
		SPI_CmpProcess();
		if( 0 == strcmp( cmd_buf, "help" )
				|| 0 == strcmp( cmd_buf, "h" )
				|| 0 == strcmp( cmd_buf, "?" ) )
		{
			printhelp();
		}
		LED1_BLUE = LED_OFF;
	}
}
