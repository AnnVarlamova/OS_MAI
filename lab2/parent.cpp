#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <vector>
#include <iostream>

int main(void) {
    char *filename = NULL;
    size_t len = 0;
    std::cout << "Enter the name of file for answers: ";
    fflush(stdout);
    if (getline(&filename, &len, stdin) == -1) {
        perror("getline");
        _exit(EXIT_FAILURE);
    }
    filename[strlen(filename) - 1] = '\0';

    std::cout << "Enter numerous:\n" << std::endl;

    int fd1[2], fd2[2];
    if ((pipe(fd1) == -1) || (pipe(fd2) == -1)){
        perror("pipe");
        _exit(EXIT_FAILURE);
    }

    int old_stdout = dup(fileno(stdout));

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        _exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        close(fd2[0]);
        close(fd1[1]);

        if (dup2(fd1[0], fileno(stdin)) == -1) {
            perror("child, dup2, stdin");
            _exit(EXIT_FAILURE);
        }

        if (dup2(fd2[1], fileno(stdout)) == -1) {
            perror("child, dup2, stdout");
            _exit(EXIT_FAILURE);
        }

        if (execl("child.out", "child.out", filename, NULL) == -1) {
            perror("execl");
            _exit(EXIT_FAILURE);
        }
    }
    close(fd1[0]);
    
    int status;
    if (pid > 0) {
        close(fd1[0]);
        close(fd2[1]);
        if (dup2(fd1[1], fileno(stdout)) == -1) {
            perror("parent, dup2, stdout");
            _exit(EXIT_FAILURE);
        }
        char err = '1', c;
        float x;
        std::vector<float> vec;
        while (scanf("%f%c", &x, &c) > 0) {
            vec.push_back(x);
            if (c == '\n') {
                int n = vec.size();
                std::cout << n << " ";
                for (int i = 0; i < n; ++i) {
                    std::cout << vec[i] << " ";
                }
                vec.clear();
                fflush(stdout);
                read(fd2[0], &err, sizeof(1));
                if (err == '0') {
                    wait(&status);
                    close(fd2[0]);
                    close(fd1[1]);
                    dup2(old_stdout, fileno(stdout));
                    close(old_stdout);
                    std::cout << "Divizion by zero" << std::endl;
                    fflush(stdout);
                    _exit(EXIT_FAILURE);
                }
                if (err == '2') {
                    wait(&status);
                    close(fd2[0]);
                    close(fd1[1]);
                    dup2(old_stdout, fileno(stdout));
                    close(old_stdout);
                    std::cout << "Opening file" << std::endl;
                    fflush(stdout);
                    _exit(EXIT_FAILURE);
                }
            }
        }
        close(fd1[1]);
        close(fd2[0]);
    }
    if (wait(&status) == -1) {
        perror("wait");
        _exit(EXIT_FAILURE);
    }
    return 0;
}
