#include <stdint.h>

#include "../include/file_manager.h"


static int row_position = 1;
static int upper_border = 0;
static int down_border  = 14;


void keyboard_wait(char symbol) {
    while (1) {
        char user_action = SYS_keyboard_wait_key();
        if (user_action == symbol)
            break;
    }
}

char* convert_date(int day, int month, int year) {
    char* date = (char*)SYS_malloc(9);

    date[0] = '0' + (day / 10);
    date[1] = '0' + (day % 10);
    date[2] = '.';
    date[3] = '0' + (month / 10);
    date[4] = '0' + (month % 10);
    date[5] = '.';
    date[6] = '0' + ((year % 100) / 10);
    date[7] = '0' + ((year % 100) % 10);
    date[8] = '\0';

    return date;
}

int set_line_color(int line, uint8_t color) {
    for (int i = 0; i < 80; i++)
        SYS_set_scrcolor(i, (line + LINE_OFFSEET) - upper_border, color);
}

void open_file_manager(struct User* user, char* path) {
    struct UFATDirectory* current_directory = SYS_opendir(path);

    SYS_clrs();
    print_directory_data(path);

    while(1) {
        char user_action = SYS_keyboard_wait_key();
        if (user_action == UP_ARROW_BUTTON && row_position > 0) {
            if (abs(row_position - upper_border) <= 1 && upper_border >= 1) {
                upper_border--;
                down_border--;

                SYS_clrs();
                print_directory_data(path);
            }

            set_line_color(row_position--, BACKGROUND_BLACK + FOREGROUND_WHITE);
            set_line_color(row_position, BACKGROUND_RED + FOREGROUND_WHITE);
        }

        else if (user_action == DOWN_ARROW_BUTTON && row_position < down_border) {
            if (abs(row_position - down_border) <= 1) {
                upper_border++;
                down_border++;

                SYS_clrs();
                print_directory_data(path);
            }

            set_line_color(row_position++, BACKGROUND_BLACK + FOREGROUND_WHITE);
            set_line_color(row_position, BACKGROUND_RED + FOREGROUND_WHITE);
        }

        else if (user_action == ENTER_BUTTON || user_action == BACKSPACE_BUTTON) {
            execute_item(user, user_action, path);
            SYS_clrs();
            print_directory_data(path);
        }

        else if (user_action == F3_BUTTON) {
            row_position = 1;
            SYS_clrs();
            break;
        }

        else if (user_action == F1_BUTTON) {
            cprintf(BACKGROUND_BLACK + FOREGROUND_WHITE, "\nDIR NAME: ");
            char* dir_name = SYS_keyboard_read(VISIBLE_KEYBOARD, BACKGROUND_BLACK + FOREGROUND_WHITE);
            SYS_mkdir(path, dir_name);
            SYS_free(dir_name); 

            SYS_clrs();
            print_directory_data(path);
        }

        else if (user_action == F2_BUTTON) {
            cprintf(BACKGROUND_BLACK + FOREGROUND_WHITE, "\nFILE NAME: ");
            char* file_name = SYS_keyboard_read(VISIBLE_KEYBOARD, BACKGROUND_BLACK + FOREGROUND_WHITE);
            SYS_mkfile(path, file_name);
            SYS_free(file_name);

            SYS_clrs();
            print_directory_data(path);
        }

        else if (user_action == F4_BUTTON) {
            execute_item(user, user_action, path);
            SYS_clrs();
            print_directory_data(path);
        }
    }

    FATLIB_unload_directories_system(current_directory);
}

