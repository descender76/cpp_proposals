// slightly revised https://github.com/TartanLlama/function_ref/blob/master/tests/constructors.cpp
// ...
//#include <tl/function_ref.hpp>
#include <https://raw.githubusercontent.com/TartanLlama/function_ref/master/include/tl/function_ref.hpp>

void foo(){}
struct bar {
    void baz(){}
};

// ...
    tl::function_ref<void(void)> fr1 = []{};
    tl::function_ref<void(void)> fr2 = foo;
    //tl::function_ref<void(bar)> fr3 = &bar::baz;// immediately dangles
    auto temp = &bar::baz
    tl::function_ref<void(bar)> fr3 = temp;

    // ...