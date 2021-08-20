// used to bind to functors and [stateful] lambdas
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