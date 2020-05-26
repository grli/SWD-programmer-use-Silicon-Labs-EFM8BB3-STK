#ifndef VAR_H
#define VAR_H

extern volatile unsigned int REMOTE_buffer1[1024];
extern volatile unsigned int REMOTE_buffer2[1024];
extern volatile unsigned int REMOTE_bebugcounter;
extern volatile unsigned int REMOTE_flashctrltype;
extern volatile unsigned int REMOTE_flashmain_num;
extern volatile unsigned int REMOTE_command;
extern volatile unsigned int REMOTE_cmdresult;
extern volatile unsigned int REMOTE_checksum;
extern volatile unsigned int REMOTE_buffer1_rdy;
extern volatile unsigned int REMOTE_buffer2_rdy;
extern volatile unsigned int REMOTE_buffer1_start_addr;
extern volatile unsigned int REMOTE_buffer2_start_addr;
extern volatile unsigned int REMOTE_buffer1_size;
extern volatile unsigned int REMOTE_buffer2_size;

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

#endif
