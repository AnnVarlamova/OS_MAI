#include <cstdlib>
#include <iostream>
#include <dlfcn.h>

int main() {
    std::cout << "Enter num library: ";
    int lib_num;
    std::cin >> lib_num;
    if(lib_num < 1 || lib_num > 2){
        std::cout << "error lib" << std::endl;
        exit(1);
    }
    --lib_num;
    int command;
    const char* libs[] = {"libd1.so", "libd2.so"};
    void* library_handle;
    library_handle = dlopen (libs[lib_num], RTLD_LAZY);
    if(!library_handle){
        std::cout << "Error in dlopen" << std::endl;
        exit(1);
    }

    float (*SinIntegral)(float A, float B, float e);
    int (*PrimeCount)(int A, int B);

    SinIntegral = (float(*)(float, float, float))dlsym(library_handle, "SinIntegral");
    PrimeCount = (int(*)(int, int))dlsym(library_handle, "PrimeCount");

    while((std::cout << "Enter command: 0, 1 or 2: ") && (std::cin >> command)) {
        switch (command) {
            case 0:
                dlclose(library_handle);
                lib_num = (lib_num + 1) % 2;
                library_handle = dlopen(libs[lib_num], RTLD_LAZY);
                if(!library_handle){
                    std::cout << "Error in dlopen" << std::endl;
                    exit(1);
                }
                SinIntegral = (float(*)(float, float, float))dlsym(library_handle, "SinIntegral");
                PrimeCount = (int(*)(int, int))dlsym(library_handle, "PrimeCount");
                std::cout << "Change library" << std::endl;
                break;
            case 1:
                float A, B, e;
                std::cout << "Enter A, B, e: ";
                std::cin >> A >> B >> e;
                std::cout << "Integral value " << SinIntegral(A, B, e) << std::endl;
                break;
            case 2:
                std::cout << "Enter A and B: ";
                int a, b;
                std::cin >> a >> b;
                std::cout << "PrimeCount in [a; b] " << PrimeCount(a, b) << std::endl;
                break;
            default:
                std::cout << "Enter 0, 1 or 2!" << std::endl;
                break;
        }
    }
    dlclose(library_handle);
}
