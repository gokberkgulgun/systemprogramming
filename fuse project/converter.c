#include "converter.h"

int xstrcmp(const char* in1, const char* in2){
	return strcmp(in1, in2) == 0;
} 
int readhtmlFile(char* html_name, char** file_buffer, char* out_enc, int* chr_pos){
	
	FILE* pFile;
	char buffer[100];
	char* temp_fbuff;
	strcpy(out_enc, "error");
	
	pFile = fopen(html_name, "r");
	if (pFile == NULL)
		return -2;
	fscanf(pFile, "%s", buffer);
	
	while (strncmp (buffer,"<meta",5)){
		if( feof(pFile) )
			return ERR_FOR;
		fscanf(pFile, "%s", buffer);
	}
	while (strncmp (buffer,"charset=\"",9)){
		fscanf(pFile, "%s", buffer);
		if( feof(pFile) )
			return ERR_FOR;
	}
	*chr_pos = ftell(pFile);
	fseek(pFile, 0, SEEK_END); 
    unsigned long file_len = (unsigned long)ftell(pFile); // find size
    *file_buffer = (char*)malloc(file_len*sizeof(char) + 1); // create buffer for all data
    rewind(pFile);
    //memory check yap
    if (!*file_buffer){
		return ERR_MEM;
	}
    memset(*file_buffer, '\0', file_len + 1);
    fread(*file_buffer, 1, file_len, pFile); //read file to buffer
	
    //printf("file buffer\n%s\nfile_len = %d\n", *file_buffer, file_len);
   
	fclose(pFile);

	int len = strlen(buffer) - 10; // subtract charset="xxx" excluding xxx
	if (len > 10){
		free(file_buffer);
		return ERR_ENC;
	}
	strncpy(out_enc, buffer + 9, len );
	out_enc[len] ='\0';
	printf("READING HTML -> output encoding -> %s\n", out_enc);
	return file_len;
}
int convertFile(char* fromcode, char* tocode, char *src, char **dst)
{
    iconv_t cd;
    char *inbuf, *outbuf;
    size_t inbytesleft, outbytesleft, nchars, outbuf_len;
    /*char* add_ignore;
    add_ignore = (char*) malloc((strlen(tocode) + 11)*sizeof(char));
	strcpy(add_ignore, tocode);
    strcat(add_ignore, "//IGNORE"); // //TRANSLIT yapılabilir
    */
    cd = iconv_open(tocode, fromcode);
    //free(add_ignore);
    if (cd == (iconv_t)-1) {
        printf("iconv_open failed: %d\n", errno);
        return -1;
    }

    inbytesleft = strlen(src);
    if (inbytesleft == 0) {
        printf("empty string\n");
        iconv_close(cd);
        return -1;
    }
    inbuf = src;
    outbuf_len = inbytesleft;
    // take guess
	if (xstrcmp(fromcode, "UTF-8") || xstrcmp(fromcode, "ISO8859-1") || xstrcmp(fromcode, "ISO8859-9")){
		if (xstrcmp(tocode, "UTF-16"))
			outbuf_len = outbuf_len * 2;
		else if (xstrcmp(tocode, "UTF-32"))
			outbuf_len = outbuf_len * 4;
	}
    *dst = malloc(outbuf_len);
    if (!*dst) {
        printf("malloc failed\n");
        iconv_close(cd);
        return -1;
    }
    outbytesleft = outbuf_len;
    outbuf = *dst;
    nchars = iconv(cd, &inbuf, &inbytesleft, &outbuf, &outbytesleft);
    
    while (nchars == (size_t)-1 && errno == E2BIG) { // if outbut buffer not enough, increased it
        char *ptr;
        size_t increase = 10;                   // increase length a bit
        size_t len;
        outbuf_len += increase;
        outbytesleft += increase;
        ptr = realloc(*dst, outbuf_len);
        if (!ptr) {
            printf("realloc failed\n");
            free(*dst);
            iconv_close(cd);
            return -1;
        }
        len = outbuf - *dst;
        *dst = ptr;
        outbuf = *dst + len;
        nchars = iconv(cd, &inbuf, &inbytesleft, &outbuf, &outbytesleft);
    }
    if (nchars == (size_t)-1) {
        printf("iconv failed: %d\n",errno);
        free(*dst);
        iconv_close(cd);
        return -1;
    }

    iconv_close(cd);

    return outbuf_len - outbytesleft;;
}

int convertHtmlFile(char* fromcode, char* tocode, char* src, char** dst, int char_pos){
	printf("CONVERTING -> from enc = %s, to enc = %s\n", fromcode,tocode);
	int len_from = strlen(fromcode);
	int len_to   = strlen(tocode);
	int src_len = strlen(src);
	
	char* new_buff;
	src_len = len_to - len_from + src_len; // update src_len
	new_buff = (char*) malloc(src_len*sizeof(char) + 1);
	//changing charset
	memset(new_buff, '\0', src_len + 1);
	strncat(new_buff, src, char_pos - (len_from + 1));
	strcat(new_buff, tocode);
	strcat(new_buff, "\"");
	strcat(new_buff, src + char_pos);
	
	//printf("Yeni buffer\n%s\nLenght = %d, %d\n", new_buff, strlen(new_buff), src_len); 
	int ret = convertFile(fromcode, tocode, new_buff, dst);
	free(new_buff);
	return ret;
}
