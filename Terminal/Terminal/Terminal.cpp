// Terminal.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Terminal.h"
#include "conio.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// The one and only application object

CWinApp theApp;

using namespace std;

BOOL TerminalProcess( void );
BOOL xmodem( void );

int main(int argc, char** argv[])
{
	int nRetCode = 0;

	// initialize MFC and print and error on failure
	if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
	{
		// TODO: change error code to suit your needs
		_tprintf(_T("Fatal Error: MFC initialization failed\n"));
		nRetCode = 1;
	}
	else
	{
		// TODO: code your application's behavior here.
		TerminalProcess();
	}

	return nRetCode;
}

HANDLE hCom;
FILE *fp;
COMSTAT ComStat;

char UART_NUM[] = "\\\\.\\COM   ";

BOOL TerminalProcess( void )
{
	unsigned int uart;

	for(;;)
	{
		printf("Input COM<0..99>:");
		if( 0 == scanf((const char*)"%d", &uart))
		{
			return 0;
		}

		if( uart < 100 )
		{
			sprintf((char*)(&UART_NUM[7]), "%d", uart );
			break;
		}
	}

	hCom=CreateFile( UART_NUM,  //COM1口 ; 注意串口号如果大于COM9应该在前面加上\\.\，比如COM10表示为"\\\\.\\COM10"
					GENERIC_READ|GENERIC_WRITE,
					0,
					NULL,  
					OPEN_EXISTING,
					0, //FILE_ATTRIBUTE_NORMAL|FILE_FLAG_OVERLAPPED,
					NULL); 
	if( hCom == INVALID_HANDLE_VALUE )
	{
		printf("%s open failure!", UART_NUM);
		return FALSE;
	}
	else
	{
		printf("%s open OK.", UART_NUM);
	}

	SetupComm(hCom,1024,1024);

	COMMTIMEOUTS TimeOuts;
	TimeOuts.ReadIntervalTimeout = 1000;
	TimeOuts.ReadTotalTimeoutMultiplier = 500; 
	TimeOuts.ReadTotalTimeoutConstant = 5000; 
	TimeOuts.WriteTotalTimeoutMultiplier = 500;
	TimeOuts.WriteTotalTimeoutConstant = 2000; 
	SetCommTimeouts(hCom,&TimeOuts);

	DCB dcb; 
	GetCommState(hCom,&dcb); 
	dcb.BaudRate = 115200;
	dcb.ByteSize = 8;
	dcb.Parity = NOPARITY;
	dcb.StopBits = ONESTOPBIT;
	SetCommState(hCom,&dcb); 
	PurgeComm(hCom,PURGE_TXCLEAR | PURGE_RXCLEAR); 

	char cmdbuf[ 256 ] = { 0 }, cmdpos = 0;
	for( ;; )
	{
		char str[1024*64] = {0};
		DWORD dwErrorFlags, dwBytesWrite;
		ClearCommError(hCom,&dwErrorFlags,&ComStat);
		if( ComStat.cbInQue != 0 )
		{
			DWORD wCount;
			ReadFile(hCom,str,ComStat.cbInQue,&wCount,NULL);
			for( DWORD i = 0; i < wCount; i++ )
			{
				if( str[i] == '\r' || str[i] == '\n' )
				{
					for( int t = 3; t < 100; t++ )
					{
						if( cmdbuf[t] == 'd' )
						{
							int var1 = 0, var2 = 0;
							char filename[256];
							int varnum = sscanf( &cmdbuf[ t ], (const char*)"download %s 0x%lx 0x%lx", filename, &var1, &var2 );
							if( varnum == 3 )
							{
								fp = fopen( filename, "rb");
								if( fp != NULL )
								{
									printf( str );
									xmodem();
									fclose( fp );
									for( int i = 0; i < 100; i++ )
									{
										cmdbuf[ i ] = 0;
										cmdpos = 0;
									}
									goto Next;
								}
								else
								{
									WriteFile(hCom," ",1,&dwBytesWrite,NULL);
								}
								break;
							}
						}
					}

					for( int i = 0; i < 100; i++ )
					{
						cmdbuf[ i ] = 0;
					}
					cmdpos = 0;
				}

				putchar( str[i] );
				if( str[i] == '\b' )
				{
					if( cmdpos > 0 )
					{
						cmdpos--;
					}
				}
				else
				{
					cmdbuf[ cmdpos++ ] =  str[i];
				}

				if( cmdpos > 99 )
				{
					cmdpos = 0;
				}
			}
		}
Next:
		if( _kbhit() )
		{
			str[0] = _getch(); //GetAsyncKeyState
			if( str[0] == 'q' || str[0] == 'Q')
			{
				break;
			}

			if( str[0] == (char)0xe0 )
			{
				_getch();
				continue;
			}

			if(( str[0] >= '0' && str[0] <= '9')
				|| ( str[0] >= 'a' && str[0] <= 'z')
				|| ( str[0] >= 'A' && str[0] <= 'Z')
				|| str[0] == '?' || str[0] == ' ' || str[0] == '.' || str[0] == ':' || str[0] == '\\'
				|| str[0] == '\b' )
			{
				WriteFile(hCom,str,1,&dwBytesWrite,NULL);
			}

			if( str[0] == '\r' || str[0] == '\n' )
			{
				WriteFile(hCom,"\n",1,&dwBytesWrite,NULL);
			}
		}
	}

	CloseHandle( hCom );
	return TRUE;
}

