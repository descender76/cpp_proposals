//#include <https://raw.githubusercontent.com/TartanLlama/function_ref/master/include/tl/function_ref.hpp>
#include <https://raw.githubusercontent.com/descender76/cpp_proposals/main/function_ref/function_ref_prime.hpp>

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

void third_party_lib_function1(tl::function_ref<void(bool, int, float)> callback) {
    callback(true, 11, 3.1459f);
}

void third_party_lib_function2(tl::function_ref<void(bar, bool, int, float)> callback) {
    bar b;
    callback(b, true, 11, 3.1459f);
}

void third_party_lib_function3(bar b, tl::function_ref<void(bar, bool, int, float)> callback) {
    callback(b, true, 11, 3.1459f);
}

int main()
{
    third_party_lib_function1([](bool, int, float){});
    third_party_lib_function1(foo);
    auto temp = &bar::baz;// needed to prevent immediate dangling
    third_party_lib_function2(temp);
    bar b;
    third_party_lib_function3(b, temp);
    // free function with type erased state: b
    //auto fsl1 = [&b](bool b1, int i, float f){type_erased_foo(b, b1, i, f);};//unnecessary stateful object creation
    //third_party_lib_function1(fsl1);
    third_party_lib_function1([&b](bool b1, int i, float f){type_erased_foo(b, b1, i, f);});// surprise
    // member function with type erased state: b
    //auto fsl2 = [&b](bool b1, int i, float f){b.baz(b1, i, f);};//unnecessary stateful object creation
    //third_party_lib_function1(fsl2);
    third_party_lib_function1([&b](bool b1, int i, float f){b.baz(b1, i, f);});// surprise
    // free function with type erased state
    third_party_lib_function1({&b, [](void* v, bool b, int i, float f){type_erased_foo(*static_cast<bar*>(v), b, i, f);}});
    // member function with type erased state
    third_party_lib_function1({&b, [](void* v, bool b, int i, float f){static_cast<bar*>(v)->baz(b, i, f);}});
}
