#include <stdio.h>
#include <termios.h>
#include <stdlib.h>
#include <error.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

#define ONE_SECNS      1000000000L
#define UTM_US         100

#ifndef WAIT_SLEEPNS
#define WAIT_SLEEPNS   15500000
#endif

#ifndef PERIOD_SLEEPNS
#define PERIOD_SLEEPNS 1000000000
#endif

#define PACK_LEN 18

#ifndef BAUDS
#define BAUDS B115200
#endif

#define N_DEVS 1

#define __tos(s) _tos(s)
#define _tos(_def) #_def

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
    printf("Start client on %s "__tos(BAUDS)",8N2\n",fname);

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
    int n_error = 0;
    for(;;){
        for(int p = 0; p < N_DEVS; p++){
            buff[0]=cntr%256; 
            buff[PACK_LEN-1]=p%256;
            int v = sizeof(buff); //snprintf(buff,sizeof(buff),"%18s","034567abcdef123\xcc\xff");
            int w = write(h,buff,v);
            if(w<=0){
            }
            tcdrain(h);
            struct timespec ts = {.tv_sec=0,.tv_nsec=WAIT_SLEEPNS};
            nanosleep(&ts,NULL);
            fd_set fds;
            FD_ZERO(&fds);
            FD_SET(h,&fds);
            int i = 0;
            int j = 0;
            char rbuff[PACK_LEN]={};
            struct timeval tm={.tv_sec = 0, .tv_usec=UTM_US};
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
            
            printf(" %04d.%03d:%d  >  writes   %d bytes: ",cntr%1000,p,n_error,w);
            for(int i = 0; i < v;i++){
                printf(" %02hhX",buff[i]);
            }
            printf(" to %s .\n",fname);
            printf("\tread answer %d bytes  ",j);
            int eq = 0;
            if(v == j){
                int n_eq = 0;
                printf("pack: ");
                for(int i = 0; i <j;i++){
                    
                    if(buff[i]==rbuff[i]){
                        ++n_eq;
                        printf(" %02hhX",rbuff[i]);
                    }else{
                        printf(" !%02hhX",rbuff[i]);
                    }
                }
                if(n_eq==j){
                    eq = 1;
                }
            }
            if (eq == 1 && j == v){
                printf(" same, ok.");
            }else{
                printf(" not equal. err");
                tcflush(h,TCIOFLUSH);
                ++n_error;
            }
            if( (p+1) == N_DEVS){
                printf(">>>>>>>>>>>>>>>>>>end: %d\n",cntr);
            }else{
                printf("\n");
            }
        }
        struct timespec step_ts = {.tv_sec=PERIOD_SLEEPNS/1000000000l,.tv_nsec=PERIOD_SLEEPNS % ONE_SECNS};
        nanosleep(&step_ts,NULL);

        ++cntr;
    }
    tcsetattr(h,TCSADRAIN,&oldtm);
    close(h);

}



