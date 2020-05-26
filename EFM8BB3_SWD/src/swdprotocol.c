#include "cmdinc.h"
#include "swdprotocol.h"

uint8_t g_protocol_error = SW_ACK_OK;

// Holds the last acknowledge error from the start of a move command.
uint8_t idata ack_error;

// Used to pass 32-bit data into and out of SWD transfer routines.
volatile BE_LONG bdata io_word;

// Used to provide bit addressable data for 8-bit and smaller shift routines.
volatile uint8_t bdata io_byte;

sbit iob_0 = io_byte^0;
sbit iob_1 = io_byte^1;
sbit iob_2 = io_byte^2;
sbit iob_3 = io_byte^3;
sbit iob_4 = io_byte^4;
sbit iob_5 = io_byte^5;
sbit iob_6 = io_byte^6;
sbit iob_7 = io_byte^7;

// Even parity lookup table, holds even parity result for a 4-bit value.
const uint8_t code even_parity[] =
{
    0x00, 0x10, 0x10, 0x00,
    0x10, 0x00, 0x00, 0x10,
    0x10, 0x00, 0x00, 0x10,
    0x00, 0x10, 0x10, 0x00
};
#if 0
BOOL SWD_Disconnect(void)
{
    _SetSWPinsIdle;

    SW_ShiftReset();
    SW_ShiftByteOut(0x3C);
    SW_ShiftByteOut(0xE7);

    // Release debug interface pins except nSRST
    _ResetDebugPins;

    return HOST_COMMAND_OK;
}
#endif


uint8_t SW_Request(uint8_t DAP_Addr)
{
    uint8_t req;

    // Convert the DAP address into a SWD packet request value
    req = DAP_Addr & DAP_CMD_MASK;      // mask off the bank select bits
    req = req | even_parity[req];       // compute and add parity bit
    req = req << 1;                     // move address/parity bits
    req = req | SW_REQ_PARK_START;      // add start and park bits
    return req;
}

void SW_DAP_Read(uint8_t cnt, uint8_t DAP_Addr)
{
    uint8_t req;

    // Format the packet request header
    req = SW_Request(DAP_Addr);

    // Shift the first packet and if DP access, send the results
    SW_ShiftPacket(req, 0);
    if (!(req & SW_REQ_APnDP))
    {
        //SendLongToHost(io_word.Long);
    }

    // Perform the requested number of reads
    for (; cnt != 0; cnt--)
    {
        SW_ShiftPacket(req, 0);
        //SendLongToHost(io_word.Long);
    }

    // For AP access, get and send results of the last read
    if (req & SW_REQ_APnDP)
    {
        SW_ShiftPacket(SW_RDBUFF_RD, 0);
        //SendLongToHost(io_word.Long);
    }
}

void SW_DAP_Write(uint8_t cnt, uint8_t DAP_Addr, BOOL final)
{
    uint8_t req;

    // Format the packet request header
    req = SW_Request(DAP_Addr);

    // Clear the upper half word for 16-bit packed writes
    io_word.Int[0] = 0;

    // Perform the requested number of writes
    do
    {
        //io_word.Int[1] = GetIntFromHost();

        // For packed transfers, write 16-bits at a time
        if (DAP_Addr & DAP_CMD_PACKED)
        {
            SW_ShiftPacket(req, 0);
            //io_word.Int[1] = GetIntFromHost();
        }
        else
        {
            //io_word.Int[0] = GetIntFromHost();
        }
        SW_ShiftPacket(req, 0);
    }
    while (cnt-- != 0);

    // For AP access, check results of last write (use default retry count
    // because previous write may need time to complete)
    if (final && (req & SW_REQ_APnDP))
    {
        SW_ShiftPacket(SW_RDBUFF_RD, 0);
    }
}

#pragma OT(8, SPEED)
void SW_ShiftByteOut(uint8_t byte)
{
    // Make sure SWDIO is an output
    _SetSWDIOasOutput;

    // Copy data to bit addressable location
    io_byte = byte;

    // Shift 8-bits out on SWDIO
    SWDIO_Out = iob_0; _StrobeSWCLK;
    SWDIO_Out = iob_1; _StrobeSWCLK;
    SWDIO_Out = iob_2; _StrobeSWCLK;
    SWDIO_Out = iob_3; _StrobeSWCLK;
    SWDIO_Out = iob_4; _StrobeSWCLK;
    SWDIO_Out = iob_5; _StrobeSWCLK;
    SWDIO_Out = iob_6; _StrobeSWCLK;
    SWDIO_Out = iob_7; _StrobeSWCLK;
}

