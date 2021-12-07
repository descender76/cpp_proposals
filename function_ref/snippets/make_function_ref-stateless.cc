//#include <https://raw.githubusercontent.com/TartanLlama/function_ref/master/include/tl/function_ref.hpp>
#include <https://raw.githubusercontent.com/descender76/cpp_proposals/main/function_ref/function_ref_prime.hpp>

struct bar {
    void baz(){}
};

void foo(bar&) {
}

// side by side examples
void examples() {
    tl::function_ref<void(bar&)> fr1 = {nullptr, [](void*, bar& b){b.baz();}};
    tl::function_ref<void(bar&)> fr2 = {nullptr, [](void*, bar& b){foo(b);}};
    tl::function_ref<void(bar&)> fr3 = [](bar& b){b.baz();};
    tl::function_ref<void(bar&)> fr4 = [](bar& b){foo(b);};
    tl::function_ref<void(bar&)> fr5 = tl::make_function_ref<foo>();
    tl::function_ref<void(bar&)> fr6 = tl::make_function_ref<&foo>();
    tl::function_ref<void(bar&)> fr7 = tl::make_function_ref<&bar::baz>();

    bar b;
    fr1(b);
    fr2(b);
    fr3(b);
    fr4(b);
    fr5(b);
    fr6(b);
    fr7(b);
}

int main() {
    return 42;
}