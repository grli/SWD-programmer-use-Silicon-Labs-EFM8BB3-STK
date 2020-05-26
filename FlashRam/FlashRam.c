#include <stdio.h>
#include "var.h"

void flash_process( void );

int main (void) 
{
//    *((unsigned int*)0x40052000) |= 0x80000000;
    flash_process();
}

#define CMD_FINISHED            1
#define CMD_FLASH_CTRL_ADDR     2
#define CMD_BLANK_CHECK         3
#define CMD_PAGE_ERASE          4
#define CMD_MASS_ERASE          5
#define CMD_WRITE_DATA          6
#define CMD_CHECKSUM            7

#define CMD_RESULT_SUCCESS      0x11
#define CMD_RESULT_FAILURE      0x12
#define CMD_RESULT_NONE         0x13

#define BUFFER_STATUS_READY     0x21
#define BUFFER_STATUS_FREE      0x22

volatile unsigned int *mscWriteCtrl;
volatile unsigned int *mscWriteCmd;
volatile unsigned int *mscAddrb;
volatile unsigned int *mscWdata;
volatile unsigned int *mscStatusReg;
volatile unsigned int *mscMassLock;
volatile unsigned int *mscLock;

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
#define MSC_MISCLOCKWORD_S2     0x40

#define MSC_WRITECTRL_WREN       (0x1UL << 0)   /**< Enable Write/Erase Controller  */
#define MSC_WRITECMD_ERASEPAGE   (0x1UL << 1)   /**< Erase Page */
#define MSC_WRITECMD_LADDRIM     (0x1UL << 0)   /**< Load MSC_ADDRB into ADDR */
#define MSC_STATUS_BUSY          (0x1UL << 0)   /**< Erase/Write Busy */
#define MSC_WRITECMD_WRITEONCE   (0x1UL << 3)   /**< Word Write-Once Trigger */
#define MSC_STATUS_WDATAREADY    (0x1UL << 3)   /**< WDATA Write Ready */
#define MSC_WRITECMD_ERASEMAIN0  (0x1UL << 8)   /**< Mass erase region 0 */
#define MSC_WRITECMD_ERASEMAIN1  (0x1UL << 9)   /**< Mass erase region 1 */

