#include <mpi.h>
#include "mpi_iittp_barrier.h"
#include <stdio.h>

int main(int argc, char* argv[]) {

    // MPI setup
    MPI_Init(&argc, &argv);

    int id, np;
    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    MPI_Comm_size(MPI_COMM_WORLD, &np);

    if (id == 0) {
        MPI_IITTP_BARRIER;
    } else {
        MPI_IITTP_BARRIER;
    }

    MPI_Finalize();
}
