#include<stdio.h>
#include<fcntl.h>
#include<string.h> //for memset and strlen


int main(int argc , char *argv[])

{
    
    char buf[100];
    char i=0;
    memset(buf,0,100);
    printf("Input: %s\n",argv[1]);
    
    int fp=open("/dev/encode",O_RDWR);
    
    write(fp,argv[1],strlen(argv[1]));
    
    while(read(fp,&buf[i++],1));
    
    printf("Converted by the driver: %s\n",buf);
    
}
