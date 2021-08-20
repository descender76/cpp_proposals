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

template <typename TSignature> struct call_operator;

template <typename TReturn, typename... TArgs>
struct call_operator<TReturn(TArgs...)>
{
    typedef TReturn (*signature_type)(void*, TArgs...);

    template<class T>
    static function_ref_prime<TReturn(TArgs...)> make_function_ref(T& obj)
    {
        signature_type tef = [](void * obj, TArgs ... args){ return (*(static_cast<T*>(obj)))(std::forward<TArgs>(args)...); };
        return function_ref_prime<TReturn(TArgs...)>(&obj, tef);
    }
    template<class T>
    static function_ref_prime<TReturn(TArgs...)> make_function_ref(const T& obj)
    {
        signature_type tef = [](void * obj, TArgs ... args){ return (*(static_cast<const T*>(obj)))(std::forward<TArgs>(args)...); };
        return function_ref_prime<TReturn(TArgs...)>(&const_cast<T&>(obj), tef);
    }
};

struct delegateTest0
{
    long operator()(int i, long l, char c) const
    {
        return 0;
    }
    long operator()(int i, long l, char c)
    {
        return 0;
    }
};

struct delegateTest
{
    int state = 0;
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
    delegateTest0 dt0;
    delegateTest dt;
    function_ref_prime<long(int, long, char)> frp = call_operator<long(int, long, char)>::make_function_ref(dt);
    std::cout << frp(7800, 91, 'l') << std::endl;// 7891
    frp = call_operator<long(int, long, char)>::make_function_ref(dt);
    std::cout << frp(7800, 91, 'l') << std::endl;// 15691
    frp = call_operator<long(int, long, char)>::make_function_ref(dt0);
    std::cout << frp(7800, 91, 'l') << std::endl;// 0
    auto l = [&dt, &dt0](int i, long l, char c){
        return dt(i, l, c) + dt0(i, l, c);
    };
    frp = call_operator<long(int, long, char)>::make_function_ref(l);
    std::cout << frp(7800, 91, 'l') << std::endl;// 23491
}