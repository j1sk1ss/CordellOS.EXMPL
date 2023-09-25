#include "ata.h"

#include "../allocator/allocator.h"
#include "../x86/x86.h"

void wait_drive_until_ready() {
	int status = 0;
	
	do {
		status = x86_inb(BASE_PORT + 7);
	} while ( (status ^ 0x80) == 128);
}

// LBA
void *read_disk(int address) {
	x86_outb(BASE_PORT + 6, (0x0e0 | ((address & 0x0F000000 ) >> 24)));    // Drive 0. Bits 0-3 = Bits 24-27 of LBA
	x86_outb(BASE_PORT + 2, 1);                                            // Sector count
	x86_outb(BASE_PORT + 3, address & 0x000000FF);                         // LBA's 0-7 bits
	x86_outb(BASE_PORT + 4, (address & 0x0000FF00) >> 8);                  // LBA's 8-15 bits
	x86_outb(BASE_PORT + 5, (address & 0x00FF0000) >> 16);                 // LBA's 16-23 bits
	x86_outb(BASE_PORT + 7, 0x20);                                         // Command: Read with Retry
	
	// ... //
	
	wait_drive_until_ready();
	
	// ... //
	
	short *buffer = kalloc(SECTOR_SIZE);
	
	for (int currByte = 0; currByte < (SECTOR_SIZE / 2); currByte++)
		buffer[currByte] = x86_inb( BASE_PORT );
	
	return buffer;
}

// LBA
void write_disk( int address, short *buffer ) {
	x86_outb(BASE_PORT + 6, (0x0e0 | ((address & 0x0F000000 ) >> 24)));    // Drive 0. Bits 0-3 = Bits 24-27 of LBA
	x86_outb(BASE_PORT + 2, 1);                                            // Sector count
	x86_outb(BASE_PORT + 3, address & 0x000000FF);                         // LBA's 0-7 bits
	x86_outb(BASE_PORT + 4, (address & 0x0000FF00) >> 8);                  // LBA's 8-15 bits
	x86_outb(BASE_PORT + 5, (address & 0x00FF0000) >> 16);                 // LBA's 16-23 bits
	x86_outb(BASE_PORT + 7, 0x30);                                         // Command: Write with Retry
	
	wait_drive_until_ready();
	
	for ( int currByte = 0; currByte < ( SECTOR_SIZE / 2 ); currByte++ )
		x86_outw( BASE_PORT, buffer[ currByte ] );
		
	wait_drive_until_ready();
}


void *read_disk_chs(int sector) {
	x86_outb(BASE_PORT + 6, 0x0a0);      // Drive 0 and Head 0
	x86_outb(BASE_PORT + 2, 1);          // Sector count
	x86_outb(BASE_PORT + 3, sector);     // Sector to read
	x86_outb(BASE_PORT + 4, 0);          // Cylinder - Low
	x86_outb(BASE_PORT + 5, 0);          // Cylinder - High
	x86_outb(BASE_PORT + 7, 0x20);       // Command: Read with Retry
	
	// ... //
	
	wait_drive_until_ready();
	
	// ... //
	
	short *buffer = kalloc( SECTOR_SIZE );
	
	for ( int currByte = 0; currByte < ( SECTOR_SIZE / 2 ); currByte++ )
		buffer[ currByte ] = x86_inb( BASE_PORT );

	return buffer;
}

void write_disk_chs(int sector, short *buffer) {
	x86_outb(BASE_PORT + 6, 0x0a0);      // Drive 0 and Head 0
	x86_outb(BASE_PORT + 2, 1);          // Sector count
	x86_outb(BASE_PORT + 3, sector);     // Sector to read
	x86_outb(BASE_PORT + 4, 0);          // Cylinder - Low
	x86_outb(BASE_PORT + 5, 0);          // Cylinder - High
	x86_outb(BASE_PORT + 7, 0x30);       // Command: Write with Retry
	
	// ... //
	
	wait_drive_until_ready();
	
	// ... //
	
	for (int currByte = 0; currByte < ( SECTOR_SIZE / 2 ); currByte++)
		x86_outw(BASE_PORT, buffer[currByte]);
	
	// ... //
	
	wait_drive_until_ready();
}