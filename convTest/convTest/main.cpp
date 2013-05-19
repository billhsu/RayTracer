#include <iostream>
#include <stddef.h>
#include <stdio.h>

void convolve(const double Signal[/* SignalLen */], size_t SignalLen,
    const double Kernel[/* KernelLen */], size_t KernelLen,
    double Result[/* SignalLen + KernelLen - 1 */])
{
    size_t n;

    for (n = 0; n < SignalLen + KernelLen - 1; n++)
    {
        size_t kmin, kmax, k;

        Result[n] = 0;

        kmin = (n >= KernelLen - 1) ? n - (KernelLen - 1) : 0;
        kmax = (n < SignalLen - 1) ? n : SignalLen - 1;

        for (k = kmin; k <= kmax; k++)
        {
            Result[n] += Signal[k] * Kernel[n - k];
        }
    }
}

void printSignal(const char* Name,
    double Signal[/* SignalLen */], size_t SignalLen)
{
    size_t i;

    for (i = 0; i < SignalLen; i++)
    {
        printf("%s[%d] = %f\n", Name, i, Signal[i]);
    }
    printf("\n");
}

#define ELEMENT_COUNT(X) (sizeof(X) / sizeof((X)[0]))

int main(void)
{
    double signal[] = { 1, 1, 1, 2, 2, 2};
    double signal1[] = { 1, 1 };
    double signal2[] = { 1, 2 };
    double signal3[] = { 2, 2 };
    double kernel[] = { 1, 2 };
    double result[ELEMENT_COUNT(signal) + ELEMENT_COUNT(kernel) - 1];
    double result1[ELEMENT_COUNT(signal1) + ELEMENT_COUNT(kernel) - 1];
    double result2[ELEMENT_COUNT(signal2) + ELEMENT_COUNT(kernel) - 1];
    double result3[ELEMENT_COUNT(signal3) + ELEMENT_COUNT(kernel) - 1];
    convolve(signal, ELEMENT_COUNT(signal),
        kernel, ELEMENT_COUNT(kernel),
        result);
    convolve(signal1, ELEMENT_COUNT(signal1),
        kernel, ELEMENT_COUNT(kernel),
        result1);
    convolve(signal2, ELEMENT_COUNT(signal2),
        kernel, ELEMENT_COUNT(kernel),
        result2);
    convolve(signal3, ELEMENT_COUNT(signal3),
        kernel, ELEMENT_COUNT(kernel),
        result3);

    printSignal("signal", signal, ELEMENT_COUNT(signal));
    printSignal("signal1", signal1, ELEMENT_COUNT(signal1));
    printSignal("signal2", signal2, ELEMENT_COUNT(signal2));
    printSignal("signal3", signal3, ELEMENT_COUNT(signal2));

    printSignal("kernel", kernel, ELEMENT_COUNT(kernel));

    printSignal("result", result, ELEMENT_COUNT(result));
    printSignal("result1", result1, ELEMENT_COUNT(result1));
    printSignal("result2", result2, ELEMENT_COUNT(result2));
    printSignal("result3", result3, ELEMENT_COUNT(result2));
    system("pause");
    return 0;
}