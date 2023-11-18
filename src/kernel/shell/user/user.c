#include "../include/user.h"

void init_users() {
    if (find_directory("security") == NULL) 
        create_directory("security");
    
    move_to_directory("security");

    while (1) {
        struct File* users_file = find_file("users");
        if (users_file != NULL) {
            if (users_file->read_level != 0 || users_file->write_level != 0 || users_file->edit_level != 0) {
                delete_file("users");
                continue;
            }
        }
        else {
            create_file(0, 0, 0, "users", "txt", USERS_SECTOR);
            write_file(find_file("users"), "admin[000[0\nguest[666[123\n");
            continue;
        }

        struct File* groups_file = find_file("groups");
        if (groups_file != NULL) {
            if (groups_file->read_level != 0 || groups_file->write_level != 0 || groups_file->edit_level != 0) {
                delete_file("groups");
                continue;
            }
        }
        else {
            create_file(0, 0, 0, "groups", "txt", GROUPS_SECTOR);
            write_file(find_file("groups"), "default[000[admin[guest\n");
            continue;
        }

        up_from_directory();
        return;
    }
}

//////////////////////////////////////////////////////////////////
//    ____ ____   ___  _   _ ____  
//   / ___|  _ \ / _ \| | | |  _ \ 
//  | |  _| |_) | | | | | | | |_) |
//  | |_| |  _ <| |_| | |_| |  __/ 
//   \____|_| \_\\___/ \___/|_|  

    struct Group* login_group(char* user_name) {
        move_to_directory("security");
        char* groups = read_file(find_file("groups"));
        up_from_directory();

        int num_lines = 0;
        char* newline_pos = groups;
        while (*newline_pos) {
            if (*newline_pos == '\n') 
                num_lines++;
            
            newline_pos++;
        }

        char** lines    = (char**)malloc(num_lines * sizeof(char*));
        char* raw_line  = strtok(groups, "\n");
        int line_index  = 0;

        while (raw_line != NULL) {
            lines[line_index] = (char*)malloc(strlen(raw_line) + 2);

            strcat(lines[line_index], raw_line);
            strcat(lines[line_index++], "\n\0");
            raw_line = strtok(NULL, "\n");
        }

        int position = 0; //default[000[admin[guest\n
        while (position < num_lines) {
            struct Group* group = malloc(sizeof(struct Group));

            char* token = strtok(lines[position], "[");
            char* parsed_name = token;

            group->name = (char*)malloc(strlen(parsed_name));
            strcpy(group->name, parsed_name);

            token = strtok(NULL, "[");
            char* parsed_access = token;

            group->read_access   = GUEST_ACCESS;
            group->write_access  = GUEST_ACCESS;
            group->edit_access   = GUEST_ACCESS;

            if (strlen(parsed_access) > 0)
                group->read_access = parsed_access[0] - '0';
            if (strlen(parsed_access) > 1)
                group->write_access = parsed_access[1] - '0';
            if (strlen(parsed_access) > 2)
                group->edit_access = parsed_access[2] - '0';

            while (token != NULL) {
                token = strtok(NULL, "[");
                if (token == NULL) 
                    break;

                char* parsed_user = token;
                if (strstr(parsed_user, user_name) == 0) return group;
            }

            free(group->name);
            free(group);

            free(lines[position++]);
        }

        free(lines);
        free(groups);

        return NULL;
    }

