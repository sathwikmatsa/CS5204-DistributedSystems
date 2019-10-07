# Multithreading Using Pthreads and Fork
## Compile and Run
```
> make
> make run  # executes ./prog 80 20 400 5 8
```
## Sample Output
```
Created 80 a_xx.txt files and populated 400 numbers in each of 'em
Created 20 b_xx.txt files and populated 400 numbers in each of 'em
Done preparing files. Total sum equals: 1990367
Spawning 5 threads from Parent process
Spawning 8 threads from Child process
Child process threads joined
Parent process threads joined
Sum computed by the parent process threads: 1582957 [A]
Sum computed by the child process threads: 407410 [B]
Total sum computed by the threads: 1990367 [A + B]
SUCCESS: Actual sum matches the computed sum.
```

## Usage
```
./prog <num_a_files> <num_b_files> <n_rand_numbers> <n_parent_threads> <n_child_threads>
```

## Question for reference

>Input:
>./your_wonderful_program 80 20 400 5 8
>
>Your program should:
>1. Create 80 (argv[1]) files with names a_0.txt, a_1.txt, a_2.txt, ....., a_79.txt
>2. Create 20 (argv[2]) files with names b_0.txt, b_1.txt, ...., b_19.txt
>3. Each file should be filled up with 400 (argv[3]) random numbers
>4. After the files are created, a new process should be created using fork()
>5. The parent process should spawn 5 (argv[4]) threads
>6. The child process should spawn 8 (argv[5]) threads
>7. The 5 threads in the parent process should process files starting with a_. If there are 5 files, each thread gets 1 file to process. If there are 20 files, each thread gets 4 files to process. If there are 21 files, thread_0 gets 6 files to process, and all other threads get 5 files each.
>8. The 8 threads in the forked process should process files starting with b_. Work should be divided as noted in the previous point.
>9. If the user commands the program to create 4 a_ files, and 8 threads in the parent process, thread_0 should process the first 500 numbers in a_0.txt, thread_1 should process the next 500 numbers in a_0.txt, threads_2 should process the first 500 numbers in a_1.txt, and so on.
>10. There should be no idle threads eating idlis or sleeping.
>11. A thread opens a file, and adds the numbers.
>12. The output of the program should be the total across all files i.e., total(a_0.txt) + total(a_1.txt) + .... + total(b_0.txt) + total(b_1.txt) + .. + total (b_19.txt).
>13. You cannot create any intermediary file(s). All files should be opened in Read mode alone.
>14. The final value can be displayed only by the parent process.
>15. The program should be written in C or C++ (sorry Java, Python, .Net experts).
>16. MPI should not be used to do this assignment (sorry MPI experts).
>17. Deep learning should not be used to identify the total (sorry everyone).

