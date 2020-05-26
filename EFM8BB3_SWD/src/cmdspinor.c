#include "cmdinc.h"

#define SPI_CMD_RDSR           0x05
#define SPI_CMD_BULK_ERASE     0x60
#define SPI_CMD_SECTOR_ERASE   0x20
#define SPI_CMD_READ           0x03
#define SPI_CMD_WRITE          0x02
#define SPI_CMD_WRITEENABLE    0x06
#define SPI_CMD_ID             0x9f

BE_LONG spi_longvar;

#if EFM8UB2
	sbit SPI_CS = P0^3;
	#define SPI_NOR_ENABLE    SPI_CS = 0;SPI_CS = 0;SPI_CS = 0;SPI_CS = 0;SPI_CS = 0;SPI_CS = 0;SPI_CS = 0;SPI_CS = 0;SPI_CS = 0;SPI_CS = 0;SPI_CS = 0;SPI_CS = 0;SPI_CS = 0;
	#define SPI_NOR_DISABLE   SPI_CS = 1;SPI_CS = 1;SPI_CS = 1;SPI_CS = 1;SPI_CS = 1;SPI_CS = 1;SPI_CS = 1;SPI_CS = 1;SPI_CS = 1;SPI_CS = 1;SPI_CS = 1;SPI_CS = 1;SPI_CS = 1;

	#define WAIT_SPI while(!( SPI0CN0&0x80 )); SPI0CN0 &= ~0x80;
#endif

#if EFM8BB3
	#define SPI_NOR_ENABLE    SPI0CN0_NSSMD0 = 0;
	#define SPI_NOR_DISABLE   SPI0CN0_NSSMD0 = 1;
	#define WAIT_SPI while( SPI0CFG & 0x01 );
#endif

void SPI_writedat( uint8_t dat )
{
	SPI0DAT = dat;
	WAIT_SPI
}

void SPI_readid(void)
{
	int16_t tmp;
	SPI_NOR_ENABLE

	printf("SPI Nor flash ID: ");

	SPI0DAT = SPI_CMD_ID;
#if EFM8BB3	
	SPI0DAT = 0x00;
#endif	
	WAIT_SPI
	tmp = SPI0DAT;

	SPI0DAT = 0x00;
	WAIT_SPI
	tmp = SPI0DAT;	printf("%02x, ", tmp);

	SPI0DAT = 0x00;
	WAIT_SPI
	tmp = SPI0DAT;	printf("%02x, ", tmp);
#if EFM8UB2
	SPI0DAT = 0x00;
#endif
	WAIT_SPI
	tmp = SPI0DAT;	printf("%02x\n", tmp);

	SPI_NOR_DISABLE
}

void SPI_writeenable( void )
{
	int16_t tmp;
	SPI_NOR_ENABLE

	SPI0DAT = SPI_CMD_WRITEENABLE;
	WAIT_SPI
	tmp = SPI0DAT;

	SPI_NOR_DISABLE
}

void SPI_chiperase( void )
{
	int16_t tmp, i;
	SPI_writeenable();

	SPI_NOR_ENABLE // Bulk erase
	SPI0DAT = SPI_CMD_BULK_ERASE;
	WAIT_SPI
	tmp = SPI0DAT;
	SPI_NOR_DISABLE

	SPI_NOR_ENABLE // read status
	SPI0DAT = SPI_CMD_RDSR;
	WAIT_SPI
	tmp = SPI0DAT;

	for( i = 0; i < 30000; i++ ) // Max 64second
	{
		delayms( 10 );
		SPI0DAT = SPI_CMD_RDSR;
		WAIT_SPI
		tmp = SPI0DAT;

		if( (tmp & 0x01) == 0 )
		{
			printf( "0x%02x finished", tmp );
			break;
		}
		if( (i % 160) == 0 )
		{
			printf( "." );
		}
	}
	SPI_NOR_DISABLE
	printf( "\n" );
}