unsigned short CRC16Check( char *ptr, short count )
{
	unsigned short crc = 0;
	char i;

	while(--count >= 0)
	{
		crc = crc ^ (int) *ptr++ << 8;
		i = 8;
		do
		{
			if (crc & 0x8000)
				crc = crc << 1 ^ 0x1021;
			else
				crc = crc << 1;
		}while(--i);
	}

	return crc;
}

BOOL xmodem( void )
{
	DWORD i = 0, cnt = 0;
	char str[1024] = {0};
	DWORD dwErrorFlags, dwBytesWrite;

	for( ;; )
	{
		ClearCommError(hCom,&dwErrorFlags,&ComStat);
		if( ComStat.cbInQue != 0 )
		{
			DWORD wCount;
			ReadFile(hCom,str,ComStat.cbInQue,&wCount,NULL);
			for( i = 0; i < wCount; i++ )
			{
				putchar( str[i] );
				if( str[i] == 'C' )
				{
					cnt++;
				}
			}

			if( cnt > 1 )
			{
				break;
			}
		}

		if( _kbhit() )
		{
			return FALSE;
		}
	}

	unsigned int trsize = 0;
	unsigned int prt = 0;
	unsigned char package = 0, package_n;
	for(;;)
	{
resend:
		char buffer[128];
		char tmp = 0x01; // SOH
		WriteFile( hCom, &tmp, 1, &dwBytesWrite, NULL ); // start

		WriteFile( hCom, &package, 1, &dwBytesWrite, NULL ); // package
		package_n = 255 - package;
		WriteFile( hCom, &package_n, 1, &dwBytesWrite, NULL ); // -package

		memset( buffer, 0xff, 128 );
		int size = (int)fread(buffer, (size_t)1, (size_t)128, fp);
		WriteFile( hCom, buffer, 128, &dwBytesWrite, NULL ); // 128 data

		unsigned short crc;
		crc = CRC16Check( buffer, 128 );
		WriteFile( hCom, ((unsigned char*)&crc + 1), 1, &dwBytesWrite, NULL );  // crc1
		WriteFile( hCom, (unsigned char*)&crc, 1, &dwBytesWrite, NULL );  // crc2

		for( ;; )
		{
			ClearCommError(hCom,&dwErrorFlags,&ComStat);
			if( ComStat.cbInQue != 0 )
			{
				DWORD wCount;
				ReadFile(hCom,str,ComStat.cbInQue,&wCount,NULL);
				for( i = 0; i < wCount; i++ )
				{
					if( str[i] == 0x06 || str[i] == 0x16 ) // ACK, NACK
					{
						break;
					}
				}

				if( str[i] == 0x06 ) // ACK
				{
					break;
				}

				if( str[i] == 0x16 ) // NACK
				{
					fseek( fp, -size, SEEK_CUR );
					goto resend;
				}

			}

			if( _kbhit() )
			{
				return FALSE;
			}
		}

		trsize+=size;
		package++;

		if( 128 != size )
		{
			tmp = 0x04; // EOT
			WriteFile( hCom, &tmp, 1, &dwBytesWrite, NULL );
			break;
		}

		if( _kbhit() )
		{
			return FALSE;
		}

		if( ((++prt)%64) == 0 )
			//putchar('.');
			printf(" %x", trsize);
	}

	printf("\r\nDownloaded 0x%x, %d bytes", trsize, trsize);
	return TRUE;
}
