#include <iostream>

// type declaration to function pointer
typedef int (*int_return_int)(int);

int f1(int n)
{
    return n * n;
}

int f2(int n)
{
    return n + n;
}

int f3(int n)
{
    return n - n;
}

// function pointers can be passed to functions
int p(int_return_int callback, int value)
{
    return callback(value);
}

// function pointers can be returned from functions
int_return_int r()
{
    return f1;
}
 
int main()
{
    int_return_int fp = r();
    std::cout << fp(7) << '\n';// 49
    std::cout << p(fp, 9) << '\n';// 81
    fp = f2;
    std::cout << fp(7) << '\n';// 14
    fp = f3;
    std::cout << fp(7) << '\n';// 0
}