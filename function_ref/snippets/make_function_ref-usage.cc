#include <iostream>
//#include <https://raw.githubusercontent.com/TartanLlama/function_ref/master/include/tl/function_ref.hpp>
#include <https://raw.githubusercontent.com/descender76/cpp_proposals/main/function_ref/function_ref_prime.hpp>

void third_party_lib_function1(tl::function_ref<void(bool, int, float)> callback) {
    callback(true, 11, 3.1459f);
}

double third_party_lib_function2(tl::function_ref<double(int, float, bool)> callback) {
    return callback(11, 3.1459f, true);
}

void third_party_lib_function3(tl::function_ref<void(bool, int, float) noexcept> callback) {
    callback(true, 11, 3.1459f);
}

double third_party_lib_function4(tl::function_ref<double(int, float, bool) noexcept> callback) {
    return callback(11, 3.1459f, true);
}

struct bar {
    void baz(bool b, int i, float f)
    {
        std::cout << "bar::baz" << std::endl;
    }
    double buz(int i, float f, bool b)
    {
        std::cout << "bar::buz" << std::endl;
        return i + f + (b ? 1 : 0);
    }
    void baznoe(bool b, int i, float f) noexcept
    {
        std::cout << "bar::baznoe" << std::endl;
    }
    double buznoe(int i, float f, bool b) noexcept
    {
        std::cout << "bar::buznoe" << std::endl;
        return i + f + (b ? 1 : 0);
    }
    void caz(bool b, int i, float f)
    {
        std::cout << "bar::caz" << std::endl;
    }
    void caz(bool b, int i, float f) const
    {
        std::cout << "bar::caz const" << std::endl;
    }
};

void application_function_ref(tl::function_ref<void(bar&, bool, int, float)> callback) {
    bar b;
    callback(b, true, 11, 3.1459f);
}

void application_function_pointer(tl::function_ref<void(bar*, bool, int, float)> callback) {
    bar b;
    callback(&b, true, 11, 3.1459f);
}

void application_function_value(tl::function_ref<void(bar, bool, int, float)> callback) {
    bar b;
    callback(b, true, 11, 3.1459f);
}

void free_baz1(bool b, int i, float f)
{
    std::cout << "free_baz1" << std::endl;
}

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

void free_baz2(bool b, int i, float f) noexcept
{
    std::cout << "free_baz2" << std::endl;
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
    // member function with type erased state: b
    third_party_lib_function1([&b](bool b1, int i, float f){b.baz(b1, i, f);});
    third_party_lib_function1([&b](auto... args){b.baz(args...);});
    std::cout << third_party_lib_function2([&b](int i, float f, bool b1){return b.buz(i, f, b1);}) << std::endl;
    std::cout << third_party_lib_function2([&b](auto... args){return b.buz(args...);}) << std::endl;
    // member function with type erasure usecase
    // i.e. delegate/closure/OOP callback/event
    third_party_lib_function1(tl::make_function_ref<&bar::baz>(b));
    std::cout << third_party_lib_function2(tl::make_function_ref<&bar::buz>(b)) << std::endl;
    third_party_lib_function3(tl::make_function_ref<&bar::baznoe>(b));
    std::cout << third_party_lib_function4(tl::make_function_ref<&bar::buznoe>(b)) << std::endl;
    // member function without type erasure usecases
    // i.e. unified function pointer
    application_function_ref(tl::make_function_ref<&bar::baz>());
    application_function_ref(tl::make_function_ref<&bar::baz, tl::ref>());
    application_function_pointer(tl::make_function_ref<&bar::baz, tl::pointer>());
    application_function_value(tl::make_function_ref<&bar::baz, tl::value>());
    application_function_ref(tl::make_function_ref<&bar::baznoe>());
    application_function_ref(tl::make_function_ref<&bar::baznoe, tl::ref>());
    application_function_pointer(tl::make_function_ref<&bar::baznoe, tl::pointer>());
    application_function_value(tl::make_function_ref<&bar::baznoe, tl::value>());
    // function without type erasure usecase
    // i.e. C callback without user data
    third_party_lib_function1(tl::make_function_ref<free_baz1>());
    third_party_lib_function1(tl::make_function_ref<static_cast<void(*)(bool b, int i, float f)>(free_baz1)>());// explicit
    third_party_lib_function1(tl::make_function_ref<free_baz2>());
    third_party_lib_function1(tl::make_function_ref<static_cast<void(*)(bool b, int i, float f)>(free_baz2)>());// explicit
    // function with type erasure usecase
    // i.e. C callback with user data
    third_party_lib_function1(tl::make_function_ref<free_bar_baz1>(b));
    third_party_lib_function1(tl::make_function_ref<free_bar_baz2>(b));
    third_party_lib_function1(tl::make_function_ref<free_bar_baz3>(b));
    third_party_lib_function1(tl::make_function_ref<free_bar_baz4>(b));
    third_party_lib_function3(tl::make_function_ref<free_bar_baz5>(b));
    third_party_lib_function1(tl::make_function_ref<free_bar_baz6>(b));
    third_party_lib_function1(tl::make_function_ref<free_bar_baz7>(b));
    third_party_lib_function1(tl::make_function_ref<free_bar_baz8>(b));
}
