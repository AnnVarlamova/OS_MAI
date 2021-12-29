#include <cmath>
#include <iostream>

extern "C" float SinIntegral(float A, float B, float e);
extern "C" int PrimeCount(int A, int B);

int PrimeCount(int A, int B) {
    int count = 0;
    if (B < 2)
        return 0;
    if (A < 3) {
        A = 3;
        ++count;
    }
    for (int n = A; n <= B; ++n) {
        for (int divider = 2; divider < n; ++divider) {
            if (n % divider == 0)
                break;
            if (divider == n - 1)
                ++count;
        }
    }
    std::cout << "1" << std::endl;
    return count;
}


float SinIntegral(float A, float B, float e) {
    float rectangle_integral = 0;
    for (float step = A; step + e < B; step += e) 
    {
        float x1 = step;
        float x2;
        if (step + e < B)
            x2 = step+e;
        else
            x2 = B;
        rectangle_integral += 0.5*(x2-x1)*(sin(x1) + sin(x2));
    }
    std::cout << "1" << std::endl;
    return rectangle_integral;
}