#ifndef FSLIB_H_
#define FSLIB_H_

#include "stdio.h"
#include "stdlib.h"
#include "ctype.h"
#include "execute.h"


#define STAT_FILE	0x00
#define STAT_DIR	0x01
#define NOT_PRESENT	0x02


typedef struct FATDate {
	uint16_t hour;
	uint16_t minute;
	uint16_t second;
	uint16_t year;
	uint16_t mounth;
	uint16_t day;
} Date;

typedef struct {
	uint8_t full_name[11];
	char file_name[8];
	char file_extension[4];
	int type;
	int size;
	uint16_t creation_time;
	uint16_t creation_date;
	uint16_t last_accessed;
	uint16_t last_modification_time;
	uint16_t last_modification_date;
} CInfo_t;

typedef struct {
	char mount[32];
	char name[24];
	uint32_t type;
	uint32_t clusters;
	uint16_t spc;
	uint32_t size;
} FSInfo_t;


char* FSLIB_change_path(const char* currentPath, const char* content);
Date* FSLIB_get_date(short data, int type);

void get_fs_info(FSInfo_t* info);
int cexists(const char* path);
void rmcontent(const char* path);
void chgcontent(const char* path, const char* meta);

void fread(int ci, int offset, uint8_t* buffer, int len);
void fread_stop(int ci, int offset, uint8_t* buffer, int len, char* stop);
void fwrite(int ci, int offset, uint8_t* buffer, int len);
int fexec(char* path, int args, char** argv);

void mkfile(const char* path, const char* name);
void mkdir(const char* path, const char* name);

int opendir(int ci);
int lsdir(int ci, char* name, int step);
int cstat(int ci, CInfo_t* info);
int copen(const char* path);
int cclose(int ci);

#endif