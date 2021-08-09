#include <iostream>

// type declaration to function pointer
typedef int (*int_return_int)(void* userdata, int);

struct closure_delegate
{
    int_return_int callback;
    void* userdata;
};

// call operator to make it convenient to call the callback
int closure_delegate_execute(closure_delegate cd, int value)
{
    return cd.callback(cd.userdata, value);
}

// constructor to make it convenient to create a callback
closure_delegate closure_delegate_bind_to_int(int_return_int callback, int* userdata)
{
    return {callback, userdata};
}

// the function to be called polymorphically
int f(void* userdata, int n)
{
    return *((int*)userdata) * n;
}

int main()
{
	int userdata = 2;
    closure_delegate cd = closure_delegate_bind_to_int(f, &userdata);
    std::cout << closure_delegate_execute(cd, 7) << '\n';// 14
    std::cout << closure_delegate_execute(cd, 9) << '\n';// 18
}