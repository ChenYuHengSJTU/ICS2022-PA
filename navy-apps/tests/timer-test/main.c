#include<stdio.h>
#include<stdint.h>
#include<sys/time.h>

int main(){
    struct timeval tv;
    struct timezone tz;
    int i=0;
    while(1){
        gettimeofday(&tv,&tz);
        uint64_t res=tv.tv_usec/1000;
        if(res%500==0) printf("time used %ds\n",i++/2);
    }
    
    return 0;
}