void SPI_SectorErase( uint32_t saddr )
{
	int16_t tmp, i;
	SPI_writeenable();

	SPI_NOR_ENABLE // Erase erase
	SPI0DAT = SPI_CMD_SECTOR_ERASE;
	WAIT_SPI
	tmp = SPI0DAT;

	SPI0DAT = (uint8_t)(saddr>>16);
	WAIT_SPI
	tmp = SPI0DAT;

	SPI0DAT = (uint8_t)(saddr>>8);
	WAIT_SPI
	tmp = SPI0DAT;

	SPI0DAT = (uint8_t)(saddr);
	WAIT_SPI
	tmp = SPI0DAT;

	SPI_NOR_DISABLE
	////
	SPI_NOR_ENABLE // read status
	SPI0DAT = SPI_CMD_RDSR;
	WAIT_SPI
	tmp = SPI0DAT;

	for( i = 0; i < 30000; i++ ) // Max 64second
	{
		delayms( 10 );
		SPI0DAT = SPI_CMD_RDSR;
		WAIT_SPI
		tmp = SPI0DAT;

		if( (tmp & 0x01) == 0 )
		{
			printf( "0x%02x finished", tmp );
			break;
		}
		if( (i % 160) == 0 )
		{
			printf( "." );
		}
	}
	SPI_NOR_DISABLE
	printf( "\n" );
}

void SPI_read( uint32_t saddr, uint32_t size )
{
	int16_t tmp, i;

	printf("Read spi flash from 0x%08lx for 0x%08lx Bytes:\n", saddr, size );

	SPI_NOR_ENABLE

	SPI0DAT = SPI_CMD_READ;
	WAIT_SPI
	tmp = SPI0DAT;

	SPI0DAT = (uint8_t)(saddr>>16);
	WAIT_SPI
	tmp = SPI0DAT;

	SPI0DAT = (uint8_t)(saddr>>8);
	WAIT_SPI
	tmp = SPI0DAT;

	SPI0DAT = (uint8_t)(saddr);
	WAIT_SPI
	tmp = SPI0DAT;

	for( i = 0; i < size; i+=4 )
	{
		SPI0DAT = 0x00;
#if EFM8BB3			
		SPI0DAT = 0x00;
#endif		
		WAIT_SPI
		spi_longvar.Byte[3] = SPI0DAT;

		SPI0DAT = 0x00;
		WAIT_SPI
		spi_longvar.Byte[2] = SPI0DAT;

		SPI0DAT = 0x00;
		WAIT_SPI
		spi_longvar.Byte[1] = SPI0DAT;
#if EFM8UB2
		SPI0DAT = 0x00;
#endif		
		WAIT_SPI
		spi_longvar.Byte[0] = SPI0DAT;

		printf("%08lx ", spi_longvar.Long );
	}

	printf("\n");
	SPI_NOR_DISABLE
}

void SPI_blank( uint32_t saddr, uint32_t size )
{
	uint32_t i;
	uint16_t tmp;
	int err = 0;

	printf("Blank check spi Addr = 0x%08lx, 0x%08lx Bytes\n", saddr, size );

	SPI_NOR_ENABLE

	SPI0DAT = SPI_CMD_READ;
	WAIT_SPI
	tmp = SPI0DAT;

	SPI0DAT = (uint8_t)(saddr>>16);
	WAIT_SPI
	tmp = SPI0DAT;

	SPI0DAT = (uint8_t)(saddr>>8);
	WAIT_SPI
	tmp = SPI0DAT;

	SPI0DAT = (uint8_t)(saddr);
	WAIT_SPI
	tmp = SPI0DAT;

	for( i = 0; i < size; i+=4 )
	{
		SPI0DAT = 0x00;
#if EFM8BB3			
		SPI0DAT = 0x00;
#endif		
		WAIT_SPI
		spi_longvar.Byte[3] = SPI0DAT;

		SPI0DAT = 0x00;
		WAIT_SPI
		spi_longvar.Byte[2] = SPI0DAT;

		SPI0DAT = 0x00;
		WAIT_SPI
		spi_longvar.Byte[1] = SPI0DAT;
#if EFM8UB2
		SPI0DAT = 0x00;
#endif		
		WAIT_SPI
		spi_longvar.Byte[0] = SPI0DAT;

		if( spi_longvar.Long != 0xffffffff )
		{
			printf("NOT BLANK at 0x%08lx, read: 0x%08lx", saddr + i, spi_longvar.Long );
			break;
		}
	}

	if( i >= size )
	{
		printf("BLANK");
	}
	printf("\n");

	SPI_NOR_DISABLE
}

