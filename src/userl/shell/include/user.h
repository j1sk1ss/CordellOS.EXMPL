#pragma once

#include "../../../libs/include/math.h"
#include "../../../libs/include/stdlib.h"
#include "../../../libs/include/fatlib.h"

#define GUEST_ACCESS    9

struct User {
    char* name;
    char* group;
    
    int read_access;
    int write_access;
    int edit_access;

    int users_count;
};

struct Group {
    char* name;

    int read_access;
    int write_access;
    int edit_access;
};

void init_users();
struct User* login(char* user_name, char* pass, int all);
