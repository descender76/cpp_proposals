//#include <https://raw.githubusercontent.com/TartanLlama/function_ref/master/include/tl/function_ref.hpp>
#include <https://raw.githubusercontent.com/descender76/cpp_proposals/main/function_ref/function_ref_prime.hpp>

struct bar {
    void baz(){}
};

void foo(bar) {
}

void correct_usage_of_current_function_ref(tl::function_ref<void(bar)> fr1, tl::function_ref<void(bar)> fr2) {
    bar b;
    fr1(b);
    fr2(b);    
}

int main() {
    auto temp = &bar::baz;// WORKAROUND
    tl::function_ref<void(bar)> fr1 = temp;
    auto temp = &foo;// WORKAROUND
    tl::function_ref<void(bar)> fr2 = temp;
    bar b;
    fr1(b);
    fr2(b);    

    correct_usage_of_current_function_ref(&foo, &bar::baz);
    return 42;
}