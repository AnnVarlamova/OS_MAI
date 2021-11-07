#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <vector>
#include <iostream>

int main(int argc, char *argv[]) {
    char err = '1';
    int myfile = open(argv[1], O_CREAT | O_WRONLY, S_IRWXU);

    if (myfile == -1) {
        err = '2';
        std::cout << err;
        fflush(stdout);
        _exit(EXIT_FAILURE);
    }
    
    float x, res;
    int n, i;
    while (std::cin >> n) {
        for (i = 0; i < n; ++i) {
            std::cin >> x;
            if (i == 0) {
                res = x;
            } else {
                if (x == 0) {
                    err = '0';
                    std::cout << err;
                    fflush(stdout);
                    close(myfile);
                    _exit(EXIT_FAILURE);
                } else {
                    if (i > 0) {
                        res /= x;
                    }
                }
            }
        }
        dprintf(myfile, "%f\n", res);
        fflush(stdout);
        std::cout << err;
        fflush(stdout);
    }
    close(myfile);
    return 0;
}