#pragma OT(8, SPEED)
uint8_t SW_ShiftByteIn(void)
{
    // Make sure SWDIO is an input
    _SetSWDIOasInput;

    // Shift 8-bits in on SWDIO
    iob_0 = SWDIO_In; _StrobeSWCLK;
    iob_1 = SWDIO_In; _StrobeSWCLK;
    iob_2 = SWDIO_In; _StrobeSWCLK;
    iob_3 = SWDIO_In; _StrobeSWCLK;
    iob_4 = SWDIO_In; _StrobeSWCLK;
    iob_5 = SWDIO_In; _StrobeSWCLK;
    iob_6 = SWDIO_In; _StrobeSWCLK;
    iob_7 = SWDIO_In; _StrobeSWCLK;

    // Return the byte that was shifted in
    return io_byte;
}

BOOL SW_Response(uint8_t SW_Ack)
{
    switch (SW_Ack)
    {
    case SW_ACK_OK:     return HOST_COMMAND_OK;
    case SW_ACK_WAIT:   return HOST_AP_TIMEOUT;
    case SW_ACK_FAULT:  return HOST_ACK_FAULT;
    default:
    	break;
    }

    return HOST_WIRE_ERROR;
}

BOOL SWD_DAP_Move(void)
{
    uint8_t cnt = 0, dap = 0;

    // Reset global error accumulator
    ack_error = SW_ACK_OK;

    // Determine if this is a read or write transfer
    if (dap & DAP_CMD_RnW)
    {
        // Perform the requested number of reads
        SW_DAP_Read(cnt, dap);
    }
    else
    {
        // Perform the requested number of writes
        SW_DAP_Write(cnt, dap, TRUE);
    }

    // Finish with idle cycles
    SW_ShiftByteOut(0);

    // Return the accumulated error result
    return SW_Response(ack_error);
}

BOOL SWD_DAP_MoveList(void)
{
    uint8_t cnt = 0, dap = 0;

    // Reset global error accumulator
    ack_error = SW_ACK_OK;

    // Get the number of transfers in the list
    //cnt = NextInRxQueue();

    // Perform each requested transfer
    do
    {
        // Get the next DAP address/command
        //dap = NextInRxQueue();

        // Determine if this is a read or write transfer
        if (dap & DAP_CMD_RnW)
        {
            SW_DAP_Read(0, dap);                // Read DAP register one time
        }
        else
        {
            SW_DAP_Write(0, dap, (cnt == 0));   // Write DAP register one time
        }
    }
    while (cnt-- != 0);

    // Finish with idle cycles
    SW_ShiftByteOut(0);

    // Return the accumulated error result
    return SW_Response(ack_error);
}

bit SW_CalcDataParity(void)
{
    uint8_t parity;

    // Calculate column parity, reducing down to 4 columns
    parity  = io_word.Byte[3];
    parity ^= io_word.Byte[2];
    parity ^= io_word.Byte[1];
    parity ^= io_word.Byte[0];
    parity ^= parity >> 4;

    // Use lookup table to get parity on 4 remaining bits. The cast (bit)
    // converts any non-zero value to 1.
    return (bit)even_parity[parity & 0xF];
}

void SW_ShiftReset(void)
{
    uint8_t i;

    // Drive SWDIO high
    SWDIO_Out = 1;
    _SetSWDIOasOutput;

    // Complete 64 SWCLK cycles
    for (i = 64; i != 0; i--)
    {
        _StrobeSWCLK;
    }
}

