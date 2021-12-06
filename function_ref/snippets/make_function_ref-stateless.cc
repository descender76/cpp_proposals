//#include <https://raw.githubusercontent.com/TartanLlama/function_ref/master/include/tl/function_ref.hpp>
#include <https://raw.githubusercontent.com/descender76/cpp_proposals/main/function_ref/function_ref_prime.hpp>

struct bar {
    void baz(){}
};

void foo(bar&) {
}

// side by side examples
void examples(tl::function_ref<void(bar&)> fr) {
    fr = {nullptr, [](void*, bar& b){b.baz();}};
    fr = {nullptr, [](void*, bar& b){foo(b);}};
    fr = [](bar& b){b.baz();};
    fr = [](bar& b){foo(b);};
    fr = tl::make_function_ref<foo>();
    fr = tl::make_function_ref<&foo>();
    fr = tl::make_function_ref<&bar::baz>();

    bar b;
    fr(b);
}

int main() {
    return 42;
}