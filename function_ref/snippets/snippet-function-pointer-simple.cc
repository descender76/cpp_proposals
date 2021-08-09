#include <iostream>

// type declaration to function pointer
typedef int (*int_return_int)(int);

int f(int n)
{
    return n * n;
}

// function pointers can be passed to functions
int g(int_return_int callback, int value)
{
    return callback(value);
}

int_return_int h()
{
    // function pointers can be returned from functions
    return f;
}
 
int main()
{
    int_return_int p = f;
    p = &f;
    p = h();
    std::cout << p(7) << '\n';// 49
    std::cout << g(p, 9) << '\n';// 81
}