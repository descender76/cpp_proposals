//#include <https://raw.githubusercontent.com/TartanLlama/function_ref/master/include/tl/function_ref.hpp>
#include <https://raw.githubusercontent.com/descender76/cpp_proposals/main/function_ref/function_ref_prime.hpp>

struct bar {
    void baz(){}
};

void foo(bar) {
}

int main() {
    bar b;
    tl::function_ref<void(bar)> fr1 = &bar::baz;// immediately dangle
    tl::function_ref<void(bar)> fr2 = foo;// does not dangle
    tl::function_ref<void(bar)> fr3 = &foo;// immediately dangle
    // the following behaves as expected
    std::function<void(bar)> fr4 = &bar::baz;
    std::function<void(bar)> fr5 = foo;
    std::function<void(bar)> fr6 = &foo;
    fr1(b);
    fr2(b);
    fr3(b);
    fr4(b);
    fr5(b);
    fr6(b);
    return 42;
}