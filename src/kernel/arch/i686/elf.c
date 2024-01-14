#include "../../include/elf.h"


void* ELF_read(const char* path) {
    uint8_t* headerBuffer;
    uint32_t filePos = 0;

    struct FATContent* content = FAT_get_content(path);
    char* file_content = FAT_read_content(content);
    char* header_content = file_content;

    Elf32_Ehdr *ehdr = (Elf32_Ehdr *)header_content;
    if (ehdr->e_type != ET_EXEC && ehdr->e_type != ET_DYN) {
        printf("\r\nError: Program is not an executable or dynamic executable.\r\n");
        return NULL;
    }

     uint32_t mem_min = 0xFFFFFFFF, mem_max = 0;

    Elf32_Phdr *phdr = (Elf32_Phdr *)(header_content + ehdr->e_phoff);

    uint32_t alignment = 4096;
    uint32_t align = alignment;

    for (uint32_t i = 0; i < ehdr->e_phnum; i++) {
        // Only interested in loadable program sections
        if (phdr[i].p_type != PT_LOAD) continue;

        // Update max alignment as needed
        if (align < phdr[i].p_align) align = phdr[i].p_align;

        uint32_t mem_begin = phdr[i].p_vaddr;
        uint32_t mem_end = phdr[i].p_vaddr + phdr[i].p_memsz + align-1;

        mem_begin &= ~(align-1);
        mem_end &= ~(align-1);

        // Get new minimum & maximum memory bounds for all program sections
        if (mem_begin < mem_min) mem_min = mem_begin;
        if (mem_end > mem_max) mem_max = mem_end;
    }

    uint32_t buffer_size = mem_max - mem_min;
    void* exe_buffer = malloc(buffer_size);
    if (exe_buffer == NULL) {
        printf("\r\nError: Could not malloc() enough memory for program\r\n");
        return NULL;
    }

    memset(exe_buffer, 0, buffer_size);

    // Load program headers into buffer
    for (uint32_t i = 0; i < ehdr->e_phnum; i++) {
        // Only interested in loadable program sections
        if (phdr[i].p_type != PT_LOAD) continue;

        // Use relative position of program section in file, to ensure it still works correctly.
        //   With PIE executables, this means we can use any entry point or addresses, as long as
        //   we use the same relative addresses.
        // In PE files this should be equivalent to the "RVA"
        uint32_t relative_offset = phdr[i].p_vaddr - mem_min;

        // Read in p_memsz amount of data from p_offset into original file buffer,
        //   to p_vaddr (offset by a relative amount) into new buffer
        uint8_t *dst = (uint8_t *)exe_buffer + relative_offset; 
        uint8_t *src = file_content + phdr[i].p_offset;
        uint32_t len = phdr[i].p_memsz;

        memcpy(dst, src, len);
    }

    void *entry_point = (void *)((uint8_t *)exe_buffer + (ehdr->e_entry - mem_min));

    return entry_point;
}