BOOL SPI_checksum( uint32_t saddr, uint32_t size, uint32_t *checksum )
{
	int16_t tmp;
	uint32_t i;
	*checksum = 0;

	printf("Checksum SPI Addr = 0x%08lx, 0x%08lx Bytes, ", saddr, size );

	SPI_NOR_ENABLE

	SPI0DAT = SPI_CMD_READ;
	WAIT_SPI
	tmp = SPI0DAT;

	SPI0DAT = (uint8_t)(saddr>>16);
	WAIT_SPI
	tmp = SPI0DAT;

	SPI0DAT = (uint8_t)(saddr>>8);
	WAIT_SPI
	tmp = SPI0DAT;

	SPI0DAT = (uint8_t)(saddr);
	WAIT_SPI
	tmp = SPI0DAT;

	for( i = 0; i < size; i+=4 )
	{
		SPI0DAT = 0x00;
#if EFM8BB3			
		SPI0DAT = 0x00;
#endif		
		WAIT_SPI
		spi_longvar.Byte[3] = SPI0DAT;

		SPI0DAT = 0x00;
		WAIT_SPI
		spi_longvar.Byte[2] = SPI0DAT;

		SPI0DAT = 0x00;
		WAIT_SPI
		spi_longvar.Byte[1] = SPI0DAT;
#if EFM8UB2
		SPI0DAT = 0x00;
#endif		
		WAIT_SPI
		spi_longvar.Byte[0] = SPI0DAT;

		*checksum += spi_longvar.Long;
	}

	SPI_NOR_DISABLE
	printf( "0x%08lx\n", *checksum );
	return *checksum;
}

void SPI_writeword( uint32_t saddr, uint32_t dat )
{
	uint16_t tmp;

	printf("spiwriteword addr 0x%lx, data 0x%lx\n", saddr, dat );

	SPI_writeenable();

	SPI_NOR_ENABLE

	SPI0DAT = SPI_CMD_WRITE;
	WAIT_SPI
	tmp = SPI0DAT;

	SPI0DAT = (uint8_t)(saddr>>16);
	WAIT_SPI
	tmp = SPI0DAT;

	SPI0DAT = (uint8_t)(saddr>>8);
	WAIT_SPI
	tmp = SPI0DAT;

	SPI0DAT = (uint8_t)(saddr);
	WAIT_SPI
	tmp = SPI0DAT;

	spi_longvar.Long = dat;
	// data
	SPI0DAT = spi_longvar.Byte[3];	WAIT_SPI	tmp = SPI0DAT;
	SPI0DAT = spi_longvar.Byte[2];	WAIT_SPI	tmp = SPI0DAT;
	SPI0DAT = spi_longvar.Byte[1];	WAIT_SPI	tmp = SPI0DAT;
	SPI0DAT = spi_longvar.Byte[0];	WAIT_SPI	tmp = SPI0DAT;

	SPI_NOR_DISABLE
}

unsigned short CRC16Check( char *ptr, short count )
{
	unsigned short crc = 0;
	char loop;

	while(--count >= 0)
	{
		crc = crc ^ (int)*ptr++ << 8;
		loop = 8;
		do
		{
			if (crc & 0x8000)
				crc = crc << 1 ^ 0x1021;
			else
				crc = crc << 1;
		}while(--loop);
	}

	return crc;
}

char xdata dat_buffer[128];

