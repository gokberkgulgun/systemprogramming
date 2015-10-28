
#ifndef _CONVERTER
#define _CONVERTER

#include <stdio.h>
#include <iconv.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

int convertFile(char* fromcode, char* tocode, char* src, char** dst);
/*
 * Return size or error values
 * out_enc must be at least 10 char
 * chr_pos end of position of charset declaration
 * */
int readhtmlFile(char* html_path, char** file_buffer, char* out_enc, int*);
int convertHtmlFile(char* fromcode, char* tocode, char* src,char** dst, int char_pos);

#define ERR_FOR -1
#define ERR_ENC -2
#define ERR_MEM -3
#endif