void execute_item(struct User* user, char action_type, char* path) {
    if (row_position == 0 && action_type != BACKSPACE_BUTTON) {
        path = FATLIB_change_path(path, NULL);
        if (strlen(path) <= 1) 
            path = ("BOOT");

        return;
    }
    
    ///////////////////////////////////
    //  SELECTED DIRECTORY
    //
        int rows = 1;
        struct UFATDirectory* currentDir = SYS_opendir(path);
        while (currentDir != NULL) {
            if (rows++ == row_position) {
                if (action_type == ENTER_BUTTON) {
                    path = FATLIB_change_path(path, currentDir->name);
                }

                else if (action_type == BACKSPACE_BUTTON) {
                    printf("\nDELETE? (Y/N): ");
                    while (1) {
                        char* answer = SYS_keyboard_read(VISIBLE_KEYBOARD, BACKGROUND_BLACK + FOREGROUND_WHITE);
                        if (strcmp(answer, "y") == 0) {
                            SYS_rmcontent(path, currentDir->name);
                            SYS_free(answer);
                            break;
                        }
                        
                        SYS_free(answer);
                        break;
                    }
                }

                else if (action_type == F4_BUTTON) { 
                    cprintf(BACKGROUND_BLACK + FOREGROUND_WHITE, "\nNEW DIR NAME: ");
                    char* new_dir_name = SYS_keyboard_read(VISIBLE_KEYBOARD, BACKGROUND_BLACK + FOREGROUND_WHITE);

                    udirectory_entry_t* new_meta = FATLIB_create_entry(new_dir_name, NULL, 1, NULL, NULL);
                    path = (FATLIB_change_path(path, currentDir->directory_meta.file_name));
                    SYS_chgcontent(path, new_meta);
                    path = (FATLIB_change_path(path, NULL));

                    SYS_free(new_dir_name);
                    SYS_free(new_meta);
                }

                break;
            }

            currentDir = currentDir->next;
        }

        FATLIB_unload_directories_system(currentDir);
    //
    //  SELECTED DIRECTORY
    ///////////////////////////////////

    ///////////////////////////////////
    //  SELECTED FILE
    //
        struct UFATFile* currentFile = SYS_opendir(path)->files;
        while (currentFile != NULL) {

            char name[25]; 
            FATLIB_fatname2name(currentFile->file_meta.file_name, name);

            if (rows++ == row_position) {
                row_position = 1;

                /////////////////
                //  EDIT
                //
                    if (action_type == F4_BUTTON) { 
                        while(1) {
                            SYS_scrclr(BACKGROUND_BLACK + FOREGROUND_WHITE);
                        
                            printf("\n%s", LINE);
                            const char* lines[4] = {
                                EDIT_LINE,
                                VIEW_LINE,
                                RENAME_LINE,
                                DELETE_LINE
                            };

                            for (int i = EDIT_POS; i <= DELETE_POS; i++) {
                                if (row_position == i) cprintf(BACKGROUND_RED + FOREGROUND_WHITE, lines[i]);
                                else printf(lines[i]);

                                printf(LINE);
                            }

                            char user_action = SYS_keyboard_wait_key();
                            if (user_action == UP_ARROW_BUTTON && row_position > 0) row_position--;
                            else if (user_action == DOWN_ARROW_BUTTON && row_position < 4) row_position++;
                            else if (user_action == ENTER_BUTTON) {
                                switch (row_position) {
                                    case EDIT_POS:
                                        path = (FATLIB_change_path(path, name));
                                        text_editor_init(path, BACKGROUND_BLACK + FOREGROUND_WHITE);
                                        path = (FATLIB_change_path(path, NULL));

                                    break;

                                    case VIEW_POS:
                                        SYS_clrs();

                                        path = (FATLIB_change_path(path, name));
                                        char* data = SYS_fread(path);
                                        printf("%sFILE: [%s]   [F3 - EXIT]\n%s\n\n\n%s\n\n%s", LINE, currentFile->name, LINE, data, LINE);
                                        path = (FATLIB_change_path(path, NULL));

                                        SYS_scrclr(BACKGROUND_BLACK + FOREGROUND_WHITE);
                                        SYS_free(data);
                                        keyboard_wait(F3_BUTTON);
                                    break;

                                    case RENAME_POS:
                                        cprintf(BACKGROUND_BLACK + FOREGROUND_WHITE, "\nNEW FILE NAME: ");
                                        char* new_file_name = SYS_keyboard_read(VISIBLE_KEYBOARD, BACKGROUND_BLACK + FOREGROUND_WHITE);

                                        cprintf(BACKGROUND_BLACK + FOREGROUND_WHITE, "\nNEW FILE EXT: ");
                                        char* new_file_ext = SYS_keyboard_read(VISIBLE_KEYBOARD, BACKGROUND_BLACK + FOREGROUND_WHITE);

                                        udirectory_entry_t* new_meta = FATLIB_create_entry(new_file_name, new_file_ext, 0, NULL, NULL);

                                        path = (FATLIB_change_path(path, name));
                                        SYS_chgcontent(path, new_meta);
                                        path = (FATLIB_change_path(path, NULL));
                                        
                                        SYS_free(new_file_name);
                                        SYS_free(new_file_ext);
                                        SYS_free(new_meta);

                                    break;

                                    case DELETE_POS:
                                        printf("\nDELETE? (Y/N): ");
                                        
                                        char* user_choose = SYS_keyboard_read(VISIBLE_KEYBOARD, BACKGROUND_BLACK + FOREGROUND_WHITE);
                                        if (strcmp(user_choose, "y") == 0) {
                                            SYS_rmcontent(path, currentFile->name);
                                        }

                                        SYS_clrs();
                                        SYS_free(user_choose);

                                    break;
                                }

                                return;
                            }
                            
                            else if (user_action == F3_BUTTON) {
                                SYS_clrs();
                                break;
                            }

                            SYS_clrs();
                        }
                    }
                //
                //  EDIT
                /////////////////
                //  EXECUTE
                //
                    else if (action_type == ENTER_BUTTON) {
                        if (strstr(currentFile->extension, "TXT") == 0) {
                            SYS_clrs();

                            path = (FATLIB_change_path(path, name));
                            char* data = SYS_fread(path);
                            printf("%sFile: [%s]   [F3 - EXIT]\n%s\n\n\n%s\n\n%s", LINE, currentFile->name, LINE, data, LINE);
                            path = (FATLIB_change_path(path, NULL));
                            SYS_free(data);

                            SYS_scrclr(BACKGROUND_BLACK + FOREGROUND_WHITE);

                            keyboard_wait(F3_BUTTON);

                            break;
                        }

                        if (strstr(currentFile->extension, "ELF") == 0) {
                            SYS_clrs();
                            
                            path = (FATLIB_change_path(path, name));
                            printf("\nEXIT CODE: [%i]", SYS_fexec(path, NULL, NULL));
                            path = (FATLIB_change_path(path, NULL));

                            printf("\n\nPress [F3] to exit");
                            keyboard_wait(F3_BUTTON);

                            break;
                        }

                        else if (strstr(currentFile->extension, "ASM") == 0) {
                            SYS_clrs();
                            path = (FATLIB_change_path(path, name));
                            char* data = SYS_fread(path);
                            asm_execute(data);
                            path = (FATLIB_change_path(path, NULL));

                            SYS_scrclr(BACKGROUND_BLACK + FOREGROUND_WHITE);

                            SYS_free(data);
                            printf("\n\nPress [F3] to exit");
                            keyboard_wait(F3_BUTTON);

                            break;
                        }

                        else if (strstr(currentFile->extension, "SHL") == 0) {
                            SYS_clrs();
                            SYS_scrclr(BACKGROUND_BLACK + FOREGROUND_WHITE);
                            
                            path = (FATLIB_change_path(path, name));
                            char* sector_data = SYS_fread(path);
                            path = (FATLIB_change_path(path, NULL));

                            char* command_for_split = (char*)SYS_malloc(strlen(sector_data));
                            strcpy(command_for_split, sector_data);

                            char* lines[100];
                            int tokenCount = 0;

                            char* splitted = strtok(command_for_split, "\n");
                            while(splitted) {
                                lines[tokenCount++] = splitted;
                                splitted = strtok(NULL, "\n");
                            }

                            for (int i = 0; i < tokenCount; i++) {
                                execute_command(lines[i], 2);
                                SYS_free(lines[i]);
                            }

                            SYS_free(command_for_split);
                            SYS_free(sector_data);
                            
                            printf("\n\nPress [F3] to exit");
                            keyboard_wait(F3_BUTTON);
                            
                            break;
                        }
                    }

                    else if (action_type == BACKSPACE_BUTTON) {
                        printf("\nDelete? (Y/N): ");
                                        
                        char* user_choose = SYS_keyboard_read(VISIBLE_KEYBOARD, BACKGROUND_BLACK + FOREGROUND_WHITE);
                        if (strcmp(user_choose, "y") == 0) {
                            SYS_rmcontent(path, name);
                        }

                        SYS_free(user_choose);
                    }
                //
                //  EXECUTE
                /////////////////

                break;
            }

            currentFile = currentFile->next;
        }

        FATLIB_unload_files_system(currentFile);
    //
    //  SELECTED DIRECTORY
    ///////////////////////////////////

}

