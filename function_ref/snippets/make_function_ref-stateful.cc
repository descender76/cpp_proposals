//#include <https://raw.githubusercontent.com/TartanLlama/function_ref/master/include/tl/function_ref.hpp>
#include <https://raw.githubusercontent.com/descender76/cpp_proposals/main/function_ref/function_ref_prime.hpp>

struct bar {
    void baz(){}
    void operator()(){}
};

void foo(bar&) {
}

// side by side examples
void examples() {
    bar b;
    tl::function_ref<void()> fr1 = [&b](){b.baz();};
    tl::function_ref<void()> fr2 = [&b](){foo(b);};
    // using the new constructor
    tl::function_ref<void()> fr3 = {&b, [](void* obj){static_cast<bar*>(obj)->baz();}};
    tl::function_ref<void()> fr4 = {&b, [](void* obj){foo(*static_cast<bar*>(obj));}};
    // using make_function_ref which uses new constructor
    tl::function_ref<void()> fr5 = tl::make_function_ref<foo>(b);
    tl::function_ref<void()> fr6 = tl::make_function_ref<&foo>(b);
    tl::function_ref<void()> fr7 = tl::make_function_ref<&bar::baz>(b);
    tl::function_ref<void()> fr8 = tl::make_function_ref<&bar::operator()>(b);
    // C#
    // delegate void some_name();
    // some_name fr = foo;
    // some_name fr = b.baz;
    ///////////////////////////////
    // Borland C++, embarcadero __closure
    // void(__closure * fr)();
    // fr = foo
    // fr = b.baz

    fr1();
    fr2();
    fr3();
    fr4();
    fr5();
    fr6();
    fr7();
    fr8();
}

int main() {
    return 42;
}