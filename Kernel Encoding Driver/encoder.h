#ifndef __ENCODER_H
#define __ENCODER_H

#include <linux/ioctl.h> /* needed for the _IOW etc stuff used later */

unsigned int utf32_to_utf8(char*,int,char*);
unsigned int utf16_to_utf8(char* ,int,char*);
unsigned int utf8_to_utf16(char* , int , char*);
unsigned int utf8_to_utf32(char* ,int,char*);
unsigned int utf8_to_iso88599(char*,int,char*);
unsigned int utf8_to_iso88591(char*,int,char*);

#define ENCODER_IOC_MAGIC  'k'

#define SET_ENCODER_CURRENT_INPUT   	_IOWR(ENCODER_IOC_MAGIC, 0, int)
#define SET_ENCODER_CURRENT_OUTPUT		_IOWR(ENCODER_IOC_MAGIC, 1, int)
#define GET_ENCODER_CURRENT_INPUT   	_IOWR(ENCODER_IOC_MAGIC, 2, int)
#define GET_ENCODER_CURRENT_OUTPUT		_IOWR(ENCODER_IOC_MAGIC, 3, int)
#define QUERY_ENCODER_CURRENT_INPUT		_IOWR(ENCODER_IOC_MAGIC, 4, int)
#define QUERY_ENCODER_CURRENT_OUTPUT	_IOWR(ENCODER_IOC_MAGIC, 5, int)


#define ENCODER_IOC_MAXNR 2

#endif
