#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <stdbool.h>


// global variables
int total_sum = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void usage() {
    printf("Usage: ./prog <num_a_files> <num_b_files> <n_rand_numbers> <n_parent_threads> <n_child_threads>\n");
}

int prepare_files(int afiles, int bfiles, int nrands) {
    srand(afiles);
    int sum = 0;
    for(int i = 0; i < afiles; i++){
        char filename[100];
        snprintf(filename, 100, "a_%d.txt", i);
        FILE* fp = fopen(filename, "w");
        for(int j = 0; j < nrands; j++){
            int random_num = rand() % 100;
            char entry[5];
            // 3 chars for num, 1 char for \n, 1 char for null character
            snprintf(entry, 5, "%03d\n", random_num);
            fwrite(entry, sizeof(char), 4, fp);
            sum += random_num;
        }
        fclose(fp);
    }
    printf("Created %d a_xx.txt files and populated %d numbers in each of 'em\n", afiles, nrands);

    for(int i = 0; i < bfiles; i++){
        char filename[100];
        snprintf(filename, 100, "b_%d.txt", i);
        FILE* fp = fopen(filename, "w");
        for(int j = 0; j < nrands; j++){
            int random_num = rand() % 101;
            char entry[5];
            snprintf(entry, 5, "%03d\n", random_num);
            fwrite(entry, sizeof(char), 4, fp);
            sum += random_num;
        }
        fclose(fp);
    }
    printf("Created %d b_xx.txt files and populated %d numbers in each of 'em\n", bfiles, nrands);

    printf("Done preparing files. Total sum equals: %d\n", sum);

    return sum;
}

typedef
struct process_file_args {
    char* file_prefix;
    int n_files;
    int nums_in_file;
    int n_threads;
    int id;
} process_file_args;

void* process_file(void* args) {
    process_file_args* arguments = (process_file_args*) args;
    char* file_prefix = arguments->file_prefix;
    int n_files = arguments->n_files;
    int nums_in_file = arguments->nums_in_file;
    int n_threads = arguments->n_threads;
    int id = arguments->id;

    int sum = 0;

    int nums_to_process = 0;
    int total_numbers = nums_in_file * n_files;
    if (id == n_threads - 1) nums_to_process = (total_numbers / n_threads) + (total_numbers % n_threads);
    else nums_to_process = total_numbers / n_threads;

    int curr_num_id = id * (total_numbers / n_threads);
    int file_id = curr_num_id / nums_in_file;

    char filename[100];
    snprintf(filename, 100, "%s%d.txt", file_prefix, file_id);
    FILE* fp = fopen(filename, "r");
    fseek(fp, sizeof(char)*4*(curr_num_id % nums_in_file) , SEEK_SET);
    bool file_open = true;
    int num;

    while(nums_to_process != 0) {
        // retrieve number
        fscanf(fp, "%d", &num);
        sum += num;
        nums_to_process--;
        curr_num_id++;

        // switch to next file, if curr_num_id exceeds
        if (curr_num_id / nums_in_file != file_id) {
            file_id = curr_num_id / nums_in_file;
            fclose(fp);
            file_open = false;
            if(nums_to_process != 0){
                snprintf(filename, 100, "%s%d.txt", file_prefix, file_id);
                fp = fopen(filename, "r");
                file_open = true;
            }
        }
    }

    if (file_open) fclose(fp);

    pthread_mutex_lock( &mutex );
    total_sum += sum;
    pthread_mutex_unlock( &mutex );

    free((process_file_args*) args);
    return (void*) 0;
}

int main(int argc, char* argv[]) {

    pid_t pid;

    // parse command line args
    int num_a_files;
    int num_b_files;
    int n_rand_numbers;
    int n_parent_threads;
    int n_child_threads;
    if (argc == 6) {
        num_a_files = atoi(argv[1]);
        num_b_files = atoi(argv[2]);
        n_rand_numbers = atoi(argv[3]);
        n_parent_threads = atoi(argv[4]);
        n_child_threads = atoi(argv[5]);
    } else {
        usage();
        return 1;
    }

    int actual_sum = prepare_files(num_a_files, num_b_files, n_rand_numbers);

    // create pipe for communication between
    // child process and parent process after fork

    int pipefd[2];
    pipe(pipefd);

    pid = fork();
    // On success, the PID of the child process is returned in the parent,
    // and 0 is returned in the child.
    if(pid == 0) {
        // Child process

        // close read end of the pipe
        close(pipefd[0]);
        char file_prefix[] = "b_";
        printf("Spawning %d threads from Child process\n", n_child_threads);
        pthread_t* child_threads = (pthread_t*) malloc(sizeof(pthread_t)*n_child_threads);
        for(int i = 0; i < n_child_threads; i++){
            process_file_args* args = (process_file_args*) malloc(sizeof(process_file_args));
            args->file_prefix = file_prefix;
            args->n_files = num_b_files;
            args->nums_in_file = n_rand_numbers;
            args->n_threads = n_child_threads;
            args->id = i;

            pthread_create( &child_threads[i], NULL, &process_file, (void*) args);
        }
        for(int i = 0; i < n_child_threads; i++){
            pthread_join(child_threads[i], NULL);
        }
        printf("Child process threads joined\n");
        free(child_threads);
        // write total sum to pipe
        write(pipefd[1], &total_sum, sizeof(total_sum));
        // close write end of pipe
        close(pipefd[1]);
    } else {
        // Parent process

        // close write end of pipe
        close(pipefd[1]);
        char file_prefix[] = "a_";
        printf("Spawning %d threads from Parent process\n", n_parent_threads);
        pthread_t* parent_threads = (pthread_t*) malloc(sizeof(pthread_t)*n_parent_threads);
        for(int i = 0; i < n_parent_threads; i++){
            process_file_args* args = (process_file_args*) malloc(sizeof(process_file_args));
            args->file_prefix = file_prefix;
            args->n_files = num_a_files;
            args->nums_in_file = n_rand_numbers;
            args->n_threads = n_parent_threads;
            args->id = i;
            pthread_create( &parent_threads[i], NULL, &process_file, (void*) args);
        }
        for(int i = 0; i < n_parent_threads; i++){
            pthread_join(parent_threads[i], NULL);
        }
        printf("Parent process threads joined\n");
        free(parent_threads);
        // wait until child process exits
        wait(NULL);
        // read sum from child process via pipe
        int sum_from_child_process;
        read(pipefd[0], &sum_from_child_process, sizeof(sum_from_child_process));
        // close read end of pipe
        close(pipefd[0]);
        // display final value
        int sum_from_parent_process = total_sum;
        printf("Sum computed by the parent process threads: %d [A]\n", sum_from_parent_process);
        printf("Sum computed by the child process threads: %d [B]\n", sum_from_child_process);
        printf("Total sum computed by the threads: %d [A + B]\n", sum_from_parent_process + sum_from_child_process);
        if(actual_sum == sum_from_child_process + sum_from_parent_process) {
            printf("SUCCESS: Actual sum matches the computed sum.\n");
        } else {
            printf("ERROR: There's some bug in the program.\n");
        }
    }

    return 0;
}
