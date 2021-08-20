// used to bind to functions and [stateless] lambdas without type erasure
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