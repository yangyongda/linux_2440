# linux_2440

烧录方式:

## uboot:
使用openjtag烧写: 接线，使用oflash烧写

## kernel:
tftp:

tftp 30000000 uImage

nand erase kernel

nand write.jffs2 30000000 kernel


## root_fs:

tftp:

tftp 30000000 fs_2440.yaffs

nand erase root

nand write.yaffs 30000000 0x00520000 $(filesize)


## 格式化nand flash

nand scrub

