#include <iostream>
//#include <https://raw.githubusercontent.com/TartanLlama/function_ref/master/include/tl/function_ref.hpp>
#include <https://raw.githubusercontent.com/descender76/cpp_proposals/main/function_ref/function_ref_prime.hpp>

void third_party_lib_function1(tl::function_ref<void(bool, int, float)> callback) {
    callback(true, 11, 3.1459f);
}

void third_party_lib_function2(tl::function_ref<void(bool, int, float) noexcept> callback) {
    callback(true, 11, 3.1459f);
}

struct bar {
};

void free_bar_baz1(bar&, bool b, int i, float f)
{
    std::cout << "free_bar_baz1" << std::endl;
}

void free_bar_baz2(const bar&, bool b, int i, float f)
{
    std::cout << "free_bar_baz2" << std::endl;
}

void free_bar_baz3(bar*, bool b, int i, float f)
{
    std::cout << "free_bar_baz3" << std::endl;
}

void free_bar_baz4(const bar*, bool b, int i, float f)
{
    std::cout << "free_bar_baz4" << std::endl;
}

void free_bar_baz5(bar&, bool b, int i, float f) noexcept
{
    std::cout << "free_bar_baz5" << std::endl;
}

void free_bar_baz6(const bar&, bool b, int i, float f) noexcept
{
    std::cout << "free_bar_baz6" << std::endl;
}

void free_bar_baz7(bar*, bool b, int i, float f) noexcept
{
    std::cout << "free_bar_baz7" << std::endl;
}

void free_bar_baz8(const bar*, bool b, int i, float f) noexcept
{
    std::cout << "free_bar_baz8" << std::endl;
}

int main()
{
    bar b;
    // function_ref: a non-owning reference to a Callable
    third_party_lib_function1([&b](auto... args){free_bar_baz1(b, args...);});
    third_party_lib_function1([&b](auto... args){free_bar_baz2(b, args...);});
    third_party_lib_function1([&b](auto... args){free_bar_baz3(&b, args...);});
    third_party_lib_function1([&b](auto... args){free_bar_baz4(&b, args...);});
    third_party_lib_function2([&b](auto... args){free_bar_baz5(b, args...);});
    third_party_lib_function2([&b](auto... args){free_bar_baz6(b, args...);});
    third_party_lib_function2([&b](auto... args){free_bar_baz7(&b, args...);});
    third_party_lib_function2([&b](auto... args){free_bar_baz8(&b, args...);});
	// make_function_ref: A More Functional function_ref
    // function with type erasure usecase
    // i.e. C callback with user data
    third_party_lib_function1(tl::make_function_ref<free_bar_baz1>(b));
    third_party_lib_function1(tl::make_function_ref<free_bar_baz2>(b));
    third_party_lib_function1(tl::make_function_ref<free_bar_baz3>(b));
    third_party_lib_function1(tl::make_function_ref<free_bar_baz4>(b));
    third_party_lib_function2(tl::make_function_ref<free_bar_baz5>(b));
    third_party_lib_function2(tl::make_function_ref<free_bar_baz6>(b));
    third_party_lib_function2(tl::make_function_ref<free_bar_baz7>(b));
    third_party_lib_function2(tl::make_function_ref<free_bar_baz8>(b));
}
