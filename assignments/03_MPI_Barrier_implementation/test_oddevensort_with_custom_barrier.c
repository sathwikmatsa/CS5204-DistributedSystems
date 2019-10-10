#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <mpi.h>
#include "mpi_iittp_barrier.h"
#include <assert.h>

#define ODD 3
#define EVEN 4
#define GATHER 0
#define EXCHANGE 9

typedef
struct Piece {
    int id;
    int np;
    int nelems;
    int* data;
    int data_len;
    int logical_index;
    bool left_even_dep;
    bool right_even_dep;
    bool left_odd_dep;
    bool right_odd_dep;
    int odd_start_index;
    int even_start_index;
} Piece;

void debug_peice(Piece* p) {
    printf("id: %d\n", p->id);
    printf("data_len: %d\n", p->data_len);
    printf("logical_index: %d\n", p->logical_index);
    printf("left_even_dep: %d\n", p->left_even_dep);
    printf("right_even_dep: %d\n", p->right_even_dep);
    printf("left_odd_dep: %d\n", p->left_odd_dep);
    printf("right_odd_dep: %d\n", p->right_odd_dep);
    printf("odd_start_index: %d\n", p->odd_start_index);
    printf("even_start_index: %d\n", p->even_start_index);
    fflush(stdout);
}

int get_distribution(int id, int np, int nelems) {
    if(id >= np) return 0;
    int average = nelems / np;
    if (average >= 2) {
        if (id == 0) return average + (nelems % np);
        else return average;
    } else {
        return get_distribution(id, np-1, nelems);
    }
}

void set_dependecies(Piece *p) {
    if(p->data_len == 0) return;
    int logical_start_index = 0;
    int id = p->id;
    int nelems = p->nelems;
    int np = p->np;
    for(int i = 0; i < id; i++) {
        logical_start_index += get_distribution(i, np, nelems);
    }
    p->logical_index = logical_start_index;
    if(logical_start_index % 2 == 0) {
        p->left_odd_dep = false;
        p->left_even_dep = logical_start_index != 0 ? true : false;
        p->odd_start_index = 0;
        p->even_start_index = 1;
    } else {
        p->left_odd_dep = true;
        p->left_even_dep = false;
        p->odd_start_index = 1;
        p->even_start_index = 0;
    }
    int logical_end_index = logical_start_index + p->data_len - 1;
    if(logical_end_index % 2 == 0) {
        p->right_odd_dep = logical_end_index != nelems-1 ? true : false;
        p->right_even_dep = false;
    } else {
        p->right_odd_dep = false;
        p->right_even_dep = logical_end_index != nelems-1 ? true : false;
    }
}

void print_array(Piece *p, const char* s) {
    if (p->id == 0) {
        printf("%s\n", s);
        for(int i = 0 ; i < p->data_len; i++) printf("%d ", p->data[i]);
        int* buffer = malloc(sizeof(int)*p->data_len);
        for(int i = 1; i < p->np; i++){
            int n;
            MPI_Status status;
            MPI_Recv(buffer, p->data_len, MPI_INT, i, GATHER, MPI_COMM_WORLD, &status);
            MPI_Get_count(&status, MPI_INT, &n);
            for(int j = 0; j < n; j++) printf("%d ", buffer[j]);
        }
        printf("\n");
        fflush(stdout);
        free(buffer);
    } else {
        MPI_Send(p->data, p->data_len, MPI_INT, 0, GATHER, MPI_COMM_WORLD);
    }
}

void swap(int* A, int i, int j, int len) {
    if((j >= len) || (i == j) || (i >= len)) return;
    int temp = A[j];
    A[j] = A[i];
    A[i] = temp;
}


void sort_phase(int phase, Piece *p) {
    int len = p->data_len;
    if(len == 0) return;
    int start_index = phase == ODD ? p->odd_start_index : p->even_start_index;
    for(int i = start_index; i < len - 1; i+=2) {
        if(p->data[i] > p->data[i+1]) swap(p->data, i, i+1, len);
    }
    bool left_dep = phase == ODD ? p->left_odd_dep : p->left_even_dep;
    bool right_dep = phase == ODD ? p->right_odd_dep : p->right_even_dep;

    if(left_dep) {
        int local = p->data[0];
        MPI_Send(&local, 1, MPI_INT, p->id-1, EXCHANGE, MPI_COMM_WORLD);
        int other = p->data[0];
        MPI_Recv(&other, 1, MPI_INT, p->id-1, EXCHANGE, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        if(other > local) p->data[0] = other;
    }
    if(right_dep) {
        int other;
        MPI_Recv(&other, 1, MPI_INT, p->id+1, EXCHANGE, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        int local = p->data[p->data_len-1];
        MPI_Send(&local, 1, MPI_INT, p->id+1, EXCHANGE, MPI_COMM_WORLD);
        if(other < local) p->data[p->data_len-1] = other;
    }
}

void unit_tests();

int main(int argc, char* argv[]) {
    // parse command line args
    int nelems = argc == 2 ? atoi(argv[1]) : 10;

    // MPI setup
    MPI_Init(&argc, &argv);

    int id, np;
    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    MPI_Comm_size(MPI_COMM_WORLD, &np);

    // testing
    //if(id == 0) unit_tests();
    //MPI_Barrier(MPI_COMM_WORLD);
    MPI_IITTP_BARRIER;

    // initialization
    srand(id+1);
    Piece p;
    p.id = id;
    p.np = np;
    p.nelems = nelems;
    p.data_len = get_distribution(id, np, nelems);
    p.data = (int*) malloc(sizeof(int)*p.data_len);
    for(int i = 0; i < p.data_len; i++) {
        p.data[i] = rand()%100;
    }
    set_dependecies(&p);
    // debug_peice(&p);

    print_array(&p, "Array elements before sorting:");

    // sort logic
    for(int i = 0; i < nelems; i++) {
        sort_phase(ODD, &p);
        //print_array(&p, "Array elements after odd phase:");
        //MPI_Barrier(MPI_COMM_WORLD);
        MPI_IITTP_BARRIER;
        sort_phase(EVEN, &p);
        //print_array(&p, "Array elements after even phase:");
        //MPI_Barrier(MPI_COMM_WORLD);
        MPI_IITTP_BARRIER;
    }

    print_array(&p, "Array elements after sorting:");

    // cleanup
    free(p.data);

    MPI_Finalize();
}

void unit_tests() {
    printf("Test: get_distribution(id, np, nelems)");
    assert(get_distribution(0, 1, 6) == 6);
    assert(get_distribution(0, 2, 6) == 3);
    assert(get_distribution(0, 3, 6) == 2);
    assert(get_distribution(0, 4, 6) == 2);
    assert(get_distribution(1, 4, 6) == 2);
    assert(get_distribution(2, 4, 6) == 2);
    assert(get_distribution(3, 4, 6) == 0);
    printf("-success\n");
    printf("Test: swap(A, i, j, len)");
    int A[3] = {1,2,3};
    swap(A, 2, 0, 3);
    assert(A[0] == 3);
    printf("-success\n");
}
