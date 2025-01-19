#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<fcntl.h>
#include<unistd.h>
#include<semaphore.h>
#include<sys/select.h>


int BufSize = 65536;

int main(int argc, char * argv[]){
    char *name = argv[1];
    int N = strtol(argv[2],NULL,10);
    int a=1;
    int tmp = 0;

    int fd[N+1][2];
    for(int i=0; i<=N; i++){
        if(pipe(fd[i])<0) return 1;
    }
    FILE *out;
    for(int i=0; i<N; i++){
        if(a>0){
            tmp++;
            a=fork();
            if(a==0){
                for(int i=1; i<=N; i++){
                    close(fd[i][1]);
                    if(tmp!=i) close(fd[i][0]);
                }
            }
        }
    }
    if(a>0){
        tmp=0;
        for(int i=1; i<=N; i++){
            close(fd[i][0]);
        }
        close(fd[0][1]);
    }

    if(a==0){
        int x;
        while(1>0){
            char mes[BufSize];
            read(fd[tmp][0],&mes,sizeof(mes));
            int i = strtol(mes, NULL, 10);
            out = fopen(name,"a");
            fprintf(out, "pid %d from ppid %d, the parent calls me %d\n",getpid(), getppid(),i);
            fclose(out);
            x=0;
            write(fd[0][1],&x,sizeof(x));
            sleep(i+rand()%5);
        
        }
        close(fd[tmp][0]);
        close(fd[0][1]);
       
    }
    else{
        out = fopen(name,"w");
        fprintf(out, "pid %d from ppid %d\n",getpid(), getppid());
        fclose(out);
        while(1>0){
            fd_set writefds;
            FD_ZERO(&writefds);
            
            for(int i=1;i<=N; i++){
                FD_SET(fd[i][1], &writefds);
            }
            int r = select(FD_SETSIZE+1, NULL, &writefds, NULL,NULL);
            char mes[BufSize];
            
            int code = rand();
            
            for(int i=1;i<=N; i++){
                snprintf (mes, sizeof(mes), "%d",i);
                if(FD_ISSET(fd[i][1], &writefds)>0){
                    write(fd[i][1],&mes,sizeof(mes));
                }
            }
        }
        for(int i=1;i<=N; i++){
            close(fd[i][1]);
        }
        close(fd[0][0]);

    }
    return 0;
}
