#include <functional>

struct bar {
    void baz(bool b, int i, float f) noexcept(false)
    {
    }
    void baz(bool b, int i, float f) const noexcept(false)
    {
    }
    void buz(bool b, int i, float f) noexcept(true)
    {
    }
    void buz(bool b, int i, float f) const noexcept(true)
    {
    }
};

void free_bar_baz(bar&, bool b, int i, float f) noexcept(false)
{
}

void free_bar_baz(const bar&, bool b, int i, float f) noexcept(false)
{
}

void free_bar_buz(bar&, bool b, int i, float f) noexcept(true)
{
}

void free_bar_buz(const bar&, bool b, int i, float f) noexcept(true)
{
}

template <typename T, T> struct internal_member_function_traits;

template <typename T, typename R, typename ...Args, R (T::*mf)(Args...)>
struct internal_member_function_traits<R (T::*)(Args...), mf>
{
	typedef R (this_as_ref_function_signature)(T&, Args ... args);
    static R this_as_ref(T& first, Args ... args)
    {
        return (first.*mf)(std::forward<Args>(args)...);
    }
};

template <typename T, typename R, typename ...Args, R (T::*mf)(Args...) const>
struct internal_member_function_traits<R (T::*)(Args...) const, mf>
{
	typedef R (this_as_ref_function_signature)(const T&, Args ... args);
    static R this_as_ref(const T& first, Args ... args)
    {
        return (first.*mf)(std::forward<Args>(args)...);
    }
};

template <typename T, typename R, typename ...Args, R (T::*mf)(Args...) noexcept>
struct internal_member_function_traits<R (T::*)(Args...) noexcept, mf>
{
	typedef R (this_as_ref_function_signature)(T&, Args ... args) noexcept;
    static R this_as_ref(T& first, Args ... args) noexcept
    {
        return (first.*mf)(std::forward<Args>(args)...);
    }
};

template <typename T, typename R, typename ...Args, R (T::*mf)(Args...) const noexcept>
struct internal_member_function_traits<R (T::*)(Args...) const noexcept, mf>
{
	typedef R (this_as_ref_function_signature)(const T&, Args ... args) noexcept;
    static R this_as_ref(const T& first, Args ... args) noexcept
    {
        return (first.*mf)(std::forward<Args>(args)...);
    }
};

template<auto MFP>
using member_function_traits = internal_member_function_traits<decltype(MFP), MFP>;

int main()
{
    bar b;
    // currently allowed initialization of member function pointer
    void (bar::*mfp1)(bool, int, float) noexcept(false) = static_cast<void (bar::*)(bool, int, float) noexcept(false)>(&bar::baz);
    void (bar::*mfp2)(bool, int, float) const noexcept(false) = static_cast<void (bar::*)(bool, int, float) const noexcept(false)>(&bar::baz);
    void (bar::*mfp3)(bool, int, float) noexcept(true) = static_cast<void(bar::*)(bool b, int i, float f) noexcept(true)>(&bar::buz);
    void (bar::*mfp4)(bool, int, float) const noexcept(true) = static_cast<void(bar::*)(bool b, int i, float f) const noexcept(true)>(&bar::buz);
    // currently allowed initialization of function pointer
    void (*fp1)(bar&, bool, int, float) noexcept(false) = static_cast<void (*)(bar&, bool, int, float) noexcept(false)>(free_bar_baz);
    void (*fp2)(const bar&, bool, int, float) noexcept(false) = static_cast<void (*)(const bar&, bool, int, float) noexcept(false)>(free_bar_baz);
    void (*fp3)(bar&, bool, int, float) noexcept(true) = static_cast<void(*)(bar&, bool b, int i, float f) noexcept(true)>(free_bar_buz);
    void (*fp4)(const bar&, bool, int, float) noexcept(true) = static_cast<void(*)(const bar&, bool b, int i, float f) noexcept(true)>(free_bar_buz);
    // currently disallowed but DESIRED initialization of function pointer via member function pointer initializer
    // example error: address of overloaded function 'baz' cannot be static_cast to type 'void (*)(bar &, bool, int, float) noexcept(false)'
    //void (*fp5)(bar&, bool, int, float) noexcept(false) = static_cast<void (*)(bar&, bool, int, float) noexcept(false)>(&bar::baz);
    //void (*fp6)(const bar&, bool, int, float) noexcept(false) = static_cast<void (*)(const bar&, bool, int, float) noexcept(false)>(&bar::baz);
    //void (*fp7)(bar&, bool, int, float) noexcept(true) = static_cast<void(*)(bar&, bool b, int i, float f) noexcept(true)>(&bar::buz);
    //void (*fp8)(const bar&, bool, int, float) noexcept(true) = static_cast<void(*)(const bar&, bool b, int i, float f) noexcept(true)>(&bar::buz);
    // just be clear this is NOT DESIRED, NOT BEING ASKED FOR, because it doesn't make sense
    // and seems to be impossible since
    // member function pointer is dependent upon an instance in order to lookup the function
    // example error: cannot cast from type 'void (bar::*)(bool, int, float) noexcept(false)' to pointer type 'void (*)(bar &, bool, int, float) noexcept(false)'
    //void (*fp9)(bar&, bool, int, float) noexcept(false) = static_cast<void (*)(bar&, bool, int, float) noexcept(false)>(mfp1);
    //void (*fp10)(const bar&, bool, int, float) noexcept(false) = static_cast<void (*)(const bar&, bool, int, float) noexcept(false)>(mfp2);
    //void (*fp11)(bar&, bool, int, float) noexcept(true) = static_cast<void(*)(bar&, bool b, int i, float f) noexcept(true)>(mfp3);
    //void (*fp12)(const bar&, bool, int, float) noexcept(true) = static_cast<void(*)(const bar&, bool b, int i, float f) noexcept(true)>(mfp4);
    // binding via templates
    void (*fp5)(bar&, bool, int, float) noexcept(false) = member_function_traits<static_cast<void (bar::*)(bool, int, float) noexcept(false)>(&bar::baz)>::this_as_ref;
    void (*fp6)(const bar&, bool, int, float) noexcept(false) = member_function_traits<static_cast<void (bar::*)(bool, int, float) const noexcept(false)>(&bar::baz)>::this_as_ref;
    void (*fp7)(bar&, bool, int, float) noexcept(true) = member_function_traits<static_cast<void(bar::*)(bool b, int i, float f) noexcept(true)>(&bar::buz)>::this_as_ref;
    void (*fp8)(const bar&, bool, int, float) noexcept(true) = member_function_traits<static_cast<void(bar::*)(bool b, int i, float f) const noexcept(true)>(&bar::buz)>::this_as_ref;
}