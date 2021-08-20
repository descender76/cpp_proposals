// used to bind to member functions without type erasure
// the this pointer is changed into the first parameter
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