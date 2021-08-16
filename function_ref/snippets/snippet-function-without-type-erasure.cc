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

template <typename T, T> struct internal_prepend_void_pointer;

template <typename R, typename ...Args, R (*f)(Args...)>
struct internal_prepend_void_pointer<R (*)(Args...), f>
{
    static R type_erased_function(void * obj, Args ... args)
    {
        return f(std::forward<Args>(args)...);
    }
    static function_ref_prime<R(Args...)> make_function_ref()
    {
        return function_ref_prime<R(Args...)>(nullptr, type_erased_function);
    }
};

template<auto FP>
using prepend_void_pointer = internal_prepend_void_pointer<decltype(FP), FP>;

long one(int i, long l, char c)
{
    return 1;
}

int main()
{
    function_ref_prime<long(int, long, char)> frp = prepend_void_pointer<&one>::make_function_ref();
    std::cout << frp(7800, 91, 'l') << std::endl;// 1
}
