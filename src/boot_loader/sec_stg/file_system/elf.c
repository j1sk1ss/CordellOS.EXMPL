#include "../include/elf.h"
#include "../include/fat.h"
#include "../include/memdefs.h"
#include "../include/memory.h"
#include "../include/math.h"
#include "../include/stdio.h"

bool ELF_Read(Partition* part, const char* path, void** entryPoint) {
    uint8_t* headerBuffer   = MEMORY_ELF_ADDR;
    uint8_t* loadBuffer     = MEMORY_LOAD_KERNEL;
    uint32_t filePos        = 0;

    uint32_t read;

    // Read header
    FAT_File* fd = FAT_open(part, path);
    if ((read = FAT_read(part, fd, sizeof(ELFHeader), headerBuffer)) != sizeof(ELFHeader)) {
        printf("[elf.c 18] FAT read error!\n");
        return false;
    }

    filePos += read;

    // validate header
    bool ok = true;
    ELFHeader* header = (ELFHeader*)headerBuffer;

    ok = ok && (memcmp(header->Magic, ELF_MAGIC, 4) != 0);
    ok = ok && (header->Bitness == ELF_BITNESS_32BIT);
    ok = ok && (header->Endianness == ELF_ENDIANNESS_LITTLE);
    ok = ok && (header->ELFHeaderVersion == 1);
    ok = ok && (header->ELFVersion == 1);
    ok = ok && (header->Type == ELF_TYPE_EXECUTABLE);
    ok = ok && (header->InstructionSet == ELF_INSTRUCTION_SET_X86);

    *entryPoint = (void*)header->ProgramEntryPosition;

    // load program header
    uint32_t programHeaderOffset            = header->ProgramHeaderTablePosition;
    uint32_t programHeaderSize              = header->ProgramHeaderTableEntrySize * header->ProgramHeaderTableEntryCount;
    uint32_t programHeaderTableEntrySize    = header->ProgramHeaderTableEntrySize;
    uint32_t programHeaderTableEntryCount   = header->ProgramHeaderTableEntryCount;

    filePos += FAT_read(part, fd, programHeaderOffset - filePos, headerBuffer);
    if ((read = FAT_read(part, fd, programHeaderSize, headerBuffer)) != programHeaderSize) {
        printf("[elf.c 46] FAT read error!\n");
        return false;
    }

    filePos += read;
    FAT_close(fd);

    // parse program header entries
    for (uint32_t i = 0; i < programHeaderTableEntryCount; i++) {
        ELFProgramHeader* progHeader = (ELFProgramHeader*)(headerBuffer + i * programHeaderTableEntrySize);
        if (progHeader->Type == ELF_PROGRAM_TYPE_LOAD) {
            // TODO: validate that the program doesn't overwrite the stage2
            uint8_t* virtAddress = (uint8_t*)progHeader->VirtualAddress;
            memset(virtAddress, 0, progHeader->MemorySize);
            
            // TODO: proper seeking
            fd = FAT_open(part, path);
            while (progHeader->Offset > 0)  {
                uint32_t shouldRead = min(progHeader->Offset, MEMORY_LOAD_SIZE);
                read = FAT_read(part, fd, shouldRead, loadBuffer);

                if (read != shouldRead) {
                    printf("[elf.c 68] FAT read error!\n");
                    return false;
                }

                progHeader->Offset -= read;
            }

            // read program
            while (progHeader->FileSize > 0) {
                uint32_t shouldRead = min(progHeader->FileSize, MEMORY_LOAD_SIZE);
                read = FAT_read(part, fd, shouldRead, loadBuffer);

                if (read != shouldRead) {
                    printf("[elf.c 81] FAT read error!\n");
                    return false;
                }

                progHeader->FileSize -= read;

                memcpy(virtAddress, loadBuffer, read);
                virtAddress += read;
            }

            FAT_close(fd);
        }
    }

    return true;
}