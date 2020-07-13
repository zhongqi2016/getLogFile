//
// Created by zhongqi on 07.06.2020.
//
#include "MPI_mod.h"
#include <string.h>
#include <stdbool.h>

struct Link {
    int procId;
    int partOfProc;
    bool seqOrPar;
    struct Link *next;
    struct Link *sibling;
};
struct proc {
    int size;
    struct Link *headOfLink;
};

int world_size;
struct proc *allProc;

void init_link(struct Link *link, int procId, int partId, bool seqOrPar) {
    link->procId = procId;
    link->partOfProc = partId;
    link->seqOrPar = seqOrPar;
    link->next = NULL;
    link->sibling = NULL;
}

int MPI_Init2(int *argc, char ***argv) {
    int res = MPI_Init(argc, argv);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    allProc = (struct proc *) malloc(world_size * sizeof(struct proc));
    int i;
    for (i = 0; i < world_size; ++i) {
        allProc[i].size = 1;
        allProc[i].headOfLink = (struct Link *) malloc(sizeof(struct Link));
        init_link(allProc[i].headOfLink, i, 0, true);
    }
    return res;
}


int MPI_Send2(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm) {
//    int size;
//    MPI_Type_size(datatype, &size);
//    struct data *d = (struct data *) malloc(sizeof(struct data));
    MPI_Status status;
    int recvSize;
    MPI_Recv(&recvSize, 1, MPI_INT, dest, tag + 1004, comm, &status);
    allProc[dest].size = recvSize;
    int sendId;
    if (world_size == 0) {
        printf("Uninitialized\n");
        return 0;
    }
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    struct Link *links = allProc[world_rank].headOfLink;
    while (links->next != NULL) {
        links = links->next;
    }
    //Par
    struct Link *sibLink = links;
    while (sibLink->sibling != NULL) {
        if (sibLink->procId == dest && sibLink->partOfProc == allProc[dest].size - 1) {
            goto next;
        }
        sibLink = sibLink->sibling;
    }
    if (sibLink->procId == dest && sibLink->partOfProc == allProc[dest].size - 1) {
        goto next;
    }
    sibLink->sibling = (struct Link *) malloc(sizeof(struct Link));
    init_link(sibLink->sibling, dest, allProc[dest].size - 1, false);
    sibLink->seqOrPar = false;
    next:;
    links->next = (struct Link *) malloc(sizeof(struct Link));
    links = links->next;

    init_link(links, world_rank, allProc[world_rank].size, true);

    allProc[world_rank].size++;
    allProc[dest].size++;
    //Seq
    links->sibling = (struct Link *) malloc(sizeof(struct Link));
    init_link(links->sibling, dest, allProc[dest].size - 1, true);

    //Next
    links->next = (struct Link *) malloc(sizeof(struct Link));
    links = links->next;
    init_link(links, world_rank, allProc[world_rank].size, false);
    allProc[world_rank].size++;
    allProc[dest].size++;
    sendId = allProc[world_rank].size;
    //Par
    links->sibling = (struct Link *) malloc(sizeof(struct Link));
    init_link(links->sibling, dest, allProc[dest].size - 1, false);

    sendId -= 2;
    int res = MPI_Send(buf, count, datatype, dest, tag, comm);
    MPI_Send(&sendId, 1, MPI_INT, dest, 100, comm);

    return res;

}

int MPI_Recv2(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Status *status) {
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    int sendSize = allProc[world_rank].size;
    MPI_Send(&sendSize, 1, MPI_INT, source, tag + 1004, comm);
    int res = MPI_Recv(buf, count, datatype, source, tag, comm, status);
    int recvId;
    MPI_Recv(&recvId, 1, MPI_INT, source, 100, comm, status);

    if (world_size == 0) {
        printf("Uninitialized\n");
        return 0;
    }


    struct Link *links = allProc[world_rank].headOfLink;
    while (links->next != NULL) {
        links = links->next;
    }

    //Par
    struct Link *sibLink = links;
    while (sibLink->sibling != NULL) {
        if (sibLink->procId == source && sibLink->partOfProc == recvId - 1) {
            goto next;
        }
        sibLink = sibLink->sibling;
    }
    if (sibLink->procId == source && sibLink->partOfProc == recvId - 1)
        goto next;
    sibLink->sibling = (struct Link *) malloc(sizeof(struct Link));
    init_link(sibLink->sibling, source, recvId - 1, false);
    sibLink->seqOrPar = false;
    next:;
    links->next = (struct Link *) malloc(sizeof(struct Link));
    links = links->next;

    init_link(links, source, recvId, true);
    allProc[world_rank].size++;

    //Seq
    links->sibling = (struct Link *) malloc(sizeof(struct Link));
    init_link(links->sibling, world_rank, allProc[world_rank].size - 1, true);

    //Next
    links->next = (struct Link *) malloc(sizeof(struct Link));
    links = links->next;
    allProc[world_rank].size++;

    init_link(links, world_rank, allProc[world_rank].size - 1, false);

    //Par
    links->sibling = (struct Link *) malloc(sizeof(struct Link));
    init_link(links->sibling, source, recvId + 1, false);

    return res;
}

