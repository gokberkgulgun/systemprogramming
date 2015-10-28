#define FUSE_USE_VERSION 26  //fuse main için gerekli

#include <stdlib.h>
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <strings.h> // for strdup
#include <dirent.h>
#include "converter.h"

static char* rw_path;

static int xstrcmp(const char* in1, const char* in2){
	return strcmp(in1, in2) == 0;
} 
static int ext_checker(const char* path, const char* ext){
	char* pch;
	pch = strrchr(path,'.');
	if (pch == NULL || !xstrcmp(pch, ext) )
		return 0;
	return 1;
}
static char* findSubDir(const char* path){
	char* subdir; 
	subdir = strrchr(path + 1 , '/');

	if (subdir == NULL)
		return NULL;
	int k = subdir - path;

	char* ret = (char*) malloc(k);
	strncpy(ret, path + 1, k - 1);
	ret[k - 1] = '\0';
	printf("Subdir = %s\n", ret);
	return ret;
}
static char* translate_path(const char* path){

	char* subdir = findSubDir(path); // utf-8, utf32 vs
	int len = (subdir == NULL) ? 0 : strlen(subdir);
	free(subdir);
	
    char *rPath= malloc(sizeof(char)*(strlen(path)+strlen(rw_path)- len +1));
    strcpy(rPath,rw_path);
    
    if (rPath[strlen(rPath)-1]=='/') {
        rPath[strlen(rPath)-1]='\0';
    }  
    if (len > 0)
		strcat(rPath,path + len + 1);
 
    return rPath;
}

static int path_checker(const char* path){
	return path != NULL &&(
		   xstrcmp(path, "/UTF-8")      ||
		   xstrcmp(path, "/UTF-16")     ||
		   xstrcmp(path, "/UTF-32")     ||
		   xstrcmp(path, "/ISO8859-1")  ||
		   xstrcmp(path, "/ISO8859-9")  ||
		   xstrcmp(path, "UTF-8")       ||
		   xstrcmp(path, "UTF-16")      ||
		   xstrcmp(path, "UTF-32")      ||
		   xstrcmp(path, "ISO8859-1")   ||
		   xstrcmp(path, "ISO8859-9") );
}


static int htmlconverter_open(const char *path, struct fuse_file_info *fi)
{
	printf("OPEN -> path = %s\n", path);
    if ((fi->flags & 3) != O_RDONLY)
        return -EACCES;

    return 0;
}

static int htmlconverter_read(const char *path, char *buf, size_t size, off_t offset,
                     struct fuse_file_info *fi)
{
    int res;
    (void)fi;
    
    char *upath=translate_path(path);
	char *in_buff;
	char fr_enc[10];
	int char_pos; 
	printf("READ -> Virtual path %s, Real Path = %s\n", path, upath); 
	res = readhtmlFile(upath,&in_buff, fr_enc, &char_pos);
	printf("READ-> Content \n%s\nEncoding = %s\n", in_buff, fr_enc);
	free(upath);
	if (res < 0)
		return -ENOENT;
	char* to_enc = findSubDir(path); // utf-8, utf32 vs
	printf("READ -> from_enc = %s , to_enc = %s\n", fr_enc, to_enc);
	char* temp_buffer;
	int f = convertHtmlFile(fr_enc, to_enc, in_buff, &temp_buffer,char_pos);
	printf("READ-> Converted Content\n%s\n", temp_buffer);
	if ( f > size)
		f = size - 1;
	memcpy(buf, temp_buffer, f);
	buf[f + 1] = '\0';
	free(in_buff);
	free(to_enc);
	if (f < 0)
		return -ENOENT;
    
    return f;
    

}

static int htmlconverter_getattr(const char *path, struct stat *stbuf)
{
 
    memset(stbuf, 0, sizeof(struct stat));
    if ( xstrcmp(path, "/") || path_checker(path) ) {
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 2;
        return 0;
    }
    else{
		char* validPath;
		printf("GET ATT -> %s\n", path);
		validPath = findSubDir(path);

		if (!path_checker(validPath) || !ext_checker(path, ".html") ){
			free(validPath);
			return -ENOENT;
		}
		
		char *upath=translate_path(path);
		char *buff;
		char* dst;
		char enc[10];
		int char_pos; 
		
		int res = readhtmlFile(upath,&buff, enc, &char_pos);
		free(upath);
		if (res < 0){ // if file doesn't exist, or opened
			free(buff);
			return -ENOENT;
		}
		int f = convertHtmlFile(enc, validPath, buff, &dst,char_pos);
		free(buff);
		if (f < 0)
			return -ENOENT;
		stbuf->st_mode = S_IFREG | 0444;
        stbuf->st_nlink = 1;
        stbuf->st_size = f;
        free(validPath);
		return 0;
	}
     
    return -ENOENT;

}
static int htmlconverter_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                         off_t offset, struct fuse_file_info *fi)
{
    (void) offset;
    (void) fi;
	int res;
 
	if (xstrcmp(path, "/")){
		filler(buf, ".", NULL, 0);
		filler(buf, "..", NULL, 0);
		filler(buf, "UTF-8", NULL, 0);
		filler(buf, "UTF-16", NULL, 0);
		filler(buf, "UTF-32", NULL, 0);
		filler(buf, "ISO8859-1", NULL, 0);
		filler(buf, "ISO8859-9", NULL, 0);
	}else if (path_checker(path)){
		DIR *dp;
		struct dirent *de;
		char *upath=translate_path(path);

		dp = opendir(upath);
		free(upath);
		if(dp == NULL) {
			res = -errno;
			return res;
		}
		struct stat st;
		while((de = readdir(dp)) != NULL) {
			
			memset(&st, 0, sizeof(st));
			st.st_ino = de->d_ino;
			st.st_mode = de->d_type << 12;
			
			if (!ext_checker(de->d_name, ".html")) // only add html files
				continue;
			
			if (filler(buf, de->d_name, NULL, 0))
				break;
		}
	}else
		return -ENOENT;
			
	
    return 0;
}
static struct fuse_operations htmlconverter_oper = {
    .getattr	= htmlconverter_getattr,
    .readdir	= htmlconverter_readdir,
    .open	= htmlconverter_open,
    .read	= htmlconverter_read,
};

static int htmlconverter_parse_opt(void *data, const char *arg, int key,
                          struct fuse_args *outargs)
{
    (void) data;
	(void) outargs;
    switch (key)
    {
    case FUSE_OPT_KEY_NONOPT:
        if (rw_path == 0)
        {
            rw_path = strdup(arg);
            return 0;
        }
        else
        {
            return 1;
        }
    case FUSE_OPT_KEY_OPT:
        return 1;
    default:
        fprintf(stderr, "Wrong parameters");
        exit(1);
    }
    return 1;
}

int main(int argc, char *argv[])
{
	
	struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
    
    int res;
    res = fuse_opt_parse(&args, &rw_path, NULL, htmlconverter_parse_opt);
    
	fprintf(stdout, "rw_path -> %s\nReturn = %d\n", rw_path, res);
    return fuse_main(args.argc, args.argv, &htmlconverter_oper, NULL);

}
