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

template <typename T, T> struct internal_type_erase_first;

template <typename R, typename FirstArg, typename ...Args, R (*f)(FirstArg&, Args...)>
struct internal_type_erase_first<R (*)(FirstArg& fa, Args...), f>
{
    static R type_erased_function(void * obj, Args ... args)
    {
        return f(*static_cast<FirstArg*>(obj), std::forward<Args>(args)...);
    }
    static function_ref_prime<R(Args...)> make_function_ref(FirstArg& firstArg)
    {
        return function_ref_prime<R(Args...)>(&firstArg, type_erased_function);
    }
};

template <typename R, typename FirstArg, typename ...Args, R (*f)(const FirstArg&, Args...)>
struct internal_type_erase_first<R (*)(const FirstArg& fa, Args...), f>
{
    static R type_erased_function(void * obj, Args ... args)
    {
        return f(*static_cast<FirstArg*>(obj), std::forward<Args>(args)...);
    }
    static function_ref_prime<R(Args...)> make_function_ref(const FirstArg& firstArg)
    {
        return function_ref_prime<R(Args...)>(&const_cast<FirstArg&>(firstArg), type_erased_function);
    }
};

template<auto FP>
using type_erase_first = internal_type_erase_first<decltype(FP), FP>;

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

long tef(delegateTest& dt, int i, long l, char c)
{
    return dt.test(i, l, c);
}

long tef(const delegateTest& dt, int i, long l, char c)
{
    return dt.current(i, l, c);
}

long unique_name(delegateTest& dt, int i, long l, char c)
{
    return dt.test(i, l, c);
}

long unique_name_const(const delegateTest& dt, int i, long l, char c)
{
    return dt.current(i, l, c);
}

int main()
{
    delegateTest dt;
    // explicit cast because function name is overloaded
    function_ref_prime<long(int, long, char)> frp = type_erase_first<static_cast<long(*)(delegateTest& dt, int i, long l, char c)>(tef)>::make_function_ref(dt);
    std::cout << frp(7800, 91, 'l') << std::endl;// 7891
    // explicit cast because function name is overloaded
    frp = type_erase_first<static_cast<long(*)(const delegateTest& dt, int i, long l, char c)>(tef)>::make_function_ref(dt);
    std::cout << frp(7800, 91, 'l') << std::endl;// 15691
    frp = type_erase_first<unique_name>::make_function_ref(dt);
    std::cout << frp(7800, 91, 'l') << std::endl;// 15691
    frp = type_erase_first<unique_name_const>::make_function_ref(dt);
    std::cout << frp(7800, 91, 'l') << std::endl;// 23491
}