#define UART_TIMEOUT   30000
BOOL SPI_download( uint32_t saddr, uint32_t size )
{
	uint32_t i = 0, PackageNum = 0, insize = 0;
	uint16_t tmp, k;
	uint16_t err = 0;

	LED1_BLUE = LED_ON;
	while( SCON0_RI == 1 )
	{
		_getkey();
	}
	printf( "Send file with XMODEM..." );
	while( SCON0_RI == 0 )
	{
		if( i > UART_TIMEOUT )
		{
			i = 0;
			putchar('C');
		}
		i++;
	}

	for(;;)
	{
		LED1_BLUE = !LED1_BLUE;
		i = 0;
		while( SCON0_RI == 0 )
		{
			if( i > UART_TIMEOUT )
			{
				LED1_BLUE = LED_OFF; LED2_RED = LED_ON;
				//printf("Get data timeout 1");
				err = 1;
				goto DOWNLOAD_ERROR;
			}
			i++;
		}
		tmp = _getkey();
		switch( tmp )
		{
		case 0x01: // SOH 128
			break;
		case 0x02: // STX 1k
			putchar( 0x18 ); // CAN
			LED1_BLUE = LED_OFF; LED2_RED = LED_ON;
			//printf("Does not support 1k package");
			err = 2;
			goto DOWNLOAD_ERROR;
		case 0x04: // EOT
			putchar(0x06); // ACK
			LED1_BLUE = LED_OFF; LED0_GREEN = LED_ON;
			printf( "\nSuccessful\n" );
			return true;
		}

		i = 0;
		while( SCON0_RI == 0 )
		{
			if( i > UART_TIMEOUT )
			{
				//printf("Get package number timeout");
				err = 3;
				goto DOWNLOAD_ERROR;
			}
			i++;
		}
		tmp = _getkey(); // Package number

		i = 0;
		while( SCON0_RI == 0 )
		{
			if( i > UART_TIMEOUT )
			{
				//printf("Get -package number timeout");
				err = 4;
				goto DOWNLOAD_ERROR;
			}
			i++;
		}
		tmp = _getkey(); // -Package number

		for( k = 0; k < 128; k++ )
		{
			i = 0;
			while( SCON0_RI == 0 )
			{
				if( i > UART_TIMEOUT )
				{
					//printf("Get package data timeout");
					err = 5;
					goto DOWNLOAD_ERROR;
				}
			}
			tmp = _getkey(); // data
			dat_buffer[k] =  tmp&0xff;
		}

		i = 0;
		while( SCON0_RI == 0 )
		{
			if( i > UART_TIMEOUT )
			{
				//printf("Get CRC1 timeout");
				err = 6;
				goto DOWNLOAD_ERROR;
			}
			i++;
		}
		tmp = 0xff&_getkey(); // CRC1
		tmp <<= 8;

		i = 0;
		while( SCON0_RI == 0 )
		{
			if( i > UART_TIMEOUT )
			{
				//printf("Get CRC2 timeout");
				err = 7;
				goto DOWNLOAD_ERROR;
			}
			i++;
		}
		tmp |= (0xff&_getkey()); // CRC2
		if( tmp != CRC16Check(dat_buffer, 128))
		{
			putchar(0x15); // NACK
			//putchar(0x18); // CAN
			printf("CRC error");
			continue;
		}

		SPI_writeenable();
		SPI_NOR_ENABLE

		SPI0DAT = SPI_CMD_WRITE;
		WAIT_SPI
		tmp = SPI0DAT;

		SPI0DAT = (uint8_t)((saddr + PackageNum*128)>>16);
		WAIT_SPI
		tmp = SPI0DAT;

		SPI0DAT = (uint8_t)((saddr + PackageNum*128)>>8);
		WAIT_SPI
		tmp = SPI0DAT;

		SPI0DAT = (uint8_t)((saddr + PackageNum*128));
		WAIT_SPI
		tmp = SPI0DAT;

		for( k = 0; k < 128; k++ )
		{
			tmp = dat_buffer[k]; // data

			// data
			if( size != 0)
			{
				if( insize < size )
				{
					SPI0DAT = tmp;
					insize++;
					WAIT_SPI
					tmp = SPI0DAT;
				}
			}
			else
			{
				SPI0DAT = tmp;
				insize++;
				WAIT_SPI
				tmp = SPI0DAT;
			}
		}
		SPI_NOR_DISABLE
		PackageNum++;

		putchar(0x06); // ACK
	}

DOWNLOAD_ERROR:
	LED1_BLUE = LED_OFF; LED2_RED = LED_ON;
	printf( "\nFailure code %d\n", err );
	return false;
}

