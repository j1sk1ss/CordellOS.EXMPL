#include "../include/syscalls.h"


void i386_syscalls_init() {
    i386_isr_registerHandler(0x80, syscall);
}
 
void syscall(struct Registers* regs) {

    //=======================
    //  PRINT SYSCALLS
    //=======================
        
        if (regs->eax == SYS_SCROLL) {
            int lines = (int)regs->ebx;
            GFX_scrollback_buffer(lines, GFX_data.physical_base_pointer);
        }

        //=======================
        //  KEYBOARD SYSCALLS
        //=======================
            
            else if (regs->eax == SYS_GET_KEY_KEYBOARD) {
                char* key_buffer = (char*)regs->ecx;
                key_buffer[0] = pop_character();
            } 
            
            else if (regs->eax == SYS_AREAD_KEYBOARD) {
                _enable_keyboard();
            } 

        //=======================
        //  KEYBOARD SYSCALLS
        //=======================

    //=======================
    //  PRINT SYSCALLS
    //=======================
    //  SYSTEM SYSCALLS
    //=======================

        else if (regs->eax == SYS_WRITE) {
            int destination = (int)regs->ebx;
            uint32_t source = (uint32_t)regs->ecx;
            size_t size     = (size_t)regs->edx;

            if (destination == 1) {
                memcpy(
                    (void*)GFX_data.virtual_second_buffer, (void*)VMM_virtual2physical((void*)source), size
                );
            }
            else if (destination == 2) {
                memcpy(
                    (void*)GFX_data.physical_base_pointer, (void*)VMM_virtual2physical((void*)source), size
                );
            }
            else {
                memcpy(
                    (void*)destination, (void*)VMM_virtual2physical((void*)source), size
                );
            }
        }

        else if (regs->eax == SYS_SWITCH_USER) {
            i386_switch2user();
        }

        else if (regs->eax == SYS_TIME) {
            _datetime_read_rtc();
            short* date_buffer = (short*)regs->ecx;
            date_buffer[0] = DTM_datetime.datetime_second;
            date_buffer[1] = DTM_datetime.datetime_minute;
            date_buffer[2] = DTM_datetime.datetime_hour;
            date_buffer[3] = DTM_datetime.datetime_day;
            date_buffer[4] = DTM_datetime.datetime_month;
            date_buffer[5] = DTM_datetime.datetime_year;
        } 

        else if (regs->eax == SYS_GET_TICKS) {
            regs->eax = DTM_get_ticks();
        }

        //=======================
        //  SYSTEM TASKING SYSCALLS
        //=======================

            else if (regs->eax == SYS_KILL_PROCESS) __kill();
            
            else if (regs->eax == SYS_START_PROCESS) {
                char* process_name = (char*)regs->ebx;
                uint32_t address   = (uint32_t)regs->ecx;
                uint32_t delay     = (uint32_t)regs->edx;
                START_PROCESS(process_name, address, USER, delay);
            }

            else if (regs->eax == SYS_GET_PID) {
                regs->eax = taskManager.tasks[taskManager.currentTask]->pid;
            }

        //=======================
        //  SYSTEM TASKING SYSCALLS
        //=======================
        //  SYSTEM MEMMANAGER SYSCALLS
        //=======================

            else if (regs->eax == SYS_PAGE_FREE) {
                void* ptr_to_free = (void*)regs->ebx;
                if (ptr_to_free != NULL)
                    _kfreep(ptr_to_free);
            }

#ifndef USERMODE
            else if (regs->eax == SYS_MALLOC) {
                uint32_t size = regs->ebx;
                void* allocated_memory = _kmalloc(size);
                regs->eax = (uint32_t)allocated_memory;
            } 
            
            else if (regs->eax == SYS_PAGE_MALLOC) {
                uint32_t address = regs->ebx;
                _kmallocp(address);
                regs->eax = address;
            } 

            else if (regs->eax == SYS_FREE) {
                void* ptr_to_free = (void*)regs->ebx;
                if (ptr_to_free != NULL)
                    _kfree(ptr_to_free);
            }
#elif defined(USERMODE)
            else if (regs->eax == SYS_MALLOC) {
                uint32_t size = regs->ebx;
                void* allocated_memory = _umalloc(size);
                regs->eax = (uint32_t)allocated_memory;
            } 
            
            else if (regs->eax == SYS_PAGE_MALLOC) {
                uint32_t address = regs->ebx;
                _umallocp(address);
                regs->eax = address;
            } 

            else if (regs->eax == SYS_FREE) {
                void* ptr_to_free = (void*)regs->ebx;
                if (ptr_to_free != NULL)
                    _ufree(ptr_to_free);
            }
#endif

            else if (regs->eax == SYS_KERN_PANIC) {
                char* message = (char*)regs->ecx;
                kernel_panic(message);
            }

        //=======================
        //  SYSTEM MEMMANAGER SYSCALLS
        //=======================
        //  SYSTEM VARS SYSCALLS
        //=======================

            else if (regs->eax == SYS_ADD_VAR) {
                char* name  = (char*)regs->ebx;
                char* value = (char*)regs->ecx;
                VARS_add(name, value);
            }

            else if (regs->eax == SYS_EXST_VAR) {
                char* name = (char*)regs->ebx;
                regs->eax  = VARS_exist(name);
            }

            else if (regs->eax == SYS_SET_VAR) {
                char* name  = (char*)regs->ebx;
                char* value = (char*)regs->ecx;
                VARS_set(name, value);
            }

            else if (regs->eax == SYS_GET_VAR) {
                char* name = (char*)regs->ebx;
                regs->eax  = (uint32_t)VARS_get(name);
            }

            else if (regs->eax == SYS_DEL_VAR) {
                char* name = (char*)regs->ebx;
                VARS_delete(name);
            }

        //=======================
        //  SYSTEM VARS SYSCALLS
        //=======================

    //=======================
    //  SYSTEM SYSCALLS
    //=======================
    //  FILE SYSTEMS SYSCALLS
    //=======================
        
        else if (regs->eax == SYS_OPENDIR) {
            regs->eax = current_vfs->lsdir((int)regs->ebx, (char)0, 0);
        }

        else if (regs->eax == SYS_LSDIR) {
            int root_ci = (int)regs->ebx;
            int step = (int)regs->edx;
            char* cname = (char*)regs->ecx;

            int local_step = 0;
            Content* root_node = _get_content_from_table(root_ci);
            
            if (root_node != NULL) {
                Directory* root_dir = root_node->directory;
                if (root_dir->subDirectory != NULL) {
                    Directory* curr_dir = root_dir->subDirectory;
                    while (curr_dir != NULL) {
                        if (local_step == step) {
                            strncpy(cname, curr_dir->name, 11);
                            goto ls_end;
                        }

                        curr_dir = curr_dir->next;
                        local_step++;
                    }
                }

                if (root_dir->files != NULL) {
                    File* curr_file = root_dir->files;
                    while (curr_file != NULL) {
                        if (local_step == step) {
                            sprintf(cname, 11, "%s.%s", curr_file->name, curr_file->extension);
                            goto ls_end;
                        }

                        curr_file = curr_file->next;
                        local_step++;
                    }
                }
            }
ls_end:
            if (local_step == step) regs->eax = step + 1;
            else regs->eax = -1;
        } 
        
        else if (regs->eax == SYS_OPEN_CONTENT) {
            regs->eax = current_vfs->openobj((char*)regs->ebx);
        } 
        
        else if (regs->eax == SYS_CONTENT_STAT) {
            CInfo_t info;
            current_vfs->objstat(regs->ebx, &info);
            memcpy((CInfo_t*)regs->ecx, &info, sizeof(CInfo_t));
        }

        else if (regs->eax == SYS_CLOSE_CONTENT) {
            current_vfs->closeobj(regs->ebx);
        }

        else if (regs->eax == SYS_CEXISTS) {
            regs->eax = current_vfs->objexist((char*)regs->ebx);
        } 
        
        else if (regs->eax == SYS_FCREATE) {
            char* mkfile_path = (char*)regs->ebx;
            char* mkfile_name = (char*)regs->ecx;

            char* fname = strtok(mkfile_name, ".");
            char* fexec = strtok(NULL, "."); 

            Content* mkfile_content = FAT_create_object(fname, 0, fexec);
            current_vfs->putobj(mkfile_path, mkfile_content);
            FAT_unload_content_system(mkfile_content);
        } 
        
        else if (regs->eax == SYS_DIRCREATE) {
            char* mkdir_path = (char*)regs->ebx;
            char* mkdir_name = (char*)regs->ecx;

            Content* mkdir_content = FAT_create_object(mkdir_name, 1, "\0");
            current_vfs->putobj(mkdir_path, mkdir_content);
            FAT_unload_content_system(mkdir_content);
        } 
        
        else if (regs->eax == SYS_CDELETE) {
            char* delete_path = (char*)regs->ebx;
            current_vfs->delobj(delete_path);
        } 
        
        else if (regs->eax == SYS_CHANGE_META) {
            char* meta_path = (char*)regs->ebx;
            char* meta = (char*)regs->ecx;
            current_vfs->objmetachg(meta_path, meta);
        } 

        else if (regs->eax == SYS_READ_FILE_OFF) {
            current_vfs->read((int)regs->ebx, (uint8_t*)regs->edx, (int)regs->ecx, (int)regs->esi);
        }

        else if (regs->eax == SYS_READ_FILE_OFF_STP) {
            current_vfs->read_stop(
                (int)regs->ebx, (uint8_t*)regs->edx, (int)regs->ecx, (int)regs->esi, (uint8_t*)regs->edi
            );
        }

        else if (regs->eax == SYS_WRITE_FILE_OFF) {
            current_vfs->write((int)regs->ebx, (uint8_t*)regs->edx, (int)regs->ecx, (int)regs->esi);
        }

        else if (regs->eax == SYS_READ_ELF) {
            int ci = current_vfs->openobj((char*)regs->ebx);
            
#ifdef USERMODE
		    regs->eax = (uint32_t)ELF_read(ci, USER);
#else
		    regs->eax = (uint32_t)ELF_read(ci, KERNEL);
#endif

        }

    //=======================
    //  FILE SYSTEMS SYSCALLS
    //=======================
    //  GRAPHICS SYSCALLS
    //=======================
    
        else if (regs->eax == SYS_VPUT_PIXEL) {
            GFX_vdraw_pixel((uint16_t)regs->ebx, (uint16_t)regs->ecx, (uint32_t)regs->edx);
        } 
        
        else if (regs->eax == SYS_PPUT_PIXEL) {
            GFX_pdraw_pixel((uint16_t)regs->ebx, (uint16_t)regs->ecx, (uint32_t)regs->edx);
        } 

        else if (regs->eax == SYS_GET_PIXEL) {
            uint32_t* pixel = (uint32_t*)regs->edx;
            pixel[0] = GFX_get_pixel((uint16_t)regs->ebx, (uint16_t)regs->ecx);
        } 

        else if (regs->eax == SYS_FBUFFER_SWIPE) {
            GFX_swap_buffers();
        }

        else if (regs->eax == SYS_GET_RESOLUTION_X) {
            int* resolution = (int*)regs->edx;
            resolution[0] = GFX_data.x_resolution;
        }

        else if (regs->eax == SYS_GET_RESOLUTION_Y) {
            int* resolution = (int*)regs->edx;
            resolution[0] = GFX_data.y_resolution;
        }

    //=======================
    //  GRAPHICS SYSCALLS
    //=======================
    //  NETWORKING SYSCALLS
    //=======================

        else if (regs->eax == SYS_SET_IP) {
            uint8_t* ip = (uint8_t*)regs->ebx;
            IP_set(ip);
        }

        else if (regs->eax == SYS_GET_IP) {
            uint8_t* buffer = (uint8_t*)regs->ebx;
            IP_get(buffer);
        }

        else if (regs->eax == SYS_GET_MAC) {
            uint8_t* buffer = (uint8_t*)regs->ebx;
            get_mac_addr(buffer);
        }

        else if (regs->eax == SYS_SEND_ETH_PACKET) {
            uint8_t* dst_ip   = (uint8_t*)regs->ebx;
            uint16_t src_port = (uint16_t)regs->ecx;
            uint16_t dst_port = (uint16_t)regs->edx;
            void* data = (void*)regs->esi;
            int len    = (int)regs->edi;
            UDP_send_packet(dst_ip, src_port, dst_port, data, len);
        }

        else if (regs->eax == SYS_GET_ETH_PACKETS) {
            uint8_t* data = (uint8_t*)regs->ebx;
            struct UDPpacket* packet = UDP_pop_packet();
            if (packet == NULL) return;

            memcpy(data, packet->data, packet->data_size);

            free(packet->data);
            free(packet);
        }

        else if (regs->eax == SYS_RESTART) {
            i386_reboot();
        }

        else if (regs->eax == SYS_GET_FS_INFO) {
            uint32_t* buffer = (uint32_t*)regs->ebx;
            buffer[0] = (uint32_t)current_vfs->device->mountpoint;
            buffer[1] = (uint32_t)current_vfs->name; // TODO: Copy to user space with malloc
            buffer[2] = (uint32_t)FAT_data.fat_type;
            buffer[3] = (uint32_t)FAT_data.total_clusters;
            buffer[4] = (uint32_t)FAT_data.total_sectors;
            buffer[5] = (uint32_t)FAT_data.bytes_per_sector;
            buffer[6] = (uint32_t)FAT_data.sectors_per_cluster;
            buffer[7] = (uint32_t)FAT_data.fat_size;
        }

    //=======================
    //  NETWORKING SYSCALLS
    //=======================
}