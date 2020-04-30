#include <unistd.h>

#include <time.h>
#include <stdlib.h>
#include <stdio.h>

int timespecToMicrosecond(const struct timespec t, const struct timespec initial_t){

    int ret = 0;
    ret += (t.tv_sec - initial_t.tv_sec) * 1000000;
    if(t.tv_nsec >= initial_t.tv_nsec){
        ret += (t.tv_nsec - initial_t.tv_nsec) / 1000;
    }else{
        ret -= 1000000;
        ret += (t.tv_nsec + 1000000000 - initial_t.tv_nsec) / 1000;
    }
    
    return ret;
}

void loop(const int total, const int resol, const struct timespec init_time){

    int reported = 0;
    int k = 0;
    const pid_t pid = getpid();

    const size_t N = total / resol;
    int elapsed_time_log[N];
    int result_log[N];
    
    while(1){
        
        struct timespec cur_time;
        clock_gettime(CLOCK_MONOTONIC_RAW, &cur_time);
        const int elapsed = timespecToMicrosecond(cur_time, init_time);
        
        k++;

        if(elapsed / resol > reported){
            elapsed_time_log[reported] = elapsed;
            result_log[reported] = k;
            reported++;
        }

        if(elapsed > total){
            break;
        }
    }

    char filename[32];
    sprintf(filename, "./sched_practice_%d.csv", pid);
    FILE* fout = fopen(filename, "w");
    if(! fout){
        fprintf(stderr, "[ERROR] Failed to open %s\n", filename);
        exit(1);
    }else{
        fprintf(stderr, "[ INFO] Opened %s\n", filename);
    }

    fprintf(fout, "PID,Index,Elapsed,Result\n");
    for(int i = 0; i < N; i++){
        fprintf(fout, "%d,%d,%d,%d\n", pid, i, elapsed_time_log[i], result_log[i]);
    }
    fclose(fout);

    return;
}


/* 
   うまいこといかない．
   clock_gettimeやwriteシステムコールにかかる時間は概ね 1 - 10 [us] だが，
   たまに 100 [us] 程度かかったりする．
 */
void busyWait(const struct timespec start_time){

    struct timespec cur_time;
    struct timespec init_time;
    int cnt = 0;
    const pid_t pid = getpid();
    
    while(1){
        cnt++;
        
        clock_gettime(CLOCK_MONOTONIC_RAW, &cur_time);
        if(cnt == 1){
            init_time = cur_time;
        }
        
        const int estimated_waiting_time = timespecToMicrosecond(cur_time, start_time);

        fprintf(stderr, "[DEBUG] Estimated_waiting_time of PID %d: %d\n", pid, estimated_waiting_time);
        
        if(estimated_waiting_time >= 0){
            const int waited_time = timespecToMicrosecond(cur_time, init_time);
            fprintf(stderr, "[DEBUG] Now it is %u, after waiting for %d [us] (PID: %d)\n", (int)(cur_time.tv_sec * 1000000 + cur_time.tv_nsec / 1000), waited_time, pid);
            return;
        }else if(estimated_waiting_time > -100){
            continue;
        }else{
            usleep((-estimated_waiting_time) / 2);
            continue;
        }
    }
}

int main(int argc, char** argv){

    if(argc != 4){
        fprintf(stderr, "Usage: %s <num of processes> <total CPU time [us]> <interval to report [us]>\n", argv[0]);
        exit(1);
    }

    const int proc_n = atoi(argv[1]);
    const int total = atoi(argv[2]);
    const int resol = atoi(argv[3]);

    if(proc_n <= 0){
        fprintf(stderr, "[ERROR] Num of processes must be larger than or equal to 1.\n");
        exit(1);
    }

    fprintf(stderr, "[ INFO] Num of processes: %d\n", proc_n);

    pid_t pid = 0;

    struct timespec init_time;
    clock_gettime(CLOCK_MONOTONIC_RAW, &init_time);

    struct timespec start_time = init_time;
    start_time.tv_sec += 1;

    for(int i = 0; i < proc_n - 1; i++){
        
        pid = fork();

        if(pid == 0){           /* Child process */
            break;
        }
    }

    busyWait(start_time);
    
    loop(total, resol, start_time);
    
    exit(0);
}

