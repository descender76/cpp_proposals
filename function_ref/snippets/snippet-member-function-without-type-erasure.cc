#include <iostream>

using namespace std;

template <typename TSignature>
class function_ref_prime;

template <typename TReturn, typename... TArgs>
class function_ref_prime<TReturn(TArgs...)> final
{
private:
    using signature_type = TReturn(void*, TArgs...);

    void* _ptr;
    TReturn (*_erased_fn)(void*, TArgs...);

public:
    function_ref_prime(void* __ptr, TReturn (*__erased_fn)(void*, TArgs...)) noexcept : _ptr{__ptr}, _erased_fn{__erased_fn}
    {
    }

    decltype(auto) operator()(TArgs... xs) const
        noexcept(noexcept(_erased_fn(_ptr, std::forward<TArgs>(xs)...)))
    {
        return _erased_fn(_ptr, std::forward<TArgs>(xs)...);
    }
};

template <typename T, T> struct internal_this_as_pointer;

template <typename T, typename R, typename ...Args, R (T::*mf)(Args...)>
struct internal_this_as_pointer<R (T::*)(Args...), mf>
{
    static R type_erased_function(void * obj, T* first, Args ... args)
    {
        return (first->*mf)(std::forward<Args>(args)...);
    }
    static function_ref_prime<R(T*, Args...)> make_function_ref()
    {
        return function_ref_prime<R(T*, Args...)>(nullptr, type_erased_function);
    }
};

template <typename T, typename R, typename ...Args, R (T::*mf)(Args...) const>
struct internal_this_as_pointer<R (T::*)(Args...) const, mf>
{
    static R type_erased_function(void * obj, T* first, Args ... args)
    {
        return (first->*mf)(std::forward<Args>(args)...);
    }
    static function_ref_prime<R(T*, Args...)> make_function_ref()
    {
        return function_ref_prime<R(T*, Args...)>(nullptr, type_erased_function);
    }
};

template<auto MFP>
using this_as_pointer = internal_this_as_pointer<decltype(MFP), MFP>;

template <typename T, T> struct internal_this_as_ref;

template <typename T, typename R, typename ...Args, R (T::*mf)(Args...)>
struct internal_this_as_ref<R (T::*)(Args...), mf>
{
    static R type_erased_function(void * obj, T& first, Args ... args)
    {
        return (first.*mf)(std::forward<Args>(args)...);
    }
    static function_ref_prime<R(T&, Args...)> make_function_ref()
    {
        return function_ref_prime<R(T&, Args...)>(nullptr, type_erased_function);
    }
};

template <typename T, typename R, typename ...Args, R (T::*mf)(Args...) const>
struct internal_this_as_ref<R (T::*)(Args...) const, mf>
{
    static R type_erased_function(void * obj, T& first, Args ... args)
    {
        return (first.*mf)(std::forward<Args>(args)...);
    }
    static function_ref_prime<R(T&, Args...)> make_function_ref()
    {
        return function_ref_prime<R(T&, Args...)>(nullptr, type_erased_function);
    }
};

template<auto MFP>
using this_as_ref = internal_this_as_ref<decltype(MFP), MFP>;

template <typename T, T> struct internal_this_as_value;

template <typename T, typename R, typename ...Args, R (T::*mf)(Args...)>
struct internal_this_as_value<R (T::*)(Args...), mf>
{
    static R type_erased_function(void * obj, T first, Args ... args)
    {
        return (first.*mf)(std::forward<Args>(args)...);
    }
    static function_ref_prime<R(T, Args...)> make_function_ref()
    {
        return function_ref_prime<R(T, Args...)>(nullptr, type_erased_function);
    }
};

template <typename T, typename R, typename ...Args, R (T::*mf)(Args...) const>
struct internal_this_as_value<R (T::*)(Args...) const, mf>
{
    static R type_erased_function(void * obj, T first, Args ... args)
    {
        return (first.*mf)(std::forward<Args>(args)...);
    }
    static function_ref_prime<R(T, Args...)> make_function_ref()
    {
        return function_ref_prime<R(T, Args...)>(nullptr, type_erased_function);
    }
};

template<auto MFP>
using this_as_value = internal_this_as_value<decltype(MFP), MFP>;

/////////////////////////////////////////////////////////////////

template <typename T, T> struct internal_this_as_cpointer;

template <typename T, typename R, typename ...Args, R (T::*mf)(Args...)>
struct internal_this_as_cpointer<R (T::*)(Args...), mf>
{
    static R type_erased_function(void * obj, const T* first, Args ... args)
    {
        return (first->*mf)(std::forward<Args>(args)...);
    }
    static function_ref_prime<R(const T*, Args...)> make_function_ref()
    {
        return function_ref_prime<R(const T*, Args...)>(nullptr, type_erased_function);
    }
};

