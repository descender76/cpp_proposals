#include <iostream>
//#include <https://raw.githubusercontent.com/TartanLlama/function_ref/master/include/tl/function_ref.hpp>
#include <https://raw.githubusercontent.com/descender76/cpp_proposals/main/function_ref/function_ref_prime.hpp>

struct bar {
    void baz(bool b, int i, float f)
    {
        std::cout << "bar::baz" << std::endl;
    }

    void baznoe(bool b, int i, float f) noexcept
    {
        std::cout << "bar::baznoe" << std::endl;
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

int main()
{
    // member function without type erased state
    application_function_ref(&bar::baz);
    application_function_pointer(&bar::baz);
    application_function_value(&bar::baz);
    application_function_ref(&bar::baznoe);
    application_function_pointer(&bar::baznoe);
    application_function_value(&bar::baznoe);
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
}
