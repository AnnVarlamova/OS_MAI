//g++ -o child  child.cpp -lpthread -lrt

#include <fcntl.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string>
#include <iostream>

#include "mem.h"

using namespace std;

int main(int argc, char **argv) {
    int map_fd = shm_open(CommonFile, O_RDWR, mode);
    if (map_fd < 0) {
	    perror("SHM_OPEN");
	    _exit(EXIT_FAILURE);
    }
    struct stat statbuf;
    fstat(map_fd, &statbuf);
    const size_t map_size = statbuf.st_size;
    char* memptr = (char*)mmap(
	    NULL,
	    map_size,
	    PROT_READ | PROT_WRITE,
	    MAP_SHARED,
	    map_fd,
	    0);
    if (memptr == MAP_FAILED) {
	    perror("MMAP");
	    _exit(EXIT_FAILURE);
    }
    sem_t *semptr = sem_open(SemaphoreName, O_CREAT, mode, 1);
    if (semptr == SEM_FAILED) {
	    perror("SEM_OPEN");
	    _exit(EXIT_FAILURE);
    }
    if (sem_wait(semptr) != 0) {
	    perror("SEM_WAIT");
	    _exit(EXIT_FAILURE);
    }
    char *out = (char *)malloc(sizeof(char));
    size_t m_size = 0;
    int flag = 0;
    string first;
    string second;

    FILE *filename = fopen(argv[1], "w");
    if (filename == NULL) {
		perror("File not opened");
	    _exit(EXIT_FAILURE);
	}

    for (int i = 0; i + 1 < map_size; ++i) {// преобразование
  	    if (flag == 0) {
  		    first.push_back(memptr[i]);
  	    } else if (flag == 1) {
  		    second.push_back(memptr[i]);
  	    }
  	    if (memptr[i] == ' ' && flag == 0) {
  		    flag = 1;
  	    } else if ((memptr[i] == ' ' || memptr[i] == '\n') && flag == 1) {
  		    if (atof(second.c_str()) == 0) { 
  			    perror("division by zero error\n");
  			    break;
  		    }
  		    first = to_string(atof(first.c_str()) / atof(second.c_str()));
  		    second = "";
  		    if (memptr[i] == '\n') {
				fprintf(filename, "%s\n", first.c_str());
  			    flag = 0;
  			    first = "";
  			    second = "";
  		    }
  	    }
    }
    fclose(filename);

    out[m_size++] = '\0';
    ftruncate(map_fd, (off_t)m_size);
    memset(memptr, '\0', m_size);
    sprintf(memptr, "%s", out);
    free(out);
    close(map_fd);
    sem_post(semptr);
    sem_close(semptr);
    return EXIT_SUCCESS;
}