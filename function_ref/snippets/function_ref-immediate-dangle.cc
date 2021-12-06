//#include <https://raw.githubusercontent.com/TartanLlama/function_ref/master/include/tl/function_ref.hpp>
#include <https://raw.githubusercontent.com/descender76/cpp_proposals/main/function_ref/function_ref_prime.hpp>

struct bar {
    void baz(){}
};

void foo(bar) {
}

// member function immediately dangle
void function_ref_member_function_dangle() {
    tl::function_ref<void(bar)> fr3 = &bar::baz;

    bar b;
    fr3(b);  // UB, fr3 is already dangling
}

// free function does not dangle
void function_ref_free_function() {
    tl::function_ref<void(bar)> fr3 = foo;

    bar b;
    fr3(b);
}

// free function dangle
void function_ref_free_function_dangle() {
    tl::function_ref<void(bar)> fr3 = &foo;

    bar b;
    fr3(b);
}

// member function does not dangle
void function_member_function_dangle() {
    std::function<void(bar)> fr3 = &bar::baz;

    bar b;
    fr3(b);
}

// free function does not dangle
void function_free_function() {
    std::function<void(bar)> fr3 = foo;

    bar b;
    fr3(b);
}

// free function does not dangle
void function_free_function_dangle() {
    std::function<void(bar)> fr3 = &foo;

    bar b;
    fr3(b);
}

int main() {
    return 42;
}