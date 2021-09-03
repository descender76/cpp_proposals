#include <tl/function_ref.hpp>
// with the following public constructor added
/*
  function_ref(void* obj_, R (*callback_)(void*,Args...)) noexcept : obj_{obj_}, callback_{callback_}
  {
  }
*/

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
    tl::function_ref<void(bool, int, float)> fr4{&bref, [](void* v, bool b, int i, float f){type_erased_foo(*static_cast<bar*>(v), b, i, f);}};
    // member function with type erased state
    tl::function_ref<void(bool, int, float)> fr5{&bref, [](void* v, bool b, int i, float f){static_cast<bar*>(v)->baz(b, i, f);}};
}