uint8_t SW_ShiftPacket(uint8_t request, uint8_t retry)
{
    uint8_t ack, limit, i;

    // If retry parameter is zero, use the default value instead
    if (retry == 0)
    {
        retry = DAP_RETRY_COUNT;
    }
    limit = retry;

    // While waiting, do request phase (8-bit request, turnaround, 3-bit ack)
    do
    {
        // Turnaround or idle cycle, makes or keeps SWDIO an output
        //SWDIO_Out = 0; _SetSWDIOasOutput; _StrobeSWCLK;

        // Shift out the 8-bit packet request
        SW_ShiftByteOut(request);

        // Turnaround cycle makes SWDIO an input
        _SetSWDIOasInput; _StrobeSWCLK;

        // Shift in the 3-bit acknowledge response
        io_byte = 0;
        iob_0 = SWDIO_In;  _StrobeSWCLK;
        iob_1 = SWDIO_In;  _StrobeSWCLK;
        iob_2 = SWDIO_In;  _StrobeSWCLK;
        ack = io_byte;

        // Check if we need to retry the request
        if ((ack == SW_ACK_WAIT) && --retry)
        {
            // Delay an increasing amount with each retry
        	_StrobeSWCLK;
            for (i=retry; i < 10; i++);
        }
        else
        {
            break;  // Request phase complete (or timeout)
        }
    }
    while (TRUE);

    // If the request was accepted, do the data transfer phase (turnaround if
    // writing, 32-bit data, and parity)
    if (ack == SW_ACK_OK)
    {
        if (request & SW_REQ_RnW)
        {
            // Swap endian order while shifting in 32-bits of data
            io_word.Byte[3] = SW_ShiftByteIn();
            io_word.Byte[2] = SW_ShiftByteIn();
            io_word.Byte[1] = SW_ShiftByteIn();
            io_word.Byte[0] = SW_ShiftByteIn();

            // Shift in the parity bit
            iob_0 = SWDIO_In; _StrobeSWCLK;

            // Check for parity error
            if (iob_0 ^ SW_CalcDataParity())
            {
                ack = SW_ACK_PARITY_ERR;
            }
        }
        else
        {
            // Turnaround cycle makes SWDIO an output
            _SetSWDIOasOutput; _StrobeSWCLK;

            // Swap endian order while shifting out 32-bits of data
            SW_ShiftByteOut(io_word.Byte[3]);
            SW_ShiftByteOut(io_word.Byte[2]);
            SW_ShiftByteOut(io_word.Byte[1]);
            SW_ShiftByteOut(io_word.Byte[0]);

            // Shift out the parity bit
            SWDIO_Out = SW_CalcDataParity(); _StrobeSWCLK;
        }
    }
    // TODO: Add error (FAULT, line, parity) handling here?  RESEND on parity error?

    // Turnaround or idle cycle, always leave SWDIO an output
    SWDIO_Out = 0; _SetSWDIOasOutput; _StrobeSWCLK;

    // Update the global error accumulator if there was an error
    if (ack != SW_ACK_OK)
    {
    	//printf("Protocol error");
        ack_error = ack;
    }
    return ack;
}

uint32_t readMem(uint32_t addr)
{
// writeAP(AP_TAR, addr);
// readAP(AP_DRW, &ret);
// readDP(DP_RDBUFF, &ret);
	io_word.Long = addr;
	g_protocol_error |= SW_ShiftPacket(0x8B, 0);
	g_protocol_error |= SW_ShiftPacket(0x9f, 0);
	g_protocol_error |= SW_ShiftPacket(SW_RDBUFF_RD, 0);

	return io_word.Long;
}

void writeMem(uint32_t addr, uint32_t wdata)
{
// writeAP(AP_TAR, addr);
// writeAP(AP_DRW, data);
	io_word.Long = addr;
	g_protocol_error |= SW_ShiftPacket(0x8B, 0);

	io_word.Long = wdata;
	g_protocol_error |= SW_ShiftPacket(0xBB, 0);
}

BOOL writeCpuReg(uint32_t reg, uint32_t value)
{
#define CoreDebug_DHCSR_S_REGRDY_Pos       16                                             /*!< CoreDebug DHCSR: S_REGRDY Position */
#define CoreDebug_DHCSR_S_REGRDY_Msk       (1UL << CoreDebug_DHCSR_S_REGRDY_Pos)          /*!< CoreDebug DHCSR: S_REGRDY Mask */

	g_protocol_error = SW_ACK_OK;

	/* Wait until debug register is ready to accept new data */
	do {
		readMem( DHCSR );
		if( SW_ACK_OK != g_protocol_error )
		{
			return false;
		}
	} while ( !(io_word.Long & CoreDebug_DHCSR_S_REGRDY_Msk) );

	/* Write value to Data Register */
	writeMem( DCRDR, value );

	/* Write register number ot Selector Register. This will update the CPU register */
	writeMem( DCRSR, 0x10000 | reg );
	return true;
}