#define SPI_CMD_READ           0x03
BOOL CopySPIToFlash( uint32_t spisaddr, uint32_t flashaddr, uint32_t size )
{
	uint16_t tmp;
	uint32_t i, incPageSize = 1024; // bytes

	printf("Copy from SPI 0x%08lx to flash 0x%08lx, size 0x%08lx Bytes\n", spisaddr, flashaddr, size );

	SPI_NOR_ENABLE

	SPI0DAT = SPI_CMD_READ;
	WAIT_SPI
	tmp = SPI0DAT;

	SPI0DAT = (uint8_t)(spisaddr>>16);
	WAIT_SPI
	tmp = SPI0DAT;

	SPI0DAT = (uint8_t)(spisaddr>>8);
	WAIT_SPI
	tmp = SPI0DAT;

	SPI0DAT = (uint8_t)(spisaddr);
	WAIT_SPI
	tmp = SPI0DAT;

	for( i = 0; i < size/incPageSize; i++ )
	{
		LED1_BLUE = !LED1_BLUE;
		//printf("writepage addr 0x%lx 0x%lx\n", flashaddr + i*incPageSize, incPageSize );
		if( true != Cmd_writeInOnePage( flashaddr + i*incPageSize, incPageSize, NO_WAIT_FINISH, AUTO_INC ))
		{
			break;
		}
	}

	//printf("#writepage addr 0x%lx 0x%lx\n", flashaddr + i*incPageSize, size%incPageSize );
	if( true != Cmd_writeInOnePage( flashaddr + i*incPageSize, size%incPageSize, NO_WAIT_FINISH, AUTO_INC ))
	{
	}

	SPI_NOR_DISABLE

	printf("\n");
	LED1_BLUE = !LED1_BLUE;
	SPI_checksum( spisaddr, size, &i );
	LED1_BLUE = !LED1_BLUE;
	Cmd_checksum( flashaddr, size, &incPageSize );
	LED1_BLUE = LED_OFF;

	if( i != incPageSize )
	{
		printf("Compare error\n");
		LED2_RED = LED_ON;
		return false;
	}
	LED0_GREEN = LED_ON;
	printf("Successful\n");
	return true;
}

extern BE_LONG spi_longvar;

uint8_t pixel = 0;

