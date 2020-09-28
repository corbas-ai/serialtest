#include <stdio.h>
#include <termios.h>
#include <stdlib.h>
#include <error.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

#define UTM 5000
#define PACK_LEN 18
#define SLEEPNS 1
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

    
    
    for(;;){
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(h,&fds);
        struct timeval tm={.tv_sec = 0, .tv_usec=UTM};
         
        char buff[PACK_LEN];
        int i = 0;
        int j = 0;
        int sel = select(h+1,&fds,NULL,NULL,&tm);
        for(; i < PACK_LEN; i++){
            if(sel>0){
                if(FD_ISSET(h,&fds)){
                    int r = read(h,buff+j,1);
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
        if(j==PACK_LEN){
            write(h,buff,PACK_LEN);
            tcdrain(h);
            printf("on %s read and echoed %d bytes:",fname,j);
            for(int i = 0; i <j;i++){
                printf(" %02hhX",buff[i]);
            }
            printf(".\n");
        }
        struct timespec ts = {.tv_sec=0,.tv_nsec=SLEEPNS};
        //nanosleep(&ts,NULL);
    }
    tcsetattr(h,TCSADRAIN,&oldtm);
    close(h);

    return 0;
}
