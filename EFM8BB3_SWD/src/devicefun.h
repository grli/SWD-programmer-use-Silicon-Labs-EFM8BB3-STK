#ifndef DEVICE_FUN_H
#define DEVICE_FUN_H

#define REMOTE_buffer1              0x20000000
#define REMOTE_buffer2              0x20001000
#define REMOTE_bebugcounter         0x20002000
#define REMOTE_flashctrltype        0x20002004
#define REMOTE_flashmain_num   		0x20002008
#define REMOTE_command              0x2000200c
#define REMOTE_cmdresult            0x20002010
#define REMOTE_checksum             0x20002014
#define REMOTE_buffer1_rdy          0x20002018
#define REMOTE_buffer2_rdy          0x2000201c
#define REMOTE_buffer1_start_addr   0x20002020
#define REMOTE_buffer2_start_addr   0x20002024
#define REMOTE_buffer1_size         0x20002028
#define REMOTE_buffer2_size      	0x2000202c

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

#define FLASH_CONTROL_ADDR_T1   0x1
#define FLASH_CONTROL_ADDR_T2   0x2
#define FLASH_CONTROL_ADDR_T3   0x3
#define FLASH_CONTROL_ADDR_T4   0x4

extern BOOL Cmd_writeInOnePage( uint32_t addr, uint32_t bytesize, BOOL wait, BOOL inc );
extern BOOL Cmd_checksum( uint32_t addr, uint32_t bytesize, uint32_t *checksum );
extern BOOL unlockChip(void);
extern int8_t SWD_Connect(void);
extern BOOL getdeviceinfo(void);
extern BOOL LoadFlashRam( void );
extern BOOL LoadFlashRam( void );
extern BOOL CheckBootloader( void );
extern BOOL Cmd_SetFlashCtrl( void );
extern void DisplayDebug( void );
extern BOOL readDisplay( uint32_t addr, uint32_t bytesize );
extern BOOL writeWordToFlash(uint32_t addr, uint32_t wdata);
extern BOOL eraseChip(void);
extern BOOL Cmd_eraseChip( void );
extern BOOL erasePage( uint32_t address );
extern BOOL Cmd_erasePage( uint32_t addr );
extern BOOL Cmd_blankcheck( uint32_t addr, uint32_t bytesize );

#endif
