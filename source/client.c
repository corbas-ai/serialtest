#include <stdio.h>
#include <termios.h>
#include <stdlib.h>
#include <error.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

#define UTM 40000
#define PACK_LEN 18
#define SLEEPNS 500000000
#define WAITSLEEPNS  10000
#define BAUDS B115200

int 
main(int argc, char** argv ){
    if(argc<2){
        fprintf(stderr,"usage: server SERIALDEVNAME\n");
        exit( EXIT_FAILURE);
    }
    const char* fname = argv[1];
    int h = open(fname,O_RDWR|O_NOCTTY);
    if(h<0){
        perror("cat");
    }
    printf("opening server on %s %u,8N2\n",fname,BAUDS);

    struct termios oldtm={};
    if(tcgetattr(h,&oldtm)){
        perror("when tcgetattr");
        exit(EXIT_FAILURE);
    }
    struct termios newtm={.c_cflag=CS8|CSTOPB|CREAD|CLOCAL};
    cfsetispeed(&newtm,BAUDS);
    cfsetospeed(&newtm,BAUDS);
    tcsetattr(h,TCSANOW,&newtm);    
    tcflush(h,TCIOFLUSH);
    
    char buff[PACK_LEN] = {0,1,2,3,4,5,6,7,8,9,0xa,0xb,0xc,0xd,0xe,0xf,0xcc,0xff};
    int cntr = 0;
    for(;;){
        struct timeval tm={.tv_sec = 0, .tv_usec=UTM};
        buff[0]=cntr%256; 
        int v = sizeof(buff); //snprintf(buff,sizeof(buff),"%18s","034567abcdef123\xcc\xff");
        int w = write(h,buff,v);
        if(w<=0){
        }
        tcdrain(h);
        struct timespec ts = {.tv_sec=0,.tv_nsec=WAITSLEEPNS};
        nanosleep(&ts,NULL);
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(h,&fds);
        int i = 0;
        int j = 0;
        char rbuff[PACK_LEN]={};
        int sel = select(h+1,&fds,NULL,NULL,&tm);
        for(; i < PACK_LEN; i++){
            if(sel>0){
                if(FD_ISSET(h,&fds)){
                    int r = read(h,rbuff+j,1);
                    if(r<=0){
                        perror("when reading");
                        break;
                    }else{
                        j++;
                    }
                }
                FD_SET(h,&fds);
                sel = select(h+1,&fds,NULL,NULL,&tm);
            }else{
                break;
            }
        }
        v = j;
        printf("on %s writes %d bytes: ",fname,w);
        for(int i = 0; i < v;i++){
            printf(" %02hhX",buff[i]);
        }
        printf(".\n");
        printf("\tread answer %d bytes       pack:",j);
        int eq = 1;
        for(int i = 0; i <j;i++){
            printf(" %02hhX",rbuff[i]);
            if(buff[i]!=rbuff[i]){
                eq = 0;
            }
        }
        if (eq == 1 && j == v){
            printf(" same, ok.\n");
        }else{
            printf(" not equal. err\n");
        }
        struct timespec step_ts = {.tv_sec=0,.tv_nsec=SLEEPNS};
        nanosleep(&step_ts,NULL);
        
        ++cntr;
    }
    tcsetattr(h,TCSADRAIN,&oldtm);
    close(h);

}
