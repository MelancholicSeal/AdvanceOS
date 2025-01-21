#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/wait.h>

#define max(x, y) ((x) > (y) ? (x) : (y))

int N;
int tmp = 0;
int* pid;
int** fd;
int** cfd;
struct msg {
    int id;
    int val;
};
void kill_child(int sig){
    printf("\nCaught signal %d (SIGINT). Exiting gracefully...\n", sig);
    
    for(int i=1; i<=N; i++){
        if(kill(pid[i],SIGINT)==-1){
            printf("%d\n", pid[i]);
        }
    }
    for(int i=1; i<=N; i++){
       
        waitpid(pid[i], NULL,0);
    }

    // close pipes
    for (int i = 1; i <= N; i++) {
        close(fd[i][1]);
        close(cfd[i][0]);
    }
    // Free the allocated memory before exiting the program
    for (int i = 0; i <= N; i++) {
        free(fd[i]);  // Free each row
    }
    free(fd);
    for (int i = 0; i <= N; i++) {
        free(cfd[i]);  // Free each row
    }    
    free(cfd);
    free(pid);
    exit(0);
}
void ter_child(int sig){
    printf("\nCaught signal %d (SIGINT). Exiting gracefully...\n", sig);
    // close pipes
    close(fd[tmp][0]);
    close(cfd[tmp][1]);

    // Free the allocated memory before exiting the program
    for (int i = 0; i <= N; i++) {
        free(fd[i]); 
    }
    free(fd);
    for (int i = 0; i <= N; i++) {
        free(cfd[i]);  
    }
    free(cfd);
    free(pid);
    exit(0);
}


int main(int argc, char *argv[]) {
    // check arguments
    if (argc != 3 || strtol(argv[2], NULL, 10) <= 0) {
        printf("The program takes 2 arguments\n1st The name of the output file\n2nd The number of workers in the worker pool (positive integer)\n");
        return 2;
    }

    char *name = argv[1];
    N = strtol(argv[2], NULL, 10);

    pid = malloc((N+1)*sizeof(int));    
    int a = 1;
    

    // Initialize pipes
    fd = (int **)malloc((N + 1) * sizeof(int *));
    cfd = (int **)malloc((N + 1) * sizeof(int *));
    for (int i = 1; i <= N; i++) {
        fd[i] = (int *)malloc(2 * sizeof(int));
        cfd[i] = (int *)malloc(2 * sizeof(int));
    }
    for (int i = 1; i <= N; i++) {
        if (pipe(fd[i]) < 0) return 1;
        if(pipe(cfd[i]) < 0) return 1;
    }

    FILE *out;
    for (int i = 1; i <=N; i++) {
        if (a > 0) {
            tmp++;
            a = fork();
            pid[i]=a;
        }
    }

    // close unnecessary pipes for parent
    if (a > 0) {
        for (int i = 1; i <= N; i++) {
            close(fd[i][0]);
            close(cfd[i][1]);
        }
        signal(SIGINT, kill_child);
    }
    // close unnecessary pipes for child
    else {
        for (int i = 1; i <= N; i++) {
            close(fd[i][1]);
            close(cfd[i][0]);
            if (tmp != i) close(fd[i][0]);
            if(tmp!=i) close(fd[i][1]);
        }
        signal(SIGINT, ter_child);
    }

    // child logic
    if (a == 0) {
        while (1) {
            struct msg mes;
            
            ssize_t read_bytes = read(fd[tmp][0], &mes, sizeof(mes));  // read task from parent
            if (read_bytes <= 0) {
                printf("Worker %d: Failed to read from pipe\n", tmp);
                continue;
            }

            int wait = mes.val;
            sleep(wait);  // simulate task execution
            int id = mes.id;

            // Log to file
            out = fopen(name, "a");
            fprintf(out, "pid %d from ppid %d, parent calls me %d\n", getpid(), getppid(), id);
            fclose(out);

            // Send back completion signal
            mes.val = 0;  // indicate completion
            ssize_t write_bytes = write(cfd[tmp][1], &mes, sizeof(mes));  // notify parent of completion
            if (write_bytes <= 0) {
                printf("Worker %d: Failed to write to pipe\n", tmp);
            }
        }
        

    } else {
        // parent logic
        out = fopen(name, "w");
        fprintf(out, "pid %d from ppid %d\n", getpid(), getppid());
        fclose(out);

        int av[N+1];
        for (int i = 0; i <=N; i++) {
            av[i] = 1;  // mark all workers as available initially
        }

        while (1) {
            fd_set writefds;
            FD_ZERO(&writefds);

            // Check which workers are available
            for (int i = 1; i <= N; i++) {
                if (av[i]) FD_SET(fd[i][1], &writefds);
            }

            struct timeval tv2 = {0, 5000};  // timeout for select
            int r = select(FD_SETSIZE + 1, NULL, &writefds, NULL, &tv2);

            if (r == -1) {
                return 3;
            }

            if (r > 0) {
                // assign task to the first available worker
                for (int i = 1; i <= N; i++) {
                    if (FD_ISSET(fd[i][1], &writefds)) {
                        struct msg mes = {i, rand() % 5 + 1};  // generate task with random wait time
                        ssize_t write_bytes = write(fd[i][1], &mes, sizeof(mes));
                        if (write_bytes <= 0) {
                            printf("Parent: Failed to write to worker %d\n", i);
                        }
                        av[i] = 0;  // mark worker as busy
                        break;
                    }
                }
            }

            // Check for completion messages from workers
            fd_set rfds;
            FD_ZERO(&rfds);
            for (int i = 1; i <= N; i++) {
                FD_SET(cfd[i][0], &rfds);
            }

            struct timeval tv = {0, 5000};  
            int retval = select(FD_SETSIZE + 1, &rfds, NULL, NULL, &tv);
            if (retval == -1) {
                return 3;
            }
            if (retval > 0) {
                // read message from worker
                for(int i=1; i<=N; i++){
                    if (FD_ISSET(cfd[i][0], &rfds)) {
                        struct msg mes;
                        ssize_t read_bytes = read(cfd[i][0], &mes, sizeof(mes));
                        if (read_bytes <= 0) {
                            printf("Parent: Failed to read from worker pipe\n");
                            continue;
                        }
                        // mark worker as available if task is complete
                        if (mes.val == 0) {
                            av[mes.id] = 1;
                        }
                    }
                }
            }
        }  
    }
    return 0;
}
