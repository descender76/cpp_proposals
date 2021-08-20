// used to bind to member functions with type erasure
// this is essential for callbacks that can point to anything like C function pointers
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