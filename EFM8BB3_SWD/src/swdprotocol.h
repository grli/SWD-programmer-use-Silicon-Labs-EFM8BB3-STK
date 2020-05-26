#ifndef SWD_PROTOCOL_H
#define SWD_PROTOCOL_H

// Command Status Response Codes
#define HOST_COMMAND_OK         0x55
#define HOST_INVALID_COMMAND    0x80
#define HOST_COMMAND_FAILED     0x81
#define HOST_AP_TIMEOUT         0x82
#define HOST_WIRE_ERROR         0x83
#define HOST_ACK_FAULT          0x84
#define HOST_DP_NOT_CONNECTED   0x85

// DR_Scan Poll parameter values
#define DAP_RETRY_COUNT         255

// ARM CoreSight SWD-DP packet request values
#define SW_IDCODE_RD            0xA5
#define SW_ABORT_WR             0x81
#define SW_CTRLSTAT_RD          0x8D
#define SW_CTRLSTAT_WR          0xA9
#define SW_RESEND_RD            0x95
#define SW_SELECT_WR            0xB1
#define SW_RDBUFF_RD            0xBD

// ARM CoreSight SW-DP packet request masks
#define SW_REQ_PARK_START       0x81
#define SW_REQ_PARITY           0x20
#define SW_REQ_A32              0x18
#define SW_REQ_RnW              0x04
#define SW_REQ_APnDP            0x02

// ARM CoreSight SW-DP packet acknowledge values
#define SW_ACK_OK               0x1
#define SW_ACK_WAIT             0x2
#define SW_ACK_FAULT            0x4
#define SW_ACK_PARITY_ERR       0x8


// ARM CoreSight DAP command masks
#define DAP_CMD_PACKED          0x80
#define DAP_CMD_A32             0x0C
#define DAP_CMD_RnW             0x02
#define DAP_CMD_APnDP           0x01
#define DAP_CMD_MASK            0x0F

#if 0
	sbit  SWDIO_Out = P0^3;
	sbit  SWDIO_In  = P0^3;
	sbit  SWCLK_Out = P0^1;
	sbit  nSRST_Out = P0^2;
	sbit  nSRST_In  = P0^2;

	#define _StrobeSWCLK      SWCLK_Out=0; SWCLK_Out=0; SWCLK_Out=1; SWCLK_Out=1; SWCLK_Out=0; SWCLK_Out=0;

	#define  _SetSWPinsIdle             { P0MDOUT |= 0x02; P0MDOUT &= ~0x28; P0 |= 0x28; }
	#define  _SetSWDIOasInput           { P0MDOUT &= ~0x08; P0 |= 0x08; }
	#define  _SetSWDIOasOutput          P0MDOUT |= 0x08
	#define  _ResetDebugPins            { P0MDOUT &= ~0x3A; P0 |= 0x3A; }
#else
	sbit  SWDIO_Out = P2^3;
	sbit  SWDIO_In  = P2^3;
	sbit  SWCLK_Out = P2^1;
	sbit  nSRST_Out = P2^2;
	sbit  nSRST_In  = P2^2;

	#define _StrobeSWCLK    SWCLK_Out=0; \
							SWCLK_Out=1; SWCLK_Out=1;\
							SWCLK_Out=0;

	#define  _SetSWPinsIdle             { P2MDOUT |= 0x02; P2MDOUT &= ~0x28; P2 |= 0x28; }
	#define  _SetSWDIOasInput           { P2MDOUT &= ~0x08; P2 |= 0x08; }
	#define  _SetSWDIOasOutput          P2MDOUT |= 0x08
	#define  _ResetDebugPins            { P2MDOUT &= ~0x3A; P2 |= 0x3A; }
#endif

#define LED_ON		0
#define LED_OFF		1

#if EFM8BB3
sbit LED0_GREEN = P1^4;
sbit LED1_BLUE = P1^5;
sbit LED2_RED = P1^6;
#endif

#if EFM8UB2
sbit LED0_GREEN = P1^6;
sbit LED1_BLUE = P1^7;
sbit LED2_RED = P2^0;
#endif

#define KEY_PRESSED  	0
#define KEY_RELEASED   	1

#if EFM8BB3
sbit KEY0 = P0^2;
sbit KEY1 = P0^3;
#endif

#if EFM8UB2
sbit KEY0 = P3^0;
sbit KEY1 = P3^0;
#endif

extern uint8_t g_protocol_error;
extern volatile BE_LONG bdata io_word;

extern uint32_t readMem(uint32_t addr);
extern void writeMem(uint32_t addr, uint32_t wdata);
extern BOOL writeCpuReg(uint32_t reg, uint32_t value);
extern BOOL HaltCore( BOOL halt );
extern uint8_t SW_ShiftPacket(uint8_t request, uint8_t retry);
extern BOOL resetAndHaltCore( void );

#define DHCSR 				0xE000EDF0
#define DCRSR 				0xE000EDF4                   /*!< Offset: 0x004 ( /W)  Debug Core Register Selector Register        */
#define DCRDR 				0xE000EDF8                   /*!< Offset: 0x008 (R/W)  Debug Core Register Data Register            */
#define DEMCR 				0xE000EDFC                   /*!< Offset: 0x00C (R/W)  Debug Exception and Monitor Control Register */

/* Commands to run/step and let CPU run. Write these to DHCSR */
#define RUN_CMD  			0xA05F0001
#define STOP_CMD 			0xA05F0003
#define STEP_CMD 			0xA05F0005

/* Valid values for the DP IDCODE register */
#define EFM32_DPID_1      	0x2BA01477  // G, LG, GG, TG, WG
#define EFM32_DPID_2      	0x0BC11477  // ZG
#define EFM32_DPID_3      	0x0bc12477  // SWMD
#define EFM32_DPID_4      	0x6ba02477  // EFR32MG21

#define ST_BLUENRG_DPID		0x0bb11477
#define GOOGIX_GR55_DPID 	0x2BA01477

/* Valid values for the AHB-AP IDR register */
#define EFM32_AHBAP_ID_1  	0x24770011 // G, LG, GG, TG, WG
#define EFM32_AHBAP_ID_2  	0x04770031 // ZG, HG
#define EFM32_AHBAP_ID_4  	0x84770001 // XG21

#define ST_BLUENRG_APID 	0x04770021
#define GOOGIX_GR55_APID	0x24770011

#define EFM32_LOCKED_CHIP1 	0x16e60001
#define EFM32_LOCKED_CHIP2 	0x26e60011


#endif
