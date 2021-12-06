//#include <https://raw.githubusercontent.com/TartanLlama/function_ref/master/include/tl/function_ref.hpp>
#include <https://raw.githubusercontent.com/descender76/cpp_proposals/main/function_ref/function_ref_prime.hpp>

struct bar {
    void baz(){}
};

void foo(bar&) {
}

// side by side examples
void examples(tl::function_ref<void()> fr) {
    bar b;
    fr = [&b](){b.baz();};
    fr = [&b](){foo(b);};
    // using the new constructor
    fr = {&b, [](void* obj){static_cast<bar*>(obj)->baz();}};
    fr = {&b, [](void* obj){foo(*static_cast<bar*>(obj));}};
    // using make_function_ref which uses new constructor
    fr = tl::make_function_ref<foo>(b);
    fr = tl::make_function_ref<&foo>(b);
    fr = tl::make_function_ref<&bar::baz>(b);
    // C#
    // delegate void some_name();
    // some_name fr = foo;
    // some_name fr = b.baz;
    ///////////////////////////////
    // Borland C++, embarcadero __closure
    // void(__closure * fr)();
    // fr = foo
    // fr = b.baz

    fr();
}

int main() {
    return 42;
}