template <typename T, typename R, typename ...Args, R (T::*mf)(Args...) const>
struct internal_this_as_cpointer<R (T::*)(Args...) const, mf>
{
    static R type_erased_function(void * obj, const T* first, Args ... args)
    {
        return (first->*mf)(std::forward<Args>(args)...);
    }
    static function_ref_prime<R(const T*, Args...)> make_function_ref()
    {
        return function_ref_prime<R(const T*, Args...)>(nullptr, type_erased_function);
    }
};

template<auto MFP>
using this_as_cpointer = internal_this_as_cpointer<decltype(MFP), MFP>;

template <typename T, T> struct internal_this_as_cref;

template <typename T, typename R, typename ...Args, R (T::*mf)(Args...)>
struct internal_this_as_cref<R (T::*)(Args...), mf>
{
    static R type_erased_function(void * obj, const T& first, Args ... args)
    {
        return (first.*mf)(std::forward<Args>(args)...);
    }
    static function_ref_prime<R(const T&, Args...)> make_function_ref()
    {
        return function_ref_prime<R(const T&, Args...)>(nullptr, type_erased_function);
    }
};

template <typename T, typename R, typename ...Args, R (T::*mf)(Args...) const>
struct internal_this_as_cref<R (T::*)(Args...) const, mf>
{
    static R type_erased_function(void * obj, const T& first, Args ... args)
    {
        return (first.*mf)(std::forward<Args>(args)...);
    }
    static function_ref_prime<R(const T&, Args...)> make_function_ref()
    {
        return function_ref_prime<R(const T&, Args...)>(nullptr, type_erased_function);
    }
};

template<auto MFP>
using this_as_cref = internal_this_as_cref<decltype(MFP), MFP>;

template <typename T, T> struct internal_this_as_cvalue;

template <typename T, typename R, typename ...Args, R (T::*mf)(Args...)>
struct internal_this_as_cvalue<R (T::*)(Args...), mf>
{
    static R type_erased_function(void * obj, const T first, Args ... args)
    {
        return (first.*mf)(std::forward<Args>(args)...);
    }
    static function_ref_prime<R(const T, Args...)> make_function_ref()
    {
        return function_ref_prime<R(const T, Args...)>(nullptr, type_erased_function);
    }
};

template <typename T, typename R, typename ...Args, R (T::*mf)(Args...) const>
struct internal_this_as_cvalue<R (T::*)(Args...) const, mf>
{
    static R type_erased_function(void * obj, const T first, Args ... args)
    {
        return (first.*mf)(std::forward<Args>(args)...);
    }
    static function_ref_prime<R(const T, Args...)> make_function_ref()
    {
        return function_ref_prime<R(const T, Args...)>(nullptr, type_erased_function);
    }
};

template<auto MFP>
using this_as_cvalue = internal_this_as_cvalue<decltype(MFP), MFP>;

struct delegateTest
{
    int state = 0;
    long current(int i, long l, char c) const
    {
        return state + i + l;
    }
    long test(int i, long l, char c)
    {
        state += i;
        return state + l;
    }
    long operator()(int i, long l, char c) const
    {
        return state + i + l;
    }
    long operator()(int i, long l, char c)
    {
        state += i;
        return state + l;
    }
};

int main()
{
    delegateTest dt;
    function_ref_prime<long(delegateTest*, int, long, char)> frp = this_as_pointer<&delegateTest::test>::make_function_ref();
    std::cout << frp(&dt, 7800, 91, 'l') << std::endl;// 7891
    frp = this_as_pointer<&delegateTest::current>::make_function_ref();
    std::cout << frp(&dt, 7800, 91, 'l') << std::endl;// 15691
    function_ref_prime<long(delegateTest&, int, long, char)> frp1 = this_as_ref<&delegateTest::test>::make_function_ref();
    std::cout << frp1(dt, 7800, 91, 'l') << std::endl;// 15691
    frp1 = this_as_ref<&delegateTest::current>::make_function_ref();
    std::cout << frp1(dt, 7800, 91, 'l') << std::endl;// 23491
    function_ref_prime<long(delegateTest, int, long, char)> frp2 = this_as_value<&delegateTest::test>::make_function_ref();
    std::cout << frp2(dt, 7800, 91, 'l') << std::endl;// 23491
    frp2 = this_as_value<&delegateTest::current>::make_function_ref();
    std::cout << frp2(dt, 7800, 91, 'l') << std::endl;// 23491
    function_ref_prime<long(const delegateTest*, int, long, char)> frp3 = this_as_cpointer<&delegateTest::current>::make_function_ref();
    std::cout << frp3(&dt, 7800, 91, 'l') << std::endl;// 23491
    function_ref_prime<long(const delegateTest&, int, long, char)> frp4 = this_as_cref<&delegateTest::current>::make_function_ref();
    std::cout << frp4(dt, 7800, 91, 'l') << std::endl;// 23491
    function_ref_prime<long(const delegateTest, int, long, char)> frp5 = this_as_cvalue<&delegateTest::current>::make_function_ref();
    std::cout << frp5(dt, 7800, 91, 'l') << std::endl;// 23491
}