BOOL Cmd_writeInOnePage( uint32_t addr, uint32_t bytesize, BOOL wait, BOOL inc )
{
	uint32_t i, waitinc = 0;
#if 0
	switch( (pixel++)&3 )
	{
	case 0:
		LcmPutStr(0,0,"-");
		break;
	case 1:
		LcmPutStr(0,0,"|");
		break;
	case 2:
		LcmPutStr(0,0,"+");
	case 3:
		LcmPutStr(0,0,"*");
	}
#endif
	g_protocol_error = SW_ACK_OK;
	printf(".");
	if( inc )
	{
		//printf("Cmd_writeInOnePage (inc) Addr = 0x%08lx, 0x%08lx Bytes ", addr, bytesize);

		// 32bits, inc on
		io_word.Long = DHCSR;
		SW_ShiftPacket(0x8B, 0);
		io_word.Long = 0x23000012;
		SW_ShiftPacket(0xA3, 0);

		for(;;)
		{
			if( SW_ACK_OK != g_protocol_error )
			{
				return false;
			}

			readMem( REMOTE_buffer1_rdy );
			if( io_word.Long == BUFFER_STATUS_FREE )
			{
				//printf("use buffer 1, ");
				// Start address
				waitinc = 0;
				io_word.Long = REMOTE_buffer1;
				SW_ShiftPacket(0x8B, 0);
				for( i = 0; i < bytesize; i+=4 )
				{
					SPI0DAT = 0x00;
#if EFM8BB3						
					SPI0DAT = 0x00;
#endif					
					WAIT_SPI
					spi_longvar.Byte[3] = SPI0DAT;

					SPI0DAT = 0x00;
					WAIT_SPI
					spi_longvar.Byte[2] = SPI0DAT;

					SPI0DAT = 0x00;
					WAIT_SPI
					spi_longvar.Byte[1] = SPI0DAT;
#if EFM8UB2
					SPI0DAT = 0x00;
#endif					
					WAIT_SPI
					spi_longvar.Byte[0] = SPI0DAT;

					io_word.Long = spi_longvar.Long;
					SW_ShiftPacket(0xBB, 0);
				}
				writeMem( REMOTE_buffer1_start_addr, addr );
				writeMem( REMOTE_buffer1_size, bytesize );
				writeMem( REMOTE_buffer1_rdy, BUFFER_STATUS_READY );
				writeMem( REMOTE_command, CMD_WRITE_DATA );

				if( wait )
				{
					do{
						readMem( REMOTE_buffer1_rdy );
						//printf("0x%08lx", io_word.Long);
					}while( io_word.Long == BUFFER_STATUS_READY );
				}
				break;
			}
			else
			{
				readMem( REMOTE_buffer2_rdy );
				if( io_word.Long == BUFFER_STATUS_FREE )
				{
					//printf("use buffer 2, ");
					// Start address
					waitinc = 0;
					io_word.Long = REMOTE_buffer2;
					SW_ShiftPacket(0x8B, 0);
					for( i = 0; i < bytesize; i+=4 )
					{
						SPI0DAT = 0x00;
#if EFM8BB3							
						SPI0DAT = 0x00;
#endif						
						WAIT_SPI
						spi_longvar.Byte[3] = SPI0DAT;

						SPI0DAT = 0x00;
						WAIT_SPI
						spi_longvar.Byte[2] = SPI0DAT;

						SPI0DAT = 0x00;
						WAIT_SPI
						spi_longvar.Byte[1] = SPI0DAT;
#if EFM8UB2
						SPI0DAT = 0x00;
#endif						
						WAIT_SPI
						spi_longvar.Byte[0] = SPI0DAT;

						io_word.Long = spi_longvar.Long;
						SW_ShiftPacket(0xBB, 0);
					}
					writeMem( REMOTE_buffer2_start_addr, addr );
					writeMem( REMOTE_buffer2_size, bytesize );
					writeMem( REMOTE_buffer2_rdy, BUFFER_STATUS_READY );
					writeMem( REMOTE_command, CMD_WRITE_DATA );

					if( wait )
					{
						do{
							readMem( REMOTE_buffer2_rdy );
							//printf("0x%08lx", io_word.Long);
						}while( io_word.Long == BUFFER_STATUS_READY );
					}
					break;
				}
			}

			printf("*");
			waitinc++;
			if(waitinc > 100)
			{
				return false;
			}
		}

		// 32bits, inc off
		io_word.Long = DHCSR;
		SW_ShiftPacket(0x8B, 0);
		io_word.Long = 0x23000002;
		SW_ShiftPacket(0xA3, 0);

		//printf("finished\n");
	}
	else
	{
		//printf("Cmd_writeInOnePage Addr = 0x%08lx, 0x%08lx Bytes, ", addr, bytesize);

		for(;;)
		{
			if( SW_ACK_OK != g_protocol_error )
			{
				return false;
			}

			readMem( REMOTE_buffer1_rdy );
			if( io_word.Long == BUFFER_STATUS_FREE )
			{
				//printf("use buffer 1, ");
				for( i = 0; i < bytesize; i+=4 )
				{
					//writeMem( REMOTE_buffer1 + i, g_data(i>>2) );
					SPI0DAT = 0x00;
#if EFM8BB3						
					SPI0DAT = 0x00;
#endif					
					WAIT_SPI
					spi_longvar.Byte[3] = SPI0DAT;

					SPI0DAT = 0x00;
					WAIT_SPI
					spi_longvar.Byte[2] = SPI0DAT;

					SPI0DAT = 0x00;
					WAIT_SPI
					spi_longvar.Byte[1] = SPI0DAT;
#if EFM8UB2
					SPI0DAT = 0x00;
#endif					
					WAIT_SPI
					spi_longvar.Byte[0] = SPI0DAT;

					io_word.Long = spi_longvar.Long;
					writeMem( REMOTE_buffer1 + i, io_word.Long );
				}

				writeMem( REMOTE_buffer1_start_addr, addr );
				writeMem( REMOTE_buffer1_size, bytesize );
				writeMem( REMOTE_buffer1_rdy, BUFFER_STATUS_READY );
				writeMem( REMOTE_command, CMD_WRITE_DATA );

				if( wait )
				{
					do{
						readMem( REMOTE_buffer1_rdy );
					}while( io_word.Long == BUFFER_STATUS_READY );
				}
				break;
			}
			else
			{
				readMem( REMOTE_buffer2_rdy );
				if( io_word.Long == BUFFER_STATUS_FREE )
				{
					//printf("use buffer 2, ");
					for( i = 0; i < bytesize; i+=4 )
					{
						SPI0DAT = 0x00;
#if EFM8BB3							
						SPI0DAT = 0x00;
#endif						
						WAIT_SPI
						spi_longvar.Byte[3] = SPI0DAT;

						SPI0DAT = 0x00;
						WAIT_SPI
						spi_longvar.Byte[2] = SPI0DAT;

						SPI0DAT = 0x00;
						WAIT_SPI
						spi_longvar.Byte[1] = SPI0DAT;
#if EFM8UB2
						SPI0DAT = 0x00;
#endif						
						WAIT_SPI
						spi_longvar.Byte[0] = SPI0DAT;

						io_word.Long = spi_longvar.Long;
						writeMem( REMOTE_buffer2 + i, io_word.Long );
					}

					writeMem( REMOTE_buffer2_start_addr, addr );
					writeMem( REMOTE_buffer2_size, bytesize );
					writeMem( REMOTE_buffer2_rdy, BUFFER_STATUS_READY );
					writeMem( REMOTE_command, CMD_WRITE_DATA );

					if( wait )
					{
						do{
							readMem( REMOTE_buffer2_rdy );
						}while( io_word.Long == BUFFER_STATUS_READY );
					}
					break;
				}
			}

			printf("*");
		}
		//printf("finished\n");
	}

	return true;
}

