# Odd Even Sort in MPI
## Compile and Run
```
> mpicc odd_even_sort.c -o oes
> mpirun --oversubscribe -np 4 ./oes 20
```
## Sample Output
```
Array elements before sorting:
83 86 77 15 93 90 19 88 75 61 46 85 68 40 25 1 83 74 26 63
Array elements after sorting:
1 15 19 25 26 40 46 61 63 68 74 75 77 83 83 85 86 88 90 93
```