BOOL HaltCore( BOOL halt )
{
	char rtn;

	/* Commands to run/step and let CPU run.*/
	//writeAP(AP_TAR, (uint32_t)&(CoreDebug->DHCSR));
	//io_word.Long = DHCSR;
	//SW_ShiftPacket(0x8B, 0);

	// Stop core
	//io_word.Long = STOP_CMD;
	//SW_ShiftPacket(0xBB, 0);
#define CoreDebug_DHCSR_S_HALT_Pos         17                                             /*!< CoreDebug DHCSR: S_HALT Position */
#define CoreDebug_DHCSR_S_HALT_Msk         (1UL << CoreDebug_DHCSR_S_HALT_Pos)            /*!< CoreDebug DHCSR: S_HALT Mask */

	if( halt == true )
	{
		writeMem(DHCSR, STOP_CMD);
		for( rtn = 0; rtn < 3; rtn++ )
		{
			SW_ShiftPacket(0x9f, 0);
			SW_ShiftPacket(SW_RDBUFF_RD, 0);
			//printf("Halt core = 0x%08lx ", io_word.Long);
			if( io_word.Long & CoreDebug_DHCSR_S_HALT_Msk )
			{
				//break;
			}
		}

		if ( !(io_word.Long & CoreDebug_DHCSR_S_HALT_Msk) )
		{
			//printf( "failed\n" );
			return false;
		}
		else
		{
			//printf( "successful\n" );
		}
	}
	else
	{
		writeMem(DHCSR, RUN_CMD);
		for( rtn = 0; rtn < 3; rtn++ )
		{
			SW_ShiftPacket(0x9f, 0);
			SW_ShiftPacket(SW_RDBUFF_RD, 0);
			//printf("Run core = 0x%08lx ", io_word.Long);
			if( (io_word.Long & CoreDebug_DHCSR_S_HALT_Msk) == 0 )
			{
				//break;
			}
		}
		if ( io_word.Long & CoreDebug_DHCSR_S_HALT_Msk )
		{
			//printf( "failed\n" );
			return false;
		}
		else
		{
			//printf( "successful\n" );
		}
	}

	return true;
}

#define CoreDebug_DEMCR_VC_CORERESET_Pos    0                                             /*!< CoreDebug DEMCR: VC_CORERESET Position */
#define CoreDebug_DEMCR_VC_CORERESET_Msk   (1UL << CoreDebug_DEMCR_VC_CORERESET_Pos)      /*!< CoreDebug DEMCR: VC_CORERESET Mask */

#define CoreDebug_DHCSR_S_RESET_ST_Pos     25                                             /*!< CoreDebug DHCSR: S_RESET_ST Position */
#define CoreDebug_DHCSR_S_RESET_ST_Msk     (1UL << CoreDebug_DHCSR_S_RESET_ST_Pos)        /*!< CoreDebug DHCSR: S_RESET_ST Mask */

#define SCB_AIRCR_VECTKEY_Pos              16                                             /*!< SCB AIRCR: VECTKEY Position */
#define SCB_AIRCR_VECTKEY_Msk              (0xFFFFUL << SCB_AIRCR_VECTKEY_Pos)            /*!< SCB AIRCR: VECTKEY Mask */

#define SCB_AIRCR_VECTCLRACTIVE_Pos         1                                             /*!< SCB AIRCR: VECTCLRACTIVE Position */
#define SCB_AIRCR_VECTCLRACTIVE_Msk        (1UL << SCB_AIRCR_VECTCLRACTIVE_Pos)           /*!< SCB AIRCR: VECTCLRACTIVE Mask */

#define SCB_AIRCR_VECTRESET_Pos             0                                             /*!< SCB AIRCR: VECTRESET Position */
#define SCB_AIRCR_VECTRESET_Msk            (1UL << SCB_AIRCR_VECTRESET_Pos)               /*!< SCB AIRCR: VECTRESET Mask */

#define SCS_BASE            (0xE000E000UL)                            /*!< System Control Space Base Address  */
#define AIRCR  (SCS_BASE+0xc)

BOOL resetAndHaltCore( void )
{
	uint16_t i;

	g_protocol_error = SW_ACK_OK;

	/* Set halt-on-reset bit */
	writeMem( DEMCR, CoreDebug_DEMCR_VC_CORERESET_Msk);

	/* Clear exception state and reset target */
	writeMem( AIRCR, (0x05FA << SCB_AIRCR_VECTKEY_Pos) | SCB_AIRCR_VECTCLRACTIVE_Msk | SCB_AIRCR_VECTRESET_Msk);

	/* Check if we timed out */
	for( i = 0; i < 100; i ++ )
	{
		if( SW_ACK_OK != g_protocol_error )
		{
			return false;
		}

		readMem( DHCSR );
		//printf("DHCSR = 0x%08lx\n", io_word.Long);
		if (!( io_word.Long & CoreDebug_DHCSR_S_RESET_ST_Msk ))
		{
			break;
		}
	}

	/* Verify that target is halted */
	if ( !(io_word.Long & CoreDebug_DHCSR_S_HALT_Msk) )
	{
	}

	return true;
#if 0
	/* Clear exception state and reset target */
	//writeAP(AP_TAR, (uint32_t)&(SCB->AIRCR));
	//writeAP(AP_DRW, (0x05FA << SCB_AIRCR_VECTKEY_Pos) |
	//				SCB_AIRCR_VECTCLRACTIVE_Msk |
	//				SCB_AIRCR_VECTRESET_Msk);

	/* Wait for target to reset */
	do {
	  delayUs(10);
	  timeout--;
	  dhcsr = readMem((uint32_t)&(CoreDebug->DHCSR));
	} while ( dhcsr & CoreDebug_DHCSR_S_RESET_ST_Msk );


	/* Check if we timed out */
	dhcsr = readMem((uint32_t)&(CoreDebug->DHCSR));
	if ( dhcsr & CoreDebug_DHCSR_S_RESET_ST_Msk )
	{
	  RAISE(SWD_ERROR_TIMEOUT_WAITING_RESET);
	}

	/* Verify that target is halted */
	if ( !(dhcsr & CoreDebug_DHCSR_S_HALT_Msk) )
	{
	  RAISE(SWD_ERROR_TARGET_NOT_HALTED);
	}
#endif
}

