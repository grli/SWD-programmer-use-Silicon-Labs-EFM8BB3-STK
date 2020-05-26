#include "var.h"

volatile unsigned int REMOTE_bebugcounter = 0;
volatile unsigned int REMOTE_flashctrltype = 0;
volatile unsigned int REMOTE_flashmain_num = 0;
volatile unsigned int REMOTE_command = CMD_FINISHED;
volatile unsigned int REMOTE_cmdresult = CMD_RESULT_NONE;
volatile unsigned int REMOTE_checksum = 0;
volatile unsigned int REMOTE_buffer1_rdy = BUFFER_STATUS_FREE;
volatile unsigned int REMOTE_buffer2_rdy = BUFFER_STATUS_FREE;
volatile unsigned int REMOTE_buffer1_start_addr = 0;
volatile unsigned int REMOTE_buffer2_start_addr = 0;
volatile unsigned int REMOTE_buffer1_size = 0;
volatile unsigned int REMOTE_buffer2_size = 0;
