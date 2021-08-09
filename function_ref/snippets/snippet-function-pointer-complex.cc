#include <iostream>

// type declaration to function pointer
typedef int (*int_return_int)(void* userdata, int);

int f(void* userdata, int n)
{
    return *((int*)userdata) * n;
}

// function pointers can be passed to functions
int g(int_return_int callback, void* userdata, int value)
{
    return callback(userdata, value);
}

int_return_int h()
{
    // function pointers can be returned from functions
    return f;
}
 
int main()
{
    int userdata = 4;
    int_return_int p = f;
    p = &f;
    p = h();
    std::cout << p(&userdata, 7) << '\n';// 28
    std::cout << g(p, &userdata, 9) << '\n';// 36
}