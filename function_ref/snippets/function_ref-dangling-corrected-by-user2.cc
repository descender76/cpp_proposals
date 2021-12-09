//#include <https://raw.githubusercontent.com/TartanLlama/function_ref/master/include/tl/function_ref.hpp>
#include <https://raw.githubusercontent.com/descender76/cpp_proposals/main/function_ref/function_ref_prime.hpp>

struct bar {
    void baz(){}
};

void foo(bar) {
}

int main() {
    tl::function_ref<void(bar)> fr1 = &bar::baz;// immediately dangle
    tl::function_ref<void(bar)> fr2 = &foo;// immediately dangle
    // no dangling, this proposal via new constructor
    tl::function_ref<void(bar)> fr3 = {nullptr, [](void*, bar b){b.baz();}};
    tl::function_ref<void(bar)> fr4 = {nullptr, [](void*, bar b){foo(b);}};
    // no dangling, original proposal via stateless lambda
    tl::function_ref<void(bar)> fr5 = [](bar b){b.baz();};
    tl::function_ref<void(bar)> fr6 = [](bar b){foo(b);};

    bar b;

    fr1(b);
    fr2(b);
    fr3(b);
    fr4(b);
    fr5(b);
    fr6(b);

    return 42;
}