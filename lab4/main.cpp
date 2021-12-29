//g++ -o main  main.cpp -lpthread -lrt

#include <fcntl.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <iostream>

#include "mem.h"

using namespace std;

int main() {
	char *filename = NULL;
    size_t len = 0;
    std::cout << "Enter the name of file for answers: ";
    if (getline(&filename, &len, stdin) == -1) {
        perror("getline");
        _exit(EXIT_FAILURE);
    }
    filename[strlen(filename) - 1] = '\0';

    std::cout << "Enter numerous:" << std::endl;

    size_t map_size = 0;
    char *in = (char *)malloc(sizeof(char));
    char c;
    while ((c = getchar()) != EOF) {
	    in[map_size] = c;
		  in = (char *)realloc(in, (++map_size + 1) * sizeof(char));
    }
    in[map_size++] = '\0';

    int fd = shm_open(CommonFile, O_RDWR | O_CREAT, mode);
    if (fd == -1) {
		  perror("OPEN");
		  _exit(EXIT_FAILURE);
    }
    sem_t *semptr = sem_open(SemaphoreName, O_CREAT, mode, 1);
    if (semptr == SEM_FAILED) {
		  perror("SEM_OPEN");
		  _exit(EXIT_FAILURE);
    }
    int val;

    ftruncate(fd, (off_t)map_size);
    char* memptr = (char*)mmap(
	    NULL,
	    map_size,
	    PROT_READ | PROT_WRITE,
	    MAP_SHARED,
	    fd,
	    0);
    if (memptr == MAP_FAILED) {
		  perror("MMAP");
		  _exit(EXIT_FAILURE);
    }
    sprintf(memptr, "%s", in);
    free(in);
    if (sem_getvalue(semptr, &val) != 0) {
		  perror("SEM_GETVALUE");
		  _exit(EXIT_FAILURE);
    }
    while (val++ < 1) {
        sem_post(semptr);
    }
    int pid = fork();

    if (pid == 0) {
		  munmap(memptr, map_size);
	    close(fd);
	    sem_close(semptr);
      execl("child", "child", filename, NULL);
	    perror("EXECL");
    } else if (pid < 0) {
	    perror("FORK");
	    exit(EXIT_FAILURE);
    }
    while (true) {
		if (sem_getvalue(semptr, &val) != 0) {
	  	    perror("SEM_GETVALUE");
	  	    _exit(EXIT_FAILURE);
		}
		if (val == 0) {
	  	    if (sem_wait(semptr) == -1) {
		        perror("SEM_WAIT");
  	            _exit(EXIT_FAILURE);
	    	}   
	  	    cout << memptr;
	  	    return EXIT_SUCCESS;
		}
    }
}