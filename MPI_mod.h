//
// Created by zhongqi on 07.06.2020.
//

#ifndef GETLOGFILE_MPI_MOD_H
#define GETLOGFILE_MPI_MOD_H

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>


int MPI_Init2(int *argc, char ***argv);

int MPI_Send2(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm);

int MPI_Recv2(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Status *status);

void LoadLinkToFile(char *path);

#endif //GETLOGFILE_MPI_MOD_H
