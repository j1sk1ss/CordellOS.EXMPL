#include "../include/execute.h"


struct ELF_program* get_entry_point(char* path) {
    struct ELF_program* program_pointer;
    __asm__ volatile(
        "mov $4, %%rax\n"
        "mov %1, %%rbx\n"
        "int $0x80\n"
        "mov %%rax, %0\n"
        : "=r" (program_pointer)
        : "r" (path)
        : "%rax", "%rbx"
    );
    
    return program_pointer;
}

int execute(struct ELF_program* program, int argc, char* argv[]) {
    int (*programEntry)(int, char* argv[]) = (int (*)(int, char* argv[]))(program->entry_point);
	if (programEntry == NULL) return 0;

    int result_code = programEntry(argc, argv);
    return result_code;
}

void free_program(struct ELF_program* program) {
    for (uint32_t i = 0; i < program->pages_count; i++) 
        freep((void*)program->pages[i]);

    free(program->pages);
    free(program);
}