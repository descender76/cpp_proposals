template <typename T, T> struct internal_member_function_traits;

template <typename T, typename R, typename ...Args, R (T::*mf)(Args...)>
struct internal_member_function_traits<R (T::*)(Args...), mf>
{
    static R type_erase_this(void * obj, Args ... args)
    {
        return (static_cast<T*>(obj)->*mf)(std::forward<Args>(args)...);
    }
};

template <typename T, typename R, typename ...Args, R (T::*mf)(Args...) const>
struct internal_member_function_traits<R (T::*)(Args...) const, mf>
{
    static R type_erase_this(void * obj, Args ... args)
    {
        return (static_cast<const T*>(obj)->*mf)(std::forward<Args>(args)...);
    }
};

template<auto MFP>
using member_function_traits = internal_member_function_traits<decltype(MFP), MFP>;