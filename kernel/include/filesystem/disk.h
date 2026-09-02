#ifndef _KERNEL_DISK_H
#define _KERNEL_DISK_H

#include <stdint.h>

#define MASTER 0xA0

#define BASE                    0x1F0
#define SECTOR_COUNT_REGISTER   BASE + 2
#define LBA_LO                  BASE + 3
#define LBA_MID                 BASE + 4
#define LBA_HI                  BASE + 5
#define DRIVE_REGISTER          BASE + 6
#define STATUS_COMMAND_REGISTER BASE + 7

#define CMD_IDENTIFY    0xEC
#define CMD_READ        0x20
#define CMD_WRITE       0x30
#define CMD_CACHE_FLUSH 0xE7

#define ERR_STATUS 0x01
#define BSY_STATUS 0x80
#define DRQ_STATUS 0x08

#define BLOCK_SIZE  4096
#define SECTOR_SIZE 512

int init_disk(void);
int disk_read(uint32_t lba, uint32_t sectors, uint16_t* buffer);
int disk_write(uint32_t lba, uint32_t sectors, const uint8_t* buffer);

#endif
