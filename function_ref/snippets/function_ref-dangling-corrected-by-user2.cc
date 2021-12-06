//#include <https://raw.githubusercontent.com/TartanLlama/function_ref/master/include/tl/function_ref.hpp>
#include <https://raw.githubusercontent.com/descender76/cpp_proposals/main/function_ref/function_ref_prime.hpp>

struct bar {
    void baz(){}
};

void foo(bar) {
}

// member function immediately dangle
void function_ref_member_function_dangling() {
    tl::function_ref<void(bar)> fr3 = &bar::baz;

    bar b;
    fr3(b);  // UB, fr3 is already dangling
}

// free function immediately dangle
void function_ref_free_function_dangling() {
    tl::function_ref<void(bar)> fr3 = &foo;

    bar b;
    fr3(b);  // UB, fr3 is already dangling
}

// member function without dangle via new constructor
void function_ref_member_function_new() {
    tl::function_ref<void(bar)> fr3 = {nullptr, [](void*, bar b){b.baz();}};

    bar b;
    fr3(b);  // UB, fr3 is NOT already dangling
}

// member function without dangle via new constructor
void function_ref_free_function_new() {
    tl::function_ref<void(bar)> fr3 = {nullptr, [](void*, bar b){foo(b);}};

    bar b;
    fr3(b);  // UB, fr3 is NOT already dangling
}

// member function without dangle via stateless lambda
void function_ref_member_function_lambda() {
    tl::function_ref<void(bar)> fr3 = [](bar b){b.baz();};

    bar b;
    fr3(b);  // UB, fr3 is NOT already dangling
}

// free function without dangle via stateless lambda
void function_ref_free_function_lambda() {
    tl::function_ref<void(bar)> fr3 = [](bar b){foo(b);};

    bar b;
    fr3(b);  // UB, fr3 is NOT already dangling
}

int main() {
    return 42;
}