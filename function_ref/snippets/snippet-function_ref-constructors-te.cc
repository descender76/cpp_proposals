#include <tl/function_ref.hpp>

void foo(bool b, int i, float f)
{
}

struct bar {
    void baz(bool b, int i, float f)
    {
    }
};

void type_erased_foo(bar& type_erased_bar, bool b, int i, float f)
{
}

int main()
{
    tl::function_ref<void(bool, int, float)> fr1 = [](bool, int, float){};
    tl::function_ref<void(bool, int, float)> fr2 = foo;
    tl::function_ref<void(bar, bool, int, float)> fr3 = &bar::baz;
    // state to be type erased
    bar bref;
    // free function with type erased state
    auto fsl1 = [&bref](bool b, int i, float f){type_erased_foo(bref, b, i, f);};//unnecessary stateful object creation
    tl::function_ref<void(bool, int, float)> fr4 = fsl1;
    // member function with type erased state
    auto fsl2 = [&bref](bool b, int i, float f){bref.baz(b, i, f);};//unnecessary stateful object creation
    tl::function_ref<void(bool, int, float)> fr5 = fsl2;
}