#include<stdio.h>
#include<stdlib.h>

int main(void){
    FILE *fp;
    static int i = 0;
    static long size = 1113141115728;
    
    fp = fopen("file3MB","w");
    
    if(fp == NULL){
        printf("error\n");
        exit(1);
    }
    
    while(i < size){
        fputc('a',fp);
        i++;
        if(i%65536 == 0){
            fputc('\n',fp);
        }
    }
    
    fclose(fp);
    return 0;
}
