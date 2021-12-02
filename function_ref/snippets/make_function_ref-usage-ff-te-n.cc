#include <iostream>
//#include <https://raw.githubusercontent.com/TartanLlama/function_ref/master/include/tl/function_ref.hpp>
#include <https://raw.githubusercontent.com/descender76/cpp_proposals/main/function_ref/function_ref_prime.hpp>

void third_party_lib_function1(tl::function_ref<void(bool, int, float)> callback) {
    callback(true, 11, 3.1459f);
}

void free_baz1(bool b, int i, float f)
{
    std::cout << "free_baz1" << std::endl;
}

void free_baz2(bool b, int i, float f) noexcept
{
    std::cout << "free_baz2" << std::endl;
}

int main()
{
    //bar b;
    // function without type erased state: b
    third_party_lib_function1(free_baz1);
    third_party_lib_function1(static_cast<void(*)(bool b, int i, float f)>(free_baz1));// explicit
    third_party_lib_function1(free_baz2);
    third_party_lib_function1(static_cast<void(*)(bool b, int i, float f)>(free_baz2));// explicit
    // function without type erasure usecase
    // i.e. C callback without user data
    third_party_lib_function1(tl::make_function_ref<free_baz1>());
    third_party_lib_function1(tl::make_function_ref<static_cast<void(*)(bool b, int i, float f)>(free_baz1)>());// explicit
    third_party_lib_function1(tl::make_function_ref<free_baz2>());
    third_party_lib_function1(tl::make_function_ref<static_cast<void(*)(bool b, int i, float f)>(free_baz2)>());// explicit
}