//
//
//////////////////////////////////////////////////////////////////
//   _   _ ____  _____ ____  
//  | | | / ___|| ____|  _ \ 
//  | | | \___ \|  _| | |_) |
//  | |_| |___) | |___|  _ < 
//   \___/|____/|_____|_| \_\

    struct User* login(char* user_name, char* pass, int all) {
        move_to_directory("security");
        char* data = read_file(find_file("users"));
        up_from_directory();

        // Determine the number of lines (count the newline characters)
        int num_lines = 0;
        char* newline_pos = data;
        while (*newline_pos) {
            if (*newline_pos == '\n') 
                num_lines++;
            
            newline_pos++;
        }

        // Allocate an array of char* to store the lines
        char** lines = (char**)malloc(num_lines * sizeof(char*));
        struct User* users = malloc(num_lines * sizeof(struct User));

        // Split the data into lines and store them in the lines array
        char* raw_line = strtok(data, "\n");
        int line_index = 0;

        while (raw_line != NULL) {
            lines[line_index] = (char*)malloc(strlen(raw_line) + 2);

            strcat(lines[line_index], raw_line);
            strcat(lines[line_index++], "\n\0");
            raw_line = strtok(NULL, "\n");
        }

        int position = 0;
        while (position < num_lines) {
            char* token = strtok(lines[position], "[");
            char* parsed_name = token;

            token = strtok(NULL, "[");
            char* parsed_access = token;

            token = strtok(NULL, "[");
            char* parsed_password = token;

            //////
            //  NAME
            //
                users[position].name = (char*)malloc(strlen(parsed_name));
                strcpy(users[position].name, parsed_name);
            //
            //  NAME
            //////

            /////////////////////
            //
            //  ACCESS LEVELS
            //
                users[position].read_access   = GUEST_ACCESS;
                users[position].write_access  = GUEST_ACCESS;
                users[position].edit_access   = GUEST_ACCESS;
                users[position].users_count   = num_lines;

                if (strlen(parsed_access) > 0)
                    users[position].read_access = parsed_access[0] - '0';
                if (strlen(parsed_access) > 1)
                    users[position].write_access = parsed_access[1] - '0';
                if (strlen(parsed_access) > 2)
                    users[position].edit_access = parsed_access[2] - '0';


                /////////////////
                //  GROUP
                //
                    users[position].group = NULL;
                    struct Group* group = login_group(parsed_name);
                    if (group != NULL) {
                        users[position].read_access     = min(group->read_access, users[position].read_access);
                        users[position].write_access    = min(group->write_access, users[position].write_access);
                        users[position].edit_access     = min(group->edit_access, users[position].edit_access);

                        users[position].group = (char*)malloc(strlen(group->name));
                        strcpy(users[position].group, group->name);
                    }

                    free(group->name);
                    free(group);
                //
                //  GROUP
                /////////////////
                
            //
            //  ACCESS LEVELS
            //
            /////////////////////

            if (strstr(parsed_name, user_name) == 0 && strstr(parsed_password, pass) == 0 && all != 1) {
                free(lines[position]);
                free(lines);
                free(data); 

                return &users[position];
            }

            if (all != 1) 
                free(users[position].name);

            free(lines[position++]);
        }

        free(lines);
        free(data);

        if (all == 1)
            return users;

        free(users);
        return NULL;
    }

//
//
//////////////////////////////////////////////////////////////////

void print_users_table() {
    struct User* users = login("", "", 1);
    int size = users[0].users_count;

    printf("\n");
    printf(" _______________________________________________________________\n");
    printf("|     NAME     |   READ   |   WRITE   |   EDIT   |     GROUP    |\n");
    printf("|--------------|----------|-----------|----------|--------------|\n");

    for (int i = 0; i < size; i++) {
        char name[13];
        memset(name, ' ', 12);
        name[12] = '\0';

        int name_length = strlen(users[i].name);
        if (name_length <= 12) strncpy(name, users[i].name, name_length);
        else strncpy(name, users[i].name, 9);   

        char group[13];
        memset(group, ' ', 12);
        group[12] = '\0';

        int group_length = strlen(users[i].group);
        if (group_length <= 12) strncpy(group, users[i].group, group_length);
        else strncpy(group, users[i].group, 9);   

        printf("| %s |   %i      |   %i       |   %i      | %s |\n", name, users[i].read_access, users[i].write_access, users[i].edit_access, group);     
    }

    printf("|_______________________________________________________________|\n");
    free(users);
} 