#define WAIT_AP_NUM  3
int8_t SWD_Connect(void)
{
	uint32_t i;

#if !defined(TOOLSTICK)
    // Route UART TX0, RX0 to pins P0.4 (Unused) and P0.5 (SWO_IN)
    XBR0 |= 0x01;
#endif
    // Initialize IO pins for SWD interface
    _SetSWPinsIdle;
    nSRST_Out = 0;
    while(nSRST_Out==1);
    delayms(1);
    nSRST_Out = 1;
    while(nSRST_Out==0);
    delayms(100);

    // Select the Serial Wire Debug Port
    SW_ShiftReset();
    SW_ShiftByteOut(0x9E);
    SW_ShiftByteOut(0xE7);

    //SW_ShiftReset();
    //SW_ShiftByteOut(0xB6);
    //SW_ShiftByteOut(0xED);

    // Complete SWD reset sequence (50 cycles high followed by 2 or more idle cycles)
    SW_ShiftReset();
    SW_ShiftByteOut(0);
    SW_ShiftByteOut(0);

    // Now read the DPIDR register to move the SWD out of reset
    SW_ShiftPacket(SW_IDCODE_RD, 0);

    printf("DPID = 0x%08lx", io_word.Long);

	switch( io_word.Long )
	{
		case EFM32_DPID_1:
		case EFM32_DPID_2:
		case EFM32_DPID_3:
		case EFM32_DPID_4:
			break;
		case ST_BLUENRG_DPID:
			break;
		//case GOOGIX_G55_DPID:
		//	break;
		default:
			printf(" unknown\n");
			g_protocol_error = SW_ACK_FAULT;
			goto CONN_ERROR;
	}

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
		g_protocol_error = SW_ACK_FAULT;
		goto CONN_ERROR;
    }

	/* Verify that the AP returns the correct ID */
	for( i = 0; i < WAIT_AP_NUM; i++ )
	{
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
		printf(", APID = 0x%08lx", io_word.Long);

		/* Valid values for the AHB-AP IDR register */
		switch(io_word.Long)
		{
			case EFM32_AHBAP_ID_1:
			case EFM32_AHBAP_ID_2:
			case EFM32_AHBAP_ID_4:
			case ST_BLUENRG_APID:
				i = WAIT_AP_NUM + 1;
				break;
			//case GOOGIX_G55_APID:
			//	break;
			case EFM32_LOCKED_CHIP1:
			case EFM32_LOCKED_CHIP2:
				unlockChip();
				printf("Locked chip\n");
				return 2;
			default:
				printf(" unknown");
				g_protocol_error = SW_ACK_FAULT;
				goto CONN_ERROR;
		}
	}

	if( i == WAIT_AP_NUM )
	{
		printf(" unknown");
		g_protocol_error = SW_ACK_FAULT;
		goto CONN_ERROR;
	}

	printf(", ");

	io_word.Long = 0x1e;
	SW_ShiftPacket(0x81,0);

	io_word.Long = 0x00;
	SW_ShiftPacket(SW_SELECT_WR, 0);

	// set 32 bits word size, no inc
	io_word.Long = DHCSR;
	SW_ShiftPacket(0x8B, 0);
	io_word.Long = 0x23000002;
	SW_ShiftPacket(0xA3, 0);

CONN_ERROR:
	if( SW_ACK_OK == g_protocol_error )
	{
		printf("Connected\n");
	}
	else
	{
		printf("Connect failed\n");
	}

	return 0;
}

