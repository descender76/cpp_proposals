// used to bind to functions with type erasure
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

template <typename R, typename FirstArg, typename ...Args, R (*f)(FirstArg*, Args...)>
struct internal_type_erase_first<R (*)(FirstArg* fa, Args...), f>
{
    static R type_erased_function(void * obj, Args ... args)
    {
        return f(static_cast<FirstArg*>(obj), std::forward<Args>(args)...);
    }
    static function_ref_prime<R(Args...)> make_function_ref(FirstArg* firstArg)
    {
        return function_ref_prime<R(Args...)>(firstArg, type_erased_function);
    }
};

template <typename R, typename FirstArg, typename ...Args, R (*f)(const FirstArg*, Args...)>
struct internal_type_erase_first<R (*)(const FirstArg* fa, Args...), f>
{
    static R type_erased_function(void * obj, Args ... args)
    {
        return f(static_cast<FirstArg*>(obj), std::forward<Args>(args)...);
    }
    static function_ref_prime<R(Args...)> make_function_ref(const FirstArg* firstArg)
    {
        return function_ref_prime<R(Args...)>(const_cast<FirstArg*>(firstArg), type_erased_function);
    }
};

template<auto FP>
using type_erase_first = internal_type_erase_first<decltype(FP), FP>;