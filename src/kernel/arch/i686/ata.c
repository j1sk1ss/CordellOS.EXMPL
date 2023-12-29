#include "../../include/ata.h"

///////////////////////////////////////////
//
//  READ SECTOR FROM DISK BY LBA ADRESS
//

    // Function to read a sector from the disk.
    char* ATA_read_sector(uint32_t lba) {
        ATA_ata_wait();
        char* buffer = (char*)malloc(SECTOR_SIZE);
        if (buffer == NULL) 
            return NULL;
        
        uint8_t slavebit = 0;
        uint8_t sectorCount = 1;

        x86_outb(DRIVE_REGISTER, 0xE0 | (slavebit << 4) | ((lba >> 24) & 0x0F));
        x86_outb(FEATURES_REGISTER, 0x00);
        x86_outb(SECTOR_COUNT_REGISTER, sectorCount);
        x86_outb(LBA_ADRESS_REGISTER, (uint8_t)(lba & 0xFF));
        x86_outb(CYLINDER_LOW_REGISTER, (uint8_t)((lba >> 8) & 0xFF));
        x86_outb(CYLINDER_HIGH_REGISTER, (uint8_t)((lba >> 16) & 0xFF));
        x86_outb(STATUS_REGISTER, ATA_CMD_READ_PIO);

        int timeout = 900000000;
        while ((x86_inb(STATUS_REGISTER) & ATA_SR_BSY) == 0) 
            if (--timeout < 0) return NULL;
            else continue;

        for (int n = 0; n < SECTOR_SIZE / 2; n++) {
            uint16_t value = x86_inw(DATA_REGISTER);
            buffer[n * 2] = value & 0xFF;
            buffer[n * 2 + 1] = value >> 8;
        }

        return buffer;
    }

//
//  READ SECTOR FROM DISK BY LBA ADRESS
//
///////////////////////////////////////////
//
//  WRITE DATA TO SECTOR ON DISK BY LBA ADRESS
//

    // Function to write a sector on the disk.
    int ATA_write_sector(uint32_t lba, const uint8_t* buffer) {
        if (lba == BOOT_SECTOR) return -1;

        ATA_ata_wait();
        x86_outb(DRIVE_REGISTER, 0xE0 | ((lba >> 24) & 0x0F));
        x86_outb(FEATURES_REGISTER, 0x00);
        x86_outb(SECTOR_COUNT_REGISTER, 1);
        x86_outb(LBA_ADRESS_REGISTER, (uint8_t)lba);
        x86_outb(CYLINDER_LOW_REGISTER, (uint8_t)(lba >> 8));
        x86_outb(CYLINDER_HIGH_REGISTER, (uint8_t)(lba >> 16));
        x86_outb(STATUS_REGISTER, ATA_CMD_WRITE_PIO);

        int timeout = 900000000;
        while ((x86_inb(STATUS_REGISTER) & ATA_SR_BSY) == 0) 
            if (--timeout < 0) return -1;
            else continue;
        
        // Write the sector data from the buffer.
        for (int i = 0; i < SECTOR_SIZE / 2; i++) {
            uint16_t data = *((uint16_t*)(buffer + i * 2));
            x86_outw(DATA_REGISTER, data);
        }

        return 1;
    }

//
//  WRITE DATA TO SECTOR ON DISK BY LBA ADRESS
//
///////////////////////////////////////////
//
//  OTHER FUNCTIONS
//

    // Function that add data to sector
    void ATA_append_sector(uint32_t lba, char* append_data) {
        char* previous_data = ATA_read_sector(lba);

        strcat(previous_data, append_data);
        ATA_write_sector(lba, previous_data);

        free(previous_data);
    }

    // Function that clear sector
    void ATA_clear_sector(uint32_t lba) {
        char buffer[SECTOR_SIZE];  // Assuming 512-byte sectors
        memset(buffer, 0, sizeof(buffer));

        // Write the buffer to the specified sector
        if (ATA_write_sector(lba, buffer) == -1) 
            printf("\n\rPulizia del settore non completata!");
    }

    // Function to check if a sector (by LBA) is empty (all bytes are zero)
    bool ATA_is_current_sector_empty(uint32_t lba) {
        char* sector_data = ATA_read_sector(lba);
        if (ATA_is_sector_empty(sector_data)) {
            free(sector_data);
            return true;
        }

        free(sector_data);
        return false;
    }

    // Function to check if a sector is empty (contains all zeros).
    bool ATA_is_sector_empty(const uint8_t* sector_data) {
        for (int i = 0; i < SECTOR_SIZE; i++) 
            if (sector_data[i] != 0x00) 
                return false;
            
        return true;
    }

    uint32_t ATA_find_empty_sector(uint32_t offset) {
        for (uint32_t lba = offset; lba <= SECTOR_COUNT; lba++) {
            char* sector_data = ATA_read_sector(lba);
            if (sector_data == NULL) 
                continue;

            if (ATA_is_sector_empty((const uint8_t*)sector_data)) {
                free(sector_data);
                return lba;
            }
                
            free(sector_data);
        }

        return -1;
    }

    // Delay for working with ATA
    void ATA_ata_wait() {
        int delay = 150000;
        while (--delay > 0)
            continue;
    }

//
//  OTHER FUNCTIONS
//
///////////////////////////////////////////