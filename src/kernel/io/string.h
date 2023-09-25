#pragma once

#include "math.h"

#include <stddef.h>

int isdigit(int c);
int isspace(int c);

const char* strchr(const char* str, char chr);
int strstr(const char* haystack, const char* needle);
char* strcpy(char* dst, const char* src);
unsigned strlen(const char* str);
int strcmp(const char* firstStr, const char* secondStr);

char* strtok(char* s, const char* delim);
char* strtok_r(char* s, const char* delim, char** last);

void reverse(char* str, int len);
char* double_to_string(double value);
double atof(const char *str);

wchar_t* utf16_to_codepoint(wchar_t*  strin, int* codePointg);
char* codepoint_to_utf8(int codePoint, char* stringOutput);
