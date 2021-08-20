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

template <typename T, T> struct internal_member_function_pointer;

template <typename T, typename R, typename ...Args, R (T::*mf)(Args...)>
struct internal_member_function_pointer<R (T::*)(Args...), mf>
{
    static R type_erased_function(void * obj, Args ... args)
    {
        return (static_cast<T*>(obj)->*mf)(std::forward<Args>(args)...);
    }
    static function_ref_prime<R(Args...)> make_function_ref(T& obj)
    {
        return function_ref_prime<R(Args...)>(&obj, type_erased_function);
    }
};

template <typename T, typename R, typename ...Args, R (T::*mf)(Args...) const>
struct internal_member_function_pointer<R (T::*)(Args...) const, mf>
{
    static R type_erased_function(void * obj, Args ... args)
    {
        return (static_cast<const T*>(obj)->*mf)(std::forward<Args>(args)...);
    }
    static function_ref_prime<R(Args...)> make_function_ref(const T& obj)
    {
        return function_ref_prime<R(Args...)>(&const_cast<T&>(obj), type_erased_function);
    }
};

template<auto MFP>
using member_function_pointer = internal_member_function_pointer<decltype(MFP), MFP>;

struct delegateTest0
{
    long current(int i, long l, char c) const
    {
        return 0;
    }
    long test(int i, long l, char c)
    {
        return 0;
    }
};

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
};

int main()
{
    delegateTest0 dt0;
    delegateTest dt;
    function_ref_prime<long(int, long, char)> frp = member_function_pointer<&delegateTest::test>::make_function_ref(dt);
    std::cout << frp(7800, 91, 'l') << std::endl;// 7891
    frp = member_function_pointer<&delegateTest::current>::make_function_ref(dt);
    std::cout << frp(7800, 91, 'l') << std::endl;// 15691
    frp = member_function_pointer<&delegateTest0::current>::make_function_ref(dt0);
    std::cout << frp(7800, 91, 'l') << std::endl;// 0
}
