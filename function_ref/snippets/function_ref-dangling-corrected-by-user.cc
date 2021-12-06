//#include <https://raw.githubusercontent.com/TartanLlama/function_ref/master/include/tl/function_ref.hpp>
#include <https://raw.githubusercontent.com/descender76/cpp_proposals/main/function_ref/function_ref_prime.hpp>

struct bar {
    void baz(){}
};

void foo(bar) {
}

// member function immediately dangle WORKAROUND
void function_ref_member_function_workaround() {
    auto temp = &bar::baz;// WORKAROUND
    tl::function_ref<void(bar)> fr3 = temp;

    bar b;
    fr3(b);  // UB, fr3 is NOT already dangling
}

// free function dangle WORKAROUND
void function_ref_free_function_workaround() {
    auto temp = &foo;// WORKAROUND
    tl::function_ref<void(bar)> fr3 = temp;

    bar b;
    fr3(b);
}

void correct_usage_of_current_function_ref_some_work(tl::function_ref<void(bar)> fr1, tl::function_ref<void(bar)> fr2) {
    bar b;
    fr1(b);
    fr2(b);    
}

void correct_usage_of_current_function_ref() {
    correct_usage_of_current_function_ref_some_work(&foo, &bar::baz);
}

int main() {
    return 42;
}