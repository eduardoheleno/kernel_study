#include "filesystem/disk.h"
#include "misc.h"
#include "tty.h"

static void wait_bsy(void)
{
    while (inb(STATUS_COMMAND_REGISTER) & BSY_STATUS);
}

static void wait_drq(void)
{
    while (!(inb(STATUS_COMMAND_REGISTER) & DRQ_STATUS));
}

int init_disk(void)
{
    outb(DRIVE_REGISTER, MASTER);
    outb(SECTOR_COUNT_REGISTER, 0);
    outb(LBA_LO, 0);
    outb(LBA_MID, 0);
    outb(LBA_HI, 0);
    outb(STATUS_COMMAND_REGISTER, CMD_IDENTIFY);

    uint8_t status = inb(STATUS_COMMAND_REGISTER);
    if (status == 0)
    {
        terminal_writestring("No device found :(\n");
        return -1;
    }

    wait_bsy();
    while (1)
    {
        status = inb(STATUS_COMMAND_REGISTER);
        if (status & ERR_STATUS)
        {
            terminal_writestring("IDENTIFY error \n");
            return -1;
        }

        if (status & DRQ_STATUS) break;
    }

    terminal_writestring("Master device found!\n");

    uint16_t identify[256];
    for (int i = 0; i < 256; i++)
    {
        identify[i] = inw(BASE);
    }

    return 1;
}

// TODO: check possible error
int disk_read(uint32_t lba, uint32_t sectors, uint16_t* buffer)
{
    wait_bsy();

    outb(DRIVE_REGISTER, 0xE0 | ((lba >> 24) & 0x0F));
    outb(SECTOR_COUNT_REGISTER, sectors);
    outb(LBA_LO, (uint8_t)lba);
    outb(LBA_MID, (uint8_t)(lba >> 8));
    outb(LBA_HI, (uint8_t)(lba >> 16));

    outb(STATUS_COMMAND_REGISTER, CMD_READ);

    wait_bsy();
    wait_drq();

    for (int i = 0; i < 256; i++)
    {
        buffer[i] = inw(BASE);
    }
    
    return 1;
}

// TODO: check possible error
int disk_write(uint32_t lba, uint32_t sectors, const uint8_t* buffer)
{
    wait_bsy();
    
    outb(DRIVE_REGISTER, 0xE0 | ((lba >> 24) & 0x0F));
    outb(SECTOR_COUNT_REGISTER, sectors);
    outb(LBA_LO, (uint8_t)lba);
    outb(LBA_MID, (uint8_t)(lba >> 8));
    outb(LBA_HI, (uint8_t)(lba >> 16));

    outb(STATUS_COMMAND_REGISTER, CMD_WRITE);

    wait_bsy();
    wait_drq();

    uint16_t* ptr = (uint16_t*)buffer;
    for (uint32_t i = 0; i < sectors; i++)
    {
        wait_bsy();
        wait_drq();

        for (uint32_t j = 0; j < 256; j++)
        {
            outw(BASE, ptr[j]);
        }
    }

    outb(STATUS_COMMAND_REGISTER, CMD_CACHE_FLUSH);
    wait_bsy();

    return 1;
}