void SPI_CmpProcess( void )
{
	if( 0 == strcmp( cmd_buf, "spiid" ))
	{
		SPI_readid();
	}
	else if( 0 == strcmp( cmd_buf, "spiwriteenable" ))
	{
		SPI_writeenable();
	}
	else if( 0 == strcmp( cmd_buf, "spierasechip" ))
	{
		SPI_chiperase();
	}
	else if( 0 == strcmp( cmd_buf, "spierasesector" ) || 0 == strcmp( cmd_buf, "spierasepage" ) )
	{
		SPI_SectorErase( g_var1 );
	}
	else if( 0 == strcmp( cmd_buf, "spiwrite" ))
	{
		//if( varnum != 3 )
		//{
		//	break;
		//}

		SPI_writeword( g_var1, g_var2 );
	}
	else if( 0 == strcmp( cmd_buf, "spiread" ))
	{
		//if( varnum > 1 )
		//{
			if( g_varnum == 2 )
			{
				g_var2 = 0x20;
			}
			SPI_read( g_var1, g_var2 );
		//}
	}
	else if( 0 == strcmp( cmd_buf, "spiblank" ))
	{
		SPI_blank( g_var1, g_var2 );
	}
	else if( 0 == strcmp( cmd_buf, "spiblankchip" ))
	{
		SPI_blank( 0, (uint32_t)1024*1024*4 );
	}
	else if( 0 == strcmp( cmd_buf, "spichecksum" ))
	{
		SPI_checksum( g_var1, g_var2, &g_var3 );
	}
	else if( 0 == strcmp( cmd_buf, "spichecksumchip" ))
	{
		SPI_checksum( 0, (uint32_t)1024*1024*4, &g_var3 );
	}
	else if( 0 == strcmp( cmd_buf, "download" ))
	{
		g_varnum = sscanf( cmd, (const char*)"download %s 0x%lx 0x%lx", cmd_buf, &g_var1, &g_var2 );
		if( g_varnum != 3 )
		{
			printf("download error 0x%lx, data 0x%lx\n", g_var1, g_var2 );
		}
		else
		{
			SPI_download( g_var1, g_var2 );
		}
	}
}
