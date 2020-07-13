#include <stdio.h>
#include "MPI_mod.h"

int main(int argc, char** argv) {

    // Initialize the MPI environment
    MPI_Init2(NULL, NULL);
    // Find out rank, size
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    int number;
    if (world_rank == 0) {
        number = -1;
        MPI_Send2(&number, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
        printf("Process 0 sent number %d to process 1\n",
               number);
    } else if (world_rank == 1) {
        MPI_Recv2(&number, 1, MPI_INT, 0, 0, MPI_COMM_WORLD,
                 MPI_STATUS_IGNORE);
        printf("Process 1 received number %d from process 0\n",
               number);
    }

    else if (world_rank == 2) {
        number = 10;
        MPI_Send2(&number, 1, MPI_INT, 3, 0, MPI_COMM_WORLD);
        printf("Process 2 sent number %d to process 3\n",
               number);
        MPI_Recv2(&number, 1, MPI_INT, 3, 0, MPI_COMM_WORLD,
                  MPI_STATUS_IGNORE);
        printf("Process 2 received number %d from process 3\n",
               number);
    } else if (world_rank == 3) {
        MPI_Recv2(&number, 1, MPI_INT, 2, 0, MPI_COMM_WORLD,
                  MPI_STATUS_IGNORE);
        printf("Process 3 received number %d from process 3\n",
               number);
        MPI_Send2(&number, 1, MPI_INT, 2, 0, MPI_COMM_WORLD);
        printf("Process 3 sent number %d to process 2\n",
               number);

    }
    LoadLinkToFile("input.txt");
    MPI_Finalize();
}