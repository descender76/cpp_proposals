#include <iostream>

// type declaration to function pointer
typedef int (*int_return_int)(void* userdata, int);

int f1(void* userdata, int n)
{
    return *((int*)userdata) * n;
}

int f2(void* userdata, int n)
{
    return *((int*)userdata) + n;
}

int f3(void* userdata, int n)
{
    return *((int*)userdata) - n;
}

// function pointers can be passed to functions
int p(int_return_int callback, void* userdata, int value)
{
    return callback(userdata, value);
}

// function pointers can be returned from functions
int_return_int r()
{
    return f1;
}
 
int main()
{
    int userdata = 4;
    int_return_int fp = r();
    std::cout << fp(&userdata, 7) << '\n';// 28
    std::cout << p(fp, &userdata, 9) << '\n';// 36
    fp = f2;
    std::cout << fp(&userdata, 7) << '\n';// 11
    fp = f3;
    std::cout << fp(&userdata, 7) << '\n';// -3
}