#include <mpi.h>

#define MPI_IITTP_BARRIER mpi_iittp_barrier(__COUNTER__)

void mpi_iittp_barrier(int ID) {
    int n_processes;
    MPI_Comm_size(MPI_COMM_WORLD, &n_processes);

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    char flag;
    const int BAR_WAIT = 9999;
    const int BAR_FREE = 9898;

    if (rank == 0) { // root node
        if(n_processes > 1) MPI_Recv(&flag, 1, MPI_BYTE, rank+1, BAR_WAIT + ID, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        if(n_processes > 2) MPI_Recv(&flag, 1, MPI_BYTE, rank+2, BAR_WAIT + ID, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        if(n_processes > 1) MPI_Send(&flag, 1, MPI_BYTE, rank+1, BAR_FREE + ID, MPI_COMM_WORLD);
        if(n_processes > 2) MPI_Send(&flag, 1, MPI_BYTE, rank+2, BAR_FREE + ID, MPI_COMM_WORLD);
    } else if ((rank+1)*2 <= n_processes) { // internal node
        MPI_Recv(&flag, 1, MPI_BYTE, (rank+1)*2 - 1, BAR_WAIT + ID, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        if((rank+1)*2 + 1 <= n_processes) MPI_Recv(&flag, 1, MPI_BYTE, (rank+1)*2, BAR_WAIT + ID, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        MPI_Send(&flag, 1, MPI_BYTE, (rank+1)/2 - 1, BAR_WAIT + ID, MPI_COMM_WORLD);
        MPI_Recv(&flag, 1, MPI_BYTE, (rank+1)/2 - 1, BAR_FREE + ID, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        MPI_Send(&flag, 1, MPI_BYTE, (rank+1)*2 - 1, BAR_FREE + ID, MPI_COMM_WORLD);
        if((rank+1)*2 + 1 <= n_processes) MPI_Send(&flag, 1, MPI_BYTE, (rank+1)*2, BAR_FREE + ID, MPI_COMM_WORLD);
    } else { // leaf node
        MPI_Send(&flag, 1, MPI_BYTE, (rank+1)/2 - 1, BAR_WAIT + ID, MPI_COMM_WORLD);
        MPI_Recv(&flag, 1, MPI_BYTE, (rank+1)/2 - 1, BAR_FREE + ID, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
}