void flash_process()
{
    unsigned int ret;

    REMOTE_flashmain_num = 1;
    
	while(1)
    {
        REMOTE_bebugcounter++;
        switch( REMOTE_command )
        {
            case CMD_FLASH_CTRL_ADDR:
                ret = CMD_RESULT_SUCCESS;                
                switch( REMOTE_flashctrltype )
                {
                    case FLASH_CONTROL_ADDR_T1:
                        mscWriteCtrl = (unsigned int*)(MSCBASE_ADDR_P1 + WRITECTRL_OFFSET);
                        mscWriteCmd = (unsigned int*)(MSCBASE_ADDR_P1 + WRITECMD_OFFSET);
                        mscAddrb = (unsigned int*)(MSCBASE_ADDR_P1 + ADDRB_OFFSET);
                        mscWdata = (unsigned int*)(MSCBASE_ADDR_P1 + WDATA_OFFSET);
                        mscStatusReg = (unsigned int*)(MSCBASE_ADDR_P1 + STATUS_OFFSET);
                        mscMassLock = (unsigned int*)(MSCBASE_ADDR_P1 + MASSLOCK_OFFSET);
                        mscLock = (unsigned int*)(MSCBASE_ADDR_P1 + MSCLOCK_OFFSET);
                        break;
                    case FLASH_CONTROL_ADDR_T2:
                        mscWriteCtrl = (unsigned int*)(MSCBASE_ADDR_P2 + WRITECTRL_OFFSET);
                        mscWriteCmd = (unsigned int*)(MSCBASE_ADDR_P2 + WRITECMD_OFFSET);
                        mscAddrb = (unsigned int*)(MSCBASE_ADDR_P2 + ADDRB_OFFSET);
                        mscWdata = (unsigned int*)(MSCBASE_ADDR_P2 + WDATA_OFFSET);
                        mscStatusReg = (unsigned int*)(MSCBASE_ADDR_P2 + STATUS_OFFSET);
                        mscMassLock = (unsigned int*)(MSCBASE_ADDR_P2 + MASSLOCK_OFFSET);
                        mscLock = (unsigned int*)(MSCBASE_ADDR_P2 + MSCLOCK_OFFSET);
                        break;
                    case FLASH_CONTROL_ADDR_T3:
                   	    mscWriteCtrl = (unsigned int*)(MSCBASE_ADDR_S2 + WRITECTRL_OFFSET_S2);
                        mscWriteCmd = (unsigned int*)(MSCBASE_ADDR_S2 + WRITECMD_OFFSET_S2);
                        mscAddrb = (unsigned int*)(MSCBASE_ADDR_S2 + ADDRB_OFFSET_S2);
                        mscWdata = (unsigned int*)(MSCBASE_ADDR_S2 + WDATA_OFFSET_S2);
                        mscLock = (unsigned int*)(MSCBASE_ADDR_S2 + MSCLOCK_OFFSET_S2);
                        mscMassLock = (unsigned int*)(MSCBASE_ADDR_S2 + MSC_MISCLOCKWORD_S2);
                        mscStatusReg = (unsigned int*)(MSCBASE_ADDR_S2 + STATUS_OFFSET_S2);
                        break;
                    case FLASH_CONTROL_ADDR_T4:
                        mscWriteCtrl = (unsigned int*)(MSCBASE_ADDR_P3 + WRITECTRL_OFFSET);
                        mscWriteCmd = (unsigned int*)(MSCBASE_ADDR_P3 + WRITECMD_OFFSET);
                        mscAddrb = (unsigned int*)(MSCBASE_ADDR_P3 + ADDRB_OFFSET);
                        mscWdata = (unsigned int*)(MSCBASE_ADDR_P3 + WDATA_OFFSET);
                        mscStatusReg = (unsigned int*)(MSCBASE_ADDR_P3 + STATUS_OFFSET);
                        mscMassLock = (unsigned int*)(MSCBASE_ADDR_P3 + MASSLOCK_OFFSET);
                        mscLock = (unsigned int*)(MSCBASE_ADDR_P3 + MSCLOCK_OFFSET);
                        break;
                    default:
                        ret = CMD_RESULT_FAILURE;
                }
                
                if( ret == CMD_RESULT_SUCCESS )
                {
                    *mscLock = 0x1B71;
                    *mscWriteCtrl |= MSC_WRITECTRL_WREN;
                    
                    *mscMassLock = 0x0;
                }
                
                REMOTE_cmdresult = ret;
                REMOTE_command = CMD_FINISHED;
                break;

            case CMD_BLANK_CHECK:
                REMOTE_cmdresult = 0xffffffff;
                for( ret = REMOTE_buffer1_start_addr; ret < REMOTE_buffer1_start_addr + REMOTE_buffer1_size; ret+=4 )
                {
                    if( *((unsigned int*)ret) != 0xffffffff )
                    {
                        REMOTE_cmdresult = ret;
                        break;
                    }
                }

                REMOTE_command = CMD_FINISHED;
                break;

            case CMD_PAGE_ERASE:
                *mscAddrb = REMOTE_buffer1_start_addr;
                *mscWriteCmd = MSC_WRITECMD_LADDRIM;
                *mscWriteCmd = MSC_WRITECMD_ERASEPAGE;

                REMOTE_cmdresult = CMD_RESULT_FAILURE;
                for( unsigned int timeOut = 0; timeOut < 0xfffffff; timeOut++ )
                {
                    if((*mscStatusReg & MSC_STATUS_BUSY) == 0)
                    {
                        REMOTE_cmdresult = CMD_RESULT_SUCCESS;
                        break;
                    }
                }
                REMOTE_command = CMD_FINISHED;
                break;

            case CMD_MASS_ERASE:
                *mscMassLock =0x631A; // Gecko and Tiny Gecko do not need this
                if( REMOTE_flashctrltype == FLASH_CONTROL_ADDR_T4 )
                {
                    *mscWriteCmd = MSC_WRITECMD_ERASEMAIN0 | MSC_WRITECMD_ERASEMAIN1; // GG11
                }
                else
                {
                    *mscWriteCmd = MSC_WRITECMD_ERASEMAIN0 | MSC_WRITECMD_ERASEMAIN1;
                }

                REMOTE_cmdresult = CMD_RESULT_FAILURE;
                for( unsigned int timeOut = 0; timeOut < 0xfffffff; timeOut++ )
                {
                    if((*mscStatusReg & MSC_STATUS_BUSY) == 0)
                    {
                        REMOTE_cmdresult = CMD_RESULT_SUCCESS;
                        break;
                    }
                }
                REMOTE_command = CMD_FINISHED;
                break;

            case CMD_WRITE_DATA:
 				if( REMOTE_buffer1_rdy == BUFFER_STATUS_READY )
                {
                    for( int array = 0, ret = REMOTE_buffer1_start_addr; ret < REMOTE_buffer1_start_addr + REMOTE_buffer1_size; array++, ret += 4 )
                    {
                        *mscAddrb = ret;
                        *mscWriteCmd = MSC_WRITECMD_LADDRIM;
                        *mscWdata = REMOTE_buffer1[ array ];
                        *mscWriteCmd = MSC_WRITECMD_WRITEONCE;

                        for( unsigned int timeOut = 0; timeOut < 500; timeOut++ )
                        {
                            //if((*mscStatusReg & MSC_STATUS_WDATAREADY) == 1)
                            if((*mscStatusReg & MSC_STATUS_BUSY) == 0)
                            {
                                break;
                            }
                        }
                    }
                    REMOTE_buffer1_rdy = BUFFER_STATUS_FREE;
                }

                if( REMOTE_buffer2_rdy == BUFFER_STATUS_READY )
                {
                    for( int array = 0, ret = REMOTE_buffer2_start_addr; ret < REMOTE_buffer2_start_addr + REMOTE_buffer2_size; array++, ret += 4 )
                    {
                        *mscAddrb = ret;
                        *mscWriteCmd = MSC_WRITECMD_LADDRIM;
                        *mscWdata = REMOTE_buffer2[ array ];
                        *mscWriteCmd = MSC_WRITECMD_WRITEONCE;

                        for( int timeOut = 0; timeOut < 500; timeOut++ )
                        {
                            //if((io_word.Long & MSC_STATUS_WDATAREADY) == 1)
                            if((*mscStatusReg & MSC_STATUS_BUSY) == 0)
                            {
                                break;
                            }
                        }
                    }
                    REMOTE_buffer2_rdy = BUFFER_STATUS_FREE;
                }
                break;

            case CMD_CHECKSUM:
                REMOTE_checksum = 0;
                for( ret = REMOTE_buffer1_start_addr; ret < REMOTE_buffer1_start_addr + REMOTE_buffer1_size; ret+=4 )
                {
                    REMOTE_checksum += (*((unsigned int*)ret));
                }

                REMOTE_command = CMD_FINISHED;
                break;
        }
    }
}

