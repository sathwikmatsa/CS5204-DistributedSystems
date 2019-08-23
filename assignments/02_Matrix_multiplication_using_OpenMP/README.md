# Matrix Multiplication using OpenMP
## Compile and Run
```
> g++ -fopenmp matrix_multiplication.cpp -o matmul
> ./matmul
```
## Sample Output
```
Generated random matrix A with dimensions 1000x1000
Generated random matrix B with dimensions 1000x1000
Calculating product of A, B -> C
Sequential algorithm took 20 seconds.
Parallel algorithm with 4 threads took 11 seconds.
```
