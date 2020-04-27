#include <unistd.h>

#include <stdio.h>

int main(void){

    puts("[ INFO] Start");

    pid_t ret = fork();

    if(ret == 0){
        printf("[ INFO] Child process (PID: %d)\n", getpid());
    }else{
        printf("[ INFO] Parent process (Parent PID: %d, Child PID: %d)\n", getpid(), ret);
    }

    return 0;
}

