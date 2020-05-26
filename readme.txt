SWD-programmer-use-Silicon-Labs-EFM8BB3-STK

Terminal: A command line tool
    VS2005 project
EFM8BB3_SWD: Programmer functions. Silicon Labs EFM32/EFR32 series chips
    Simplicity Studio project
FlashRam:
    A flash loader which download to RAM to realize the flash functions
    Put to C:\
    Keil ARM project.


Demo of Terminal.exe

Input COM<0..99>:4
\\.\COM4 open OK.
cmd>spiid
SPI Nor flash ID: 01, 40, 15
cmd>spierasechip
..........................0x00 finished
cmd>spiblankchip
Blank check spi Addr = 0x00000000, 0x00400000 Bytes
BLANK
cmd>download c:\aaa.bin 0x0 0x20000
Send file with XMODEM...CC 2000 4000 6000 8000 a000 c000 e000 10000
Downloaded 0x104b7, 66743 bytes
Successful
cmd>connect
DPID = 0x2ba01477, APID = 0x24770011, Connected
Series 0/1 chip
S0/1_PART_INFO = 0x082e00e7
S0/1_MEMORY_INFO = 0x00400200
Part number = ZGM130S037/EFR32FG13P231F512
Family = 46, Flash = 512K Bytes, RAM = 64K Bytes, PageSize = 2048 Bytes, User = 512 Bytes, ProductionID = 8
mscWriteCtrl = 0x400e0008, mscWriteCmd  = 0x400e000c, mscAddrb = 0x400e0010
    mscWdata = 0x400e0018, mscStatusReg = 0x400e001c, mscLock  = 0x400e0040, mscMassLock = 0x400e0054
LoadFlashRam finished
FlashRam counter T0 = 00000000, T1 = 00000872, T2 = 00000d67, is running
Set MSC control addr(T2) 0x400e0008
Ram loader OK
cmd>erasechip
Erase chip finished
cmd>blankchip
Blankcheck Addr = 0x00000000, 0x00080000 Bytes
BLANK
cmd>copy 0x0 0x0 0x20000
Copy from SPI 0x00000000 to flash 0x00000000, size 0x00020000 Bytes
.................................................................................................................................
Checksum SPI Addr = 0x00000000, 0x00020000 Bytes, 0x5d46f889
Checksum flash Addr = 0x00000000, 0x00020000 Bytes, 0x5d46f889
Successful
cmd>spiread 0x0 0x100
Read spi flash from 0x00000000 for 0x00000100 Bytes:
20006000 10046cd5 10045fe9 10044249 aa5555aa 424c5545 00000000 00000000 00000000 00000000 00000000 1004847f 00000000 00000000 1004654d 10048885 100428cb 100460b9 00000000 00000000 10048c85 1004847d 10040c21 10048e11 00000000 00000000 00000000 00000000 00000000 10040351 1004424d 1004424b 00000000 10045a67 10045a69 10045a6b 10045a6d 10046cf5 100460d3 100413b9 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000 8214f3ef 8109f3ef f380481c f3bf8814 b5f08f6f 464c4643 465e4655 b4f84667 f3ef4c17 60238308 4c16b406 4c166820 4c166020 4c166820 4c166020 4c166820
cmd>read 0x0 0x100
Read from 0x00000000 for 0x00000100 Bytes:
20006000 10046cd5 10045fe9 10044249 aa5555aa 424c5545 00000000 00000000 00000000 00000000 00000000 1004847f 00000000 00000000 1004654d 10048885 100428cb 100460b9 00000000 00000000 10048c85 1004847d 10040c21 10048e11 00000000 00000000 00000000 00000000 00000000 10040351 1004424d 1004424b 00000000 10045a67 10045a69 10045a6b 10045a6d 10046cf5 100460d3 100413b9 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000 8214f3ef 8109f3ef f380481c f3bf8814 b5f08f6f 464c4643 465e4655 b4f84667 f3ef4c17 60238308 4c16b406 4c166820 4c166020 4c166820 4c166020 4c166820
cmd>checksum 0x0 0x20000
Checksum flash Addr = 0x00000000, 0x00020000 Bytes, 0x5d46f889
cmd>spichecksum 0x0 0x20000
Checksum SPI Addr = 0x00000000, 0x00020000 Bytes, 0x5d46f889
cmd>



cmd>auto
DPID = 0x2ba01477, APID = 0x24770011, Connected
Series 0/1 chip
S0/1_PART_INFO = 0x082e00e7
S0/1_MEMORY_INFO = 0x00400200
Part number = ZGM130S037/EFR32FG13P231F512
Family = 46, Flash = 512K Bytes, RAM = 64K Bytes, PageSize = 2048 Bytes, User = 512 Bytes, ProductionID = 8
mscWriteCtrl = 0x400e0008, mscWriteCmd  = 0x400e000c, mscAddrb = 0x400e0010
    mscWdata = 0x400e0018, mscStatusReg = 0x400e001c, mscLock  = 0x400e0040, mscMassLock = 0x400e0054
Erase chip finished
LoadFlashRam finished
FlashRam counter T0 = 00000000, T1 = 00000871, T2 = 00000d66, is running
Set MSC control addr(T2) 0x400e0008
Ram loader OK
Copy from SPI 0x00000000 to flash 0x00000000, size 0x00080000 Bytes
.................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................
Checksum SPI Addr = 0x00000000, 0x00080000 Bytes, 0x5d457889
Checksum flash Addr = 0x00000000, 0x00080000 Bytes, 0x5d457889
Successful
cmd>