void print_directory_data(char* path) {
    printf("Directory: [%s]\n%s%s%s", path, LINE, HEADER, LINE);

    int rows = upper_border;
    int all_rows = 0;
    if (all_rows++ == rows) {
        printf(UPPED_DIR);
        rows++;
    }

    struct UFATDirectory* currentDir = SYS_opendir(path)->subDirectory;
    while (currentDir != NULL) {
        char name[12];
        name[11] = '\0';

        strncpy(name, currentDir->name, 11);
        for (size_t i = strlen(name); i < 11; i++) name[i] = ' ';

        if (all_rows++ == rows) {
            printf("| %s | DIR    | N/A      | N/A      | N/A       | N/A  | N/A         |\n", name);
            rows++;
        }

        currentDir = currentDir->next;
    }

    struct UFATFile* currentFile = currentDir->files;
    while (currentFile != NULL) {
        //////////////////////////
        //  FILE INFO PREPARATIONS
        //
            char file_name[12];
            char file_extension[5];
            char file_size[12];

            file_name[11] = '\0';
            file_extension[4] = '\0';
            file_size[11] = '\0';

            strncpy(file_name, currentFile->name, 11);
            strncpy(file_extension, currentFile->extension, 4);
            
            char* file_size_str = itoa(currentFile->file_meta.file_size);
            strncpy(file_size, file_size_str, 10);
            strcat(file_size, "B");

            struct UFATDate* creation_date = FATLIB_get_date(currentFile->file_meta.creation_date, 1);
            struct UFATDate* modification_date = FATLIB_get_date(currentFile->file_meta.last_modification_date, 1);
            struct UFATDate* access_date = FATLIB_get_date(currentFile->file_meta.last_accessed, 1);

            for (size_t i = strlen(file_name); i < 11; i++) file_name[i] = ' ';
            for (size_t i = strlen(file_extension); i < 4; i++) file_extension[i] = ' ';
            for (size_t i = strlen(file_size); i < 11; i++) file_size[i] = ' ';
        //
        //  FILE INFO PREPARATIONS
        //////////////////////////

        char* cdate = convert_date(creation_date->day, creation_date->mounth, creation_date->year);
        char* mdate = convert_date(modification_date->day, modification_date->mounth, modification_date->year);
        char* adate = convert_date(access_date->day, access_date->mounth, access_date->year);

        if (all_rows++ == rows) {
            printf("| %s | FILE   | %s | %s | %s  | %s | %s |\n", file_name, cdate, mdate, adate, file_extension, file_size);
            rows++;
        }

        currentFile = currentFile->next;

        SYS_free(creation_date);
        SYS_free(modification_date);
        SYS_free(access_date);
        SYS_free(file_size_str);

        SYS_free(cdate);
        SYS_free(mdate);
        SYS_free(adate);
    }

    FATLIB_unload_directories_system(currentDir);
    FATLIB_unload_files_system(currentFile);

    for (int i = 0; i < 15 - (rows - upper_border); i++) printf(EMPTY);
    printf("%s[F1 - MKDIR] [F2 - MKFILE] [F3 - EXIT] [F4 - EDIT] [ENTER - EXEC] [BSPACE - RM]\n", LINE);
}
