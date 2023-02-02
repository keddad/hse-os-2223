#include <stdio.h>
#include <unistd.h>

int fibonachi(int n) {
    int a = 0, b = 1;

    for (int i = 1; i < n; ++i) {
        int tmp = b;
        b += a;
        a = tmp;
    }
    
    return a;
}

int factorial(int n) {
    int target = 1;

    for (int i = 2; i <= n; ++i) {
        target *= i;
    }

    return target;
}

int main() {
    int n;
    scanf("%d", &n);

    int pid = fork();

    if (pid) { // not zero => parent process
        int x = fibonachi(n);
        printf("Nth fibonachi: %d\n", x);
    } else { // zero => child process
        int x = factorial(n);
        printf("Factorial of n: %d\n", x);
    }

    return 0;
}