int getLengthOfNum(int num) {
    int length = 1;
    while (num > 9) {
        ++length;
        num /= 10;
    }
    return length;
}

void getStrOfNum(char *str, int *i, int num) {
    int num2 = num;
    int k = *i;
    while (num2 > 9) {
        str[*i] = num2 % 10 + 48;
        i++;
        num2 /= 10;
    }
    int j = *i;
    for (; j > k; j--, k++) {
        str[j] = str[j] ^ str[k];
        str[k] = str[j] ^ str[k];
        str[j] = str[j] ^ str[k];
    }
}

void LoadLinkToFile(char *path) {
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    struct proc curProc = allProc[world_rank];

    struct Link *curLink = curProc.headOfLink;
    int length[curProc.size + 1];
    char *str[curProc.size + 1];
    length[0] = 2 + getLengthOfNum(world_rank);
    str[0] = malloc(length[0] * sizeof(char));
    sprintf(str[0], "t%d\n", world_rank);

    int i = 1;
    do {
        struct Link *sibLink = curLink;
        length[i] = 4;
        do {
            length[i] += 2 + getLengthOfNum(sibLink->procId) + getLengthOfNum(sibLink->partOfProc);
            sibLink = sibLink->sibling;
        } while (sibLink != NULL);

        sibLink = curLink;
        str[i] = malloc(length[i] * sizeof(char));
        sprintf(str[i], "%s", curLink->seqOrPar ? "seq" : "par");
        do {
            char subStr[10];
            sprintf(subStr, " %d.%d", sibLink->procId, sibLink->partOfProc);
            strcat(str[i], subStr);
            sibLink = sibLink->sibling;
        } while (sibLink != NULL);
        strcat(str[i], "\n");
//            str[i][j] = '\0';
//            printf("%s\n", str[i]);
        i++;
        curLink = curLink->next;
    } while (curLink != NULL);
    int sum_length = 1;
    int k;
    for (k = 0; k < curProc.size + 1; ++k) {
        sum_length += length[k];
    }
    char *strProc = malloc(sum_length * sizeof(char));
    strcpy(strProc, str[0]);
    for (k = 1; k < curProc.size + 1; ++k) {
//        printf("get %s",str[k]);
        strcat(strProc, str[k]);
        free(str[k]);
    }
    if (world_rank != 0) {
        MPI_Send(&sum_length, 1, MPI_INT, 0, 1002, MPI_COMM_WORLD);
        MPI_Send(strProc, sum_length + 1, MPI_CHAR, 0, 1001, MPI_COMM_WORLD);
        free(strProc);
    } else {
        MPI_Status status;

        FILE *fp = NULL;
        fp = fopen(path, "w+");

        int sum_length;
        char *str_recv;
//        printf("%s\n", strProc);
        fprintf(fp, "%s\n", strProc);
        free(strProc);
        int i;
        for (i = 1; i < world_size; ++i) {
            MPI_Recv(&sum_length, 1, MPI_INT, MPI_ANY_SOURCE, 1002, MPI_COMM_WORLD, &status);
            str_recv = malloc((size_t) sum_length + 1);
            MPI_Recv(str_recv, sum_length + 1, MPI_CHAR, MPI_ANY_SOURCE, 1001, MPI_COMM_WORLD, &status);
//            printf("%s\n", str_recv);
            fprintf(fp, "%s\n", str_recv);
            free(str_recv);
        }
        fclose(fp);
        printf("Результат был записан в %s\n",path);
    }
}

//int MPI_Recv2
void print_state_file(FILE *fp) {
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
//    int i;
//    for (i = 0; i < world_size; ++i) {
    fprintf(fp, "t%d\n", world_rank);
    struct Link *curLink = allProc[world_rank].headOfLink;
    do {
        fprintf(fp, curLink->seqOrPar ? "seq" : "par");
        struct Link *sibLink = curLink;
        while (sibLink != NULL) {
            fprintf(fp, " %d.%d", sibLink->procId, sibLink->partOfProc);
            sibLink = sibLink->sibling;
        }
        fprintf(fp, "\n");
        curLink = curLink->next;
    } while (curLink != NULL);
    fprintf(fp, "\n");
//    }
}

void print_state() {
    int world_rank = 0;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
//    int i;
//    for (i = 0; i < world_size; ++i) {
    printf("t%d\n", world_rank);

    struct Link *curLink = allProc[world_rank].headOfLink;
    do {
        printf(curLink->seqOrPar ? "seq" : "par");
        struct Link *sibLink = curLink;
        while (sibLink != NULL) {
            printf(" %d.%d", sibLink->procId, sibLink->partOfProc);
            sibLink = sibLink->sibling;
        }
        printf("\n");
        curLink = curLink->next;
    } while (curLink != NULL);
    printf("\n");
//    }
}